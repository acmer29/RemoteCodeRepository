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
    /// <summary>
    /// Interaction logic for fileWindow.xaml
    /// </summary>
    
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

        public void getAllRecordInfo(HashSet<FileComplex> repoRecords)
        {
            allRecords = repoRecords;
        }

        public void getAllCategories(HashSet<string> repoCategories)
        {
            allCategories = repoCategories;
        }
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            loadFileCode();
            loadFileInformation();
            loadFileDependencies();
            loadFileCategories();
            applyChanges.IsEnabled = false;
        }
        

        private string[] stringToArray(string toConvert)
        {
            return toConvert.Split('$');
        }

        private void loadFileCode()
        {
            string fileContent = File.ReadAllText(filePath);
            Paragraph paragraph = new Paragraph();
            paragraph.Inlines.Add(new Run(fileContent));
            fileCode.Blocks.Add(paragraph);
        }

        private void loadFileInformation()
        {
            basicDataList.Items.Add(new KeyValuePair("Namespace", theFile.NameSpace, false));
            basicDataList.Items.Add(new KeyValuePair("Name", theFile.Name, false));
            basicDataList.Items.Add(new KeyValuePair("Status", theFile.Status, false));
            basicDataList.Items.Add(new KeyValuePair("Owner", theFile.Owner, false));
            basicDataList.Items.Add(new KeyValuePair("Version", theFile.Version, false));
            basicDataList.Items.Add(new KeyValuePair("Last Modified", theFile.DateTime, false));
            description.Text = theFile.Description;
        }

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

        private void addDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            selectedDependencies.Add(toAdd);
            applyChanges.IsEnabled = true;
        }

        private void removeDependency(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove= selected.Tag.ToString();
            selectedCategories.Remove(toRemove);
            applyChanges.IsEnabled = true;
        }

        private void addCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toAdd = selected.Tag.ToString();
            selectedCategories.Add(toAdd);
            applyChanges.IsEnabled = true;
        }

        private void removeCategory(object sender, RoutedEventArgs e)
        {
            CheckBox selected = sender as CheckBox;
            string toRemove = selected.Tag.ToString();
            selectedCategories.Remove(toRemove);
            applyChanges.IsEnabled = true;
        }

        private void applyDependencies_Click(object sender, RoutedEventArgs e)
        {
            List<string> result = new List<string>();
            foreach (string item in selectedDependencies)
            {
                result.Add(item);
            }
            theFile.Dependencies = result.ToArray();
        }

        private void applyCategories_Click(object sender, RoutedEventArgs e)
        {
            List<string> result = new List<string>();
            foreach (string item in selectedCategories)
            {
                result.Add(item);
            }
            theFile.Dependencies = result.ToArray();
        }

        //private void basicDataListItem_DoubleClick(object sender, RoutedEventArgs e)
        //{
        //    //ListViewItem selected = sender as ListViewItem;
        //    KeyValuePair selected = basicDataList.SelectedItem as KeyValuePair;
        //    string theKey = selected.Key;
        //    string theValue = selected.Value;
        //    if (theKey == "Name" || theKey == "Namespace") return;
        //    KeyValuePopup editPopup = new KeyValuePopup();
        //    editPopup.getKey(theKey);
        //    editPopup.getValue(theValue);
        //    editPopup.submitResult += submission =>
        //    {
        //        selected = submission;
        //        Console.Write(selected.Key + ": " + selected.Value);
        //    };
        //    editPopup.Owner = this;
        //    editPopup.Show();
        //}

        private void submitFileComplex(object sender, RoutedEventArgs e)
        {

        }
    }
}
