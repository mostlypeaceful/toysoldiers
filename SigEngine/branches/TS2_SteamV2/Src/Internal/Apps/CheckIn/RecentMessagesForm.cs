using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace CheckIn
{
    public partial class RecentMessagesForm : Form
    {
        private string recentMessagesPath = null;
        private string selectedMessage = "";
        private bool useServerMessages = false;
        private int listSize = 10;
        private string workspaceName;

        public string SelectedMessage
        {
            get
            {
                return selectedMessage;
            }
        }

        private void AddServerMessages()
        {
            // Get the change output
            P4Process process = new P4Process("changes -l -c " + workspaceName + " -s submitted -m " + listSize);
            process.Start();
            while (!process.StandardOutput.EndOfStream || !process.HasExited)
            {
                string line = process.ReadLine(process.StandardOutput);
                string desc = CheckInForm.GetTagData(line, "desc");
                if (desc != "")
                {
                    // We replace all instances of multiple whitespace characters with a
                    // single space - this handles multiline descriptions nicely
                    AddMessageToBottom(Regex.Replace(desc.Trim(), @"\s+", " "));
                }
            }
        }

        public RecentMessagesForm(string workspaceName)
        {
            InitializeComponent();

            this.workspaceName = workspaceName;

            recentMessagesPath = "C:\\" + this.workspaceName.Replace(" ", "") + "_recent_messages.txt";

            if (useServerMessages)
            {
                AddServerMessages();
            }
            else
            {
                List<string> recentMessages = GetRecentMessagesFromFile();
                if (recentMessages.Count > 0)
                {
                    foreach (string message in recentMessages)
                        AddMessageToBottom(message);
                }
                else
                    AddServerMessages();
            }
        }

        ~RecentMessagesForm()
        {
            using (TextWriter messageFile = new StreamWriter(recentMessagesPath))
            {
                foreach (string message in recentMessagesListBox.Items)
                    messageFile.WriteLine(message);
            }
        }

        private void closeButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void recentMessagesListBox_DoubleClick(object sender, EventArgs e)
        {
            selectedMessage = recentMessagesListBox.SelectedItem.ToString();
            Close();
        }

        private List<string> GetRecentMessagesFromFile()
        {
            List<string> messageList = new List<string>();

            if (recentMessagesPath == null || recentMessagesPath == "")
                return messageList;

            if (!File.Exists(recentMessagesPath))
                File.Create(recentMessagesPath).Close();

            using (StreamReader messageFile = new StreamReader(recentMessagesPath))
            {
                string line = "";
                while ((line = messageFile.ReadLine()) != null)
                    messageList.Add(line);
            }

            return messageList;
        }

        public void AddMessageToTop(string message)
        {
            message = message.Replace("\r", "");
            message = message.Replace("\n", " ");
            message = Regex.Replace(message, @"\s+", " ");

            if (!recentMessagesListBox.Items.Contains(message))
                recentMessagesListBox.Items.Insert(0, message);

            TrimList();
        }

        public void AddMessageToBottom(string message)
        {
            message = message.Replace("\r", "");
            message = message.Replace("\n", " ");
            message = Regex.Replace(message, @"\s+", " ");

            if (!recentMessagesListBox.Items.Contains(message))
                recentMessagesListBox.Items.Add(message);

            TrimList();
        }

        public void TrimList()
        {
            // Limit the size of the list that's stored by removing
            // from the bottom
            while (recentMessagesListBox.Items.Count >= listSize)
                recentMessagesListBox.Items.RemoveAt(recentMessagesListBox.Items.Count - 1);
        }
    }
}
