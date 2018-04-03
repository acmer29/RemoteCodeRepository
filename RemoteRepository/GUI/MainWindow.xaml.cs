using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
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
        private Translater translater;
        private CsEndPoint endPoint_;
        private Thread rcvThrd = null;
        private Dictionary<string, Action<CsMessage>> dispatcher_
          = new Dictionary<string, Action<CsMessage>>();

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
            getDirsHandler();
            getFilesHandler();
            listContentsHandler();
            showFileHandler();
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

        //----< add client processing for message with key >---------------
        private void registerHandler(string key, Action<CsMessage> clientProc)
        {
            dispatcher_[key] = clientProc;
        }

        private void getDirsHandler()
        {
            Action<CsMessage> getDirs = (CsMessage rcvMsg) =>
            {
                Action clrDirs = () =>
                {
                    clearDirs();
                };
                Dispatcher.Invoke(clrDirs, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("dir"))
                    {
                        Action<string> doDir = (string dir) =>
                        {
                            addDir(dir);
                        };
                        Dispatcher.Invoke(doDir, new Object[] { enumer.Current.Value });
                    }
                }
                Action insertUp = () =>
                {
                    addParent();
                };
                Dispatcher.Invoke(insertUp, new Object[] { });
            };
            registerHandler("getDirs", getDirs);
        }
        //----< load getFiles processing into dispatcher dictionary >------

        private void getFilesHandler()
        {
            Action<CsMessage> getFiles = (CsMessage rcvMsg) =>
            {
                Action clrFiles = () =>
                {
                    clearFiles();
                };
                Dispatcher.Invoke(clrFiles, new Object[] { });
                var enumer = rcvMsg.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    if (key.Contains("file"))
                    {
                        Action<string> doFile = (string file) =>
                        {
                            addFile(file);
                        };
                        Dispatcher.Invoke(doFile, new Object[] { enumer.Current.Value });
                    }
                }
            };
            registerHandler("getFiles", getFiles);
        }

        private void showFileHandler()
        {
            Action<CsMessage> showFile = (CsMessage receiveMessage) =>
            {
                var enumer = receiveMessage.attributes.GetEnumerator();
                while (enumer.MoveNext())
                {
                    string key = enumer.Current.Key;
                    string value = enumer.Current.Value;
                    Console.WriteLine("\n" + key + ": " + value);
                }
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
                receiveMessage.show();
            };
            registerHandler("checkinCallback", checkinCallback);
        }
        private void Check_In_Click(object sender, RoutedEventArgs e)
        {
            CsEndPoint serverEndPoint = new CsEndPoint();
            serverEndPoint.machineAddress = "localhost";
            serverEndPoint.port = 8080;
            CsMessage message = new CsMessage();
            message.add("to", CsEndPoint.toString(serverEndPoint));
            message.add("from", CsEndPoint.toString(endPoint_));
            message.add("file", "server.js");
            message.add("name", "Checkin File");
            message.add("command", "fileCheckin");
            System.IO.FileInfo file = new System.IO.FileInfo("D:/Spring2018/cse687/RemoteRepository/server.js");
            message.add("content-length", file.Length.ToString());
            Action<CsMessage> debug = (CsMessage msg) =>
            {
                debugDisplay(msg, "send");
            };
            Dispatcher.Invoke(debug, new Object[] { message });
            translater.postMessage(message);
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
            endPoint_ = new CsEndPoint();
            endPoint_.machineAddress = "localhost";
            endPoint_.port = 8082;
            translater = new Translater();
            translater.listen(endPoint_);

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
        }
    }
}
