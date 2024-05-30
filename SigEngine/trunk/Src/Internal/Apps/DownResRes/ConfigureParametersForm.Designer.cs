namespace DownResRes
{
	partial class ConfigureParametersForm
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
			this.lblSourceFolder = new System.Windows.Forms.Label( );
			this.tbSourceFolder = new System.Windows.Forms.TextBox( );
			this.groupBackups = new System.Windows.Forms.GroupBox( );
			this.cbOverwriteBackups = new System.Windows.Forms.CheckBox( );
			this.cbCheckinBackups = new System.Windows.Forms.CheckBox( );
			this.tbBackupsFolder = new System.Windows.Forms.TextBox( );
			this.lblBackupsFolder = new System.Windows.Forms.Label( );
			this.cbCreateBackups = new System.Windows.Forms.CheckBox( );
			this.labelBackupsPreamble = new System.Windows.Forms.Label( );
			this.groupConditions = new System.Windows.Forms.GroupBox( );
			this.numMaximumDivisor = new System.Windows.Forms.NumericUpDown( );
			this.lblMaximumReductionFactor = new System.Windows.Forms.Label( );
			this.numMinimumResolution = new System.Windows.Forms.NumericUpDown( );
			this.lblMinimumResolution = new System.Windows.Forms.Label( );
			this.progressBarConversion = new System.Windows.Forms.ProgressBar( );
			this.buttonProcess = new System.Windows.Forms.Button( );
			this.buttonCancel = new System.Windows.Forms.Button( );
			this.lblProgress = new System.Windows.Forms.Label( );
			this.rtConsole = new System.Windows.Forms.RichTextBox( );
			this.lblSourcePattern = new System.Windows.Forms.Label( );
			this.tbSourcePattern = new System.Windows.Forms.TextBox( );
			this.groupBackups.SuspendLayout( );
			this.groupConditions.SuspendLayout( );
			( (System.ComponentModel.ISupportInitialize)( this.numMaximumDivisor ) ).BeginInit( );
			( (System.ComponentModel.ISupportInitialize)( this.numMinimumResolution ) ).BeginInit( );
			this.SuspendLayout( );
			// 
			// lblSourceFolder
			// 
			this.lblSourceFolder.AutoSize = true;
			this.lblSourceFolder.Location = new System.Drawing.Point( 12, 9 );
			this.lblSourceFolder.Name = "lblSourceFolder";
			this.lblSourceFolder.Size = new System.Drawing.Size( 76, 13 );
			this.lblSourceFolder.TabIndex = 0;
			this.lblSourceFolder.Text = "Source Folder:";
			// 
			// tbSourceFolder
			// 
			this.tbSourceFolder.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.tbSourceFolder.Location = new System.Drawing.Point( 94, 6 );
			this.tbSourceFolder.Name = "tbSourceFolder";
			this.tbSourceFolder.Size = new System.Drawing.Size( 367, 20 );
			this.tbSourceFolder.TabIndex = 1;
			// 
			// groupBackups
			// 
			this.groupBackups.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.groupBackups.Controls.Add( this.cbOverwriteBackups );
			this.groupBackups.Controls.Add( this.cbCheckinBackups );
			this.groupBackups.Controls.Add( this.tbBackupsFolder );
			this.groupBackups.Controls.Add( this.lblBackupsFolder );
			this.groupBackups.Controls.Add( this.cbCreateBackups );
			this.groupBackups.Controls.Add( this.labelBackupsPreamble );
			this.groupBackups.Location = new System.Drawing.Point( 15, 53 );
			this.groupBackups.Name = "groupBackups";
			this.groupBackups.Size = new System.Drawing.Size( 449, 103 );
			this.groupBackups.TabIndex = 4;
			this.groupBackups.TabStop = false;
			this.groupBackups.Text = "Backups";
			// 
			// cbOverwriteBackups
			// 
			this.cbOverwriteBackups.AutoSize = true;
			this.cbOverwriteBackups.Checked = true;
			this.cbOverwriteBackups.CheckState = System.Windows.Forms.CheckState.Checked;
			this.cbOverwriteBackups.Enabled = false;
			this.cbOverwriteBackups.Location = new System.Drawing.Point( 294, 50 );
			this.cbOverwriteBackups.Name = "cbOverwriteBackups";
			this.cbOverwriteBackups.Size = new System.Drawing.Size( 153, 17 );
			this.cbOverwriteBackups.TabIndex = 3;
			this.cbOverwriteBackups.Text = "Overwrite existing backups";
			this.cbOverwriteBackups.UseVisualStyleBackColor = true;
			// 
			// cbCheckinBackups
			// 
			this.cbCheckinBackups.AutoSize = true;
			this.cbCheckinBackups.Enabled = false;
			this.cbCheckinBackups.Location = new System.Drawing.Point( 144, 50 );
			this.cbCheckinBackups.Name = "cbCheckinBackups";
			this.cbCheckinBackups.Size = new System.Drawing.Size( 144, 17 );
			this.cbCheckinBackups.TabIndex = 2;
			this.cbCheckinBackups.Text = "Add backups to Perforce";
			this.cbCheckinBackups.UseVisualStyleBackColor = true;
			// 
			// tbBackupsFolder
			// 
			this.tbBackupsFolder.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.tbBackupsFolder.Enabled = false;
			this.tbBackupsFolder.Location = new System.Drawing.Point( 100, 73 );
			this.tbBackupsFolder.Name = "tbBackupsFolder";
			this.tbBackupsFolder.Size = new System.Drawing.Size( 343, 20 );
			this.tbBackupsFolder.TabIndex = 5;
			// 
			// lblBackupsFolder
			// 
			this.lblBackupsFolder.AutoSize = true;
			this.lblBackupsFolder.Enabled = false;
			this.lblBackupsFolder.Location = new System.Drawing.Point( 6, 76 );
			this.lblBackupsFolder.Name = "lblBackupsFolder";
			this.lblBackupsFolder.Size = new System.Drawing.Size( 88, 13 );
			this.lblBackupsFolder.TabIndex = 4;
			this.lblBackupsFolder.Text = "Backup to folder:";
			// 
			// cbCreateBackups
			// 
			this.cbCreateBackups.AutoSize = true;
			this.cbCreateBackups.Location = new System.Drawing.Point( 6, 50 );
			this.cbCreateBackups.Name = "cbCreateBackups";
			this.cbCreateBackups.Size = new System.Drawing.Size( 132, 17 );
			this.cbCreateBackups.TabIndex = 1;
			this.cbCreateBackups.Text = "Create Backup Copies";
			this.cbCreateBackups.UseVisualStyleBackColor = true;
			this.cbCreateBackups.CheckedChanged += new System.EventHandler( this.cbCreateBackups_CheckedChanged );
			// 
			// labelBackupsPreamble
			// 
			this.labelBackupsPreamble.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.labelBackupsPreamble.Location = new System.Drawing.Point( 6, 16 );
			this.labelBackupsPreamble.Name = "labelBackupsPreamble";
			this.labelBackupsPreamble.Size = new System.Drawing.Size( 437, 31 );
			this.labelBackupsPreamble.TabIndex = 0;
			this.labelBackupsPreamble.Text = "Assuming resources are checked into perforce, you probably don\'t need this option" +
				" unless you want a local duplicate copy for ease of use.";
			// 
			// groupConditions
			// 
			this.groupConditions.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.groupConditions.Controls.Add( this.numMaximumDivisor );
			this.groupConditions.Controls.Add( this.lblMaximumReductionFactor );
			this.groupConditions.Controls.Add( this.numMinimumResolution );
			this.groupConditions.Controls.Add( this.lblMinimumResolution );
			this.groupConditions.Location = new System.Drawing.Point( 15, 162 );
			this.groupConditions.Name = "groupConditions";
			this.groupConditions.Size = new System.Drawing.Size( 449, 67 );
			this.groupConditions.TabIndex = 5;
			this.groupConditions.TabStop = false;
			this.groupConditions.Text = "Conditions && Parameters";
			// 
			// numMaximumDivisor
			// 
			this.numMaximumDivisor.Location = new System.Drawing.Point( 151, 40 );
			this.numMaximumDivisor.Name = "numMaximumDivisor";
			this.numMaximumDivisor.Size = new System.Drawing.Size( 120, 20 );
			this.numMaximumDivisor.TabIndex = 3;
			// 
			// lblMaximumReductionFactor
			// 
			this.lblMaximumReductionFactor.AutoSize = true;
			this.lblMaximumReductionFactor.Location = new System.Drawing.Point( 6, 42 );
			this.lblMaximumReductionFactor.Name = "lblMaximumReductionFactor";
			this.lblMaximumReductionFactor.Size = new System.Drawing.Size( 139, 13 );
			this.lblMaximumReductionFactor.TabIndex = 2;
			this.lblMaximumReductionFactor.Text = "Maximum Reduction Factor:";
			// 
			// numMinimumResolution
			// 
			this.numMinimumResolution.Location = new System.Drawing.Point( 151, 14 );
			this.numMinimumResolution.Maximum = new decimal( new int[] {
            -727379968,
            232,
            0,
            0} );
			this.numMinimumResolution.Name = "numMinimumResolution";
			this.numMinimumResolution.Size = new System.Drawing.Size( 120, 20 );
			this.numMinimumResolution.TabIndex = 1;
			// 
			// lblMinimumResolution
			// 
			this.lblMinimumResolution.AutoSize = true;
			this.lblMinimumResolution.Location = new System.Drawing.Point( 6, 16 );
			this.lblMinimumResolution.Name = "lblMinimumResolution";
			this.lblMinimumResolution.Size = new System.Drawing.Size( 104, 13 );
			this.lblMinimumResolution.TabIndex = 0;
			this.lblMinimumResolution.Text = "Minimum Resolution:";
			// 
			// progressBarConversion
			// 
			this.progressBarConversion.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.progressBarConversion.Location = new System.Drawing.Point( 12, 451 );
			this.progressBarConversion.Name = "progressBarConversion";
			this.progressBarConversion.Size = new System.Drawing.Size( 449, 23 );
			this.progressBarConversion.TabIndex = 4;
			// 
			// buttonProcess
			// 
			this.buttonProcess.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left ) ) );
			this.buttonProcess.Location = new System.Drawing.Point( 12, 480 );
			this.buttonProcess.Name = "buttonProcess";
			this.buttonProcess.Size = new System.Drawing.Size( 94, 23 );
			this.buttonProcess.TabIndex = 5;
			this.buttonProcess.Text = "Process Files";
			this.buttonProcess.UseVisualStyleBackColor = true;
			this.buttonProcess.Click += new System.EventHandler( this.buttonProcess_Click );
			// 
			// buttonCancel
			// 
			this.buttonCancel.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right ) ) );
			this.buttonCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.buttonCancel.Location = new System.Drawing.Point( 386, 480 );
			this.buttonCancel.Name = "buttonCancel";
			this.buttonCancel.Size = new System.Drawing.Size( 75, 23 );
			this.buttonCancel.TabIndex = 6;
			this.buttonCancel.Text = "Cancel";
			this.buttonCancel.UseVisualStyleBackColor = true;
			this.buttonCancel.Click += new System.EventHandler( this.buttonCancel_Click );
			// 
			// lblProgress
			// 
			this.lblProgress.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.lblProgress.Location = new System.Drawing.Point( 112, 480 );
			this.lblProgress.Name = "lblProgress";
			this.lblProgress.Size = new System.Drawing.Size( 268, 23 );
			this.lblProgress.TabIndex = 7;
			this.lblProgress.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
			// 
			// rtConsole
			// 
			this.rtConsole.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom )
						| System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.rtConsole.Location = new System.Drawing.Point( 12, 235 );
			this.rtConsole.Name = "rtConsole";
			this.rtConsole.Size = new System.Drawing.Size( 449, 210 );
			this.rtConsole.TabIndex = 8;
			this.rtConsole.Text = "";
			// 
			// lblSourcePattern
			// 
			this.lblSourcePattern.AutoSize = true;
			this.lblSourcePattern.Location = new System.Drawing.Point( 12, 30 );
			this.lblSourcePattern.Name = "lblSourcePattern";
			this.lblSourcePattern.Size = new System.Drawing.Size( 44, 13 );
			this.lblSourcePattern.TabIndex = 2;
			this.lblSourcePattern.Text = "Pattern:";
			// 
			// tbSourcePattern
			// 
			this.tbSourcePattern.Anchor = ( (System.Windows.Forms.AnchorStyles)( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left )
						| System.Windows.Forms.AnchorStyles.Right ) ) );
			this.tbSourcePattern.Location = new System.Drawing.Point( 94, 27 );
			this.tbSourcePattern.Name = "tbSourcePattern";
			this.tbSourcePattern.Size = new System.Drawing.Size( 367, 20 );
			this.tbSourcePattern.TabIndex = 3;
			// 
			// ConfigureParametersForm
			// 
			this.AcceptButton = this.buttonProcess;
			this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.buttonCancel;
			this.ClientSize = new System.Drawing.Size( 473, 513 );
			this.Controls.Add( this.tbSourcePattern );
			this.Controls.Add( this.lblSourcePattern );
			this.Controls.Add( this.rtConsole );
			this.Controls.Add( this.lblProgress );
			this.Controls.Add( this.buttonCancel );
			this.Controls.Add( this.buttonProcess );
			this.Controls.Add( this.progressBarConversion );
			this.Controls.Add( this.groupConditions );
			this.Controls.Add( this.groupBackups );
			this.Controls.Add( this.tbSourceFolder );
			this.Controls.Add( this.lblSourceFolder );
			this.Name = "ConfigureParametersForm";
			this.Text = "DownResRes";
			this.groupBackups.ResumeLayout( false );
			this.groupBackups.PerformLayout( );
			this.groupConditions.ResumeLayout( false );
			this.groupConditions.PerformLayout( );
			( (System.ComponentModel.ISupportInitialize)( this.numMaximumDivisor ) ).EndInit( );
			( (System.ComponentModel.ISupportInitialize)( this.numMinimumResolution ) ).EndInit( );
			this.ResumeLayout( false );
			this.PerformLayout( );

		}

		#endregion

		private System.Windows.Forms.Label lblSourceFolder;
		private System.Windows.Forms.TextBox tbSourceFolder;
		private System.Windows.Forms.GroupBox groupBackups;
		private System.Windows.Forms.Label labelBackupsPreamble;
		private System.Windows.Forms.CheckBox cbCreateBackups;
		private System.Windows.Forms.TextBox tbBackupsFolder;
		private System.Windows.Forms.Label lblBackupsFolder;
		private System.Windows.Forms.CheckBox cbCheckinBackups;
		private System.Windows.Forms.CheckBox cbOverwriteBackups;
		private System.Windows.Forms.GroupBox groupConditions;
		private System.Windows.Forms.Label lblMinimumResolution;
		private System.Windows.Forms.NumericUpDown numMaximumDivisor;
		private System.Windows.Forms.Label lblMaximumReductionFactor;
		private System.Windows.Forms.NumericUpDown numMinimumResolution;
		private System.Windows.Forms.ProgressBar progressBarConversion;
		private System.Windows.Forms.Button buttonProcess;
		private System.Windows.Forms.Button buttonCancel;
		private System.Windows.Forms.Label lblProgress;
		private System.Windows.Forms.RichTextBox rtConsole;
		private System.Windows.Forms.Label lblSourcePattern;
		private System.Windows.Forms.TextBox tbSourcePattern;
	}
}