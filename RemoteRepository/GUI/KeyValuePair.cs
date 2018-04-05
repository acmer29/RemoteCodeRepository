using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI
{
    class KeyValuePair
    {
        public KeyValuePair() { }
        public KeyValuePair(string key, string val)
        {
            this.key = key;
            this.val = val;
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
        private string key;
        private string val;
    }
}
