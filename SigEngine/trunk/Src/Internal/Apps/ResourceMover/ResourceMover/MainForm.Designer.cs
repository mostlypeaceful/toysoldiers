namespace ResourceMover
{
	partial class MainForm
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
			this.components = new System.ComponentModel.Container();
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
			this.openFileDialog1 = new System.Windows.Forms.OpenFileDialog();
			this.srcFileBrowse = new System.Windows.Forms.Button();
			this.targetFileBrowse = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.ExecFileMove = new System.Windows.Forms.Button();
			this.folderBrowserDialog1 = new System.Windows.Forms.FolderBrowserDialog();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.TargetFolderBrowse = new System.Windows.Forms.Button();
			this.tgtFolderText = new System.Windows.Forms.TextBox();
			this.srcFolderText = new System.Windows.Forms.TextBox();
			this.SrcFolderBrowse = new System.Windows.Forms.Button();
			this.saveFileDialog1 = new System.Windows.Forms.SaveFileDialog();
			this.moveProgress = new System.Windows.Forms.ProgressBar();
			this.bgWorker = new System.ComponentModel.BackgroundWorker();
			this.workText = new System.Windows.Forms.Label();
			this.timer1 = new System.Windows.Forms.Timer(this.components);
			this.srcFileBox = new System.Windows.Forms.ListBox();
			this.tgtFileBox = new System.Windows.Forms.ListBox();
			this.srcFileRemove = new System.Windows.Forms.Button();
			this.groupBox1 = new System.Windows.Forms.GroupBox();
			this.tgtFileRemove = new System.Windows.Forms.Button();
			this.groupBox2 = new System.Windows.Forms.GroupBox();
			this.ExecFolderMove = new System.Windows.Forms.Button();
			this.groupBox3 = new System.Windows.Forms.GroupBox();
			this.refLog = new System.Windows.Forms.TreeView();
			this.groupBox1.SuspendLayout();
			this.groupBox2.SuspendLayout();
			this.groupBox3.SuspendLayout();
			this.SuspendLayout();
			// 
			// openFileDialog1
			// 
			this.openFileDialog1.FileName = "openFileDialog1";
			this.openFileDialog1.Multiselect = true;
			// 
			// srcFileBrowse
			// 
			this.srcFileBrowse.Location = new System.Drawing.Point(525, 36);
			this.srcFileBrowse.Name = "srcFileBrowse";
			this.srcFileBrowse.Size = new System.Drawing.Size(43, 23);
			this.srcFileBrowse.TabIndex = 0;
			this.srcFileBrowse.Text = "+";
			this.srcFileBrowse.UseVisualStyleBackColor = true;
			this.srcFileBrowse.Click += new System.EventHandler(this.SrcFileBrowse_Click);
			// 
			// targetFileBrowse
			// 
			this.targetFileBrowse.Location = new System.Drawing.Point(525, 196);
			this.targetFileBrowse.Name = "targetFileBrowse";
			this.targetFileBrowse.Size = new System.Drawing.Size(43, 23);
			this.targetFileBrowse.TabIndex = 3;
			this.targetFileBrowse.Text = "...";
			this.targetFileBrowse.UseVisualStyleBackColor = true;
			this.targetFileBrowse.Click += new System.EventHandler(this.TargetFileBrowse_Click);
			// 
			// label1
			// 
			this.label1.AutoSize = true;
			this.label1.Location = new System.Drawing.Point(18, 20);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(74, 13);
			this.label1.TabIndex = 4;
			this.label1.Text = "Source Files...";
			// 
			// label2
			// 
			this.label2.AutoSize = true;
			this.label2.Location = new System.Drawing.Point(18, 180);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(71, 13);
			this.label2.TabIndex = 5;
			this.label2.Text = "Target Files...";
			// 
			// ExecFileMove
			// 
			this.ExecFileMove.Location = new System.Drawing.Point(21, 336);
			this.ExecFileMove.Name = "ExecFileMove";
			this.ExecFileMove.Size = new System.Drawing.Size(75, 23);
			this.ExecFileMove.TabIndex = 6;
			this.ExecFileMove.Text = "Execute";
			this.ExecFileMove.UseVisualStyleBackColor = true;
			this.ExecFileMove.Click += new System.EventHandler(this.ExecFileMove_Click);
			// 
			// folderBrowserDialog1
			// 
			this.folderBrowserDialog1.RootFolder = System.Environment.SpecialFolder.Recent;
			// 
			// label3
			// 
			this.label3.AutoSize = true;
			this.label3.Location = new System.Drawing.Point(19, 68);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(79, 13);
			this.label3.TabIndex = 12;
			this.label3.Text = "Target Folder...";
			// 
			// label4
			// 
			this.label4.AutoSize = true;
			this.label4.Location = new System.Drawing.Point(16, 24);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(82, 13);
			this.label4.TabIndex = 11;
			this.label4.Text = "Source Folder...";
			// 
			// TargetFolderBrowse
			// 
			this.TargetFolderBrowse.Location = new System.Drawing.Point(523, 82);
			this.TargetFolderBrowse.Name = "TargetFolderBrowse";
			this.TargetFolderBrowse.Size = new System.Drawing.Size(43, 23);
			this.TargetFolderBrowse.TabIndex = 10;
			this.TargetFolderBrowse.Text = "...";
			this.TargetFolderBrowse.UseVisualStyleBackColor = true;
			this.TargetFolderBrowse.Click += new System.EventHandler(this.TargetFolderBrowse_Click);
			// 
			// tgtFolderText
			// 
			this.tgtFolderText.Enabled = false;
			this.tgtFolderText.Location = new System.Drawing.Point(19, 84);
			this.tgtFolderText.Name = "tgtFolderText";
			this.tgtFolderText.Size = new System.Drawing.Size(498, 20);
			this.tgtFolderText.TabIndex = 9;
			// 
			// srcFolderText
			// 
			this.srcFolderText.Enabled = false;
			this.srcFolderText.Location = new System.Drawing.Point(19, 40);
			this.srcFolderText.Name = "srcFolderText";
			this.srcFolderText.Size = new System.Drawing.Size(498, 20);
			this.srcFolderText.TabIndex = 8;
			// 
			// SrcFolderBrowse
			// 
			this.SrcFolderBrowse.Location = new System.Drawing.Point(523, 37);
			this.SrcFolderBrowse.Name = "SrcFolderBrowse";
			this.SrcFolderBrowse.Size = new System.Drawing.Size(43, 23);
			this.SrcFolderBrowse.TabIndex = 7;
			this.SrcFolderBrowse.Text = "...";
			this.SrcFolderBrowse.UseVisualStyleBackColor = true;
			this.SrcFolderBrowse.Click += new System.EventHandler(this.SrcFolderBrowse_Click);
			// 
			// moveProgress
			// 
			this.moveProgress.Location = new System.Drawing.Point(9, 568);
			this.moveProgress.Name = "moveProgress";
			this.moveProgress.Size = new System.Drawing.Size(591, 23);
			this.moveProgress.TabIndex = 14;
			// 
			// bgWorker
			// 
			this.bgWorker.WorkerReportsProgress = true;
			// 
			// workText
			// 
			this.workText.AutoSize = true;
			this.workText.Location = new System.Drawing.Point(9, 552);
			this.workText.Name = "workText";
			this.workText.Size = new System.Drawing.Size(37, 13);
			this.workText.TabIndex = 15;
			this.workText.Text = "Status";
			// 
			// srcFileBox
			// 
			this.srcFileBox.FormattingEnabled = true;
			this.srcFileBox.Location = new System.Drawing.Point(21, 36);
			this.srcFileBox.Name = "srcFileBox";
			this.srcFileBox.ScrollAlwaysVisible = true;
			this.srcFileBox.Size = new System.Drawing.Size(501, 134);
			this.srcFileBox.TabIndex = 16;
			this.srcFileBox.SelectedIndexChanged += new System.EventHandler(this.srcFileBox_SelectedIndexChanged);
			// 
			// tgtFileBox
			// 
			this.tgtFileBox.FormattingEnabled = true;
			this.tgtFileBox.Location = new System.Drawing.Point(24, 196);
			this.tgtFileBox.Name = "tgtFileBox";
			this.tgtFileBox.ScrollAlwaysVisible = true;
			this.tgtFileBox.Size = new System.Drawing.Size(498, 134);
			this.tgtFileBox.TabIndex = 17;
			this.tgtFileBox.SelectedIndexChanged += new System.EventHandler(this.tgtFileBox_SelectedIndexChanged);
			// 
			// srcFileRemove
			// 
			this.srcFileRemove.Location = new System.Drawing.Point(525, 65);
			this.srcFileRemove.Name = "srcFileRemove";
			this.srcFileRemove.Size = new System.Drawing.Size(43, 23);
			this.srcFileRemove.TabIndex = 18;
			this.srcFileRemove.Text = "-";
			this.srcFileRemove.UseVisualStyleBackColor = true;
			this.srcFileRemove.Click += new System.EventHandler(this.srcFileRemove_Click);
			// 
			// groupBox1
			// 
			this.groupBox1.Controls.Add(this.tgtFileRemove);
			this.groupBox1.Controls.Add(this.srcFileRemove);
			this.groupBox1.Controls.Add(this.tgtFileBox);
			this.groupBox1.Controls.Add(this.srcFileBox);
			this.groupBox1.Controls.Add(this.ExecFileMove);
			this.groupBox1.Controls.Add(this.label2);
			this.groupBox1.Controls.Add(this.label1);
			this.groupBox1.Controls.Add(this.targetFileBrowse);
			this.groupBox1.Controls.Add(this.srcFileBrowse);
			this.groupBox1.Location = new System.Drawing.Point(9, 13);
			this.groupBox1.Name = "groupBox1";
			this.groupBox1.Size = new System.Drawing.Size(592, 371);
			this.groupBox1.TabIndex = 19;
			this.groupBox1.TabStop = false;
			this.groupBox1.Text = "Manage Files";
			// 
			// tgtFileRemove
			// 
			this.tgtFileRemove.Location = new System.Drawing.Point(525, 225);
			this.tgtFileRemove.Name = "tgtFileRemove";
			this.tgtFileRemove.Size = new System.Drawing.Size(43, 23);
			this.tgtFileRemove.TabIndex = 19;
			this.tgtFileRemove.Text = "-";
			this.tgtFileRemove.UseVisualStyleBackColor = true;
			this.tgtFileRemove.Click += new System.EventHandler(this.tgtFileRemove_Click);
			// 
			// groupBox2
			// 
			this.groupBox2.Controls.Add(this.ExecFolderMove);
			this.groupBox2.Controls.Add(this.label3);
			this.groupBox2.Controls.Add(this.label4);
			this.groupBox2.Controls.Add(this.TargetFolderBrowse);
			this.groupBox2.Controls.Add(this.tgtFolderText);
			this.groupBox2.Controls.Add(this.srcFolderText);
			this.groupBox2.Controls.Add(this.SrcFolderBrowse);
			this.groupBox2.Location = new System.Drawing.Point(9, 390);
			this.groupBox2.Name = "groupBox2";
			this.groupBox2.Size = new System.Drawing.Size(592, 145);
			this.groupBox2.TabIndex = 20;
			this.groupBox2.TabStop = false;
			this.groupBox2.Text = "Manage Folders";
			// 
			// ExecFolderMove
			// 
			this.ExecFolderMove.Location = new System.Drawing.Point(19, 110);
			this.ExecFolderMove.Name = "ExecFolderMove";
			this.ExecFolderMove.Size = new System.Drawing.Size(75, 23);
			this.ExecFolderMove.TabIndex = 13;
			this.ExecFolderMove.Text = "Execute";
			this.ExecFolderMove.UseVisualStyleBackColor = true;
			this.ExecFolderMove.Click += new System.EventHandler(this.ExecFolderMove_Click);
			// 
			// groupBox3
			// 
			this.groupBox3.Controls.Add(this.refLog);
			this.groupBox3.Location = new System.Drawing.Point(9, 614);
			this.groupBox3.Name = "groupBox3";
			this.groupBox3.Size = new System.Drawing.Size(591, 154);
			this.groupBox3.TabIndex = 21;
			this.groupBox3.TabStop = false;
			this.groupBox3.Text = "Failed";
			// 
			// refLog
			// 
			this.refLog.Location = new System.Drawing.Point(7, 20);
			this.refLog.Name = "refLog";
			this.refLog.Size = new System.Drawing.Size(578, 128);
			this.refLog.TabIndex = 0;
			// 
			// MainForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(624, 781);
			this.Controls.Add(this.groupBox3);
			this.Controls.Add(this.groupBox2);
			this.Controls.Add(this.groupBox1);
			this.Controls.Add(this.workText);
			this.Controls.Add(this.moveProgress);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
			this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
			this.Name = "MainForm";
			this.Text = "SigMove";
			this.groupBox1.ResumeLayout(false);
			this.groupBox1.PerformLayout();
			this.groupBox2.ResumeLayout(false);
			this.groupBox2.PerformLayout();
			this.groupBox3.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.OpenFileDialog openFileDialog1;
		private System.Windows.Forms.Button srcFileBrowse;
		private System.Windows.Forms.Button targetFileBrowse;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Button ExecFileMove;
		private System.Windows.Forms.FolderBrowserDialog folderBrowserDialog1;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Button TargetFolderBrowse;
		private System.Windows.Forms.TextBox tgtFolderText;
		private System.Windows.Forms.TextBox srcFolderText;
		private System.Windows.Forms.Button SrcFolderBrowse;
		private System.Windows.Forms.SaveFileDialog saveFileDialog1;
		private System.Windows.Forms.ProgressBar moveProgress;
		private System.ComponentModel.BackgroundWorker bgWorker;
		private System.Windows.Forms.Label workText;
		private System.Windows.Forms.Timer timer1;
		private System.Windows.Forms.ListBox srcFileBox;
		private System.Windows.Forms.ListBox tgtFileBox;
		private System.Windows.Forms.Button srcFileRemove;
		private System.Windows.Forms.GroupBox groupBox1;
		private System.Windows.Forms.GroupBox groupBox2;
		private System.Windows.Forms.Button ExecFolderMove;
		private System.Windows.Forms.Button tgtFileRemove;
		private System.Windows.Forms.GroupBox groupBox3;
		private System.Windows.Forms.TreeView refLog;
	}
}

