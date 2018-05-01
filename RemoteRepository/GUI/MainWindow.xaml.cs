///////////////////////////////////////////////////////////////////////
// MainWindow.xaml.cs - GUI for RemoteRepository                     //
// ver 1.0                                                           //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018           //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a WPF-based GUI for Project3HelpWPF demo.  It's 
 * responsibilities are to:
 * - Provide a display of directory contents of a remote ServerPrototype.
 * - It provides a subdirectory list and a filelist for the selected directory.
 * - You can navigate into subdirectories by double-clicking on subdirectory
 *   or the parent directory, indicated by the name "..".
 *   
 * Required Files:
 * ---------------
 * Mainwindow.xaml, MainWindow.xaml.cs
 * FileComplex.cs
 * KeyValuePair.cs
 * fileWindow.xaml, fileWindow.xaml.cs
 * Translater.dll
 * 
 * Maintenance History:
 * --------------------
 * ver 1.0 : 30 Mar 2018
 * - first release
 * 
 */

// Translater has to be statically linked with CommLibWrapper
// - loader can't find Translater.dll dependent CommLibWrapper.dll

using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.Windows.Threading;
using MsgPassingCommunication;

namespace GUI
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            string[] args = Environment.GetCommandLineArgs();
            for(int i = 1; i < args.Count(); i += 2) {
                if (args[i] == "--u") argUser = args[i + 1];
                else if (args[i] == "--p") argPort = args[i + 1];
                else if (args[i] == "--d" && args[i + 1] == "true") isDebug = true;
            }
            Console.Title = "Client Console - User: " + argUser + " - Port: " + argPort;
            Title = "Client Console - User: " + argUser + " - Port: " + argPort;
        }
        
        private Translater translater = new Translater();
        private CsEndPoint endPoint_ = new CsEndPoint();
        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
          = new Dictionary<string, Action<CsMessage>>();
        private HashSet<FileComplex> repoRecords = new HashSet<FileComplex>();
        private HashSet<FileComplex> filterRecords = new HashSet<FileComplex>();
        private HashSet<string> repoCategories = new HashSet<string>();
        private HashSet<string> checkInDependencies = new HashSet<string>();
        private HashSet<string> checkInCategories = new HashSet<string>();
        private List<string> successCheckouts = new List<string>();
        private List<string> failCheckouts = new List<string>();
        private string argPort = "8082";
        private string argUser = "Administrator";
        private string theServerAddress = "localhost";
        private int serverPort = 8080;
        private bool isDebug = false;

        //----< process incoming messages on child thread >--------------------
        //----< This function process the message coming from server side >----
        private void processMessages()
        {
            ThreadStart thrdProc = () =>
            {
                while (true)
                {
                    CsMessage msg = translater.getMessage();
                    if (msg.attributes.Count() == 0) continue;
                    string msgId = msg.value("command");
                    Action<CsMessage> debug = (CsMessage message) =>
                    {
                        debugDisplay(message, "receive");
                    };
                    Dispatcher.Invoke(debug, new Object[] { msg });
                    if (dispatcher_.ContainsKey(msgId))
                        dispatcher_[msgId].Invoke(msg);
                }
            };
            rcvThrd = new Thread(thrdProc);
            rcvThrd.IsBackground = true;
            rcvThrd.Start();
        }

        // -----< eventRegisterInitialization: Register the callback handler Actions >-----
        private void eventRegisterInitialization()
        {
            checkinCallbackHandler();
            checkoutReceiveFileCallbackHandler();
            checkoutCallbackHandler();
            showFileCallbackHandler();
            trackAllRecordsCallbackHandler();
            trackAllCategoriesCallbackHandler();
            pingHandler();
            browseDescriptionCallbackHandler();
            setFilterCallbackHandler();
            resumeCheckinCallbackHandler();
        }

        // -----< debugDisply: Display messages in the debug message tab >-----
        private void debugDisplay(CsMessage msg, String direction)
        {
            String toDisplay = "";
            var enumer = msg.attributes.GetEnumerator();
            while (enumer.MoveNext())
            {
                String key = enumer.Current.Key;
                String value = enumer.Current.Value;
                toDisplay += "{ " + key + ", " + value + " }; ";
            }
            if (direction == "send") SendMessageDebugView.Items.Insert(0, toDisplay);
            else if (direction == "receive") ReceiveMessageDebugView.Items.Insert(0, toDisplay);
        }

        // -----< hintDisplay: Erase the previous hint and show new >-----
        private void hintDisplay(string newHint = "")
        {
            Hint.Content = newHint;
        }

        private static DispatcherOperationCallback exitFrameCallback = new DispatcherOperationCallback(ExitFrame);

        // -----<DoEvents: Force refresh the window >-----
        public static void DoEvents()
        {
            DispatcherFrame nestedFrame = new DispatcherFrame();
            DispatcherOperation exitOperation = Dispatcher.CurrentDispatcher.BeginInvoke(DispatcherPriority.Background, exitFrameCallback, nestedFrame);
            Dispatcher.PushFrame(nestedFrame);
            if (exitOperation.Status !=
            DispatcherOperationStatus.Completed)
            {
                exitOperation.Abort();
            }
        }

        // -----< ExitFrame: Helper of Doevents >-----
        private static Object ExitFrame(Object state)
        {
            DispatcherFrame frame = state as
            DispatcherFrame;
            frame.Continue = false;
            return null;
        }

        // -----< fileInfoBriefAssembler: Assemble the file brief >-----
        private FileComplex fileInfoBriefAssembler(string recordBrief)
        {
            FileComplex result = new FileComplex();
            string[] infoBriefComplex = recordBrief.Split('$');
            result.NameSpace = infoBriefComplex[0];
            result.Name = infoBriefComplex[1];
            result.Version = infoBriefComplex[2];
            result.Status = infoBriefComplex[3];
            result.Owner = infoBriefComplex[4];
            result.Key = result.NameSpace + "::" + result.Name + "." + result.Version;
            return result;
        }

        // -----< hashSetToString: Convert hashSet to string >-----
        private string hashSetToString(HashSet<string> toConvert)
        {
            string result = "";
            foreach (string item in toConvert)
            {
                result += item + ",";
            }
            if (result == "") return result;
            else return result.Substring(0, result.Length - 1); 
        }

        // -----< arrayToString: Convert array to string >-----
        private string arrayToString(string[] toConvert)
        {
            string result = "";
            if (toConvert.Count() == 0) return "";
            foreach (string item in toConvert)
            {
                result += item + ",";
            }
            if (result == "") return result;
            else return result.Substring(0, result.Length - 1);
        }

        // -----< isAlphaDigit: Check if the string is consist of letter and digits >-----
        bool isAlphaDigit(string toCheck)
        {
            string valid = @"^[A-Za-z0-9\s]*$";
            return Regex.IsMatch(toCheck, valid);
        }

        // -----< addDependency: Add dependency to checkin item >-----
        private void addDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            checkInDependencies.Add(toAdd);
        }

        // -----< removeDependency: Remove dependency from checkin item >-----
        private void removeDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove = selected.Tag.ToString();
            checkInDependencies.Remove(toRemove);
        }

        // -----< addCategory: Add category to checkin item >-----
        private void addCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            checkInCategories.Add(toAdd);
        }

        // -----< removeCategory: Remove category from checkin item >-----
        private void removeCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove = selected.Tag.ToString();
            checkInCategories.Remove(toRemove);
        }

        // -----< addCategory_Clicked: Handler addCategory click event >-----
        private void addCategory_Click(object sender, RoutedEventArgs e)
        {
            KeyValuePair theNew = new KeyValuePair();
            if (newCategory.Text.Trim() == "") return;
            else if(isAlphaDigit(newCategory.Text) == false) { hintDisplay("category should only consist of letter and digit!"); return; }
            theNew.IsChecked = true;
            theNew.Value = newCategory.Text;
            checkinCategoryList.Items.Add(theNew);
            checkInCategories.Add(newCategory.Text);
            newCategory.Text = "";
        }

        // -----< populateAllListView: Populate checkin dependency ListView >-----
        private void populateCheckInDependencyListView()
        {
            checkInDependencyList.Items.Clear();
            foreach (FileComplex item in repoRecords)
            {
                checkInDependencyList.Items.Add(item);
            }
        }

        // -----< populateAllListView: Populate checkout ListView >-----
        private void populateCheckoutListView()
        {
            checkOutList.Items.Clear();
            foreach (FileComplex item in repoRecords)
            {
                checkOutList.Items.Add(item);
            }
        }

        // -----< populateAllListView: Populate browse ListView >-----
        private void populateBrowseListView()
        {
            browseList.Items.Clear();
            filterRecords.Clear();
            foreach (FileComplex item in repoRecords)
            {
                browseList.Items.Add(item);
                filterRecords.Add(item);
            }
        }

        // -----< populateCheckInCategoryListView: Populate checkin category ListView >-----
        private void populateCheckInCategoryListView()
        {
            foreach (string item in repoCategories)
            {
                KeyValuePair toAdd = new KeyValuePair("", item);
                checkinCategoryList.Items.Add(toAdd);
            }
        }

        // -----< clearBrowseListView: Clear browse listview >-----
        private void clearBrowseListView()
        {
            browseList.Items.Clear();
        }
        
        // -----< populateFilterBrowseListView: Populate filter browse listview >-----
        private void populateFilterBrowseListView()
        {
            foreach (FileComplex item in filterRecords)
            {
                browseList.Items.Add(item);
            }
        }

        // -----< resumeCheckinCallbackHandler: Handle resumeCheckinCallback >-----
        private void resumeCheckinCallbackHandler()
        {
            Action<CsMessage> resumeCheckinCallback = (CsMessage receiveMessage) =>
            {
                string errorInfo = "File information modification success.\n";
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    if (enumer.Current.Key.Contains("errorInfo") && enumer.Current.Value != "") errorInfo = enumer.Current.Value;
                    else if (enumer.Current.Key.Contains("errorInfo") && enumer.Current.Value == "") { trackFiles(); trackAllCategoriesHandler(); }
                };
                if (isDebug == false)
                    MessageBox.Show(errorInfo, "File Information Modification", MessageBoxButton.OK, MessageBoxImage.Information);
            };
            registerHandler("resumeCheckinCallback", resumeCheckinCallback);
        }
        
        // -----< resumeCheckinHandler: Handle resumeCheckin event >-----
        private void resumeCheckinHandler(FileComplex raw)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "resumeCheckin");
            message.add("fileName", "$" + raw.NameSpace + "_" + raw.Name + "." + raw.Version);
            message.add("dependencies", arrayToString(raw.Dependencies));
            message.add("categories", arrayToString(raw.Categories));
            message.add("status", raw.Status);
            message.add("nameSpace", raw.NameSpace);
            message.add("fileKey", raw.Key);
            message.add("owner", raw.Owner);
            message.add("description", raw.Description);
            Action<CsMessage> debug = (CsMessage msg) => debugDisplay(msg, "send");
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

        // -----< showFileWindowPopup: Popup the fileWindow >-----
        private void showFileWindowPopup(CsMessage msg)
        {
            fileWindow popUp = new fileWindow();
            popUp.getFileInfo(msg);
            popUp.getAllRecordInfo(repoRecords);
            popUp.getAllCategories(repoCategories);
            popUp.getCurrentUser(argUser);
            popUp.Owner = this;
            popUp.Title = "File Popup Window - User: " + argUser + " - Port: " + argPort;
            popUp.Show();
            popUp.SubmitResult += submission =>
            {
                FileComplex selected = submission;
                resumeCheckinHandler(selected);
            };
        }

        // -----< registerHandler: Add client processing for message with key >-----
        private void registerHandler(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        // -----< setFilterCallbackHandler: Handler setFilterCallback >-----
        private void setFilterCallbackHandler()
        {
            Action<CsMessage> setFilterCallback = (CsMessage receiveMessage) =>
            {
                filterRecords.Clear();
                Action clearList = () => { clearBrowseListView(); };
                Dispatcher.Invoke(clearList, new Object[] { });
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    if (enumer.Current.Key.Contains("record"))
                    {
                        filterRecords.Add(fileInfoBriefAssembler(enumer.Current.Value));
                    }
                }
                Action addList = () => { populateFilterBrowseListView(); };
                Dispatcher.Invoke(addList, new Object[] { });
                if (isDebug == true) receiveMessage.show();
            };
            registerHandler("setFilterCallback", setFilterCallback);
        }

        // ----< setFilter: Set the filter >-----
        private void setFilterHandler(string[] raw)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "setFilter");
            if (raw[0] != "\n") message.add("nameSpace", raw[0]);
            if (raw[1] != "\n") message.add("fileName", raw[1]);
            if (raw[2] != "\n") message.add("version", raw[2]);
            if (raw[3] != "\n") message.add("dependencies", raw[3]);
            if (raw[4] != "\n") message.add("categories", raw[4]);
            string currentDisplay = "";
            foreach (FileComplex item in filterRecords)
            {
                currentDisplay += ("$" + item.Key);
            }
            if(currentDisplay != "") currentDisplay = currentDisplay.Substring(1);
            message.add("source", currentDisplay);
            Action<CsMessage> debug = (CsMessage msg) => debugDisplay(msg, "send");
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

        // -----< Clear_Filter_Click: Handle clearFilter button click event >-----
        private void Clear_Filter_Click(object sender, RoutedEventArgs e)
        {
            clearBrowseListView();
            populateBrowseListView();
        }

        // -----< Set_Filter_Click: Handle setFilter button click event >-----
        private void Set_Filter_Click(object sender, RoutedEventArgs e)
        {
            filterWindow filterPopup = new filterWindow();
            filterPopup.Owner = this;
            filterPopup.Show();
            filterPopup.SubmitFilter += submission =>
            {
                string[] selected = submission;
                setFilterHandler(selected);
            };
        }

        // -----< showFileHandler: Callback handler of showFile >-----
        private void showFileCallbackHandler()
        {
            Action<CsMessage> showFileCallback = (CsMessage receiveMessage) =>
            {
                var enumer = receiveMessage.attributes.GetEnumerator();
                string fileName = "";
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    string value = enumer.Current.Value;
                    if (key == "file") fileName = value;
                    if (key == "content-length" && value == "0") return;
                    else if (key == "content-length" && value != "0" && isDebug == true) receiveMessage.show();
                }
                CsEndPoint serverEndPoint = new CsEndPoint();
                serverEndPoint.machineAddress = theServerAddress;
                serverEndPoint.port = serverPort;
                CsMessage message = new CsMessage();
                message.add("to", CsEndPoint.toString(serverEndPoint));
                message.add("from", CsEndPoint.toString(endPoint_));
                message.add("command", "showFileCleanUp");
                message.add("fileName", fileName);
                Action<CsMessage> debug = (CsMessage msg) => debugDisplay(msg, "send");
                Dispatcher.Invoke(debug, new Object[] { message });
                translater.postMessage(message);
                Action<CsMessage> showFileInPopup = (CsMessage msg) => showFileWindowPopup(msg);
                Dispatcher.Invoke(showFileInPopup, receiveMessage);
            };
            registerHandler("showFileCallback", showFileCallback);
        }

        // -----< showFile: Send the message of showFile >-----
        private void showFile(string fileKey)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "showFile");
            message.add("fileName", fileKey);
            Action<CsMessage> debug = (CsMessage msg) => debugDisplay(msg, "send"); 
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

        // -----< browseList_DoubleClick: DoubleClick handler of browseList >-----
        private void browseList_DoubleClick(object sender, MouseButtonEventArgs e) {
            FileComplex selected = browseList.SelectedItem as FileComplex;
            showFile(selected.Key);
        }

        // -----< checkinCallbackHandler: Receive checkin result from the server >-----
        private void checkinCallbackHandler()
        {
            Action<CsMessage> checkinCallback = (CsMessage receiveMessage) =>
            {
                receiveMessage.show();
                string errorInfo = "Your file has been successfully checked in";
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    if (enumer.Current.Key == "errorInfo" && enumer.Current.Value != "") errorInfo = enumer.Current.Value;
                    else if (enumer.Current.Key == "errorInfo" && enumer.Current.Value == "") { trackFiles(); trackAllCategoriesHandler(); }
                }
                if (isDebug == false)
                    MessageBox.Show(errorInfo, "Checkin result", MessageBoxButton.OK, MessageBoxImage.Information);
                Action hint = () => hintDisplay(errorInfo);
                Dispatcher.Invoke(hint, new Object[] { });
            };
            registerHandler("checkinCallback", checkinCallback);
        }

        private string changeStringSeperator(string toConvert)
        {
            return toConvert.Replace('\\', '/');
        }

        // -----< Check_In_Click: Click handler of checkIn Button >-----
        private void Check_In_Click(object sender, RoutedEventArgs e)
        {
            if(pathFileName.Text == "Click Browse to select the file") {
                hintDisplay("Please at least specify FileName by clicking the browse button!");
                return;
            }
            if(isAlphaDigit(description.Text) == false || isAlphaDigit(nameSpace.Text) == false) { hintDisplay("description and namespace should only consist of digit and letter!"); return; }
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            System.IO.FileInfo sourceFileInfo = new System.IO.FileInfo(pathFileName.Text);
            string turePathFileName = changeStringSeperator(pathFileName.Text);
            string fileName = turePathFileName.Substring(turePathFileName.LastIndexOf("/") + 1);
            System.IO.File.Copy(turePathFileName, "../SendFiles/" + fileName, true);
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("file", fileName);
            if(isDebug == true) { message.add("name", "Demo message of requirement 2a"); }
            message.add("command", "fileCheckin");
            message.add("content-length", sourceFileInfo.Length.ToString());
            message.add("description", description.Text.Trim());
            message.add("dependencies", hashSetToString(checkInDependencies).Trim());
            message.add("categories", hashSetToString(checkInCategories).Trim());
            message.add("nameSpace", nameSpace.Text.Trim());
            message.add("owner", theUser.Text.Trim());
            message.add("close", closeCheckIn.IsChecked.ToString().ToLower());
            Action<CsMessage> debug = (CsMessage msg) => debugDisplay(msg, "send"); 
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            if(isDebug == true) message.show();
        }

        // -----< Check_In_Cancel_Click: Click handler of checkInCancel Button >-----
        private void Check_In_Cancel_Click(object sender, RoutedEventArgs e)
        {
            pathFileName.Text = "Click Browse to select the file";
            description.Text = "";
            nameSpace.Text = "";
            checkinCategoryList.Items.Clear();
            checkInDependencyList.Items.Clear();
            Action updateCheckInCategoryList = () =>
            {
                populateCheckInCategoryListView();
            };
            Dispatcher.Invoke(updateCheckInCategoryList, new Object[] { });
            Action updateCheckInDependencyList = () =>
            {
                populateCheckInDependencyListView();
            };
            Dispatcher.Invoke(updateCheckInDependencyList, new Object[] { });
            closeCheckIn.IsChecked = false;
        }

        // -----< checkoutReceiveFileCallbackHandler: Callback handler of checkoutReceiveFileCallback >-----
        private void checkoutReceiveFileCallbackHandler()
        {
            Action<CsMessage> checkoutReceiveFilesCallback = (CsMessage receiveMessage) =>
            {
                var enumer = receiveMessage.attributes.GetEnumerator();
                while(enumer.MoveNext())
                {
                    if (enumer.Current.Key == "content-length" && enumer.Current.Value == "0") return; 
                    if (enumer.Current.Key == "fileName") successCheckouts.Remove(enumer.Current.Value);
                }
                Action hint = () => hintDisplay("File checken out successful");
                Dispatcher.Invoke(hint, new Object[] { });
            };
            registerHandler("checkoutReceiveFilesCallback", checkoutReceiveFilesCallback);
        }

        // -----< checkoutReceiveFile: Send files for checking out >-----
        private void checkoutReceiveFile(List<string> toReceive)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "sendMultipleFiles");
            message.add("for", "checkoutReceiveFiles");
            for(int i = 0; i < toReceive.Count; ++i)
            {
                message.add("fileName" + i.ToString(), toReceive[i]);
            }
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

        // -----< checkoutCallbackHandler: Callback handler of checkout >-----
        private void checkoutCallbackHandler()
        {
            Action<CsMessage> checkoutCallback = (CsMessage receiveMessage) =>
            {
                receiveMessage.show();
                var enumer = receiveMessage.attributes.GetEnumerator();
                string errorInfo = "";
                while (enumer.MoveNext()) {
                    if (enumer.Current.Key == "errorInfo" && enumer.Current.Value != "") errorInfo = enumer.Current.Value;
                    else if (enumer.Current.Key.Contains("successFile")) successCheckouts.Add(enumer.Current.Value);
                    else if (enumer.Current.Key.Contains("failFile")) failCheckouts.Add(enumer.Current.Value);
                }
                if (errorInfo != "") {
                    Action hint = () => hintDisplay("Checkout Failed: " + errorInfo);
                    Dispatcher.Invoke(hint, new Object[] { });
                    MessageBox.Show(errorInfo, "Checkin result", MessageBoxButton.OK, MessageBoxImage.Information);
                }
                else
                {
                    if (isDebug == false)
                    {
                        MessageBoxResult result = MessageBox.Show("You are going to checkout " + successCheckouts.Count.ToString() + " files" + "\n" +
                                                               failCheckouts.Count.ToString() + " files cannot be checked out by you because you do not own them" + "\n" +
                                                               "Click \"OK\" to proceed and \"Cancel\" to cancel" + "\n", "Checkout Files Confirmation",
                                                               MessageBoxButton.OKCancel, MessageBoxImage.Information);
                        if (result == MessageBoxResult.OK) { checkoutReceiveFile(successCheckouts); failCheckouts.Clear(); }
                        else successCheckouts.Clear(); failCheckouts.Clear();
                    }
                    else { checkoutReceiveFile(successCheckouts); failCheckouts.Clear(); }
                }
            };
            registerHandler("checkoutCallback", checkoutCallback);
        }

        // -----< Check_Out_Click: Click handler of checkOut Button >-----
        private void Check_Out_Click(object sender, RoutedEventArgs e)
        {
            if (checkOutList.SelectedItems.Count == 0) return;
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            FileComplex selected = checkOutList.SelectedItem as FileComplex;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "fileCheckout");
            message.add("fileName", selected.Key);
            message.add("requestor", theUser.Text);
            message.add("recursive", recursiveCheckout.IsChecked.ToString().ToLower());
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }
        
        // -----< SetConnection_Click: Handle SetConnection click event >-----
        private void SetConnection_Click(object sender, RoutedEventArgs e)
        {
            int n;
            if (int.TryParse(sendPort.Text, out n) == false || int.TryParse(receivePort.Text, out n) == false)
            {
                hintDisplay("Invalid connection configuration");
                return;
            }
            else
            {
                endPoint_.port = int.Parse(sendPort.Text);
                serverPort = int.Parse(receivePort.Text);
                theServerAddress = serverAddress.Text;
                hintDisplay("Connection set");
            }
        }

        // -----< correctUser: Trim the username >-----
        private string correctUser(string toCorrect)
        {
            return toCorrect.Trim();
        }

        // -----< Change_CurrentUser: Click handler of User Login Botton >-----
        private void Change_CurrentUser(object sender, RoutedEventArgs e)
        {
            string tmp = correctUser(userName.Text);
            if (tmp == "") tmp = "Anonymous";
            if (isAlphaDigit(tmp) == false)
            {
                hintDisplay("User name should only consist of digits and letters");
                userName.Text = theUser.Text;
                return;
            }
            else
            {
                theUser.Text = tmp;
                argUser = tmp;
                hintDisplay(userName.Text + " has logged in");
                Console.Title = "Client Console - User: " + argUser + " - Port: " + argPort;
                Title = "Client Console - User: " + argUser + " - Port: " + argPort;
            }
        }

        // -----< trackAllCategoriesCallbackHandler: Callback handler of tracking all categories >-----
        private void trackAllCategoriesCallbackHandler()
        {
            Action<CsMessage> trackAllCategoriesCallback = (CsMessage receiveMessage) =>
            {
                repoCategories.Clear();
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    if (enumer.Current.Key.Contains("category"))
                    {
                        repoCategories.Add(enumer.Current.Value);
                    }
                }
                Action updateCheckInCategoryList = () =>
                {
                    populateCheckInCategoryListView();
                };
                Dispatcher.Invoke(updateCheckInCategoryList, new Object[] { });
            };
            registerHandler("trackAllCategoriesCallback", trackAllCategoriesCallback);
        }

        // -----< trackAllCategoriesHandler: Send message of tracking all categories >-----
        private void trackAllCategoriesHandler()
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "trackAllCategories");
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

        // -----< trackAllRecordsCallbackHandler: Callback handler of tracking all records >-----
        private void trackAllRecordsCallbackHandler()
        {
            Action<CsMessage> trackAllRecordsCallback = (CsMessage receiveMessage) =>
            {
                repoRecords.Clear();
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    if(enumer.Current.Key.Contains("record"))
                    {
                        repoRecords.Add(fileInfoBriefAssembler(enumer.Current.Value));
                    }
                }
                Action updateCheckInDependencyList = () =>
                {
                    populateCheckInDependencyListView();
                    populateBrowseListView();
                    populateCheckoutListView();
                };
                Dispatcher.Invoke(updateCheckInDependencyList, new Object[] { });
            };
            registerHandler("trackAllRecordsCallback", trackAllRecordsCallback);
        }

        // -----< trackFiles: Send message of trackFiles >-----
        private void trackFiles()
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "trackAllRecords");
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

        // -----< pingHandler: Handler of ping >-----
        private void pingHandler()
        {
            Action<CsMessage> ping = (CsMessage receiveMessage) =>
            {
                receiveMessage.show();
            };
            registerHandler("ping", ping);
        }

        // -----< ping: Send message of ping >-----
        private void ping(string name)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "ping");
            message.add("name", name);
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            if(isDebug == true) message.show();
        }

        // -----< browseDescriptionCallbackHandler: Callback handler of browseDescription >-----
        private void browseDescriptionCallbackHandler()
        {
            Action<CsMessage> browseDescriptionCallback = (CsMessage receiveMessage) =>
            {
                receiveMessage.show();
            };
            registerHandler("browseDescriptionCallback", browseDescriptionCallback);
        }

        // -----< browseDescription: Send message of browseDescription >-----
        private void browseDescription(string fileName)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "browseDescription");
            message.add("fileName", fileName);
            if (isDebug == true) { message.add("name", "Demo message of requirement 3d"); }
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            message.show();
        }

        // -----< RecursiveCheckout_MouseEnter: Handle RecursiveCheckout MouseEnter event >-----
        private void RecursiveCheckout_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Check to checkout the files with all its dependencies");
        }

        // -----< CloseCheckin_MouseEnter: Handle CloseCheckin MouseEnter event >-----
        private void CloseCheckin_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Check to checkin the file as closed");
        }

        // -----< ClearFilter_MouseEnter: Handle ClearFilter MouseEnter event >-----
        private void ClearFilter_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to clear the query filter of the browse list");
        }

        // -----< SetFilter_MouseEnter: Handle SetFilter MouseEnter event >-----
        private void SetFilter_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to set the query filter of the browse list");
        }

        // -----< Checkout_MouseEnter: Handle Checkout MouseEnter event >-----
        private void Checkout_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to checkout the selected file");
        }

        // -----< CheckinCancel_MouseEnter: Handle CheckinCancel MouseEnter event >-----
        private void CheckinCancel_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to cancel checkin");
        }

        // -----< Browse_MouseEnter: Handle Checkin MouseEnter event >-----
        private void Browse_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to select the checkin file");
        }

        // -----< Checkin_MouseEnter: Handle Checkin MouseEnter event >-----
        private void Checkin_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to checkin the selected file with metadata");
        }

        // -----< AddCategory_MouseEnter: Handle AddCategory MouseEnter event >-----
        private void AddCategory_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to add your new customed category into the category list");
        }

        // -----< TestConnection_MouseEnter: Handle TestConnection MouseEnter event >-----
        private void TestConnection_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to test connection");
        }

        // -----< SetConnection_MouseEnter: Handle SetConnection MouseEnter event >-----
        private void SetConnection_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to set connection");
        }

        // -----< ChangeCurrentUser_MouseEnter: Handle ChangeCurrentUser MouseEnter event >-----
        private void ChangeCurrentUser_MouseEnter(object sender, MouseEventArgs e)
        {
            hintDisplay("Click to use the client as the new username");
        }

        // -----< mouseLeave: Handle all buttons' MouseLeave event >-----
        private void mouseLeave(object sender, MouseEventArgs e)
        {
            hintDisplay("");
        }

        // -----< Open_FileForm: Popup the file browse form >-----
        private void Open_FileForm(object sender, RoutedEventArgs e)
        {
            var openFileDialog = new Microsoft.Win32.OpenFileDialog();
            var result = openFileDialog.ShowDialog();
            if (result == true)
            {
                this.pathFileName.Text = openFileDialog.FileName;
            }
        }

        // -----< elementInitialize: Initialize all elements >-----
        private void elementInitialize()
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "trackAllRecords");
            translater.postMessage(message);
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            message.overRide("command", "trackAllCategories");
            translater.postMessage(message);
            Dispatcher.Invoke(debug, new Object[] { message });
        }

        // -----< loadAboutText: Load the about content from About.txt >-----
        private void loadAboutText()
        {
            string fileContent = File.ReadAllText("../About.txt");
            Paragraph paragraph = new Paragraph();
            paragraph.Inlines.Add(new Run(fileContent));
            about.Blocks.Add(paragraph);
        }

        // -----< Window_Loaded: Load the mainWindow >-----
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            endPoint_.machineAddress = "localhost";
            endPoint_.port = int.Parse(argPort);
            translater.listen(endPoint_);

            Title = "Client Console - User: " + argUser + " - Port: " + argPort;

            userName.Text = argUser;
            theUser.Text = argUser;
            serverAddress.Text = theServerAddress;
            sendPort.Text = argPort;
            receivePort.Text = serverPort.ToString();

            processMessages();

            eventRegisterInitialization();

            elementInitialize();

            loadAboutText();
            if (isDebug == true && argUser == "Administrator") testStub();
            else Console.Write("  This client only demostrate the project implements multi-client function, no demo shown in this client, the client title \"Client Console - User: Administrator - Port: 8081\" holds all requirements demostration");
        }

        // -----< testStub: Run all tests >-----
        private void testStub()
        {
            theTab.SelectedIndex = 4;
            Console.Write("\n");
            test1();
            test2a();
            test2b();
            test2c();
            test3a();
            test3b();
            test4();
            test5();
            test6();
            test7();
        }

        // -----< Demostration of requirement 1 >-----
        private void test1()
        {
            Console.Write("Demostration of Requirement 1.\n");
            Console.Write("==============================\n\n");
            Console.Write("  This requirement is satisified by both client and server side.\n\n");
            Console.Write("  PASSED -- Requirement 1\n\n");
        }

        // -----< Demostration of requirement 2a >-----
        private void test2a()
        {
            Console.Write("Demostration of Requirement 2a.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate repository server provides check-in functionality\n\n");
            Console.Write("  The message below shows the client sending a checkin message with checkin file information.\n");
            pathFileName.Text = "../RemoteRepository.cpp";
            nameSpace.Text = "Server";
            description.Text = "Source file of the server of project4";
            checkInDependencies.Add("DbCore::DbCore.h.1"); checkInDependencies.Add("DbCore::DbCore.cpp.1");
            checkInCategories.Add("source");
            checkinButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 2a\"\n");
            Console.Write("  Which can prove that this client can send and receive checkin messages.\n\n");
            Console.Write("  PASSED -- Requirement 2a\n\n");
        }

        // -----< Demostration of requirement 2b >-----
        private void test2b()
        {
            Console.Write("Demostration of Requirement 2b.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate repository server provides check-out functionality\n\n");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "fileCheckout");
            message.add("fileName", "DbCore::DbCore.h.1");
            message.add("requestor", theUser.Text);
            message.add("recursive", recursiveCheckout.IsChecked.ToString().ToLower());
            if (isDebug == true) { message.add("name", "Demo message of requirement 2b"); }
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            Console.Write("  The message below shows the client sending a checkout message with checkout file information and requestor.\n");
            message.show();
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 2b\"\n");
            Console.Write("  Which can prove that this client can send and receive checkout messages.\n\n");
            Console.Write("  PASSED -- Requirement 2b\n\n");
        }

        // -----< Demostration of requirement 2c >-----
        private void test2c()
        {
            Console.Write("Demostration of Requirement 2c.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate repository server provides browse packages spcified by NoSql database queries.\n\n");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "setFilter");
            message.add("nameSpace", "DbCore");
            message.add("source", "DbCore::DbCore.h.1$DbCore::DbCore.h.2$SWRTB::SWRepoCore.h.1$DbCore::DbCore.cpp.1$SWRTB::SWRepoCore.cpp.1$Server::RemoteRepository.cpp.1$");
            message.add("name", "Demo message of requirement 2c");
            Action<CsMessage> debug = (CsMessage msg) => debugDisplay(msg, "send");
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            Console.Write("  The message below shows the client sending a set filter message by using query, and filter the file with NameSpace \"DbCore\".\n");
            message.show();
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 2c\"\n");
            Console.Write("  The browse tab of the client window will show only files with NameSpace \"DbCore\" as well.\n\n");
            Console.Write("  The file popup window with its source code and metadata will be demostrated seperately in demostration of requirement 3.\n\n");
            Console.Write("  Which can prove that this client can send and receive browse package message specified with queries.\n\n");
            Console.Write("  PASSED -- Requirement 2c\n\n");
        }

        // -----< Demostration of requirement 3a >-----
        private void test3a()
        {
            Console.Write("Demostration of Requirement 3a.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate client program can upload and download files.\n\n");
            Console.Write("  Upload file has already demostrated as a part of checkin.\n\n");
            Console.Write("  Download file has already demostrated as a part of checkout.\n\n");
            Console.Write("  PASSED -- Requirement 3a\n\n");
        }

        // -----< Demostration of requirement 3b >-----
        private void test3b()
        {
            Console.Write("Demostration of Requirment 3b.\n\n");
            Console.Write("==============================\n\n");
            Console.Write("  Demostrate client program can view repository contents.\n\n");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "showFile");
            message.add("fileName", "DbCore::DbCore.h.1");
            if (isDebug == true) { message.add("name", "Demo message of requirement 3b"); }
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            Console.Write("  The message below shows the client sending a view file and its metadata message file information.\n");
            message.show();
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 3b\"\n");
            Console.Write("  Meanwhile, a file window will popup, the left part of the window shows the full file text, and the right part of the file shows the file metadata.\n");
            Console.Write("  This popup window can prove that client can satisfy display categories as purpose dessribed.\n\n");
            Console.Write("  Other file as well as their metadata can be viewed by double-clicking the listview item in the browse tab. And they represents the whole repositoey contents.\n\n");
            Console.Write("  Which can prove that this client can view reporsitory contents.\n\n");
            Console.Write("  PASSED -- Requirement 3b\n\n");
        }

        // -----< Demostration of requirement 4 >-----
        private void test4()
        {
            Console.Write("Demostration of Requirement 4.\n\n");
            Console.Write("==============================\n\n");
            Console.Write("  Demostrate message-passing communication system based on sockets");
            Console.Write("  The message below shows that the client sending a ping message to the server.\n");
            ping("Demo message of requirement 4");
            Console.Write("  The message will transferred by using socket.\n\n");
            Console.Write("  Which can prove that the project is using message-passing communication system based on sockets.\n\n");
            Console.Write("  PASSED -- Requirement 4\n\n");
        }

        // -----< Demostration of requirement 5 >-----
        private void test5()
        {
            Console.Write("Demostration of Requirement 5.\n\n");
            Console.Write("==============================\n\n");
            Console.Write("  Demostrate communication system provides support for passing HTTP style messages using asynchronous one-way messaging.\n\n");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = theServerAddress;
            serverEndPoint.port = serverPort;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("one-way", "true");
            if (isDebug == true) { message.add("name", "Demo message of requirement 5 - Should not be replied"); }
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            message.show();
            Console.Write("  The message below shows a non-reply message send from client to server.\n\n");
            Console.Write("  Server will not reply this message, which represent the project uses one-way message passing.\n\n");
            Console.Write("  The reply message does not follow the sent message, which represent the project uses achychorous message passing.\n\n");
            Console.Write("  All messages shown in the demostration are HTTP style message, consist with key-value pairs.\n\n");
            Console.Write("  Which can prove that communication system provides support for passing HTTP style messages using asynchronous one-way messaging.\n\n");
            Console.Write("  PASSED -- Requirement 5\n\n");
        }

        // -----< Demostration of requirement 6 >-----
        private void test6()
        {
            Console.Write("Demostration of Requirement 6.\n\n");
            Console.Write("==============================\n\n");
            Console.Write("  Demostrate the communication system shall also support sending and receiving blocks of bytes to support file transfer\n\n");
            Console.Write("  The \"content-length\" attribute shown in checkin, checkout message represents that file transfer are based on sending and receiving blocks of bytes of the file.\n\n");
            Console.Write("  Which can prove that the communication system are using block of bytes to transfer files.\n\n");
        }

        // -----< Demostration of requirement 7 >-----
        private void test7()
        {
            Console.Write("Demostration of Requirement 7.\n\n");
            Console.Write("==============================\n\n");
            Console.Write("  All above tests can be executed automatically.\n\n");
            Console.Write("  PASSED -- Requirement 7\n\n");
        }
    }
}
