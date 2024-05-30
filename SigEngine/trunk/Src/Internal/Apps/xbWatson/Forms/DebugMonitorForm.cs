#region File Information
//-----------------------------------------------------------------------------
// DebugMonitorForm.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class DebugMonitorForm : Form
    {
        private string consoleNameOrIP;
        private DebugMonitor monitor;
    
        public DebugMonitorForm(string consoleNameOrIP)
        {
            InitializeComponent();

            // Start each log window maximized
            WindowState = FormWindowState.Maximized;
            
            this.consoleNameOrIP = consoleNameOrIP;
            Text = consoleNameOrIP;

            monitor = new DebugMonitor(this);
            
            ScrollToRecentMessages = true;

            // First add the text and color to a dictionary so it's easy
            // to add more of them later without changing much
            Dictionary<string, Color> filtersList = new Dictionary<string, Color>();
            filtersList.Add("None", Color.Black);
            filtersList.Add("Rtti", Color.Brown);
            filtersList.Add("File", Color.BurlyWood);
            filtersList.Add("Thread", Color.Peru);
            filtersList.Add("Physics", Color.BlueViolet);
            filtersList.Add("Input", Color.Chocolate);
            filtersList.Add("Graphics", Color.DarkBlue);
            filtersList.Add("Resource", Color.DarkGray);
            filtersList.Add("Script", Color.DarkOrange);
            filtersList.Add("Simulation", Color.DarkSlateGray);
            filtersList.Add("SceneGraph", Color.DodgerBlue);
            filtersList.Add("DevMenu", Color.Firebrick);
            filtersList.Add("Audio", Color.Gold);
            filtersList.Add("Network", Color.Green);
            filtersList.Add("Memory", Color.IndianRed);
            filtersList.Add("Animation", Color.Sienna);
            filtersList.Add("Session", Color.Salmon);
            filtersList.Add("!WARNING!", Color.Red);

            // All all the filtering sub-menu items to the right-click menu
            ToolStripItem filteringToolStripItem = contextMenuStrip.Items.Find("filteringToolStripMenuItem", false)[0];
            foreach (KeyValuePair<string, Color> filter in filtersList)
            {
                log.AddToListOfAllFilters(filter.Key);

                ToolStripMenuItem toolStripItem = new ToolStripMenuItem();
                toolStripItem.Text = filter.Key;
                toolStripItem.ForeColor = filter.Value;
                toolStripItem.CheckOnClick = true;
                
                // By linking the event handler first and then setting the Checked property,
                // we cause the event to fire and add this to the log filter list for us too
                toolStripItem.CheckStateChanged += new EventHandler(toolStripItem_CheckStateChanged);
                toolStripItem.Checked = true;
                
                filteringToolStripMenuItem.DropDownItems.Add(toolStripItem);
            }
        }

        void toolStripItem_CheckStateChanged(object sender, EventArgs e)
        {
            ToolStripMenuItem menuItem = (ToolStripMenuItem)sender;

            if (menuItem.Checked)
                log.AddFilter(menuItem.Text, menuItem.ForeColor);
            else
                log.RemoveFilter(menuItem.Text);
        }

        #region Public Methods/Properties

        /// <summary>
        /// Specifies whether the log should be cleared when the Xbox 360 console reboots.
        /// </summary>
        public bool ClearWindowAfterReboot
        {
            get { return clearWindowAfterRebootToolStripMenuItem.Checked; }
            set { clearWindowAfterRebootToolStripMenuItem.Checked = value; }
        }
        
        /// <summary>
        /// Specifies whether log entries should be marked with the current time.
        /// </summary>
        public bool AddTimestamps
        {
            get { return addTimestampsToolStripMenuItem.Checked; }
            set { addTimestampsToolStripMenuItem.Checked = value; }
        }
        
        /// <summary>
        /// Specifies whether the log should scroll to show recent messages.
        /// </summary>
        public bool ScrollToRecentMessages
        {
            get { return scrollToRecentMessagesToolStripMenuItem.Checked; }
            set { scrollToRecentMessagesToolStripMenuItem.Checked = value; }
        }
        
        /// <summary>
        /// The current running process displayed in the status bar.
        /// </summary>
        public string RunningProcess
        {
            set { runningProcessLabel.Text = value; }
        }
        
        /// <summary>
        /// The current status icon displayed in the status bar.
        /// </summary>
        public StatusImage StatusImage
        {
            set
            {
                int index = (int)value;
                statusLabel.Image = statusImageList.Images[index];
                statusLabel.Text = string.Format("{0}...", value);
            }
        }
        
        /// <summary>
        /// The log control associated with this form.
        /// </summary>
        public Log Log { get { return log; } }

        public DebugMonitor Monitor { get { return monitor; } }
        
        #endregion

        #region Internal Methods/Properties

        /// <summary>
        /// Copies the specified RTF and plain text to the clipboard.
        /// </summary>
        private void CopyToClipboard(string rtf, string text)
        {
            Clipboard.Clear();

            DataObject data = new DataObject();
            bool empty = true;

            // Add RTF data.
            if (!string.IsNullOrEmpty(rtf))
            {
                data.SetText(rtf, TextDataFormat.Rtf);
                empty = false;
            }

            // Add plain-text data.
            if (!string.IsNullOrEmpty(text))
            {
                string newlinesFixed = text.Replace("\n", "\r\n");
                data.SetText(newlinesFixed, TextDataFormat.UnicodeText);
                empty = false;
            }

            if (!empty)            
                Clipboard.SetDataObject(data);
        }
        
        #endregion

        #region Event Handlers

        private void copySelectionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CopyToClipboard(log.SelectedRtf, log.SelectedText);
        }

        private void copyAllContentsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CopyToClipboard(log.Rtf, log.Text);
        }

        private void selectAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            log.SelectAll();
        }

        private void clearWindowToolStripMenuItem_Click(object sender, EventArgs e)
        {
            log.Clear();
        }

        private void scrollToRecentMessagesToolStripMenuItem_CheckedChanged(object sender, EventArgs e)
        {
            log.ScrollToRecentMessages = scrollToRecentMessagesToolStripMenuItem.Checked;
        }

        private void DebugMonitorForm_Load(object sender, EventArgs e)
        {
            monitor.Connect(consoleNameOrIP);
        }
        
        private void DebugMonitorForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (monitor.ConnectionState == ConnectionState.NotConnected)
                return;
        
            // Confirm action.
            if (e.CloseReason == CloseReason.UserClosing ||
                e.CloseReason == CloseReason.MdiFormClosing)
            {
                DialogResult result = MessageBox.Show(
                    "Are you sure you wish to disconnect from this console?",
                    Text,
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Question
                );
                
                if (result == DialogResult.No)
                    e.Cancel = true;
            }
        }

        private void DebugMonitorForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            monitor.Disconnect();
            monitor = null;
        }

        private void rebootToolStripSplitButton_ButtonClick(object sender, EventArgs e)
        {
            monitor.RebootConsole(false);
        }

        private void coldRebootToolStripMenuItem_Click(object sender, EventArgs e)
        {
            monitor.RebootConsole(true);
        }

        #endregion

        private void copySelectionToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            CopyToClipboard(log.SelectedRtf, log.SelectedText);
        }

        private void copyAllContentsToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            CopyToClipboard(log.Rtf, log.Text);
        }

        private void selectAllToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            log.SelectAll();
        }

        private void clearWindowToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            log.Clear();
        }

        private void saveAsToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.CheckPathExists = true;
            saveFileDialog.Title = Text;
            saveFileDialog.Filter = "Rich Text Format|*.rtf|Plain Text|*.txt";

            DialogResult dialogResult = saveFileDialog.ShowDialog();
            if (dialogResult == DialogResult.OK && saveFileDialog.FileName.Length > 0)
            {
                string fileName = saveFileDialog.FileName;
                string extension = fileName.Substring(fileName.LastIndexOf('.'));
                if (extension == ".txt")
                    log.SaveFile(saveFileDialog.FileName, RichTextBoxStreamType.PlainText);
                else
                    log.SaveFile(saveFileDialog.FileName, RichTextBoxStreamType.RichText);
            }
        }
    }
    
    public enum StatusImage
    {
        Disconnected,
        Connecting,
        Stopped,
        Running,
        Pending,
        Rebooting,
    }
}