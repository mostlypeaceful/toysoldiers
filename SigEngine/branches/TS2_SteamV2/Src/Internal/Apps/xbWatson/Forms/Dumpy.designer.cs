namespace Atg.Samples.xbWatson.Forms
{
    partial class Dumpy
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Dumpy));
            this.descTextBox = new System.Windows.Forms.TextBox();
            this.submitButton = new System.Windows.Forms.Button();
            this.openDMPFileDialog = new System.Windows.Forms.OpenFileDialog();
            this.statusLabel = new System.Windows.Forms.Label();
            this.fogbugzIDLabel = new System.Windows.Forms.Label();
            this.descLabel = new System.Windows.Forms.Label();
            this.fogbugzIDTextBox = new System.Windows.Forms.TextBox();
            this.saveMinidumpButton = new System.Windows.Forms.Button();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.SuspendLayout();
            // 
            // descTextBox
            // 
            this.descTextBox.Location = new System.Drawing.Point(12, 55);
            this.descTextBox.Multiline = true;
            this.descTextBox.Name = "descTextBox";
            this.descTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.descTextBox.Size = new System.Drawing.Size(444, 249);
            this.descTextBox.TabIndex = 0;
            this.descTextBox.TextChanged += new System.EventHandler(this.descTextBox_TextChanged);
            // 
            // submitButton
            // 
            this.submitButton.Location = new System.Drawing.Point(382, 350);
            this.submitButton.Name = "submitButton";
            this.submitButton.Size = new System.Drawing.Size(75, 23);
            this.submitButton.TabIndex = 2;
            this.submitButton.Text = "Submit";
            this.submitButton.UseVisualStyleBackColor = true;
            this.submitButton.Click += new System.EventHandler(this.submitButton_Click);
            // 
            // openDMPFileDialog
            // 
            this.openDMPFileDialog.Filter = "Dump Files|*.dmp";
            // 
            // statusLabel
            // 
            this.statusLabel.AutoSize = true;
            this.statusLabel.Location = new System.Drawing.Point(12, 355);
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(61, 13);
            this.statusLabel.TabIndex = 4;
            this.statusLabel.Text = "Status Text";
            // 
            // fogbugzIDLabel
            // 
            this.fogbugzIDLabel.AutoSize = true;
            this.fogbugzIDLabel.Location = new System.Drawing.Point(9, 12);
            this.fogbugzIDLabel.Name = "fogbugzIDLabel";
            this.fogbugzIDLabel.Size = new System.Drawing.Size(66, 13);
            this.fogbugzIDLabel.TabIndex = 5;
            this.fogbugzIDLabel.Text = "FogBugz ID:";
            // 
            // descLabel
            // 
            this.descLabel.AutoSize = true;
            this.descLabel.Location = new System.Drawing.Point(9, 39);
            this.descLabel.Name = "descLabel";
            this.descLabel.Size = new System.Drawing.Size(63, 13);
            this.descLabel.TabIndex = 6;
            this.descLabel.Text = "Description:";
            // 
            // fogbugzIDTextBox
            // 
            this.fogbugzIDTextBox.Location = new System.Drawing.Point(81, 9);
            this.fogbugzIDTextBox.Name = "fogbugzIDTextBox";
            this.fogbugzIDTextBox.Size = new System.Drawing.Size(50, 20);
            this.fogbugzIDTextBox.TabIndex = 7;
            this.fogbugzIDTextBox.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.fogbugzIDTextBox_KeyPress);
            // 
            // saveMinidumpButton
            // 
            this.saveMinidumpButton.Location = new System.Drawing.Point(313, 9);
            this.saveMinidumpButton.Name = "saveMinidumpButton";
            this.saveMinidumpButton.Size = new System.Drawing.Size(144, 24);
            this.saveMinidumpButton.TabIndex = 8;
            this.saveMinidumpButton.Text = "&Save Minidump Locally...";
            this.saveMinidumpButton.UseVisualStyleBackColor = true;
            this.saveMinidumpButton.Click += new System.EventHandler(this.saveMinidumpButton_Click);
            // 
            // saveFileDialog
            // 
            this.saveFileDialog.DefaultExt = "dmp";
            this.saveFileDialog.Filter = "Minidump (*.dmp)|*.dmp|All files (*.*)|*.*";
            this.saveFileDialog.Title = "Save Minidump";
            // 
            // Dumpy
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(469, 385);
            this.Controls.Add(this.saveMinidumpButton);
            this.Controls.Add(this.fogbugzIDTextBox);
            this.Controls.Add(this.descLabel);
            this.Controls.Add(this.fogbugzIDLabel);
            this.Controls.Add(this.statusLabel);
            this.Controls.Add(this.submitButton);
            this.Controls.Add(this.descTextBox);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "Dumpy";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Dumpy the Crash Dump Submitter";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Dumpy_FormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox descTextBox;
        private System.Windows.Forms.Button submitButton;
        private System.Windows.Forms.OpenFileDialog openDMPFileDialog;
        private System.Windows.Forms.Label statusLabel;
        private System.Windows.Forms.Label fogbugzIDLabel;
        private System.Windows.Forms.Label descLabel;
        private System.Windows.Forms.TextBox fogbugzIDTextBox;
        private System.Windows.Forms.Button saveMinidumpButton;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
    }
}

