using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Atg.Samples.xbWatson.Forms {
    public partial class IgnoreSettingsDialog : Form {
        public IgnoreSettingsDialog () {
            InitializeComponent();

            deleteButton.Click += new EventHandler(deleteButton_Click);
            CloseButton.Click += new EventHandler(CloseButton_Click);
            ignoredStringTextBox.KeyDown += new KeyEventHandler(ignoredStringTextBox_KeyDown);
        }

        void CloseButton_Click (object sender, EventArgs e) {
            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        void deleteButton_Click (object sender, EventArgs e) {
            if (ignoredStringsListBox.Items.Count > 0) {
                int selectedIndex = ignoredStringsListBox.SelectedIndex;
                if (selectedIndex >= 0) {
                    ignoredStringsListBox.Items.RemoveAt(selectedIndex);

                    if (ignoredStringsListBox.Items.Count > 0) {
                        selectedIndex = (selectedIndex > 0) ? selectedIndex - 1  : 0;
                        ignoredStringsListBox.SelectedIndex = selectedIndex;
                    }
                }
            }
        }

        void ignoredStringTextBox_KeyDown (object sender, KeyEventArgs e) {
            if (e.KeyCode == Keys.Enter || e.KeyCode == Keys.Return) {
                if (string.IsNullOrEmpty(ignoredStringTextBox.Text))
                    return;

                ignoredStringsListBox.Items.Add(ignoredStringTextBox.Text);
                ignoredStringTextBox.Text = string.Empty;
            }
        }

        public void SetIgnoredStrings ( List<string> ignoreList )
        {
            ignoredStringsListBox.Items.Clear( );
            ignoredStringsListBox.Items.AddRange( ignoreList.ToArray( ) );
        }

        public List<string> GetIgnoredStrings () {
            List<string> ignored = new List<string>();

            foreach( var entry in ignoredStringsListBox.Items )
                ignored.Add(entry.ToString());

            return ignored;
        }
    }
}
