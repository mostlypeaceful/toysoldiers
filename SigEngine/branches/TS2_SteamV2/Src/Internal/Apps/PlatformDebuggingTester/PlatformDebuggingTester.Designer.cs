namespace PlatformDebuggingClientControlPanel
{
    partial class PlatformDebuggingTester
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose( bool disposing )
        {
            if( disposing && ( components != null ) )
            {
                components.Dispose( );
            }
            base.Dispose( disposing );
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent( )
        {
            this.stringLabel = new System.Windows.Forms.Label();
            this.stringTextBox = new System.Windows.Forms.TextBox();
            this.sendButton = new System.Windows.Forms.Button();
            this.reconnectButton = new System.Windows.Forms.Button();
            this.serverIPTextBox = new System.Windows.Forms.TextBox();
            this.serverIPLabel = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // stringLabel
            // 
            this.stringLabel.AutoSize = true;
            this.stringLabel.Location = new System.Drawing.Point(30, 13);
            this.stringLabel.Name = "stringLabel";
            this.stringLabel.Size = new System.Drawing.Size(37, 13);
            this.stringLabel.TabIndex = 0;
            this.stringLabel.Text = "String:";
            // 
            // stringTextBox
            // 
            this.stringTextBox.Location = new System.Drawing.Point(73, 10);
            this.stringTextBox.Name = "stringTextBox";
            this.stringTextBox.Size = new System.Drawing.Size(199, 20);
            this.stringTextBox.TabIndex = 1;
            // 
            // sendButton
            // 
            this.sendButton.Location = new System.Drawing.Point(197, 67);
            this.sendButton.Name = "sendButton";
            this.sendButton.Size = new System.Drawing.Size(75, 23);
            this.sendButton.TabIndex = 2;
            this.sendButton.Text = "Send";
            this.sendButton.UseVisualStyleBackColor = true;
            this.sendButton.Click += new System.EventHandler(this.sendButton_Click);
            // 
            // reconnectButton
            // 
            this.reconnectButton.Location = new System.Drawing.Point(12, 67);
            this.reconnectButton.Name = "reconnectButton";
            this.reconnectButton.Size = new System.Drawing.Size(75, 23);
            this.reconnectButton.TabIndex = 3;
            this.reconnectButton.Text = "Reconnect";
            this.reconnectButton.UseVisualStyleBackColor = true;
            this.reconnectButton.Click += new System.EventHandler(this.reconnectButton_Click);
            // 
            // serverIPTextBox
            // 
            this.serverIPTextBox.Location = new System.Drawing.Point(73, 35);
            this.serverIPTextBox.Name = "serverIPTextBox";
            this.serverIPTextBox.Size = new System.Drawing.Size(199, 20);
            this.serverIPTextBox.TabIndex = 5;
            // 
            // serverIPLabel
            // 
            this.serverIPLabel.AutoSize = true;
            this.serverIPLabel.Location = new System.Drawing.Point(13, 38);
            this.serverIPLabel.Name = "serverIPLabel";
            this.serverIPLabel.Size = new System.Drawing.Size(54, 13);
            this.serverIPLabel.TabIndex = 4;
            this.serverIPLabel.Text = "Server IP:";
            // 
            // PlatformDebuggingTester
            // 
            this.AcceptButton = this.sendButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 102);
            this.Controls.Add(this.serverIPTextBox);
            this.Controls.Add(this.serverIPLabel);
            this.Controls.Add(this.reconnectButton);
            this.Controls.Add(this.sendButton);
            this.Controls.Add(this.stringTextBox);
            this.Controls.Add(this.stringLabel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "PlatformDebuggingTester";
            this.Text = "Platform Debugging Tester";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label stringLabel;
        private System.Windows.Forms.TextBox stringTextBox;
        private System.Windows.Forms.Button sendButton;
        private System.Windows.Forms.Button reconnectButton;
        private System.Windows.Forms.TextBox serverIPTextBox;
        private System.Windows.Forms.Label serverIPLabel;
    }
}

