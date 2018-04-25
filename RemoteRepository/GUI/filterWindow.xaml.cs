using System;
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
using System.Windows.Shapes;

namespace GUI
{
    /// <summary>
    /// Interaction logic for filterWindow.xaml
    /// </summary>
    public partial class filterWindow : Window
    {
        public filterWindow()
        {
            InitializeComponent();
        }

        private string[] theQuery = new string [5];

        public event Action<string[]> SubmitFilter;

        public void submitFilter()
        {
            constructQuery();
            SubmitFilter(theQuery);
        }

        private string[] constructQuery()
        {
            if (keepFileName.IsChecked == false) theQuery[0] = fileName.Text;
            else theQuery[0] = "\n";
            if (keepVersion.IsChecked == false) theQuery[1] = version.Text;
            else theQuery[1] = "\n";
            if (keepDependencies.IsChecked == false) theQuery[2] = dependencies.Text;
            else theQuery[2] = "\n";
            if (keepCategories.IsChecked == false) theQuery[3] = categories.Text;
            else theQuery[3] = "\n";
            return theQuery;
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {

        }

        private void keepFileName_Checked(object sender, RoutedEventArgs e)
        {
            fileName.IsEnabled = false;
        }

        private void keepFileName_Unchecked(object sender, RoutedEventArgs e)
        {
            fileName.IsEnabled = true;
        }

        private void keepVersion_Checked(object sender, RoutedEventArgs e)
        {
            version.IsEnabled = false;
        }

        private void keepVersion_Unchecked(object sender, RoutedEventArgs e)
        {
            version.IsEnabled = true;
        }
        private void keepDependencies_Checked(object sender, RoutedEventArgs e)
        {
            dependencies.IsEnabled = false;
        }

        private void keepDependenices_Unchecked(object sender, RoutedEventArgs e)
        {
            dependencies.IsEnabled = true;
        }
        private void keepCategories_Checked(object sender, RoutedEventArgs e)
        {
            categories.IsEnabled = false;
        }

        private void keepCategories_Unchecked(object sender, RoutedEventArgs e)
        {
            categories.IsEnabled = true;
        }

        private void SetFilter_Click(object sender, RoutedEventArgs e)
        {
            submitFilter();
            this.Close();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
