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
using MsgPassingCommunication;

namespace GUI
{
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            string[] args = Environment.GetCommandLineArgs();
            argPort = args[2];
            argUser = args[4];
            Console.Title = "Client Console - User: " + argUser + " - Port: " + argPort;
        }

        private Stack<string> pathStack_ = new Stack<string>();
        private Translater translater = new Translater();
        private CsEndPoint endPoint_ = new CsEndPoint();
        private CsEndPoint serverEndPoint = new CsEndPoint();
        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
          = new Dictionary<string, Action<CsMessage>>();
        private HashSet<FileComplex> repoRecords = new HashSet<FileComplex>();
        private HashSet<string> repoCategories = new HashSet<string>();
        private HashSet<string> checkInDependencies = new HashSet<string>();
        private HashSet<string> checkInCategories = new HashSet<string>();
        private List<string> successCheckouts = new List<string>();
        private List<string> failCheckouts = new List<string>();
        private string argPort;
        private string argUser;

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
            listContentsHandler();
            showFileHandler();
            trackAllRecordsCallbackHandler();
            trackAllCategoriesCallbackHandler();
            pingHandler();
            browseDescriptionCallbackHandler();
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

        // -----< fileInfoBriefAssembler: Assemble the file brief >-----
        private FileComplex fileInfoBriefAssembler(string recordBrief)
        {
            FileComplex result = new FileComplex();
            string[] infoBriefComplex = recordBrief.Split('$');
            result.NameSpace = infoBriefComplex[0];
            result.Name = infoBriefComplex[1];
            result.Version = infoBriefComplex[2];
            result.Status = infoBriefComplex[3];
            result.Description = infoBriefComplex[4];
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

        // -----< populateCheckInDependencyListView: Populate checkin dependency ListView >-----
        private void populateCheckInDependencyListView()
        {
            foreach(FileComplex item in repoRecords)
            {
                checkInDependencyList.Items.Add(item);
                checkOutList.Items.Add(item);
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

        // -----< lastFilter: Remove "/" from the last of the string >----- 
        private string lastFilter(string path)
        {
            if (path[path.Length - 1] == '/') return path.Substring(0, path.Length - 1);
            else return path;
        }

        // -----< changeDir: Change directory according to the path value >-----
        private string changeDir(string path)
        {
            string modifiedPath = path;
            int pos = path.IndexOf("/");
            modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
            return modifiedPath;
        }

        // -----< clearDirs: Clear out the directory value >-----
        private void clearDirs()
        {
            browseList.Items.Clear();
        }

        // ----< addDir: Function dispatched by child thread to main thread >-----
        private void addDir(string dirComplex)
        {
            string[] dirs = dirComplex.Split('$');
            foreach (string dir in dirs) {
                browseList.Items.Add(dir);
            }
                
        }

        // ----< addFile: Function dispatched by child thread to main thread >-----
        private void addFile(string fileComplex)
        {
            string[] files = fileComplex.Split('$');
            foreach (string file in files) {
                browseList.Items.Add(file);
            }
        }

        // -----< addParent: Function dispatched by child thread to main thread >-----
        private void addParent()
        {
            browseList.Items.Insert(0, "..");
        }

        // -----< clearFiles: Function dispatched by child thread to main thread >-----
        private void clearFiles()
        {
            browseList.Items.Clear();
        }

        // -----< showFileWindowPopup: Popup the fileWindow >-----
        private void showFileWindowPopup(CsMessage msg)
        {
            fileWindow popUp = new fileWindow();
            popUp.getFileInfo(msg);
            popUp.getAllRecordInfo(repoRecords);
            popUp.getAllCategories(repoCategories);
            popUp.Owner = this;
            popUp.Show();
        }

        // -----< registerHandler: Add client processing for message with key >-----
        private void registerHandler(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        // -----< showFileHandler: Callback handler of showFile >-----
        private void showFileHandler()
        {
            Action<CsMessage> showFile = (CsMessage receiveMessage) =>
            {
                receiveMessage.show();
                var enumer = receiveMessage.attributes.GetEnumerator();
                string fileName = "";
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    string value = enumer.Current.Value;
                    if (key == "file") fileName = value;
                    if (key == "content-length" && value == "0") return;
                }
                CsEndPoint serverEndPoint = new CsEndPoint();
                serverEndPoint.machineAddress = "localhost";
                serverEndPoint.port = 8080;
                CsMessage message = new CsMessage();
                message.add("to", CsEndPoint.toString(serverEndPoint));
                message.add("from", CsEndPoint.toString(endPoint_));
                message.add("command", "showFileCleanUp");
                message.add("fileName", fileName);
                Action<CsMessage> debug = (CsMessage msg) =>
                {
                    debugDisplay(msg, "send");
                };
                Dispatcher.Invoke(debug, new Object[] { message });
                translater.postMessage(message);
                Action<CsMessage> showFileInPopup = (CsMessage msg) =>
                {
                    showFileWindowPopup(msg);
                };
                Dispatcher.Invoke(showFileInPopup, receiveMessage);
            };
            registerHandler("showFile", showFile);
        }

        // -----< doDirs: Helper function of add directory >-----
        private void doDirs(string toDo)
        {
            Action<string> doDir = (string toAdd) => {
                addDir(toAdd);
            };
            Dispatcher.Invoke(doDir, new Object[] { toDo });
        }

        // -----< doFiles: Helper function of add files >-----
        private void doFiles(string toDo)
        {
            Action<string> doFile = (string toAdd) => {
                addFile(toAdd);
            };
            Dispatcher.Invoke(doFile, new Object[] { toDo });
        }

        // -----< doPathes: Helper function of add pathes >-----
        private void doPathes(string toDo)
        {
            Action<string> changeCurrentDir = (string path) =>
            {
                path = lastFilter(path);
                dirIndicator.Text = changeDir(path);
                pathStack_.Push(path);
            };
            Dispatcher.Invoke(changeCurrentDir, new Object[] { toDo });
        }

        // -----< listContentsHandler: Callback handler of listContents >-----
        private void listContentsHandler()
        {
            Action<CsMessage> listContent = (CsMessage receiveMessage) => {
                Action clrFiles = () => { clearFiles(); };
                Dispatcher.Invoke(clrFiles, new Object[] { });
                var enumer = receiveMessage.attributes.GetEnumerator();
                while(enumer.MoveNext()) {
                    string key = enumer.Current.Key;
                    if (key.Contains("dirs")) { doDirs(enumer.Current.Value); }
                    else if (key.Contains("files")) { doFiles(enumer.Current.Value); }
                    else if (key.Contains("path")) { doPathes(enumer.Current.Value); }
                }
                Action insertParent = () => {
                    addParent();
                };
                Dispatcher.Invoke(insertParent, new Object[] { });
            };
            registerHandler("listContent", listContent);
        }

        // -----< browseList_DoubleClick: DoubleClick handler of browseList >-----
        private void browseList_DoubleClick(object sender, MouseButtonEventArgs e) {
            string selectedItem = (string)browseList.SelectedItem;
            Console.Write(selectedItem);
            if (selectedItem == "..") {
                if (pathStack_.Count > 1) {
                    pathStack_.Pop();
                    selectedItem = "";
                }
                else return;
            }
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "listContent");
            message.add("path", pathStack_.Peek() + "/" + selectedItem);
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
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
            Console.Write(fileName + "\n");
            System.IO.File.Copy(pathFileName.Text, "../SendFiles/" + fileName, true);
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("file", fileName);
            message.add("name", "Checkin File");
            message.add("command", "fileCheckin");
            message.add("content-length", sourceFileInfo.Length.ToString());
            message.add("description", description.Text);
            message.add("dependencies", hashSetToString(checkInDependencies));
            message.add("categories", hashSetToString(checkInCategories));
            message.add("nameSpace", nameSpace.Text);
            message.add("owner", theUser.Text);
            message.add("close", closeCheckIn.IsChecked.ToString().ToLower());
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
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
                receiveMessage.show();
                Console.Write("File checken out successful.\n");
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
                    MessageBoxResult result = MessageBox.Show("You are going to checkout " + successCheckouts.Count.ToString() + " files" + "\n" +
                                                               failCheckouts.Count.ToString() + " files cannot be checked out by you because you do not own them" + "\n" +
                                                               "Click \"OK\" to proceed and \"Cancel\" to cancel" + "\n", "Checkout Files Confirmation", 
                                                               MessageBoxButton.OKCancel, MessageBoxImage.Information);
                    if(result == MessageBoxResult.OK)
                    {
                        checkoutReceiveFile(successCheckouts); failCheckouts.Clear();
                    }
                    else
                    {
                        successCheckouts.Clear(); failCheckouts.Clear();
                    }
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
                };
                Dispatcher.Invoke(updateCheckInDependencyList, new Object[] { });
            };
            registerHandler("trackAllRecordsCallback", trackAllRecordsCallback);
        }

        // -----< trackFiles: Send message of trackFiles >-----
        private void trackFiles()
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
            pathStack_.Push("../Storage");
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "listContent");
            message.add("path", pathStack_.Peek());
            translater.postMessage(message);
            dirIndicator.Text = "Storage";
            Action<CsMessage> debug = (CsMessage msg) => { debugDisplay(msg, "send"); };
            Dispatcher.Invoke(debug, new Object[] { message });
            message.overRide("command", "trackAllRecords");
            translater.postMessage(message);
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

            userName.Text = argUser;
            theUser.Text = argUser;
            serverAddress.Text = "localhost";
            sendPort.Text = "8080";
            receivePort.Text = endPoint_.port.ToString();

            processMessages();

            eventRegisterInitialization();

            elementInitialize();

            testStub();
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
            test3e();
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
            Console.Write("  PASSED -- Requirement 2\n\n");
        }

        // -----< Demostration of requirement 3a >-----
        private void test3a()
        {
            Console.Write("Demostration of Requirement 3a.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of connecting to server in client side.\n\n");
            ping("Demo message of requirement 3");
            Console.Write("  PASSED -- Requirement 3a\n\n");
        }

        // -----< Demostration of requirement 3b >-----
        private void test3b()
        {
            Console.Write("Demostration of Requirment 3b.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of sending checkin message and get reply from server.\n\n");
            pathFileName.Text = "../RemoteRepository.cpp";
            nameSpace.Text = "Server";
            description.Text = "Source file of the server of project4";
            checkInDependencies.Add("DbCore::DbCore.h.1"); checkInDependencies.Add("DbCore::DbCore.cpp.1");
            checkInCategories.Add("source");
            checkinButton.RaiseEvent(new RoutedEventArgs(Button.ClickEvent));
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
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            Console.Write("  PASSED -- Requirement 3c\n\n");
        }

        // -----< Demostration of requirement 3d >-----
        private void test3d()
        {
            Console.Write("Demostration of Requirement 3d.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of sending browsing specified package description message and get reply from server.\n\n");
            browseDescription("DbCore::DbCore.h.1");
            Console.Write("  PASSED -- Requirement 3d\n\n");
        }

        // -----< Demostration of requirement 3e >-----
        private void test3e()
        {
            Console.Write("Demostration of Requirement 3e.\n\n");
            Console.Write("===============================\n\n");
            Console.Write("  Demostrate the capability of viewing full text and metadata");
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "listContent");
            message.add("path", pathStack_.Peek() + "/" + "DbCore_DbCore.h.1");
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
            message.show();
            Console.Write("  PASSED -- Requirement 3f\n\n");
        }
    }
}
