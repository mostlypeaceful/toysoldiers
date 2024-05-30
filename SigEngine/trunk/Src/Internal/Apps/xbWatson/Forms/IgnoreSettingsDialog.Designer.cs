namespace Atg.Samples.xbWatson.Forms {
    partial class IgnoreSettingsDialog {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose (bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent () {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(IgnoreSettingsDialog));
            this.CloseButton = new System.Windows.Forms.Button();
            this.ignoredStringTextBox = new System.Windows.Forms.TextBox();
            this.deleteButton = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.ignoredStringsListBox = new System.Windows.Forms.ListBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // CloseButton
            // 
            this.CloseButton.Location = new System.Drawing.Point(171, 216);
            this.CloseButton.Name = "CloseButton";
            this.CloseButton.Size = new System.Drawing.Size(75, 23);
            this.CloseButton.TabIndex = 1;
            this.CloseButton.Text = "Close";
            this.CloseButton.UseVisualStyleBackColor = true;
            // 
            // ignoredStringTextBox
            // 
            this.ignoredStringTextBox.Location = new System.Drawing.Point(6, 19);
            this.ignoredStringTextBox.Name = "ignoredStringTextBox";
            this.ignoredStringTextBox.Size = new System.Drawing.Size(144, 20);
            this.ignoredStringTextBox.TabIndex = 2;
            // 
            // deleteButton
            // 
            this.deleteButton.Image = ((System.Drawing.Image)(resources.GetObject("deleteButton.Image")));
            this.deleteButton.Location = new System.Drawing.Point(217, 92);
            this.deleteButton.Name = "deleteButton";
            this.deleteButton.Size = new System.Drawing.Size(29, 23);
            this.deleteButton.TabIndex = 3;
            this.deleteButton.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.ignoredStringsListBox);
            this.groupBox1.Location = new System.Drawing.Point(12, 76);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(199, 134);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Ignored Strings:";
            // 
            // ignoredStringsListBox
            // 
            this.ignoredStringsListBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ignoredStringsListBox.FormattingEnabled = true;
            this.ignoredStringsListBox.Location = new System.Drawing.Point(3, 16);
            this.ignoredStringsListBox.Name = "ignoredStringsListBox";
            this.ignoredStringsListBox.Size = new System.Drawing.Size(193, 108);
            this.ignoredStringsListBox.TabIndex = 6;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.ignoredStringTextBox);
            this.groupBox2.Location = new System.Drawing.Point(12, 12);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(234, 58);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Ignore String:";
            // 
            // IgnoreSettingsDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(253, 251);
            this.ControlBox = false;
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.deleteButton);
            this.Controls.Add(this.CloseButton);
            this.Name = "IgnoreSettingsDialog";
            this.Text = "IgnoreSettingsDialog";
            this.groupBox1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button CloseButton;
        private System.Windows.Forms.TextBox ignoredStringTextBox;
        private System.Windows.Forms.Button deleteButton;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.ListBox ignoredStringsListBox;
        private System.Windows.Forms.GroupBox groupBox2;
    }
}