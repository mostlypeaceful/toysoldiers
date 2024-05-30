namespace CheckIn
{
    partial class CheckInForm
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CheckInForm));
            this.changelistDescTextBox = new System.Windows.Forms.TextBox();
            this.cancelButton = new System.Windows.Forms.Button();
            this.submitButton = new System.Windows.Forms.Button();
            this.ignoredButton = new System.Windows.Forms.Button();
            this.addedFilesCheckBox = new System.Windows.Forms.CheckBox();
            this.deletedFilesCheckBox = new System.Windows.Forms.CheckBox();
            this.statusLabel = new System.Windows.Forms.Label();
            this.changelistLabel = new System.Windows.Forms.Label();
            this.modifiedFilesCheckBox = new System.Windows.Forms.CheckBox();
            this.outerSplitContainer = new System.Windows.Forms.SplitContainer();
            this.topSplitContainer = new System.Windows.Forms.SplitContainer();
            this.bottomSplitContainer = new System.Windows.Forms.SplitContainer();
            this.recentMessagesButton = new System.Windows.Forms.Button();
            this.searchAdvancedLocationsCheckBox = new System.Windows.Forms.CheckBox();
            this.changesComboBox = new System.Windows.Forms.ComboBox();
            this.changesLabel = new System.Windows.Forms.Label();
            this.multipleChangelistsToolTip = new System.Windows.Forms.ToolTip(this.components);
            this.modifiedList = new CheckIn.SigListView();
            this.pathHeader = new System.Windows.Forms.ColumnHeader();
            this.addedList = new CheckIn.SigListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.deletedList = new CheckIn.SigListView();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.outerSplitContainer.Panel1.SuspendLayout();
            this.outerSplitContainer.Panel2.SuspendLayout();
            this.outerSplitContainer.SuspendLayout();
            this.topSplitContainer.Panel1.SuspendLayout();
            this.topSplitContainer.Panel2.SuspendLayout();
            this.topSplitContainer.SuspendLayout();
            this.bottomSplitContainer.Panel1.SuspendLayout();
            this.bottomSplitContainer.Panel2.SuspendLayout();
            this.bottomSplitContainer.SuspendLayout();
            this.SuspendLayout();
            // 
            // changelistDescTextBox
            // 
            this.changelistDescTextBox.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.changelistDescTextBox.Location = new System.Drawing.Point(6, 22);
            this.changelistDescTextBox.Multiline = true;
            this.changelistDescTextBox.Name = "changelistDescTextBox";
            this.changelistDescTextBox.Size = new System.Drawing.Size(921, 144);
            this.changelistDescTextBox.TabIndex = 1;
            // 
            // cancelButton
            // 
            this.cancelButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(866, 765);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 8;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // submitButton
            // 
            this.submitButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.submitButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.submitButton.Location = new System.Drawing.Point(785, 765);
            this.submitButton.Name = "submitButton";
            this.submitButton.Size = new System.Drawing.Size(75, 23);
            this.submitButton.TabIndex = 9;
            this.submitButton.Text = "Submit";
            this.submitButton.UseVisualStyleBackColor = true;
            this.submitButton.Click += new System.EventHandler(this.submitButton_Click);
            // 
            // ignoredButton
            // 
            this.ignoredButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.ignoredButton.Location = new System.Drawing.Point(12, 765);
            this.ignoredButton.Name = "ignoredButton";
            this.ignoredButton.Size = new System.Drawing.Size(75, 23);
            this.ignoredButton.TabIndex = 10;
            this.ignoredButton.Text = "Ignored...";
            this.ignoredButton.UseVisualStyleBackColor = true;
            this.ignoredButton.Click += new System.EventHandler(this.ignoredButton_Click);
            // 
            // addedFilesCheckBox
            // 
            this.addedFilesCheckBox.AutoSize = true;
            this.addedFilesCheckBox.Checked = true;
            this.addedFilesCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.addedFilesCheckBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.addedFilesCheckBox.Location = new System.Drawing.Point(6, 3);
            this.addedFilesCheckBox.Name = "addedFilesCheckBox";
            this.addedFilesCheckBox.Size = new System.Drawing.Size(105, 21);
            this.addedFilesCheckBox.TabIndex = 11;
            this.addedFilesCheckBox.Text = "Added Files:";
            this.addedFilesCheckBox.UseVisualStyleBackColor = true;
            this.addedFilesCheckBox.CheckedChanged += new System.EventHandler(this.addedFilesCheckBox_CheckedChanged);
            // 
            // deletedFilesCheckBox
            // 
            this.deletedFilesCheckBox.AutoSize = true;
            this.deletedFilesCheckBox.Checked = true;
            this.deletedFilesCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.deletedFilesCheckBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.deletedFilesCheckBox.Location = new System.Drawing.Point(3, 3);
            this.deletedFilesCheckBox.Name = "deletedFilesCheckBox";
            this.deletedFilesCheckBox.Size = new System.Drawing.Size(113, 21);
            this.deletedFilesCheckBox.TabIndex = 13;
            this.deletedFilesCheckBox.Text = "Deleted Files:";
            this.deletedFilesCheckBox.UseVisualStyleBackColor = true;
            this.deletedFilesCheckBox.CheckedChanged += new System.EventHandler(this.deletedFilesCheckBox_CheckedChanged);
            // 
            // statusLabel
            // 
            this.statusLabel.AutoSize = true;
            this.statusLabel.Location = new System.Drawing.Point(12, 9);
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(87, 13);
            this.statusLabel.TabIndex = 14;
            this.statusLabel.Text = "Placeholder Text";
            // 
            // changelistLabel
            // 
            this.changelistLabel.AutoSize = true;
            this.changelistLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.changelistLabel.Location = new System.Drawing.Point(3, 2);
            this.changelistLabel.Name = "changelistLabel";
            this.changelistLabel.Size = new System.Drawing.Size(153, 17);
            this.changelistLabel.TabIndex = 0;
            this.changelistLabel.Text = "Changelist Description:";
            // 
            // modifiedFilesCheckBox
            // 
            this.modifiedFilesCheckBox.AutoSize = true;
            this.modifiedFilesCheckBox.Checked = true;
            this.modifiedFilesCheckBox.CheckState = System.Windows.Forms.CheckState.Checked;
            this.modifiedFilesCheckBox.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.modifiedFilesCheckBox.Location = new System.Drawing.Point(6, 3);
            this.modifiedFilesCheckBox.Name = "modifiedFilesCheckBox";
            this.modifiedFilesCheckBox.Size = new System.Drawing.Size(117, 21);
            this.modifiedFilesCheckBox.TabIndex = 12;
            this.modifiedFilesCheckBox.Text = "Modified Files:";
            this.modifiedFilesCheckBox.UseVisualStyleBackColor = true;
            this.modifiedFilesCheckBox.CheckedChanged += new System.EventHandler(this.modifiedFilesCheckBox_CheckedChanged);
            // 
            // outerSplitContainer
            // 
            this.outerSplitContainer.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.outerSplitContainer.BackColor = System.Drawing.Color.Gray;
            this.outerSplitContainer.ForeColor = System.Drawing.SystemColors.ControlText;
            this.outerSplitContainer.Location = new System.Drawing.Point(12, 28);
            this.outerSplitContainer.Name = "outerSplitContainer";
            this.outerSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // outerSplitContainer.Panel1
            // 
            this.outerSplitContainer.Panel1.Controls.Add(this.topSplitContainer);
            // 
            // outerSplitContainer.Panel2
            // 
            this.outerSplitContainer.Panel2.Controls.Add(this.bottomSplitContainer);
            this.outerSplitContainer.Size = new System.Drawing.Size(930, 731);
            this.outerSplitContainer.SplitterDistance = 380;
            this.outerSplitContainer.SplitterWidth = 10;
            this.outerSplitContainer.TabIndex = 15;
            // 
            // topSplitContainer
            // 
            this.topSplitContainer.BackColor = System.Drawing.Color.DimGray;
            this.topSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.topSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.topSplitContainer.Name = "topSplitContainer";
            this.topSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // topSplitContainer.Panel1
            // 
            this.topSplitContainer.Panel1.BackColor = System.Drawing.SystemColors.Control;
            this.topSplitContainer.Panel1.Controls.Add(this.changelistDescTextBox);
            this.topSplitContainer.Panel1.Controls.Add(this.changelistLabel);
            // 
            // topSplitContainer.Panel2
            // 
            this.topSplitContainer.Panel2.BackColor = System.Drawing.SystemColors.Control;
            this.topSplitContainer.Panel2.Controls.Add(this.modifiedList);
            this.topSplitContainer.Panel2.Controls.Add(this.modifiedFilesCheckBox);
            this.topSplitContainer.Size = new System.Drawing.Size(930, 380);
            this.topSplitContainer.SplitterDistance = 169;
            this.topSplitContainer.SplitterWidth = 10;
            this.topSplitContainer.TabIndex = 0;
            // 
            // bottomSplitContainer
            // 
            this.bottomSplitContainer.BackColor = System.Drawing.Color.DarkGray;
            this.bottomSplitContainer.Dock = System.Windows.Forms.DockStyle.Fill;
            this.bottomSplitContainer.Location = new System.Drawing.Point(0, 0);
            this.bottomSplitContainer.Name = "bottomSplitContainer";
            this.bottomSplitContainer.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // bottomSplitContainer.Panel1
            // 
            this.bottomSplitContainer.Panel1.BackColor = System.Drawing.SystemColors.Control;
            this.bottomSplitContainer.Panel1.Controls.Add(this.addedList);
            this.bottomSplitContainer.Panel1.Controls.Add(this.addedFilesCheckBox);
            // 
            // bottomSplitContainer.Panel2
            // 
            this.bottomSplitContainer.Panel2.BackColor = System.Drawing.SystemColors.Control;
            this.bottomSplitContainer.Panel2.Controls.Add(this.deletedList);
            this.bottomSplitContainer.Panel2.Controls.Add(this.deletedFilesCheckBox);
            this.bottomSplitContainer.Size = new System.Drawing.Size(930, 341);
            this.bottomSplitContainer.SplitterDistance = 161;
            this.bottomSplitContainer.SplitterWidth = 10;
            this.bottomSplitContainer.TabIndex = 0;
            // 
            // recentMessagesButton
            // 
            this.recentMessagesButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.recentMessagesButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.recentMessagesButton.Location = new System.Drawing.Point(830, 9);
            this.recentMessagesButton.Name = "recentMessagesButton";
            this.recentMessagesButton.Size = new System.Drawing.Size(119, 23);
            this.recentMessagesButton.TabIndex = 16;
            this.recentMessagesButton.Text = "Recent Messages...";
            this.recentMessagesButton.UseVisualStyleBackColor = true;
            this.recentMessagesButton.Click += new System.EventHandler(this.recentMessagesButton_Click);
            // 
            // searchAdvancedLocationsCheckBox
            // 
            this.searchAdvancedLocationsCheckBox.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.searchAdvancedLocationsCheckBox.AutoSize = true;
            this.searchAdvancedLocationsCheckBox.Location = new System.Drawing.Point(663, 13);
            this.searchAdvancedLocationsCheckBox.Name = "searchAdvancedLocationsCheckBox";
            this.searchAdvancedLocationsCheckBox.Size = new System.Drawing.Size(161, 17);
            this.searchAdvancedLocationsCheckBox.TabIndex = 17;
            this.searchAdvancedLocationsCheckBox.Text = "Search Advanced Locations";
            this.searchAdvancedLocationsCheckBox.UseVisualStyleBackColor = true;
            // 
            // changesComboBox
            // 
            this.changesComboBox.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.changesComboBox.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.changesComboBox.FormattingEnabled = true;
            this.changesComboBox.Location = new System.Drawing.Point(416, 765);
            this.changesComboBox.Name = "changesComboBox";
            this.changesComboBox.Size = new System.Drawing.Size(121, 21);
            this.changesComboBox.TabIndex = 18;
            this.changesComboBox.SelectedIndexChanged += new System.EventHandler(this.changesComboBox_SelectedIndexChanged);
            // 
            // changesLabel
            // 
            this.changesLabel.Anchor = System.Windows.Forms.AnchorStyles.Bottom;
            this.changesLabel.AutoSize = true;
            this.changesLabel.Location = new System.Drawing.Point(351, 768);
            this.changesLabel.Name = "changesLabel";
            this.changesLabel.Size = new System.Drawing.Size(59, 13);
            this.changesLabel.TabIndex = 19;
            this.changesLabel.Text = "Changelist:";
            // 
            // multipleChangelistsToolTip
            // 
            this.multipleChangelistsToolTip.AutomaticDelay = 0;
            this.multipleChangelistsToolTip.AutoPopDelay = 0;
            this.multipleChangelistsToolTip.InitialDelay = 0;
            this.multipleChangelistsToolTip.IsBalloon = true;
            this.multipleChangelistsToolTip.ReshowDelay = 0;
            this.multipleChangelistsToolTip.ShowAlways = true;
            // 
            // modifiedList
            // 
            this.modifiedList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.modifiedList.CheckBoxes = true;
            this.modifiedList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.pathHeader});
            this.modifiedList.DoubleClickDoesCheck = false;
            this.modifiedList.Location = new System.Drawing.Point(6, 30);
            this.modifiedList.Name = "modifiedList";
            this.modifiedList.Size = new System.Drawing.Size(921, 168);
            this.modifiedList.TabIndex = 13;
            this.modifiedList.UseCompatibleStateImageBehavior = false;
            this.modifiedList.View = System.Windows.Forms.View.Details;
            this.modifiedList.DoubleClick += new System.EventHandler(this.modifiedList_DoubleClick);
            this.modifiedList.MouseDown += new System.Windows.Forms.MouseEventHandler(this.modifiedList_MouseDown);
            // 
            // pathHeader
            // 
            this.pathHeader.Text = "Path";
            this.pathHeader.Width = 917;
            // 
            // addedList
            // 
            this.addedList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.addedList.CheckBoxes = true;
            this.addedList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.addedList.DoubleClickDoesCheck = false;
            this.addedList.Location = new System.Drawing.Point(6, 30);
            this.addedList.Name = "addedList";
            this.addedList.Size = new System.Drawing.Size(921, 103);
            this.addedList.TabIndex = 14;
            this.addedList.UseCompatibleStateImageBehavior = false;
            this.addedList.View = System.Windows.Forms.View.Details;
            this.addedList.MouseDown += new System.Windows.Forms.MouseEventHandler(this.addedList_MouseDown);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Path";
            this.columnHeader1.Width = 917;
            // 
            // deletedList
            // 
            this.deletedList.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.deletedList.CheckBoxes = true;
            this.deletedList.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2});
            this.deletedList.DoubleClickDoesCheck = false;
            this.deletedList.Location = new System.Drawing.Point(3, 30);
            this.deletedList.Name = "deletedList";
            this.deletedList.Size = new System.Drawing.Size(921, 130);
            this.deletedList.TabIndex = 15;
            this.deletedList.UseCompatibleStateImageBehavior = false;
            this.deletedList.View = System.Windows.Forms.View.Details;
            this.deletedList.MouseDown += new System.Windows.Forms.MouseEventHandler(this.deletedList_MouseDown);
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "Path";
            this.columnHeader2.Width = 917;
            // 
            // CheckInForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(961, 798);
            this.Controls.Add(this.changesLabel);
            this.Controls.Add(this.changesComboBox);
            this.Controls.Add(this.searchAdvancedLocationsCheckBox);
            this.Controls.Add(this.recentMessagesButton);
            this.Controls.Add(this.outerSplitContainer);
            this.Controls.Add(this.statusLabel);
            this.Controls.Add(this.ignoredButton);
            this.Controls.Add(this.submitButton);
            this.Controls.Add(this.cancelButton);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.KeyPreview = true;
            this.MinimumSize = new System.Drawing.Size(800, 663);
            this.Name = "CheckInForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Check In";
            this.Load += new System.EventHandler(this.CheckInForm_Load);
            this.Shown += new System.EventHandler(this.CheckInForm_Shown);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.CheckInForm_FormClosing);
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.CheckInForm_KeyDown);
            this.outerSplitContainer.Panel1.ResumeLayout(false);
            this.outerSplitContainer.Panel2.ResumeLayout(false);
            this.outerSplitContainer.ResumeLayout(false);
            this.topSplitContainer.Panel1.ResumeLayout(false);
            this.topSplitContainer.Panel1.PerformLayout();
            this.topSplitContainer.Panel2.ResumeLayout(false);
            this.topSplitContainer.Panel2.PerformLayout();
            this.topSplitContainer.ResumeLayout(false);
            this.bottomSplitContainer.Panel1.ResumeLayout(false);
            this.bottomSplitContainer.Panel1.PerformLayout();
            this.bottomSplitContainer.Panel2.ResumeLayout(false);
            this.bottomSplitContainer.Panel2.PerformLayout();
            this.bottomSplitContainer.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox changelistDescTextBox;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button submitButton;
        private System.Windows.Forms.Button ignoredButton;
        private System.Windows.Forms.CheckBox addedFilesCheckBox;
        private System.Windows.Forms.CheckBox deletedFilesCheckBox;
        private System.Windows.Forms.Label statusLabel;
        private System.Windows.Forms.Label changelistLabel;
        private System.Windows.Forms.CheckBox modifiedFilesCheckBox;
        private System.Windows.Forms.SplitContainer outerSplitContainer;
        private System.Windows.Forms.SplitContainer topSplitContainer;
        private System.Windows.Forms.SplitContainer bottomSplitContainer;
        private SigListView modifiedList;
        private System.Windows.Forms.ColumnHeader pathHeader;
        private SigListView addedList;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private SigListView deletedList;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.Button recentMessagesButton;
        private System.Windows.Forms.CheckBox searchAdvancedLocationsCheckBox;
        private System.Windows.Forms.ComboBox changesComboBox;
        private System.Windows.Forms.Label changesLabel;
        private System.Windows.Forms.ToolTip multipleChangelistsToolTip;
    }
}

