namespace DataTableSchema
{
    partial class Form1
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
            this.mFileList = new System.Windows.Forms.ListBox();
            this.label1 = new System.Windows.Forms.Label();
            this.bRefresh = new System.Windows.Forms.Button();
            this.bLoadSchema = new System.Windows.Forms.Button();
            this.bSaveSchema = new System.Windows.Forms.Button();
            this.bApply = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // mFileList
            // 
            this.mFileList.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
            this.mFileList.FormattingEnabled = true;
            this.mFileList.HorizontalScrollbar = true;
            this.mFileList.Location = new System.Drawing.Point(17, 40);
            this.mFileList.Name = "mFileList";
            this.mFileList.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.mFileList.Size = new System.Drawing.Size(520, 290);
            this.mFileList.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(14, 24);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(71, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Affected files:";
            // 
            // bRefresh
            // 
            this.bRefresh.Location = new System.Drawing.Point(565, 46);
            this.bRefresh.Name = "bRefresh";
            this.bRefresh.Size = new System.Drawing.Size(71, 19);
            this.bRefresh.TabIndex = 2;
            this.bRefresh.Text = "Refresh";
            this.bRefresh.UseVisualStyleBackColor = true;
            this.bRefresh.Click += new System.EventHandler(this.bRefresh_Click);
            // 
            // bLoadSchema
            // 
            this.bLoadSchema.Location = new System.Drawing.Point(565, 71);
            this.bLoadSchema.Name = "bLoadSchema";
            this.bLoadSchema.Size = new System.Drawing.Size(71, 19);
            this.bLoadSchema.TabIndex = 3;
            this.bLoadSchema.Text = "Load";
            this.bLoadSchema.UseVisualStyleBackColor = true;
            this.bLoadSchema.Click += new System.EventHandler(this.bLoadSchema_Click);
            // 
            // bSaveSchema
            // 
            this.bSaveSchema.Location = new System.Drawing.Point(565, 96);
            this.bSaveSchema.Name = "bSaveSchema";
            this.bSaveSchema.Size = new System.Drawing.Size(71, 19);
            this.bSaveSchema.TabIndex = 4;
            this.bSaveSchema.Text = "Save";
            this.bSaveSchema.UseVisualStyleBackColor = true;
            this.bSaveSchema.Click += new System.EventHandler(this.bSaveSchema_Click);
            // 
            // bApply
            // 
            this.bApply.Location = new System.Drawing.Point(565, 121);
            this.bApply.Name = "bApply";
            this.bApply.Size = new System.Drawing.Size(71, 19);
            this.bApply.TabIndex = 5;
            this.bApply.Text = "Apply";
            this.bApply.UseVisualStyleBackColor = true;
            this.bApply.Click += new System.EventHandler(this.bApply_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(786, 367);
            this.Controls.Add(this.bApply);
            this.Controls.Add(this.bSaveSchema);
            this.Controls.Add(this.bLoadSchema);
            this.Controls.Add(this.bRefresh);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.mFileList);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListBox mFileList;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button bRefresh;
        private System.Windows.Forms.Button bLoadSchema;
        private System.Windows.Forms.Button bSaveSchema;
        private System.Windows.Forms.Button bApply;
    }
}

