///////////////////////////////////////////////////////////////////////
// filterWindow.xaml.cs - GUI for filterWindow                       //
// ver 1.0                                                           //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018           //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a WPF-based GUI for Project4. It's 
 * responsibilities is to :
 * 
 * Provide a popup window for filter of query.
 *   
 * Required Files:
 * ---------------
 * Mainwindow.xaml, MainWindow.xaml.cs
 * FileComplex.cs
 * KeyValuePair.cs
 * fileWindow.xaml, fileWindow.xaml.cs
 * filterWindow.xaml, filterWindow.xaml.cs
 * Translater.dll
 * 
 * Maintenance History:
 * --------------------
 * ver 1.0 : 30 Apr 2018
 * - first release
 * 
 */

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
    public partial class filterWindow : Window
    {
        public filterWindow()
        {
            InitializeComponent();
        }

        private string[] theQuery = new string [5];

        // -----< SubmitFilter: Submit filter event handler >-----
        public event Action<string[]> SubmitFilter;

        // -----< submitFilter: Submit the result >-----
        public void submitFilter()
        {
            constructQuery();
            SubmitFilter(theQuery);
        }

        // -----< constructQuery: Construct the query >-----
        private string[] constructQuery()
        {
            if (keepNameSpace.IsChecked == false) theQuery[0] = nameSpace.Text;
            else theQuery[0] = "\n";
            if (keepFileName.IsChecked == false) theQuery[1] = fileName.Text;
            else theQuery[1] = "\n";
            if (keepVersion.IsChecked == false) theQuery[2] = version.Text;
            else theQuery[2] = "\n";
            if (keepDependencies.IsChecked == false) theQuery[3] = dependencies.Text;
            else theQuery[3] = "\n";
            if (keepCategories.IsChecked == false) theQuery[4] = categories.Text;
            else theQuery[4] = "\n";
            return theQuery;
        }

        // -----< Window_Loaded: Load the window >-----
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            keepNameSpace.IsChecked = true;
            nameSpace.IsEnabled = false;
            keepFileName.IsChecked = true;
            fileName.IsEnabled = false;
            keepVersion.IsChecked = true;
            version.IsEnabled = false;
            keepDependencies.IsChecked = true;
            dependencies.IsEnabled = false;
            keepCategories.IsChecked = true;
            categories.IsEnabled = false;
        }

        // -----< keepNameSpace_Checked: Handle keepNameSpace checked event >-----
        private void keepNameSpace_Checked(object sender, RoutedEventArgs e)
        {
            nameSpace.Text = "";
            nameSpace.IsEnabled = false;
        }

        // -----< keepNameSpace_Unchecked: Handle keepNameSpace unchecked event >-----
        private void keepNameSpace_Unchecked(object sender, RoutedEventArgs e)
        {
            nameSpace.IsEnabled = true;
        }

        // -----< keepFileName_Checked: Handle keepFileName checked event >-----
        private void keepFileName_Checked(object sender, RoutedEventArgs e)
        {
            fileName.Text = "";
            fileName.IsEnabled = false;
        }

        // -----< keepFileName_Unchecked: Handle keepFileName unchecked event >-----
        private void keepFileName_Unchecked(object sender, RoutedEventArgs e)
        {
            fileName.IsEnabled = true;
        }

        // -----< keepVersion_Checked: Handle keepVersion checked event >-----
        private void keepVersion_Checked(object sender, RoutedEventArgs e)
        {
            version.Text = "";
            version.IsEnabled = false;
        }

        // -----< keepVersion_Unchecked: Handle keepVersion unchecked event >-----
        private void keepVersion_Unchecked(object sender, RoutedEventArgs e)
        {
            version.IsEnabled = true;
        }

        // -----< keepVersion_Unchecked: Handle keepVersion unchecked event >-----
        private void keepDependencies_Checked(object sender, RoutedEventArgs e)
        {
            dependencies.Text = "";
            dependencies.IsEnabled = false;
        }

        // -----< keepDependencies_Unchecked: Handle keepDependencies unchecked event >-----
        private void keepDependenices_Unchecked(object sender, RoutedEventArgs e)
        {
            dependencies.IsEnabled = true;
        }

        // -----< keepCategories_Checked: Handle keepCategories checked event >-----
        private void keepCategories_Checked(object sender, RoutedEventArgs e)
        {
            categories.Text = "";
            categories.IsEnabled = false;
        }

        // -----< keepCategories_Unchecked: Handle keepCategories unchecked event >-----
        private void keepCategories_Unchecked(object sender, RoutedEventArgs e)
        {
            categories.IsEnabled = true;
        }

        // -----< SetFilter_Click: Handle SetFilter click event >-----
        private void SetFilter_Click(object sender, RoutedEventArgs e)
        {
            int result = canSubmit();
            if(result == 0)
            {
                submitFilter();
                this.Close();
            }
            else
            {
                string errorInfo = "";
                if (result == 1) errorInfo = "The version must be decimal integar!";
                else if (result == 2) errorInfo = "Filename must not be empty unless select \"Select all files\"!";
                else if (result == 3) errorInfo = "name and namespace must only consist of digit and letter!";
                MessageBox.Show(errorInfo, "Set Filter", MessageBoxButton.OK, MessageBoxImage.Information);
            }
        }

        // -----< Cancel_Click: Handle Cancel click event >-----
        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }

        // -----< canSubmit: Check if the filter can be submitted >-----
        private int canSubmit()
        {
            int n;
            if (int.TryParse(version.Text, out n) == false && keepVersion.IsChecked != true) return 1;
            if (fileName.Text == "" && keepFileName.IsChecked != true) return 2;
            if (isAlphaDigit(fileName.Text) == false || isAlphaDigit(nameSpace.Text) == false) return 3;
            return 0;
        }

        // -----< isAlphaDigit: Check if the string is consist of letter and digits >-----
        bool isAlphaDigit(string toCheck)
        {
            string valid = @"^[A-Za-z0-9\s]*$";
            return Regex.IsMatch(toCheck, valid);
        }
    }
}
