#region File Information
//-----------------------------------------------------------------------------
// ConnectionDialog.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Net;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Windows.Forms;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class ConnectionDialog : Form
    {
        private delegate void VoidDelegate();
                
        public ConnectionDialog()
        {
            InitializeComponent();

            // Load settings.
            AddConsolesToHistory = Program.GetSetting<bool>("AddConsolesToHistory", false);
            IgnoreIPAddresses = Program.GetSetting<bool>("IgnoreIPAddresses", false);
            
            connectButton.Enabled = false;
            removeSelectedButton.Enabled = false;
        }

        #region Public Methods/Properties

        /// <summary>
        /// Specifies whether new consoles should be remembered.
        /// </summary>
        public bool AddConsolesToHistory
        {
            get { return addConsolesToHistoryCheckBox.Checked; }
            set { addConsolesToHistoryCheckBox.Checked = value; }
        }
        
        /// <summary>
        /// Specifies whether IP addresses should be added to the history.
        /// </summary>
        public bool IgnoreIPAddresses
        {
            get { return ignoreIPAddressesCheckBox.Checked; }
            set { ignoreIPAddressesCheckBox.Checked = value; }
        }
        
        /// <summary>
        /// The consoles selected by the user.
        /// </summary>
        public List<string> SelectedConsoles
        {
            get { return MergeLists<string>(ConsolesFromTextBox, ConsolesFromListBox); }
        }

        /// <summary>
        /// Determines whether the specified input is a well-formed console name or IP address.
        /// NOTE: This does not check whether the name or IP address is accessible.
        /// </summary>
        public static bool IsValidNameOrIP(string consoleNameOrIP)
        {
            string trimmed = consoleNameOrIP.Trim();

            // Is this a well-formed IP address?
            if (IsValidIP(trimmed))
                return true;

            // Validate each character in the name.
            string validSymbols = "-_";
            foreach (char ch in trimmed)
            {
                if (char.IsLetterOrDigit(ch))
                    continue;

                if (validSymbols.IndexOf(ch) == -1)
                    return false;
            }

            return true;
        }
        
        #endregion

        #region Internal Methods/Properties
        
        private struct ConsoleInList
        {
            public string NameOrIP;
            public bool IsInNeighborhood;
            public bool IsDefault;
            
            public ConsoleInList(string nameOrIP, bool isInNeighborhood, bool isDefault)
            {
                NameOrIP = nameOrIP;
                IsInNeighborhood = isInNeighborhood;
                IsDefault = isDefault;
            }

            public override string ToString()
            {
                return NameOrIP;
            }
        }

        /// <summary>
        /// Adds entries to the registry.
        /// </summary>
        private static void AddHistory(List<string> consolesToAdd)
        {
            List<string> consoles = MergeLists<string>(ConsolesFromHistory, consolesToAdd);
            Program.SetSetting<List<string>>("ConsoleHistory", consoles);
        }

        /// <summary>
        /// Remove entries from the registry.
        /// </summary>
        private static void RemoveHistory(List<string> consolesToRemove)
        {
            List<string> consoles = ConsolesFromHistory;
            foreach (string console in consolesToRemove)
            {
                if (consoles.Contains(console))
                    consoles.Remove(console);
            }
            Program.SetSetting<List<string>>("ConsoleHistory", consoles);
        }
        
        /// <summary>
        /// Consoles stored in the registry.
        /// </summary>
        private static List<string> ConsolesFromHistory
        {
            get
            {
                List<string> empty = new List<string>();
                return Program.GetSetting<List<string>>("ConsoleHistory", empty);
            }
        }
        
        /// <summary>
        /// Consoles registered with Xbox 360 Neighborhood.
        /// </summary>
        private static List<string> ConsolesFromXbox360Neighborhood
        {
            get
            {
                List<string> consoles = new List<string>();
                foreach (object o in DebugMonitor.XboxManager.Consoles)
                {
                    string console = o as string;
                    if (!string.IsNullOrEmpty(console))
                        consoles.Add(console);
                }
                
                return consoles;
            }
        }
        
        /// <summary>
        /// Consoles selected in the list box.
        /// </summary>
        private List<string> ConsolesFromListBox
        {
            get
            {
                List<string> consoles = new List<string>();
                
                // Traverse selected items in the list.
                foreach (ConsoleInList item in consoleListBox.SelectedItems)
                    consoles.Add(item.NameOrIP);
                
                return consoles;
            }
        }
        
        /// <summary>
        /// Consoles entered in the text box.
        /// </summary>
        private List<string> ConsolesFromTextBox
        {
            get
            {
                List<string> consoles = new List<string>();
                
                // Parse the entries, cleaning up white-space.
                string[] entries = consoleTextBox.Text.Split(",;".ToCharArray());
                foreach (string entry in entries)
                {
                    string trimmed = entry.Trim();
                    if (!string.IsNullOrEmpty(trimmed))
                        consoles.Add(trimmed);
                }
                
                return consoles;
            }
        }

        /// <summary>
        /// Determines whether the specified input is a well-formed IP address.
        /// NOTE: This does not check whether the IP address is accessible.
        /// </summary>
        private static bool IsValidIP(string consoleNameOrIP)
        {
            string trimmed = consoleNameOrIP.Trim();
            
            IPAddress address;
            return IPAddress.TryParse(trimmed, out address);
        }
        
        /// <summary>
        /// Combines two arrays, removing duplicate entries.
        /// </summary>
        private static List<T> MergeLists<T>(List<T> list1, List<T> list2)
        {
            List<T> both = new List<T>();
            both.AddRange(list1);
            both.AddRange(list2);
            
            List<T> merged = new List<T>();
            foreach (T item in both)
            {
                if (!merged.Contains(item))
                    merged.Add(item);
            }
            
            return merged;
        }
        
        /// <summary>
        /// Generates the list of consoles (from history and Xbox 360 Neighborhood).
        /// </summary>
        private void PopulateList()
        {
            lock (consoleListBox)
            {
                consoleListBox.UseWaitCursor = true;
                consoleListBox.Enabled = false;
                consoleListBox.Items.Clear();
                consoleListBox.Items.Add("Refreshing list...");
                
                // Spawn a worker thread.
                new Thread(new ThreadStart(PopulateListThread)).Start();
            }
        }

        /// <summary>
        /// Worker thread for populating the list of consoles.
        /// </summary>
        private void PopulateListThread()
        {
            lock (consoleListBox)
            {
                // Compile the list of consoles.
                List<string> consolesFromXbox360Neighborhood = ConsolesFromXbox360Neighborhood;
                List<string> consolesFromHistory = ConsolesFromHistory;
                List<string> consoles = MergeLists<string>(consolesFromXbox360Neighborhood, consolesFromHistory);

                // Get default console.
                string defaultConsole = string.Empty;
                try { defaultConsole = DebugMonitor.XboxManager.DefaultConsole; }
                catch (COMException) { }

                // Update the list box.
                try
                {
                    consoleListBox.BeginInvoke(new VoidDelegate(delegate
                    {
                        consoleListBox.Items.Clear();
                        if (consoles.Count > 0)
                        {
                            foreach (string console in consoles)
                            {
                                // Add annotated item to list.
                                ConsoleInList item = new ConsoleInList(console,
                                    consolesFromXbox360Neighborhood.Contains(console),
                                    console == defaultConsole);
                                consoleListBox.Items.Add(item);
                            }
                            consoleListBox.Enabled = true;
                        }
                        else
                        {
                            consoleListBox.Items.Add("No consoles found.");
                        }

                        connectButton.Enabled = (SelectedConsoles.Count > 0);
                        removeSelectedButton.Enabled = (consoleListBox.SelectedIndex != -1);
                        consoleListBox.UseWaitCursor = false;
                    }));
                }
                catch (InvalidOperationException) { }
            }
        }
        
        #endregion
        
        #region Event Handlers

        private void consoleTextBox_Validating(object sender, CancelEventArgs e)
        {
            // Check for invalid entries.
            List<string> invalid = new List<string>();
            foreach (string console in ConsolesFromTextBox)
            {
                if (!IsValidNameOrIP(console))
                    invalid.Add(console);
            }
            if (invalid.Count > 0)
            {
                e.Cancel = true;
            
                // Warn user of invalid entries.
                StringBuilder message = new StringBuilder();
                message.Append("One or more entries are not valid console names or IP addresses:\n");
                foreach (string console in invalid)
                    message.AppendFormat("\n\t{0}", console);
                
                MessageBox.Show(
                    message.ToString(),
                    this.Text,
                    MessageBoxButtons.OK,
                    MessageBoxIcon.Warning
                );
            }
        }

        private void consoleTextBox_TextChanged(object sender, EventArgs e)
        {
            // Combine multiple lines, if any, into a single line.
            if (consoleTextBox.Lines.Length > 1)
            {
                StringBuilder combined = new StringBuilder();
                foreach (string line in consoleTextBox.Lines)
                {
                    string trimmed = line.Trim().Trim(",;".ToCharArray());
                    if (string.IsNullOrEmpty(trimmed))
                        continue;

                    if (combined.Length > 0)
                        combined.Append(";");
                    
                    combined.Append(trimmed);
                }
                
                consoleTextBox.Text = combined.ToString();
            }

            connectButton.Enabled = (SelectedConsoles.Count > 0);
        }

        private void consoleListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            connectButton.Enabled = (SelectedConsoles.Count > 0);
            removeSelectedButton.Enabled = (consoleListBox.SelectedIndex != -1);
        }

        private void addConsolesToHistoryCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            ignoreIPAddressesCheckBox.Enabled = AddConsolesToHistory;
        }

        private void ConnectionDialog_Load(object sender, EventArgs e)
        {
            PopulateList();
        }

        private void removeSelectedButton_Click(object sender, EventArgs e)
        {
            // Confirm action.
            DialogResult result = MessageBox.Show(
                "The selected entries will be removed from the list.\n" +
                "Do you want to continue?\n\n" +
                "NOTE: Consoles listed in Xbox 360 Neighborhood will not be affected.",
                removeSelectedButton.Text,
                MessageBoxButtons.YesNo,
                MessageBoxIcon.Question
            );
            
            if (result == DialogResult.Yes)
            {
                RemoveHistory(ConsolesFromListBox);
                PopulateList();
            }
        }

        private void connectButton_Click(object sender, EventArgs e)
        {
            if (AddConsolesToHistory)
            {
                // Update history.
                List<string> consoles = new List<string>();

                if (IgnoreIPAddresses)
                {
                    // Add only console names.
                    foreach (string console in SelectedConsoles)
                    {
                        if (!IsValidIP(console))
                            consoles.Add(console);
                    }
                }
                else
                {
                    // Add everything.
                    consoles.AddRange(SelectedConsoles);
                }
                
                AddHistory(consoles);
            }
            
            // Save settings.
            Program.SetSetting<bool>("AddConsolesToHistory", AddConsolesToHistory);
            Program.SetSetting<bool>("IgnoreIPAddresses", IgnoreIPAddresses);
        }

        private void consoleListBox_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            // Convert double-click into accepting dialog.
            if (connectButton.Enabled)
            {
                connectButton_Click(sender, EventArgs.Empty);
                
                DialogResult = connectButton.DialogResult;
                Hide();
            }
        }

        private void consoleListBox_DrawItem(object sender, DrawItemEventArgs e)
        {
            SolidBrush brush = new SolidBrush(e.ForeColor);
            Image icon = xboxIcon.Image;
            Font font = e.Font;
        
            object o = consoleListBox.Items[e.Index];
            if (o is string)
            {
                // Handle simple text item.
                string text = (string)o;

                e.DrawBackground();
                e.Graphics.DrawString(text, font, brush,
                    e.Bounds.Left + e.Bounds.Height, e.Bounds.Top);
            
                return;
            }
            else if (o is ConsoleInList)
            {
                // Handle console with annotations.
                ConsoleInList item = (ConsoleInList)o;

                e.DrawBackground();
                
                if (item.IsInNeighborhood)
                {
                    Rectangle rect = e.Bounds;
                    rect.Width = rect.Height;
                    e.Graphics.DrawImage(icon, rect);
                }
                if (item.IsDefault)
                {
                    font = new Font(font, FontStyle.Bold);
                }
                
                e.Graphics.DrawString(item.NameOrIP, font, brush,
                    e.Bounds.Left + e.Bounds.Height, e.Bounds.Top);
            }
        }

        private void consoleListBox_MeasureItem(object sender, MeasureItemEventArgs e)
        {
            ConsoleInList item = (ConsoleInList)consoleListBox.Items[e.Index];
            Font font = new Font(consoleListBox.Font, FontStyle.Bold);
            SizeF size = e.Graphics.MeasureString(item.NameOrIP, font);
            e.ItemHeight = consoleListBox.ItemHeight;
            e.ItemWidth = (int)(consoleListBox.ItemHeight + size.Width);
        }

        #endregion
    }
}