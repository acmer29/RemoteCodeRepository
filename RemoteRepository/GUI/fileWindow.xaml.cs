///////////////////////////////////////////////////////////////////////
// fileWindow.xaml.cs - GUI for RemoteRepository                     //
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
    public partial class fileWindow : Window
    {
        public fileWindow()
        {
            InitializeComponent();
        }
        private FileComplex theFile = new FileComplex();
        private string filePath;
        private string errorInformation;
        private HashSet<FileComplex> allRecords = new HashSet<FileComplex>();
        private HashSet<string> allCategories = new HashSet<string>();
        private HashSet<string> selectedDependencies = new HashSet<string>();
        private HashSet<string> selectedCategories = new HashSet<string>();

        // -----< getFileInfo: get file info from mainWindow >-----
        public void getFileInfo(CsMessage receiveMessage)
        {
            errorInformation = "";
            var enumer = receiveMessage.attributes.GetEnumerator();
            while (enumer.MoveNext())
            {
                if (enumer.Current.Key == "file") filePath = "../SaveFiles/" + enumer.Current.Value;
                else if (enumer.Current.Key == "file-Namespace") theFile.NameSpace = enumer.Current.Value;
                else if (enumer.Current.Key == "file-Name") theFile.Name = enumer.Current.Value;
                else if (enumer.Current.Key == "file-Version") theFile.Version = enumer.Current.Value;
                else if (enumer.Current.Key == "file-Description") theFile.Description = enumer.Current.Value;
                else if (enumer.Current.Key == "file-DateTime") theFile.DateTime = enumer.Current.Value;
                else if (enumer.Current.Key == "file-Dependencies") theFile.Dependencies = stringToArray(enumer.Current.Value);
                else if (enumer.Current.Key == "file-Categories") theFile.Categories = stringToArray(enumer.Current.Value);
                else if (enumer.Current.Key == "file-Status") theFile.Status = enumer.Current.Value;
                else if (enumer.Current.Key == "file-Owner") theFile.Owner = enumer.Current.Value;
                else if (enumer.Current.Key == "error") errorInformation = enumer.Current.Value;
            }
            theFile.Key = theFile.NameSpace + "::" + theFile.Name + "." + theFile.Version;
        }

        // -----< getAllRecordInfo: get all record info from mainWindow >-----
        public void getAllRecordInfo(HashSet<FileComplex> repoRecords)
        {
            allRecords = repoRecords;
        }

        // -----< getAllCategories: get all categories info from mainWindow >-----
        public void getAllCategories(HashSet<string> repoCategories)
        {
            allCategories = repoCategories;
        }

        // -----< SubmitResult: Public SubmitResult handler >-----
        public event Action<FileComplex> SubmitResult;

        // -----< submiteResult: Submit the file >-----
        public void submitResult()
        {
            SubmitResult(theFile);
        }

        // -----< Window_Loaded: load the window >-----
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            loadFileCode();
            loadFileInformation();
            loadFileDependencies();
            loadFileCategories();
            applyChanges.IsEnabled = false;
        }

        // -----< stringToArray: Convert string to array >-----
        private string[] stringToArray(string toConvert)
        {
            return toConvert.Split('$');
        }

        // -----< loadFileCode: Load file code >-----
        private void loadFileCode()
        {
            string fileContent = File.ReadAllText(filePath);
            Paragraph paragraph = new Paragraph();
            paragraph.Inlines.Add(new Run(fileContent));
            fileCode.Blocks.Add(paragraph);
        }

        // -----< loadFileInformation: Load file information >-----
        private void loadFileInformation()
        {
            nameSpace.Text = theFile.NameSpace;
            fileName.Text = theFile.Name;
            version.Text = theFile.Version;
            owner.Text = theFile.Owner;
            status.Text = theFile.Status;
            description.Text = theFile.Description;
            dateTime.Text = theFile.DateTime;
            if (theFile.Status != "open") closeCheckin.IsEnabled = false;
        }

        // -----< loadFileDependencies: Load file dependencies >-----
        private void loadFileDependencies()
        {
            foreach(FileComplex item in allRecords)
            {
                if (item.NameSpace == theFile.NameSpace && item.Name == theFile.Name) continue;
                item.IsChecked = false;
                foreach (string dependencies in theFile.Dependencies)
                {
                    if (dependencies == item.NameSpace + "::" + item.Name + "." + item.Version) { item.IsChecked = true; }
                }
                allRecordBriefList.Items.Add(item);
            }
        }

        // -----< loadFileCategories: Load file categories >-----
        private void loadFileCategories()
        {
            foreach(string item in allCategories)
            {
                KeyValuePair toAdd = new KeyValuePair("", item);
                foreach(string category in theFile.Categories)
                {
                    if(category == item)
                    {
                        toAdd.IsChecked = true;
                    }
                }
                allCategoryList.Items.Add(toAdd);
            }
        }

        // -----< addDependency: Add file dependencies >-----
        private void addDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            selectedDependencies.Add(toAdd);
            applyChanges.IsEnabled = true;
        }

        // -----< removeDependency: Remove file dependencies >-----
        private void removeDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove= selected.Tag.ToString();
            selectedCategories.Remove(toRemove);
            applyChanges.IsEnabled = true;
        }

        // -----< addCategory: Add file categories >-----
        private void addCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            selectedCategories.Add(toAdd);
            applyChanges.IsEnabled = true;
        }

        // -----< removeCategory: Remove file categories >-----
        private void removeCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove = selected.Tag.ToString();
            selectedCategories.Remove(toRemove);
            applyChanges.IsEnabled = true;
        }

        // -----< applyDependencies_Click: Click handler of addDependencyList checkbox >-----
        private void applyDependencies_Click(object sender, RoutedEventArgs e)
        {
            List<string> result = new List<string>();
            foreach (string item in selectedDependencies)
            {
                result.Add(item);
            }
            theFile.Dependencies = result.ToArray();
            submitResult();
        }

        // -----< applyCategories_Click: Click handler of addCategoryList checkbox >-----
        private void applyCategories_Click(object sender, RoutedEventArgs e)
        {
            List<string> result = new List<string>();
            foreach (string item in selectedCategories)
            {
                result.Add(item);
            }
            theFile.Categories = result.ToArray();
            submitResult();
        }

        // -----< applyBasicInfo_Click: Handle applyBasicInfo button click event >-----
        private void applyBasicInfo_Click(object sender, RoutedEventArgs e)
        {
            theFile.Owner = owner.Text;
            theFile.Status = status.Text;
            theFile.Description = description.Text;
            theFile.DateTime = dateTime.Text;
            submitResult();
        }

        // -----< cancel_Click: Handle cancel button click event >-----
        private void cancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
