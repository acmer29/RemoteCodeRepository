using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI
{
    public class KeyValuePair
    {
        public KeyValuePair() { isChecked = false; }
        public KeyValuePair(string key, string val)
        {
            this.key = key;
            this.val = val;
            this.isChecked = false;
        }
        public KeyValuePair(string key, string val, bool isChecked)
        {
            this.key = key;
            this.val = val;
            this.isChecked = isChecked;
        }
        public string Key
        {
            get { return key; }
            set { key = value; }
        }
        public string Value
        {
            get { return val; }
            set { val = value; }
        }
        public bool IsChecked
        {
            get { return isChecked; }
            set { isChecked = value; }
        }
        private string key;
        private string val;
        private bool isChecked;
    }
}
