namespace CheckIn
{
    partial class IgnoredForm
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
            this.workspaceIgnoresFileLabel = new System.Windows.Forms.Label();
            this.cancelButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            this.addButton = new System.Windows.Forms.Button();
            this.workspaceIgnoreListBox = new System.Windows.Forms.ListBox();
            this.deleteButton = new System.Windows.Forms.Button();
            this.globalIgnoreListBox = new System.Windows.Forms.ListBox();
            this.globalIgnoresLabel = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // workspaceIgnoresFileLabel
            // 
            this.workspaceIgnoresFileLabel.AutoSize = true;
            this.workspaceIgnoresFileLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.workspaceIgnoresFileLabel.Location = new System.Drawing.Point(12, 218);
            this.workspaceIgnoresFileLabel.Name = "workspaceIgnoresFileLabel";
            this.workspaceIgnoresFileLabel.Size = new System.Drawing.Size(134, 17);
            this.workspaceIgnoresFileLabel.TabIndex = 0;
            this.workspaceIgnoresFileLabel.Text = "Workspace Ignores:";
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(697, 430);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 2;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(616, 430);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 3;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // addButton
            // 
            this.addButton.Enabled = false;
            this.addButton.Location = new System.Drawing.Point(12, 430);
            this.addButton.Name = "addButton";
            this.addButton.Size = new System.Drawing.Size(75, 23);
            this.addButton.TabIndex = 4;
            this.addButton.Text = "Add...";
            this.addButton.UseVisualStyleBackColor = true;
            this.addButton.Click += new System.EventHandler(this.addButton_Click);
            // 
            // workspaceIgnoreListBox
            // 
            this.workspaceIgnoreListBox.Enabled = false;
            this.workspaceIgnoreListBox.FormattingEnabled = true;
            this.workspaceIgnoreListBox.Location = new System.Drawing.Point(12, 238);
            this.workspaceIgnoreListBox.Name = "workspaceIgnoreListBox";
            this.workspaceIgnoreListBox.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.workspaceIgnoreListBox.Size = new System.Drawing.Size(760, 186);
            this.workspaceIgnoreListBox.TabIndex = 5;
            // 
            // deleteButton
            // 
            this.deleteButton.Enabled = false;
            this.deleteButton.Location = new System.Drawing.Point(93, 430);
            this.deleteButton.Name = "deleteButton";
            this.deleteButton.Size = new System.Drawing.Size(75, 23);
            this.deleteButton.TabIndex = 6;
            this.deleteButton.Text = "Delete";
            this.deleteButton.UseVisualStyleBackColor = true;
            this.deleteButton.Click += new System.EventHandler(this.deleteButton_Click);
            // 
            // globalIgnoreListBox
            // 
            this.globalIgnoreListBox.BackColor = System.Drawing.SystemColors.ControlDark;
            this.globalIgnoreListBox.ForeColor = System.Drawing.SystemColors.InactiveCaptionText;
            this.globalIgnoreListBox.FormattingEnabled = true;
            this.globalIgnoreListBox.Location = new System.Drawing.Point(12, 29);
            this.globalIgnoreListBox.Name = "globalIgnoreListBox";
            this.globalIgnoreListBox.SelectionMode = System.Windows.Forms.SelectionMode.None;
            this.globalIgnoreListBox.Size = new System.Drawing.Size(760, 186);
            this.globalIgnoreListBox.TabIndex = 8;
            // 
            // globalIgnoresLabel
            // 
            this.globalIgnoresLabel.AutoSize = true;
            this.globalIgnoresLabel.Font = new System.Drawing.Font("Microsoft Sans Serif", 10F);
            this.globalIgnoresLabel.Location = new System.Drawing.Point(12, 9);
            this.globalIgnoresLabel.Name = "globalIgnoresLabel";
            this.globalIgnoresLabel.Size = new System.Drawing.Size(104, 17);
            this.globalIgnoresLabel.TabIndex = 7;
            this.globalIgnoresLabel.Text = "Global Ignores:";
            // 
            // IgnoredForm
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(784, 463);
            this.Controls.Add(this.globalIgnoreListBox);
            this.Controls.Add(this.globalIgnoresLabel);
            this.Controls.Add(this.deleteButton);
            this.Controls.Add(this.workspaceIgnoreListBox);
            this.Controls.Add(this.addButton);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.workspaceIgnoresFileLabel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "IgnoredForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Ignored Files";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label workspaceIgnoresFileLabel;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button addButton;
        private System.Windows.Forms.ListBox workspaceIgnoreListBox;
        private System.Windows.Forms.Button deleteButton;
        private System.Windows.Forms.ListBox globalIgnoreListBox;
        private System.Windows.Forms.Label globalIgnoresLabel;
    }
}