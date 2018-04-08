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
    /// Interaction logic for KeyValuePopup.xaml
    /// </summary>
    public partial class KeyValuePopup : Window
    {
        public KeyValuePopup()
        {
            InitializeComponent();
        }
        public void getKey(string key)
        {
            toShow.Key = key;
        }
        public void getValue(string value)
        {
            toShow.Value = value;
        }
        public void submit()
        {
            submitResult(toShow);
        }
        public void Window_Loaded(object sender, RoutedEventArgs e)
        {
            Console.Write("the key is " + toShow.Key);
            Console.Write("the value is " + toShow.Value);
            keyLabel.Content = toShow.Key;
            valueTextBox.Text = toShow.Value;
        }

        public event Action<KeyValuePair> submitResult;
        private KeyValuePair toShow = new KeyValuePair();
        
        private void OK_Click(object sender, RoutedEventArgs e)
        {
            toShow.Value = valueTextBox.Text;
            submit();
            this.Close();
        }

        private void Cancel_Click(object sender, RoutedEventArgs e)
        {
            this.Close();
        }
    }
}
