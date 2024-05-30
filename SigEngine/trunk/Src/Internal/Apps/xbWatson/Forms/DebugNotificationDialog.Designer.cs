namespace Atg.Samples.xbWatson.Forms
{
    partial class DebugNotificationDialog
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
            System.Windows.Forms.Panel upperPanel;
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DebugNotificationDialog));
            this.detailsText = new System.Windows.Forms.TextBox();
            this.notificationIcon = new System.Windows.Forms.PictureBox();
            this.overviewLabel = new System.Windows.Forms.Label();
            this.lowerPanel = new System.Windows.Forms.Panel();
            this.continueButton = new System.Windows.Forms.Button();
            this.rebootButton = new System.Windows.Forms.Button();
            this.notificationIcons = new System.Windows.Forms.ImageList(this.components);
            upperPanel = new System.Windows.Forms.Panel();
            upperPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.notificationIcon)).BeginInit();
            this.lowerPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // upperPanel
            // 
            upperPanel.Controls.Add(this.detailsText);
            upperPanel.Controls.Add(this.notificationIcon);
            upperPanel.Controls.Add(this.overviewLabel);
            upperPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            upperPanel.Location = new System.Drawing.Point(0, 0);
            upperPanel.Name = "upperPanel";
            upperPanel.Size = new System.Drawing.Size(597, 262);
            upperPanel.TabIndex = 0;
            // 
            // detailsText
            // 
            this.detailsText.Font = new System.Drawing.Font("Courier New", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.detailsText.Location = new System.Drawing.Point(12, 44);
            this.detailsText.Multiline = true;
            this.detailsText.Name = "detailsText";
            this.detailsText.ReadOnly = true;
            this.detailsText.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.detailsText.Size = new System.Drawing.Size(575, 215);
            this.detailsText.TabIndex = 1;
            this.detailsText.Text = "<details>";
            // 
            // notificationIcon
            // 
            this.notificationIcon.Location = new System.Drawing.Point(12, 9);
            this.notificationIcon.Name = "notificationIcon";
            this.notificationIcon.Size = new System.Drawing.Size(32, 32);
            this.notificationIcon.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.notificationIcon.TabIndex = 5;
            this.notificationIcon.TabStop = false;
            // 
            // overviewLabel
            // 
            this.overviewLabel.Location = new System.Drawing.Point(50, 9);
            this.overviewLabel.Name = "overviewLabel";
            this.overviewLabel.Size = new System.Drawing.Size(537, 32);
            this.overviewLabel.TabIndex = 0;
            this.overviewLabel.Text = "<overview>";
            this.overviewLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // lowerPanel
            // 
            this.lowerPanel.Controls.Add(this.continueButton);
            this.lowerPanel.Controls.Add(this.rebootButton);
            this.lowerPanel.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.lowerPanel.Location = new System.Drawing.Point(0, 262);
            this.lowerPanel.Name = "lowerPanel";
            this.lowerPanel.Padding = new System.Windows.Forms.Padding(8);
            this.lowerPanel.Size = new System.Drawing.Size(597, 48);
            this.lowerPanel.TabIndex = 1;
            // 
            // continueButton
            // 
            this.continueButton.DialogResult = System.Windows.Forms.DialogResult.Retry;
            this.continueButton.Location = new System.Drawing.Point(389, 13);
            this.continueButton.Name = "continueButton";
            this.continueButton.Size = new System.Drawing.Size(96, 24);
            this.continueButton.TabIndex = 2;
            this.continueButton.Text = "&Continue";
            this.continueButton.UseVisualStyleBackColor = true;
            this.continueButton.Click += new System.EventHandler(this.continueButton_Click);
            // 
            // rebootButton
            // 
            this.rebootButton.DialogResult = System.Windows.Forms.DialogResult.Abort;
            this.rebootButton.Location = new System.Drawing.Point(491, 13);
            this.rebootButton.Name = "rebootButton";
            this.rebootButton.Size = new System.Drawing.Size(96, 24);
            this.rebootButton.TabIndex = 4;
            this.rebootButton.Text = "&Reboot";
            this.rebootButton.UseVisualStyleBackColor = true;
            this.rebootButton.Click += new System.EventHandler(this.rebootButton_Click);
            // 
            // notificationIcons
            // 
            this.notificationIcons.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("notificationIcons.ImageStream")));
            this.notificationIcons.TransparentColor = System.Drawing.Color.Transparent;
            this.notificationIcons.Images.SetKeyName(0, "INFO.ICO");
            this.notificationIcons.Images.SetKeyName(1, "warning.ico");
            this.notificationIcons.Images.SetKeyName(2, "error.ico");
            // 
            // DebugNotificationDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(597, 310);
            this.Controls.Add(upperPanel);
            this.Controls.Add(this.lowerPanel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "DebugNotificationDialog";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "<title>";
            this.Load += new System.EventHandler(this.DebugNotificationDialog_Load);
            upperPanel.ResumeLayout(false);
            upperPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.notificationIcon)).EndInit();
            this.lowerPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button continueButton;
        private System.Windows.Forms.Button rebootButton;
        private System.Windows.Forms.TextBox detailsText;
        private System.Windows.Forms.PictureBox notificationIcon;
        private System.Windows.Forms.Label overviewLabel;
        private System.Windows.Forms.ImageList notificationIcons;
        private System.Windows.Forms.Panel lowerPanel;
    }
}