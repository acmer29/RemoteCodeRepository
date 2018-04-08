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
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            Console.Title = "Client Console";
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

        // Register
        private void eventRegisterInitialization()
        {
            checkinCallbackHandler();
            checkoutCallbackHandler();
            listContentsHandler();
            showFileHandler();
            trackAllRecordsCallbackHandler();
            trackAllCategoriesCallbackHandler();
        }

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

        private FileComplex fileInfoBriefAssembler(string recordBrief)
        {
            FileComplex result = new FileComplex();
            string[] infoBriefComplex = recordBrief.Split('$');
            result.NameSpace = infoBriefComplex[0];
            result.Name = infoBriefComplex[1];
            result.Version = infoBriefComplex[2];
            result.Status = infoBriefComplex[3];
            result.Key = result.NameSpace + "::" + result.Name + "." + result.Version;
            return result;
        }

        private string hashSetToString(HashSet<string> toConvert)
        {
            string result = "";
            foreach (string item in toConvert)
            {
                result += item + ",";
            }
            return result.Substring(0, result.Length - 1); 
        }

        private void addDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            checkInDependencies.Add(toAdd);
        }

        private void removeDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove = selected.Tag.ToString();
            checkInDependencies.Remove(toRemove);
        }

        private void addCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            checkInCategories.Add(toAdd);
        }

        private void removeCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove = selected.Tag.ToString();
            checkInCategories.Remove(toRemove);
        }

        private void populateCheckInDependencyListView()
        {
            foreach(FileComplex item in repoRecords)
            {
                checkInDependencyList.Items.Add(item);
            }
        }

        private void populateCheckInCategoryListView()
        {
            foreach (string item in repoCategories)
            {
                KeyValuePair toAdd = new KeyValuePair("", item);
                checkinCategoryList.Items.Add(toAdd);
            }
        }

        private string lastFilter(string path)
        {
            if (path[path.Length - 1] == '/') return path.Substring(0, path.Length - 1);
            else return path;
        }

        private string changeDir(string path)
        {
            string modifiedPath = path;
            int pos = path.IndexOf("/");
            modifiedPath = path.Substring(pos + 1, path.Length - pos - 1);
            return modifiedPath;
        }

        private void clearDirs()
        {
            browseList.Items.Clear();
        }

        //----< function dispatched by child thread to main thread >-------
        private void addDir(string dirComplex)
        {
            string[] dirs = dirComplex.Split('$');
            foreach (string dir in dirs) {
                browseList.Items.Add(dir);
            }
                
        }

        //----< function dispatched by child thread to main thread >-------
        private void addFile(string fileComplex)
        {
            string[] files = fileComplex.Split('$');
            foreach (string file in files) {
                browseList.Items.Add(file);
            }
        }

        //----< function dispatched by child thread to main thread >-------
        private void addParent()
        {
            browseList.Items.Insert(0, "..");
        }

        //----< function dispatched by child thread to main thread >-------
        private void clearFiles()
        {
            browseList.Items.Clear();
        }

        private void showFileWindowPopup(CsMessage msg)
        {
            fileWindow popUp = new fileWindow();
            popUp.getFileInfo(msg);
            popUp.getAllRecordInfo(repoRecords);
            popUp.getAllCategories(repoCategories);
            popUp.Owner = this;
            popUp.Show();
        }

        //----< add client processing for message with key >---------------
        private void registerHandler(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        private void showFileHandler()
        {
            Action<CsMessage> showFile = (CsMessage receiveMessage) =>
            {
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

        private void listContentsHandler()
        {
            Action<CsMessage> listContent = (CsMessage receiveMessage) => {
                Action clrFiles = () => { clearFiles(); };
                Dispatcher.Invoke(clrFiles, new Object[] { });
                var enumer = receiveMessage.attributes.GetEnumerator();
                while(enumer.MoveNext()) {
                    string key = enumer.Current.Key;
                    Console.Write(key);
                    if(key.Contains("dirs")) {
                        Action<string> doDir = (string dirComplex) => {
                            addDir(dirComplex);
                        };
                        Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
                    }
                    else if (key.Contains("files")) {
                        Action<string> doFile = (string fileComplex) => {
                            addFile(fileComplex);
                        };
                        Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
                    }
                    else if(key.Contains("path")) {
                        Action<string> changeCurrentDir = (string path) => {
                            path = lastFilter(path);
                            dirIndicator.Text = changeDir(path);
                            pathStack_.Push(path);
                        };
                        Dispatcher.Invoke(changeCurrentDir, new Object[] { enumer.Current.Value });
                    }
                }
                Action insertParent = () => {
                    addParent();
                };
                Dispatcher.Invoke(insertParent, new Object[] { });
            };
            registerHandler("listContent", listContent);
        }

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

        // -----< Receive checkin result from the server >-----
        private void checkinCallbackHandler()
        {
            Action<CsMessage> checkinCallback = (CsMessage receiveMessage) =>
            {
                string errorInfo = "Your file has been successfully checked in";
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    if (enumer.Current.Key == "errorInfo") errorInfo = enumer.Current.Value;
                }
                MessageBoxResult result = MessageBox.Show(errorInfo, "Checkin result", MessageBoxButton.OK, MessageBoxImage.Information);
            };
            registerHandler("checkinCallback", checkinCallback);
        }
        private void Check_In_Click(object sender, RoutedEventArgs e)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            System.IO.FileInfo sourceFileInfo = new System.IO.FileInfo(pathFileName.Text);
            string fileName = pathFileName.Text.Substring(pathFileName.Text.LastIndexOf("\\") + 1);
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

        private void checkoutCallbackHandler()
        {
            Action<CsMessage> checkoutCallback = (CsMessage receiveMessage) =>
            {
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    string value = enumer.Current.Value;
                    Console.WriteLine("\n" + key + ": " + value);
                }
            };
            registerHandler("checkoutCallback", checkoutCallback);
        }

        private void Check_Out_Click(object sender, RoutedEventArgs e)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("command", "fileCheckout");
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
        }

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

        private void Open_FileForm(object sender, RoutedEventArgs e)
        {
            var openFileDialog = new Microsoft.Win32.OpenFileDialog();
            var result = openFileDialog.ShowDialog();
            if (result == true)
            {
                this.pathFileName.Text = openFileDialog.FileName;
            }
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            endPoint_.machineAddress = "localhost";
            endPoint_.port = 8082;
            translater.listen(endPoint_);

            userName.Text = "Administrator";
            theUser.Text = "Administrator";
            serverAddress.Text = "localhost";
            sendPort.Text = "8080";
            receivePort.Text = "8082";

            processMessages();

            eventRegisterInitialization();

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
            dirIndicator.Text= "Storage";
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            message.overRide("command", "trackAllRecords");
            translater.postMessage(message);
            Dispatcher.Invoke(debug, new Object[] { message });
            message.overRide("command", "trackAllCategories");
            translater.postMessage(message);
            Dispatcher.Invoke(debug, new Object[] { message });
        }
    }
}
