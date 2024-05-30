namespace PlatformMonitor
{
    partial class AssertResponseDialog
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AssertResponseDialog));
            this.label1 = new System.Windows.Forms.Label();
            this.ContinueButton = new System.Windows.Forms.Button();
            this.AssertTextBox = new System.Windows.Forms.TextBox();
            this.BreakpointButton = new System.Windows.Forms.Button();
            this.CrashButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(77, 154);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(126, 20);
            this.label1.TabIndex = 0;
            this.label1.Text = "Choose Action";
            // 
            // ContinueButton
            // 
            this.ContinueButton.Location = new System.Drawing.Point(12, 177);
            this.ContinueButton.Name = "ContinueButton";
            this.ContinueButton.Size = new System.Drawing.Size(75, 23);
            this.ContinueButton.TabIndex = 1;
            this.ContinueButton.Text = "Continue";
            this.ContinueButton.UseVisualStyleBackColor = true;
            this.ContinueButton.Click += new System.EventHandler(this.ContinueButton_Click);
            // 
            // AssertTextBox
            // 
            this.AssertTextBox.BackColor = System.Drawing.SystemColors.ControlLightLight;
            this.AssertTextBox.Location = new System.Drawing.Point(12, 12);
            this.AssertTextBox.Multiline = true;
            this.AssertTextBox.Name = "AssertTextBox";
            this.AssertTextBox.ReadOnly = true;
            this.AssertTextBox.Size = new System.Drawing.Size(260, 139);
            this.AssertTextBox.TabIndex = 3;
            // 
            // BreakpointButton
            // 
            this.BreakpointButton.Location = new System.Drawing.Point(104, 177);
            this.BreakpointButton.Name = "BreakpointButton";
            this.BreakpointButton.Size = new System.Drawing.Size(75, 23);
            this.BreakpointButton.TabIndex = 4;
            this.BreakpointButton.Text = "Breakpoint";
            this.BreakpointButton.UseVisualStyleBackColor = true;
            this.BreakpointButton.Click += new System.EventHandler(this.BreakpointButton_Click);
            // 
            // CrashButton
            // 
            this.CrashButton.Location = new System.Drawing.Point(197, 177);
            this.CrashButton.Name = "CrashButton";
            this.CrashButton.Size = new System.Drawing.Size(75, 23);
            this.CrashButton.TabIndex = 6;
            this.CrashButton.Text = "Crash";
            this.CrashButton.UseVisualStyleBackColor = true;
            this.CrashButton.Click += new System.EventHandler(this.CrashButton_Click);
            // 
            // AssertResponseDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 214);
            this.ControlBox = false;
            this.Controls.Add(this.CrashButton);
            this.Controls.Add(this.BreakpointButton);
            this.Controls.Add(this.AssertTextBox);
            this.Controls.Add(this.ContinueButton);
            this.Controls.Add(this.label1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "AssertResponseDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Assert Hit";
            this.TopMost = true;
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button ContinueButton;
        private System.Windows.Forms.TextBox AssertTextBox;
        private System.Windows.Forms.Button BreakpointButton;
        private System.Windows.Forms.Button CrashButton;
    }
}