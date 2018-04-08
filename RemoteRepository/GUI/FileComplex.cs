using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GUI
{
    public class FileComplex
    {
        public FileComplex() { }
        public FileComplex(string nameSpace, string name, string version, string status)
        {
            NameSpace = nameSpace; Name = name; Version = version; Status = status;
        }
        public string NameSpace
        {
            get { return nameSpace; }
            set { nameSpace = value; }
        }
        public string Name
        {
            get { return name; }
            set { name = value; }
        }
        public string Description
        {
            get { return description; }
            set { description = value; }
        }
        public string DateTime
        {
            get { return dateTime; }
            set { dateTime = value; }
        }
        public string Version
        {
            get { return version; }
            set { version = value; }
        }
        public string Status
        {
            get { return status; }
            set { status = value; }
        }
        public string Owner
        {
            get { return owner; }
            set { owner = value; }
        }
        public string[] Dependencies
        {
            get { return dependencies; }
            set { dependencies = value; }
        }
        public string[] Categories
        {
            get { return categories; }
            set { categories = value; }
        }
        public bool IsChecked
        {
            get { return isChecked; }
            set { isChecked = value; }
        }
        public string Key
        {
            get { return key; }
            set { key = value; }
        }
        private string nameSpace;
        private string name;
        private string description;
        private string dateTime;
        private string version;
        private string status;
        private string owner;
        private string[] dependencies;
        private string[] categories;
        private bool isChecked;
        private string key;
    }
}
