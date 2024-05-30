#region File Information
//-----------------------------------------------------------------------------
// MainForm.cs
//
// XNA Developer Connection
// Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#endregion

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class MainForm : Form
    {
        private List<string> consolesToLoadOnStartup;
        
        public MainForm(List<string> consolesToLoadOnStartup)
        {
            InitializeComponent();

            // Load settings.
            ConnectToDefaultConsoleOnStartup =
                Program.GetSetting<bool>("ConnectToDefaultConsoleOnStartup", true);

            this.consolesToLoadOnStartup = consolesToLoadOnStartup;
        }

        #region Public Methods/Properties

        /// <summary>
        /// Specifies whether to connect to the default console upon startup.
        /// </summary>
        public bool ConnectToDefaultConsoleOnStartup
        {
            get { return connectToDefaultConsoleOnStartupToolStripMenuItem.Checked; }
            set { connectToDefaultConsoleOnStartupToolStripMenuItem.Checked = value; }
        }

        #endregion
        
        #region Internal Methods/Properties
        
        /// <summary>
        /// Open a debug monitor form for the specified console.
        /// </summary>
        private void ConnectToConsole(string consoleNameOrIP)
        {
            DebugMonitorForm form = new DebugMonitorForm(consoleNameOrIP);
            form.MdiParent = this;
            form.Show();
        }
        
        #endregion
        
        #region Event Handlers

        private void MainForm_Load(object sender, EventArgs e)
        {
            if (consolesToLoadOnStartup != null && consolesToLoadOnStartup.Count > 0)
            {
                // Connect to consoles specified on the command-line.
                foreach (string console in consolesToLoadOnStartup)
                    ConnectToConsole(console);
            }
            else
            {
                // Connect to default console.
                if (ConnectToDefaultConsoleOnStartup)
                {
                    try
                    {
                        string console = DebugMonitor.XboxManager.DefaultConsole;
                        ConnectToConsole(console);
                    }
                    catch (COMException) { }
                }
            }            
        }

        private void MainForm_FormClosed(object sender, FormClosedEventArgs e)
        {
            // Save settings.
            Program.SetSetting<bool>("ConnectToDefaultConsoleOnStartup", ConnectToDefaultConsoleOnStartup);
        }

        private void connectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ConnectionDialog dialog = new ConnectionDialog();
            DialogResult result = dialog.ShowDialog();
            
            if (result == DialogResult.OK)
            {
                foreach (string console in dialog.SelectedConsoles)
                {
                    DebugMonitorForm form = new DebugMonitorForm(console);
                    form.MdiParent = this;
                    form.Show();
                }
            }
        }

        private void tileHorizontalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.TileHorizontal);
        }

        private void tileVerticalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.TileVertical);
        }

        private void cascadeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.Cascade);
        }

        private void closeAllWindowsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            foreach (Form child in MdiChildren)
                child.Close();
        }

        private void automationSettingsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AutomationSettingsDialog dialog = new AutomationSettingsDialog();
            dialog.ShowDialog();
        }

        private void aboutXbWatsonToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutDialog dialog = new AboutDialog();
            dialog.ShowDialog();
        }

        #endregion        
    }
}