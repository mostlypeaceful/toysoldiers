using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace CheckIn
{
    public partial class IgnoredForm : Form
    {
        static string globalIgnorePath = "p4_global_ignores.txt";

        string workspaceIgnorePath = null;
        static bool localIgnoresEnabled = true;

        public bool okClicked = false;

        public IgnoredForm(string workspaceName)
        {
            InitializeComponent();

            globalIgnorePath = Environment.GetEnvironmentVariable("SigEngine") + "\\Bin\\" + globalIgnorePath;

            if (localIgnoresEnabled)
            {
                workspaceIgnoreListBox.Enabled = true;
                addButton.Enabled = true;
                deleteButton.Enabled = true;

                if (workspaceName == "")
                    throw (new Exception("Could not determine the workspace name."));
                workspaceIgnorePath = "C:\\" + workspaceName.Replace(" ", "") + "_ignores.txt";
            }

            InitializeListBoxes();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            // Reset the lists back to the on-file version
            InitializeListBoxes();

            okClicked = false;

            Close();
        }

        public List<string> GetIgnoredList()
        {
            List<string> ignoredList = new List<string>();

            foreach (string file in globalIgnoreListBox.Items)
                ignoredList.Add(file);

            foreach (string file in workspaceIgnoreListBox.Items)
                ignoredList.Add(file);

            return ignoredList;
        }

        private void addButton_Click(object sender, EventArgs e)
        {
            while (true)
            {
                InputBox inputBox = new InputBox();
                inputBox.ShowDialog();
                if (inputBox.okClicked)
                {
                    if (IsValidRegex(inputBox.data))
                    {
                        workspaceIgnoreListBox.Items.Add(inputBox.data);
                        okButton.Focus();
                        break;
                    }
                    else
                        MessageBox.Show("That is an invalid filter. Try again.");
                }
                else
                    break;
            }
        }

        private void deleteButton_Click(object sender, EventArgs e)
        {
            for (int i = 0; i < workspaceIgnoreListBox.Items.Count; i++)
            {
                if (workspaceIgnoreListBox.GetSelected(i))
                {
                    workspaceIgnoreListBox.Items.RemoveAt(i);
                    i--;
                }
            }
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            if (workspaceIgnorePath != null && workspaceIgnorePath != "")
            {
                TextWriter workspaceIgnoreFile = new StreamWriter(workspaceIgnorePath, false);
                foreach (string file in workspaceIgnoreListBox.Items)
                    workspaceIgnoreFile.WriteLine(file);
                workspaceIgnoreFile.Close();
            }

            okClicked = true;

            Close();
        }

        private List<string> GetIgnoredListFromFile(string path)
        {
            List<string> ignoredList = new List<string>();

            if (!File.Exists(path))
                File.Create(path).Close();

            using (StreamReader logFile = new StreamReader(path))
            {
                string line = "";
                while ((line = logFile.ReadLine()) != null)
                {
                    // Any environment variable needs to be escaped for use in a regular expression
                    Hashtable envVars = (Hashtable)Environment.GetEnvironmentVariables();
                    foreach (DictionaryEntry envVar in envVars)
                        line = line.Replace("%" + (string)envVar.Key + "%", Regex.Escape((string)envVar.Value));

                    if (IsValidRegex(line))
                        ignoredList.Add(line);
                }
            }

            return ignoredList;
        }

        private void InitializeListBoxes()
        {
            globalIgnoreListBox.Items.Clear();
            List<string> globalIgnoreList = GetIgnoredListFromFile(globalIgnorePath);
            foreach (string file in globalIgnoreList)
                globalIgnoreListBox.Items.Add(file);

            if (localIgnoresEnabled)
            {
                workspaceIgnoreListBox.Items.Clear();
                List<string> workspaceIgnoreList = GetIgnoredListFromFile(workspaceIgnorePath);
                foreach (string file in workspaceIgnoreList)
                    workspaceIgnoreListBox.Items.Add(file);
            }
        }

        private static bool IsValidRegex(string pattern)
        {
            if (pattern == null || pattern == "")
                return false;

            try
            {
                Regex.Match("", pattern);
            }
            catch (ArgumentException)
            {
                return false;
            }

            return true;
        }
    }
}
