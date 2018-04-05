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
                else if (enumer.Current.Key == "error") errorInformation = enumer.Current.Value;
            }
        }

        public void getAllRecordInfo(List<FileComplex> repoRecords)
        {
            allRecords = repoRecords;
        }
        public void Window_Loaded(object sender, RoutedEventArgs e)
        {
            loadFileCode();
            loadFileInformation();
            loadFileDependencies();
            loadFileCategories();
            
        }
        private FileComplex theFile = new FileComplex();
        private string filePath;
        private string errorInformation;
        private List<FileComplex> allRecords;

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
            basicDataList.Items.Add(new KeyValuePair("Namespace", theFile.NameSpace));
            basicDataList.Items.Add(new KeyValuePair("Name", theFile.Name));
            basicDataList.Items.Add(new KeyValuePair("Version", theFile.Version));
            basicDataList.Items.Add(new KeyValuePair("Description", theFile.Description));
            basicDataList.Items.Add(new KeyValuePair("Last Modified", theFile.DateTime));
            basicDataList.Items.Add(new KeyValuePair("Status", theFile.Status));
        }

        private void loadFileDependencies()
        {
            foreach(FileComplex item in allRecords)
            {
                if (item.NameSpace == theFile.NameSpace && item.Name == theFile.Name) continue;
                allRecordBriefList.Items.Add(item);
            }
        }

        private void loadFileCategories()
        {

        }
    }
}
