namespace GetLatest
{
    partial class GetLatest
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
            this.OutputLog = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // OutputLog
            // 
            this.OutputLog.Location = new System.Drawing.Point(12, 12);
            this.OutputLog.Multiline = true;
            this.OutputLog.Name = "OutputLog";
            this.OutputLog.ReadOnly = true;
            this.OutputLog.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.OutputLog.Size = new System.Drawing.Size(760, 388);
            this.OutputLog.TabIndex = 0;
            // 
            // GetLatest
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 412);
            this.Controls.Add(this.OutputLog);
            this.Name = "GetLatest";
            this.Text = "Get Latest";
            this.Shown += new System.EventHandler(this.GetLatest_Shown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox OutputLog;

    }
}

