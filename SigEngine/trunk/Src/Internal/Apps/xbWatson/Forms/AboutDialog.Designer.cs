namespace Atg.Samples.xbWatson.Forms
{
    partial class AboutDialog
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
            System.Windows.Forms.Panel upperPanel;
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AboutDialog));
            this.okButton = new System.Windows.Forms.Button();
            this.legalLabel = new System.Windows.Forms.Label();
            this.xbWatsonIcon = new System.Windows.Forms.PictureBox();
            this.versionLabel = new System.Windows.Forms.Label();
            panel1 = new System.Windows.Forms.Panel();
            upperPanel = new System.Windows.Forms.Panel();
            panel1.SuspendLayout();
            upperPanel.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.xbWatsonIcon)).BeginInit();
            this.SuspendLayout();
            // 
            // panel1
            // 
            panel1.Controls.Add(this.okButton);
            panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            panel1.Location = new System.Drawing.Point(0, 166);
            panel1.Name = "panel1";
            panel1.Padding = new System.Windows.Forms.Padding(8);
            panel1.Size = new System.Drawing.Size(396, 48);
            panel1.TabIndex = 7;
            // 
            // okButton
            // 
            this.okButton.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.okButton.Location = new System.Drawing.Point(288, 12);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(96, 24);
            this.okButton.TabIndex = 1;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            // 
            // upperPanel
            // 
            upperPanel.Controls.Add(this.legalLabel);
            upperPanel.Controls.Add(this.xbWatsonIcon);
            upperPanel.Controls.Add(this.versionLabel);
            upperPanel.Dock = System.Windows.Forms.DockStyle.Fill;
            upperPanel.Location = new System.Drawing.Point(0, 0);
            upperPanel.Name = "upperPanel";
            upperPanel.Size = new System.Drawing.Size(396, 166);
            upperPanel.TabIndex = 8;
            // 
            // legalLabel
            // 
            this.legalLabel.AutoSize = true;
            this.legalLabel.Location = new System.Drawing.Point(50, 98);
            this.legalLabel.Name = "legalLabel";
            this.legalLabel.Size = new System.Drawing.Size(327, 65);
            this.legalLabel.TabIndex = 6;
            this.legalLabel.Text = resources.GetString("legalLabel.Text");
            // 
            // xbWatsonIcon
            // 
            this.xbWatsonIcon.Image = ((System.Drawing.Image)(resources.GetObject("xbWatsonIcon.Image")));
            this.xbWatsonIcon.Location = new System.Drawing.Point(12, 9);
            this.xbWatsonIcon.Name = "xbWatsonIcon";
            this.xbWatsonIcon.Size = new System.Drawing.Size(32, 32);
            this.xbWatsonIcon.SizeMode = System.Windows.Forms.PictureBoxSizeMode.CenterImage;
            this.xbWatsonIcon.TabIndex = 5;
            this.xbWatsonIcon.TabStop = false;
            // 
            // versionLabel
            // 
            this.versionLabel.AutoSize = true;
            this.versionLabel.Location = new System.Drawing.Point(50, 9);
            this.versionLabel.Name = "versionLabel";
            this.versionLabel.Size = new System.Drawing.Size(58, 13);
            this.versionLabel.TabIndex = 0;
            this.versionLabel.Text = "<version>";
            this.versionLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // AboutDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(396, 214);
            this.Controls.Add(upperPanel);
            this.Controls.Add(panel1);
            this.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "AboutDialog";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "About xbWatson";
            panel1.ResumeLayout(false);
            upperPanel.ResumeLayout(false);
            upperPanel.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.xbWatsonIcon)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Label versionLabel;
        private System.Windows.Forms.PictureBox xbWatsonIcon;
        private System.Windows.Forms.Label legalLabel;
    }
}