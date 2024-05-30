#region File Information
//-----------------------------------------------------------------------------
// AutomationSettingsDialog.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Windows.Forms;
using XDevkit;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class AutomationSettingsDialog : Form
    {
        public AutomationSettingsDialog()
        {
            InitializeComponent();
            
            // Load settings.
            XboxDebugEventType[] eventTypes = new XboxDebugEventType[]
            {
                XboxDebugEventType.AssertionFailed,
                XboxDebugEventType.Exception,
                XboxDebugEventType.RIP,
            };
            foreach (XboxDebugEventType eventType in eventTypes)
            {
                AutomationSetting setting = new AutomationSetting(eventType);
                setting = Program.GetSetting<AutomationSetting>(eventType, setting);
                notificationsListBox.Items.Add(setting);
            }
            notificationsListBox.SelectedIndex = 0;

            MinidumpSaveLocation = Program.GetSetting<string>("MinidumpSaveLocation", string.Empty);
        }

        #region Internal Methods\Properties
        
        /// <summary>
        /// The folder in which to save automated minidumps.
        /// </summary>
        private string MinidumpSaveLocation
        {
            get { return minidumpLocationTextBox.Text; }
            set { minidumpLocationTextBox.Text = value; }
        }

        #endregion

        #region Event Handlers

        private void notificationsListBox_SelectedIndexChanged(object sender, EventArgs e)
        {
            AutomationSetting selected = notificationsListBox.SelectedItem as AutomationSetting;
            if (selected == null)
            {
                actionGroupBox.Enabled = false;
                return;
            }

            actionGroupBox.Enabled = true;
            
            switch (selected.Action)
            {
                case AutomationAction.Prompt: promptRadioButton.Checked = true; break;
                case AutomationAction.Continue: continueRadioButton.Checked = true; break;
                case AutomationAction.Break: breakRadioButton.Checked = true; break;
                case AutomationAction.Reboot: rebootRadioButton.Checked = true; break;
            }

            saveMinidumpCheckBox.Checked = selected.SaveMinidump;
            withHeapCheckBox.Checked = selected.WithHeap;
        }

        private void action_CheckedChanged(object sender, EventArgs e)
        {
            if (promptRadioButton.Checked)
            {
                saveMinidumpCheckBox.Enabled = false;
                withHeapCheckBox.Enabled = false;
            }
            else
            {
                saveMinidumpCheckBox.Enabled = true;
                withHeapCheckBox.Enabled = saveMinidumpCheckBox.Checked;
            }
        }

        private void action_Click(object sender, EventArgs e)
        {
            AutomationSetting selected = notificationsListBox.SelectedItem as AutomationSetting;
            if (selected == null)
                return;

            if (promptRadioButton.Checked) selected.Action = AutomationAction.Prompt;
            if (continueRadioButton.Checked) selected.Action = AutomationAction.Continue;
            if (breakRadioButton.Checked) selected.Action = AutomationAction.Break;
            if (rebootRadioButton.Checked) selected.Action = AutomationAction.Reboot;
        }

        private void saveMinidumpCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            withHeapCheckBox.Enabled = saveMinidumpCheckBox.Checked;
        }

        private void saveMinidumpCheckBox_Click(object sender, EventArgs e)
        {
            AutomationSetting selected = notificationsListBox.SelectedItem as AutomationSetting;
            if (selected == null)
                return;

            selected.SaveMinidump = saveMinidumpCheckBox.Checked;
        }

        private void withHeapCheckBox_Click(object sender, EventArgs e)
        {
            AutomationSetting selected = notificationsListBox.SelectedItem as AutomationSetting;
            if (selected == null)
                return;
            
            selected.WithHeap = withHeapCheckBox.Checked;
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            // Save settings.
            foreach (object o in notificationsListBox.Items)
            {
                AutomationSetting setting = o as AutomationSetting;
                if (setting != null)
                {
                    Program.SetSetting<AutomationSetting>(setting.EventType, setting);
                }
            }
            
            Program.SetSetting<string>("MinidumpSaveLocation", MinidumpSaveLocation);
        }

        private void browseButton_Click(object sender, EventArgs e)
        {
            folderBrowserDialog.SelectedPath = MinidumpSaveLocation;
            DialogResult result = folderBrowserDialog.ShowDialog(this);
            
            if (result == DialogResult.OK)
            {
                MinidumpSaveLocation = folderBrowserDialog.SelectedPath;
            }
        }

        #endregion
    }
}