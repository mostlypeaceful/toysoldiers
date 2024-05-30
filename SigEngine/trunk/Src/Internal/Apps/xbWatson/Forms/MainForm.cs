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

            CurrentIgnoredStrings = new List<string>();

            // Load settings.
            ConnectToDefaultConsoleOnStartup =
                Program.GetSetting<bool>("ConnectToDefaultConsoleOnStartup", true);

            SaveHeapWithMinidump =
                Program.GetSetting<bool>( "SaveHeapWithMinidump", false );

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

        /// <summary>
        /// Specifies whether to save the heap with minidumps.
        /// </summary>
        public bool SaveHeapWithMinidump
        {
            get { return saveHeapWithMinidumpToolStripMenuItem.Checked; }
            set { saveHeapWithMinidumpToolStripMenuItem.Checked = value; }
        }

        /// <summary>
        /// Built from the IgnoreSettingsDialog, this is a list of strings that
        /// whenever a log message gets pumped through, if the log message contains
        /// any of these strings then it will be ignored and not outputted.
        /// </summary>
        public List<string> CurrentIgnoredStrings { get; set; }

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

        void UpdateIgnoredStrings (List<string> ignoredStrings) {
            CurrentIgnoredStrings = ignoredStrings;
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

            // Load the WatsonIgnore.txt file for the ignored log strings            
            if ( System.IO.File.Exists( Application.StartupPath + "\\WatsonIgnore.txt" ) )
            {
                System.IO.StreamReader ignoreFile = System.IO.File.OpenText( Application.StartupPath + "\\WatsonIgnore.txt" );

                for (; ; ) 
                {
                    string ignoreString = ignoreFile.ReadLine();

                    // Reached the end of the file: break out.
                    if (ignoreString == null)
                        break;

                    // Ignore empty strings
                    if (ignoreString.Length == 0)
                        continue;

                    // Add to ignore list
                    if (CurrentIgnoredStrings.Contains(ignoreString))
                        continue;

                    CurrentIgnoredStrings.Add(ignoreString);
                }

                ignoreFile.Close();
            }
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

        void ignoreLogSettingsToolStripMenuItem_Click (object sender, System.EventArgs e) {
            IgnoreSettingsDialog dialog = new IgnoreSettingsDialog();
            dialog.SetIgnoredStrings( CurrentIgnoredStrings );
            if (dialog.ShowDialog() == DialogResult.OK) {
                UpdateIgnoredStrings(dialog.GetIgnoredStrings());
                System.IO.StreamWriter ignoreFile = System.IO.File.CreateText( Application.StartupPath + "\\WatsonIgnore.txt" );
                foreach( var ignoreString in dialog.GetIgnoredStrings( ) )
                    ignoreFile.WriteLine( ignoreString );
                ignoreFile.Close( );
            }
        }

        private void aboutXbWatsonToolStripMenuItem_Click(object sender, EventArgs e)
        {
            AboutDialog dialog = new AboutDialog();
            dialog.ShowDialog();
        }

        #endregion        

        private void saveHeapWithMinidumpToolStripMenuItem_Click( object sender, EventArgs e )
        {
            Program.SetSetting<bool>( "SaveHeapWithMinidump", saveHeapWithMinidumpToolStripMenuItem.Checked );
        }

        private void connectToDefaultConsoleOnStartupToolStripMenuItem_Click( object sender, EventArgs e )
        {
            Program.SetSetting<bool>( "ConnectToDefaultConsoleOnStartup", connectToDefaultConsoleOnStartupToolStripMenuItem.Checked );
        }
    }
}