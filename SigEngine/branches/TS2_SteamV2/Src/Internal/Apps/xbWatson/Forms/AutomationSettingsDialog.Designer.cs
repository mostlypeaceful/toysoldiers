namespace Atg.Samples.xbWatson.Forms
{
    partial class AutomationSettingsDialog
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
            System.Windows.Forms.Panel panel1;
            System.Windows.Forms.Label label1;
            System.Windows.Forms.Label label2;
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.actionGroupBox = new System.Windows.Forms.GroupBox();
            this.withHeapCheckBox = new System.Windows.Forms.CheckBox();
            this.saveMinidumpCheckBox = new System.Windows.Forms.CheckBox();
            this.rebootRadioButton = new System.Windows.Forms.RadioButton();
            this.breakRadioButton = new System.Windows.Forms.RadioButton();
            this.continueRadioButton = new System.Windows.Forms.RadioButton();
            this.promptRadioButton = new System.Windows.Forms.RadioButton();
            this.notificationsListBox = new System.Windows.Forms.ListBox();
            this.minidumpLocationTextBox = new System.Windows.Forms.TextBox();
            this.browseButton = new System.Windows.Forms.Button();
            this.folderBrowserDialog = new System.Windows.Forms.FolderBrowserDialog();
            panel1 = new System.Windows.Forms.Panel();
            label1 = new System.Windows.Forms.Label();
            label2 = new System.Windows.Forms.Label();
            panel1.SuspendLayout();
            this.actionGroupBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            panel1.Controls.Add(this.okButton);
            panel1.Controls.Add(this.cancelButton);
            panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            panel1.Location = new System.Drawing.Point(0, 262);
            panel1.Name = "panel1";
            panel1.Padding = new System.Windows.Forms.Padding(8);
            panel1.Size = new System.Drawing.Size(498, 48);
            panel1.TabIndex = 6;
            // 
            // okButton
            // 
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okButton.Location = new System.Drawing.Point(288, 12);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(96, 24);
            this.okButton.TabIndex = 0;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(390, 12);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(96, 24);
            this.cancelButton.TabIndex = 1;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(9, 20);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(96, 13);
            label1.TabIndex = 0;
            label1.Text = "&Notification Event:";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new System.Drawing.Point(12, 211);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(125, 13);
            label2.TabIndex = 3;
            label2.Text = "&Minidump Save Location:";
            // 
            // actionGroupBox
            // 
            this.actionGroupBox.Controls.Add(this.withHeapCheckBox);
            this.actionGroupBox.Controls.Add(this.saveMinidumpCheckBox);
            this.actionGroupBox.Controls.Add(this.rebootRadioButton);
            this.actionGroupBox.Controls.Add(this.breakRadioButton);
            this.actionGroupBox.Controls.Add(this.continueRadioButton);
            this.actionGroupBox.Controls.Add(this.promptRadioButton);
            this.actionGroupBox.Location = new System.Drawing.Point(312, 36);
            this.actionGroupBox.Name = "actionGroupBox";
            this.actionGroupBox.Size = new System.Drawing.Size(174, 160);
            this.actionGroupBox.TabIndex = 2;
            this.actionGroupBox.TabStop = false;
            this.actionGroupBox.Text = "&Action:";
            // 
            // withHeapCheckBox
            // 
            this.withHeapCheckBox.AutoSize = true;
            this.withHeapCheckBox.Location = new System.Drawing.Point(18, 136);
            this.withHeapCheckBox.Name = "withHeapCheckBox";
            this.withHeapCheckBox.Size = new System.Drawing.Size(66, 17);
            this.withHeapCheckBox.TabIndex = 5;
            this.withHeapCheckBox.Text = "w/ &Heap";
            this.withHeapCheckBox.UseVisualStyleBackColor = true;
            this.withHeapCheckBox.Click += new System.EventHandler(this.withHeapCheckBox_Click);
            // 
            // saveMinidumpCheckBox
            // 
            this.saveMinidumpCheckBox.AutoSize = true;
            this.saveMinidumpCheckBox.Location = new System.Drawing.Point(6, 115);
            this.saveMinidumpCheckBox.Name = "saveMinidumpCheckBox";
            this.saveMinidumpCheckBox.Size = new System.Drawing.Size(97, 17);
            this.saveMinidumpCheckBox.TabIndex = 4;
            this.saveMinidumpCheckBox.Text = "&Save Minidump";
            this.saveMinidumpCheckBox.UseVisualStyleBackColor = true;
            this.saveMinidumpCheckBox.Click += new System.EventHandler(this.saveMinidumpCheckBox_Click);
            this.saveMinidumpCheckBox.CheckedChanged += new System.EventHandler(this.saveMinidumpCheckBox_CheckedChanged);
            // 
            // rebootRadioButton
            // 
            this.rebootRadioButton.AutoSize = true;
            this.rebootRadioButton.Location = new System.Drawing.Point(6, 82);
            this.rebootRadioButton.Name = "rebootRadioButton";
            this.rebootRadioButton.Size = new System.Drawing.Size(60, 17);
            this.rebootRadioButton.TabIndex = 3;
            this.rebootRadioButton.TabStop = true;
            this.rebootRadioButton.Text = "&Reboot";
            this.rebootRadioButton.UseVisualStyleBackColor = true;
            this.rebootRadioButton.Click += new System.EventHandler(this.action_Click);
            this.rebootRadioButton.CheckedChanged += new System.EventHandler(this.action_CheckedChanged);
            // 
            // breakRadioButton
            // 
            this.breakRadioButton.AutoSize = true;
            this.breakRadioButton.Location = new System.Drawing.Point(6, 61);
            this.breakRadioButton.Name = "breakRadioButton";
            this.breakRadioButton.Size = new System.Drawing.Size(52, 17);
            this.breakRadioButton.TabIndex = 2;
            this.breakRadioButton.TabStop = true;
            this.breakRadioButton.Text = "&Break";
            this.breakRadioButton.UseVisualStyleBackColor = true;
            this.breakRadioButton.Click += new System.EventHandler(this.action_Click);
            this.breakRadioButton.CheckedChanged += new System.EventHandler(this.action_CheckedChanged);
            // 
            // continueRadioButton
            // 
            this.continueRadioButton.AutoSize = true;
            this.continueRadioButton.Location = new System.Drawing.Point(6, 40);
            this.continueRadioButton.Name = "continueRadioButton";
            this.continueRadioButton.Size = new System.Drawing.Size(68, 17);
            this.continueRadioButton.TabIndex = 1;
            this.continueRadioButton.TabStop = true;
            this.continueRadioButton.Text = "&Continue";
            this.continueRadioButton.UseVisualStyleBackColor = true;
            this.continueRadioButton.Click += new System.EventHandler(this.action_Click);
            this.continueRadioButton.CheckedChanged += new System.EventHandler(this.action_CheckedChanged);
            // 
            // promptRadioButton
            // 
            this.promptRadioButton.AutoSize = true;
            this.promptRadioButton.Location = new System.Drawing.Point(6, 19);
            this.promptRadioButton.Name = "promptRadioButton";
            this.promptRadioButton.Size = new System.Drawing.Size(59, 17);
            this.promptRadioButton.TabIndex = 0;
            this.promptRadioButton.TabStop = true;
            this.promptRadioButton.Text = "&Prompt";
            this.promptRadioButton.UseVisualStyleBackColor = true;
            this.promptRadioButton.Click += new System.EventHandler(this.action_Click);
            this.promptRadioButton.CheckedChanged += new System.EventHandler(this.action_CheckedChanged);
            // 
            // notificationsListBox
            // 
            this.notificationsListBox.FormattingEnabled = true;
            this.notificationsListBox.Location = new System.Drawing.Point(12, 36);
            this.notificationsListBox.Name = "notificationsListBox";
            this.notificationsListBox.Size = new System.Drawing.Size(294, 160);
            this.notificationsListBox.TabIndex = 1;
            this.notificationsListBox.SelectedIndexChanged += new System.EventHandler(this.notificationsListBox_SelectedIndexChanged);
            // 
            // minidumpLocationTextBox
            // 
            this.minidumpLocationTextBox.Location = new System.Drawing.Point(12, 227);
            this.minidumpLocationTextBox.Name = "minidumpLocationTextBox";
            this.minidumpLocationTextBox.Size = new System.Drawing.Size(441, 21);
            this.minidumpLocationTextBox.TabIndex = 4;
            // 
            // browseButton
            // 
            this.browseButton.Location = new System.Drawing.Point(453, 227);
            this.browseButton.Name = "browseButton";
            this.browseButton.Size = new System.Drawing.Size(33, 21);
            this.browseButton.TabIndex = 5;
            this.browseButton.Text = "…";
            this.browseButton.UseVisualStyleBackColor = true;
            this.browseButton.Click += new System.EventHandler(this.browseButton_Click);
            // 
            // AutomationSettingsDialog
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(498, 310);
            this.Controls.Add(this.browseButton);
            this.Controls.Add(this.minidumpLocationTextBox);
            this.Controls.Add(this.notificationsListBox);
            this.Controls.Add(label2);
            this.Controls.Add(label1);
            this.Controls.Add(this.actionGroupBox);
            this.Controls.Add(panel1);
            this.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AutomationSettingsDialog";
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Automation Settings";
            panel1.ResumeLayout(false);
            this.actionGroupBox.ResumeLayout(false);
            this.actionGroupBox.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.CheckBox withHeapCheckBox;
        private System.Windows.Forms.CheckBox saveMinidumpCheckBox;
        private System.Windows.Forms.RadioButton rebootRadioButton;
        private System.Windows.Forms.RadioButton breakRadioButton;
        private System.Windows.Forms.RadioButton continueRadioButton;
        private System.Windows.Forms.RadioButton promptRadioButton;
        private System.Windows.Forms.ListBox notificationsListBox;
        private System.Windows.Forms.TextBox minidumpLocationTextBox;
        private System.Windows.Forms.Button browseButton;
        private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog;
        private System.Windows.Forms.GroupBox actionGroupBox;
    }
}