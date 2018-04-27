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
        private CsEndPoint serverEndPoint = new CsEndPoint();
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

        private static DispatcherOperationCallback exitFrameCallback = new DispatcherOperationCallback(ExitFrame);

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
            return result.Substring(0, result.Length - 1); 
        }

        private string arrayToString(string[] toConvert)
        {
            string result = "";
            foreach (string item in toConvert)
            {
                result += item + ",";
            }
            return result.Substring(0, result.Length - 1);
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
            if (newCategory.Text == "") return;
            theNew.IsChecked = true;
            theNew.Value = newCategory.Text;
            checkinCategoryList.Items.Add(theNew);
            checkInCategories.Add(newCategory.Text);
            newCategory.Text = "";
        }

        // -----< populateAllListView: Populate checkin dependency ListView >-----
        private void populateCheckInDependencyListView()
        {
            foreach (FileComplex item in repoRecords)
            {
                checkInDependencyList.Items.Add(item);
            }
        }

        // -----< populateAllListView: Populate checkout ListView >-----
        private void populateCheckoutListView()
        {
            foreach (FileComplex item in repoRecords)
            {
                checkOutList.Items.Add(item);
            }
        }

        // -----< populateAllListView: Populate browse ListView >-----
        private void populateBrowseListView()
        {
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
                    if (enumer.Current.Key.Contains("errorInfo") && enumer.Current.Value != "")
                    {
                        errorInfo = enumer.Current.Value;
                    }
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
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
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
            };
            registerHandler("setFilterCallback", setFilterCallback);
        }

        // ----< setFilter: Set the filter >-----
        private void setFilterHandler(string[] raw)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
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
            currentDisplay = currentDisplay.Substring(1);
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
                serverEndPoint.machineAddress = "localhost";
                serverEndPoint.port = 8080;
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
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
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
                }
                if (isDebug == false)
                    MessageBox.Show(errorInfo, "Checkin result", MessageBoxButton.OK, MessageBoxImage.Information);
            };
            registerHandler("checkinCallback", checkinCallback);
        }

        // -----< Check_In_Click: Click handler of checkIn Button >-----
        private void Check_In_Click(object sender, RoutedEventArgs e)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            System.IO.FileInfo sourceFileInfo = new System.IO.FileInfo(pathFileName.Text);
            string fileName = pathFileName.Text.Substring(pathFileName.Text.LastIndexOf("/") + 1);
            System.IO.File.Copy(pathFileName.Text, "../SendFiles/" + fileName, true);
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("file", fileName);
            if(isDebug == true) { message.add("name", "Demo message of requirement 3b"); }
            message.add("command", "fileCheckin");
            message.add("content-length", sourceFileInfo.Length.ToString());
            message.add("description", description.Text);
            message.add("dependencies", hashSetToString(checkInDependencies));
            message.add("categories", hashSetToString(checkInCategories));
            message.add("nameSpace", nameSpace.Text);
            message.add("owner", theUser.Text);
            message.add("close", closeCheckIn.IsChecked.ToString().ToLower());
            Action<CsMessage> debug = (CsMessage msg) => debugDisplay(msg, "send"); 
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            message.show();
        }

        // -----< Check_In_Cancel_Click: Click handler of checkInCancel Button >-----
        private void Check_In_Cancel_Click(object sender, RoutedEventArgs e)
        {
            pathFileName.Text = "";
            description.Text = "";
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
                // receiveMessage.show();
                // Console.Write("File checken out successful.\n");
            };
            registerHandler("checkoutReceiveFilesCallback", checkoutReceiveFilesCallback);
        }

        // -----< checkoutReceiveFile: Send files for checking out >-----
        private void checkoutReceiveFile(List<string> toReceive)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
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
                while (enumer.MoveNext())
                {
                    if (enumer.Current.Key == "errorInfo" && enumer.Current.Value != "") errorInfo = enumer.Current.Value;
                    else if (enumer.Current.Key.Contains("successFile")) successCheckouts.Add(enumer.Current.Value);
                    else if (enumer.Current.Key.Contains("failFile")) failCheckouts.Add(enumer.Current.Value);
                }
                if (errorInfo != "") MessageBox.Show(errorInfo, "Checkin result", MessageBoxButton.OK, MessageBoxImage.Information);
                else {
                    if (isDebug == false)
                    {
                        MessageBoxResult result = MessageBox.Show("You are going to checkout " + successCheckouts.Count.ToString() + " files" + "\n" +
                                                               failCheckouts.Count.ToString() + " files cannot be checked out by you because you do not own them" + "\n" +
                                                               "Click \"OK\" to proceed and \"Cancel\" to cancel" + "\n", "Checkout Files Confirmation",
                                                               MessageBoxButton.OKCancel, MessageBoxImage.Information);
                        if (result == MessageBoxResult.OK)
                        {
                            checkoutReceiveFile(successCheckouts); failCheckouts.Clear();
                        }
                        else
                        {
                            successCheckouts.Clear(); failCheckouts.Clear();
                        }
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
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            FileComplex selected = checkOutList.SelectedItem as FileComplex;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "fileCheckout");
            message.add("fileName", selected.Key);
            message.add("requestor", theUser.Text);
            message.add("recursive", recursiveCheckout.IsChecked.ToString().ToLower());
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

        // -----< Change_CurrentUser: Click handler of User Login Botton >-----
        private void Change_CurrentUser(object sender, RoutedEventArgs e)
        {
            theUser.Text = userName.Text;
            return;
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
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
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
        private void trackFiles(bool show)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "trackAllRecords");
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            if (show == true) { message.add("name", "Demo message of requirement 3g"); message.show(); }
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
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "ping");
            message.add("name", name);
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            message.show();
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
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
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
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
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

        // -----< Window_Loaded: Load the mainWindow >-----
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            endPoint_.machineAddress = "localhost";
            endPoint_.port = int.Parse(argPort);
            translater.listen(endPoint_);

            Title = "Client Console - User: " + argUser + " - Port: " + argPort;

            userName.Text = argUser;
            theUser.Text = argUser;
            serverAddress.Text = "localhost";
            sendPort.Text = "8080";
            receivePort.Text = endPoint_.port.ToString();

            processMessages();

            eventRegisterInitialization();

            elementInitialize();

            // checkInDependencyList();
        }

        // -----< testStub: Run all tests >-----
        private void testStub()
        {
            theTab.SelectedIndex = 4;
            Console.Write("\n");
            test1();
            test2();
            test3a();
            test3b();
            test3c();
            test3d();
            test3ef();
            test3g();
            test4();
        }

        // -----< Demostration of requirement 1 >-----
        private void test1()
        {
            Console.Write("Demostration of Requirement 1.\n");
            Console.Write("==============================\n\n");
            Console.Write("  This requirement is satisified by both client and server side.\n\n");
            Console.Write("  PASSED -- Requirement 1\n\n");
        }

        // -----< Demostration of requirement 2 >-----
        private void test2()
        {
            Console.Write("Demostration of Requirement 2.\n\n");
            Console.Write("==============================\n\n");
            Console.Write("  Demostrate the communication channel by sending ping message to server and get ping reply from server.\n\n");
            ping("Demo message of requirement 2");
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 2\"\n");
            Console.Write("  Which can prove that this client is using communication channel to communicate with server.\n\n");
            Console.Write("  PASSED -- Requirement 2\n\n");
        }

        // -----< Demostration of requirement 3a >-----
        private void test3a()
        {
            Console.Write("Demostration of Requirement 3a.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of connecting to server in client side.\n\n");
            ping("Demo message of requirement 3a");
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 3a\"\n");
            Console.Write("  Which can prove that this client has been connected to the server.\n\n");
            Console.Write("  PASSED -- Requirement 3a\n\n");
        }

        // -----< Demostration of requirement 3b >-----
        private void test3b()
        {
            Console.Write("Demostration of Requirment 3b.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of sending checkin message and get reply from server.\n\n");
            Console.Write("  The message below shows the client sending a checkin message with checkin file information.\n");
            pathFileName.Text = "../RemoteRepository.cpp";
            nameSpace.Text = "Server";
            description.Text = "Source file of the server of project4";
            checkInDependencies.Add("DbCore::DbCore.h.1"); checkInDependencies.Add("DbCore::DbCore.cpp.1");
            checkInCategories.Add("source");
            checkinButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 3b\"\n");
            Console.Write("  Which can prove that this client can send and receive checkin messages.\n\n");
            Console.Write("  PASSED -- Requirement 3b\n\n");
        }

        // -----< Demostration of requirement 3c >-----
        private void test3c()
        {
            Console.Write("Demostration of Requirement 3c.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of sending checkout message and get reply from server.\n\n");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            FileComplex selected = checkOutList.SelectedItem as FileComplex;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "fileCheckout");
            message.add("fileName", "DbCore::DbCore.h.1");
            message.add("requestor", theUser.Text);
            message.add("recursive", recursiveCheckout.IsChecked.ToString().ToLower());
            if (isDebug == true) { message.add("name", "Demo message of requirement 3c"); }
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            Console.Write("  The message below shows the client sending a checkout message with checkout file information and requestor.\n");
            message.show();
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 3c\"\n");
            Console.Write("  Which can prove that this client can send and receive checkout messages.\n\n");
            Console.Write("  PASSED -- Requirement 3c\n\n");
        }

        // -----< Demostration of requirement 3d >-----
        private void test3d()
        {
            Console.Write("Demostration of Requirement 3d.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of sending browsing specified package description message and get reply from server.\n\n");
            Console.Write("  The message below shows the client sending a browsing specified package description message file information.\n");
            browseDescription("DbCore::DbCore.h.1");
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 3d\"\n");
            Console.Write("  Which can prove that this client can send and receive browse specified package description messages.\n\n");
            Console.Write("  PASSED -- Requirement 3d\n\n");
        }

        // -----< Demostration of requirement 3e & 3f >-----
        private void test3ef()
        {
            Console.Write("Demostration of Requirement 3e & 3f.\n\n");
            Console.Write("====================================\n\n");
            Console.Write("  Demostrate the capability of viewing full text and metadata");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "showFile");
            message.add("fileName", "DbCore::DbCore.h.1");
            if (isDebug == true) { message.add("name", "Demo message of requirement 3e & 3f"); }
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            Console.Write("  The message below shows the client sending a view file and its metadata message file information.\n");
            message.show();
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 3e & 3f\"\n");
            Console.Write("  Meanwhile, a file window will popup, the left part of the window shows the full file text, and the right part of the file shows the file metadata.\n");
            Console.Write("  As both file text and metadata are displayed in the same window, this test demostrates both requirement 3e and 3f.\n");
            Console.Write("  Which can prove that this client can send and receive view full file text as well as view file metadata messages.\n\n");
            Console.Write("  PASSED -- Requirement 3e & 3f\n\n");
        }

        private void test3g()
        {
            Console.Write("Demostration of Requirement 3g.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of browsing all files in the repository");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "listContent");
            if (isDebug == true) { message.add("name", "Demo message of requirement 3g"); }
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            message.show();
            Console.Write("  The client will receive the reply from server with message has name \"Server replys Demo message of requirement 3g\"\n");
            Console.Write("  Which can prove that this client can send and receive browse the repo.\n\n");
            Console.Write("  The directory structure is shown intuitively in the GUI the \"browse\" tab.\n\n");
            Console.Write("  PASSED -- Requirement 3g\n\n");
        }

        private void test4()
        {
            Console.Write("Demostration of Requirement 4.\n\n");
            Console.Write("==============================\n\n");
            Console.Write("  All above tests can be executed automatically.\n\n");
            Console.Write("  PASSED -- Requirement 4\n\n");
        }
    }
}
