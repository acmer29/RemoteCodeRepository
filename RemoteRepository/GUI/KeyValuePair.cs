﻿///////////////////////////////////////////////////////////////////////
// KeyValuePair.cs - Key Value Pair Class for ListView Display       //
// ver 1.0                                                           //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018           //
///////////////////////////////////////////////////////////////////////
/*
 * Package Operations:
 * -------------------
 * This package provides a KeyValue for Project4. It's 
 * responsibilities are to:
 * - Provide a data structure of key value pair which is useful for 
 *   listview display.
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
