//------------------------------------------------------------------------------
// \file EditProfileForm.Designer.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

namespace Signal.Config.ProjectSelector
{
	partial class EditProfileForm
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
			if( disposing && (components != null) )
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
			this.lblProfileName = new System.Windows.Forms.Label();
			this.tbProfileName = new System.Windows.Forms.TextBox();
			this.tbWorkspaceOverride = new System.Windows.Forms.TextBox();
			this.lblWorkspaceOverride = new System.Windows.Forms.Label();
			this.tbDeployToDir = new System.Windows.Forms.TextBox();
			this.lblDeployToDir = new System.Windows.Forms.Label();
			this.tbPullFromPath = new System.Windows.Forms.TextBox();
			this.lblPullFromPath = new System.Windows.Forms.Label();
			this.tbPerforcePort = new System.Windows.Forms.TextBox();
			this.lblPerforcePort = new System.Windows.Forms.Label();
			this.lblEnginePath = new System.Windows.Forms.Label();
			this.bEnginePathBrowse = new System.Windows.Forms.Button();
			this.tbEnginePath = new System.Windows.Forms.TextBox();
			this.tbProjectPath = new System.Windows.Forms.TextBox();
			this.bProjectPathBrowse = new System.Windows.Forms.Button();
			this.lblProjectPath = new System.Windows.Forms.Label();
			this.cbActiveProject = new System.Windows.Forms.CheckBox();
			this.cbSyncChangelistToBuild = new System.Windows.Forms.CheckBox();
			this.bOK = new System.Windows.Forms.Button();
			this.bCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// lblProfileName
			// 
			this.lblProfileName.AutoSize = true;
			this.lblProfileName.Location = new System.Drawing.Point(50, 15);
			this.lblProfileName.Name = "lblProfileName";
			this.lblProfileName.Size = new System.Drawing.Size(70, 13);
			this.lblProfileName.TabIndex = 0;
			this.lblProfileName.Text = "Profile Name:";
			// 
			// tbProfileName
			// 
			this.tbProfileName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbProfileName.Location = new System.Drawing.Point(123, 12);
			this.tbProfileName.Name = "tbProfileName";
			this.tbProfileName.Size = new System.Drawing.Size(298, 20);
			this.tbProfileName.TabIndex = 1;
			// 
			// tbWorkspaceOverride
			// 
			this.tbWorkspaceOverride.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbWorkspaceOverride.Location = new System.Drawing.Point(123, 38);
			this.tbWorkspaceOverride.Name = "tbWorkspaceOverride";
			this.tbWorkspaceOverride.Size = new System.Drawing.Size(298, 20);
			this.tbWorkspaceOverride.TabIndex = 3;
			// 
			// lblWorkspaceOverride
			// 
			this.lblWorkspaceOverride.AutoSize = true;
			this.lblWorkspaceOverride.Location = new System.Drawing.Point(12, 41);
			this.lblWorkspaceOverride.Name = "lblWorkspaceOverride";
			this.lblWorkspaceOverride.Size = new System.Drawing.Size(108, 13);
			this.lblWorkspaceOverride.TabIndex = 2;
			this.lblWorkspaceOverride.Text = "Workspace Override:";
			// 
			// tbDeployToDir
			// 
			this.tbDeployToDir.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbDeployToDir.Location = new System.Drawing.Point(123, 64);
			this.tbDeployToDir.Name = "tbDeployToDir";
			this.tbDeployToDir.Size = new System.Drawing.Size(298, 20);
			this.tbDeployToDir.TabIndex = 5;
			// 
			// lblDeployToDir
			// 
			this.lblDeployToDir.AutoSize = true;
			this.lblDeployToDir.Location = new System.Drawing.Point(49, 67);
			this.lblDeployToDir.Name = "lblDeployToDir";
			this.lblDeployToDir.Size = new System.Drawing.Size(71, 13);
			this.lblDeployToDir.TabIndex = 4;
			this.lblDeployToDir.Text = "Deploy to Dir:";
			// 
			// tbPullFromPath
			// 
			this.tbPullFromPath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbPullFromPath.Location = new System.Drawing.Point(123, 90);
			this.tbPullFromPath.Name = "tbPullFromPath";
			this.tbPullFromPath.Size = new System.Drawing.Size(298, 20);
			this.tbPullFromPath.TabIndex = 7;
			// 
			// lblPullFromPath
			// 
			this.lblPullFromPath.AutoSize = true;
			this.lblPullFromPath.Location = new System.Drawing.Point(45, 93);
			this.lblPullFromPath.Name = "lblPullFromPath";
			this.lblPullFromPath.Size = new System.Drawing.Size(75, 13);
			this.lblPullFromPath.TabIndex = 6;
			this.lblPullFromPath.Text = "Pull from Path:";
			// 
			// tbPerforcePort
			// 
			this.tbPerforcePort.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbPerforcePort.Location = new System.Drawing.Point(123, 116);
			this.tbPerforcePort.Name = "tbPerforcePort";
			this.tbPerforcePort.Size = new System.Drawing.Size(298, 20);
			this.tbPerforcePort.TabIndex = 9;
			// 
			// lblPerforcePort
			// 
			this.lblPerforcePort.AutoSize = true;
			this.lblPerforcePort.Location = new System.Drawing.Point(45, 119);
			this.lblPerforcePort.Name = "lblPerforcePort";
			this.lblPerforcePort.Size = new System.Drawing.Size(72, 13);
			this.lblPerforcePort.TabIndex = 8;
			this.lblPerforcePort.Text = "Perforce Port:";
			// 
			// lblEnginePath
			// 
			this.lblEnginePath.AutoSize = true;
			this.lblEnginePath.Location = new System.Drawing.Point(9, 149);
			this.lblEnginePath.Name = "lblEnginePath";
			this.lblEnginePath.Size = new System.Drawing.Size(68, 13);
			this.lblEnginePath.TabIndex = 10;
			this.lblEnginePath.Text = "Engine Path:";
			// 
			// bEnginePathBrowse
			// 
			this.bEnginePathBrowse.Location = new System.Drawing.Point(101, 144);
			this.bEnginePathBrowse.Name = "bEnginePathBrowse";
			this.bEnginePathBrowse.Size = new System.Drawing.Size(75, 23);
			this.bEnginePathBrowse.TabIndex = 11;
			this.bEnginePathBrowse.Text = "Browse...";
			this.bEnginePathBrowse.UseVisualStyleBackColor = true;
			this.bEnginePathBrowse.Click += new System.EventHandler(this.bEnginePathBrowse_Click);
			// 
			// tbEnginePath
			// 
			this.tbEnginePath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbEnginePath.Location = new System.Drawing.Point(12, 173);
			this.tbEnginePath.Name = "tbEnginePath";
			this.tbEnginePath.Size = new System.Drawing.Size(409, 20);
			this.tbEnginePath.TabIndex = 12;
			// 
			// tbProjectPath
			// 
			this.tbProjectPath.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbProjectPath.Location = new System.Drawing.Point(12, 232);
			this.tbProjectPath.Name = "tbProjectPath";
			this.tbProjectPath.Size = new System.Drawing.Size(409, 20);
			this.tbProjectPath.TabIndex = 15;
			// 
			// bProjectPathBrowse
			// 
			this.bProjectPathBrowse.Location = new System.Drawing.Point(101, 203);
			this.bProjectPathBrowse.Name = "bProjectPathBrowse";
			this.bProjectPathBrowse.Size = new System.Drawing.Size(75, 23);
			this.bProjectPathBrowse.TabIndex = 14;
			this.bProjectPathBrowse.Text = "Browse...";
			this.bProjectPathBrowse.UseVisualStyleBackColor = true;
			this.bProjectPathBrowse.Click += new System.EventHandler(this.bProjectPathBrowse_Click);
			// 
			// lblProjectPath
			// 
			this.lblProjectPath.AutoSize = true;
			this.lblProjectPath.Location = new System.Drawing.Point(9, 208);
			this.lblProjectPath.Name = "lblProjectPath";
			this.lblProjectPath.Size = new System.Drawing.Size(68, 13);
			this.lblProjectPath.TabIndex = 13;
			this.lblProjectPath.Text = "Project Path:";
			// 
			// cbActiveProject
			// 
			this.cbActiveProject.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.cbActiveProject.AutoSize = true;
			this.cbActiveProject.Location = new System.Drawing.Point(12, 271);
			this.cbActiveProject.Name = "cbActiveProject";
			this.cbActiveProject.Size = new System.Drawing.Size(92, 17);
			this.cbActiveProject.TabIndex = 16;
			this.cbActiveProject.Text = "Active Project";
			this.cbActiveProject.UseVisualStyleBackColor = true;
			// 
			// cbSyncChangelistToBuild
			// 
			this.cbSyncChangelistToBuild.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.cbSyncChangelistToBuild.AutoSize = true;
			this.cbSyncChangelistToBuild.Location = new System.Drawing.Point(110, 271);
			this.cbSyncChangelistToBuild.Name = "cbSyncChangelistToBuild";
			this.cbSyncChangelistToBuild.Size = new System.Drawing.Size(140, 17);
			this.cbSyncChangelistToBuild.TabIndex = 17;
			this.cbSyncChangelistToBuild.Text = "Sync Changelist to Build";
			this.cbSyncChangelistToBuild.UseVisualStyleBackColor = true;
			// 
			// bOK
			// 
			this.bOK.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.bOK.Location = new System.Drawing.Point(265, 265);
			this.bOK.Name = "bOK";
			this.bOK.Size = new System.Drawing.Size(75, 23);
			this.bOK.TabIndex = 18;
			this.bOK.Text = "Ok";
			this.bOK.UseVisualStyleBackColor = true;
			this.bOK.Click += new System.EventHandler(this.bOK_Click);
			// 
			// bCancel
			// 
			this.bCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.bCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.bCancel.Location = new System.Drawing.Point(346, 265);
			this.bCancel.Name = "bCancel";
			this.bCancel.Size = new System.Drawing.Size(75, 23);
			this.bCancel.TabIndex = 19;
			this.bCancel.Text = "Cancel";
			this.bCancel.UseVisualStyleBackColor = true;
			this.bCancel.Click += new System.EventHandler(this.bCancel_Click);
			// 
			// EditProfileForm
			// 
			this.AcceptButton = this.bOK;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.bCancel;
			this.ClientSize = new System.Drawing.Size(433, 300);
			this.Controls.Add(this.bCancel);
			this.Controls.Add(this.bOK);
			this.Controls.Add(this.cbSyncChangelistToBuild);
			this.Controls.Add(this.cbActiveProject);
			this.Controls.Add(this.tbProjectPath);
			this.Controls.Add(this.bProjectPathBrowse);
			this.Controls.Add(this.lblProjectPath);
			this.Controls.Add(this.tbEnginePath);
			this.Controls.Add(this.bEnginePathBrowse);
			this.Controls.Add(this.lblEnginePath);
			this.Controls.Add(this.tbPerforcePort);
			this.Controls.Add(this.lblPerforcePort);
			this.Controls.Add(this.tbPullFromPath);
			this.Controls.Add(this.lblPullFromPath);
			this.Controls.Add(this.tbDeployToDir);
			this.Controls.Add(this.lblDeployToDir);
			this.Controls.Add(this.tbWorkspaceOverride);
			this.Controls.Add(this.lblWorkspaceOverride);
			this.Controls.Add(this.tbProfileName);
			this.Controls.Add(this.lblProfileName);
			this.Name = "EditProfileForm";
			this.Text = "Project Settings";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label lblProfileName;
		private System.Windows.Forms.TextBox tbProfileName;
		private System.Windows.Forms.TextBox tbWorkspaceOverride;
		private System.Windows.Forms.Label lblWorkspaceOverride;
		private System.Windows.Forms.TextBox tbDeployToDir;
		private System.Windows.Forms.Label lblDeployToDir;
		private System.Windows.Forms.TextBox tbPullFromPath;
		private System.Windows.Forms.Label lblPullFromPath;
		private System.Windows.Forms.TextBox tbPerforcePort;
		private System.Windows.Forms.Label lblPerforcePort;
		private System.Windows.Forms.Label lblEnginePath;
		private System.Windows.Forms.Button bEnginePathBrowse;
		private System.Windows.Forms.TextBox tbEnginePath;
		private System.Windows.Forms.TextBox tbProjectPath;
		private System.Windows.Forms.Button bProjectPathBrowse;
		private System.Windows.Forms.Label lblProjectPath;
		private System.Windows.Forms.CheckBox cbActiveProject;
		private System.Windows.Forms.CheckBox cbSyncChangelistToBuild;
		private System.Windows.Forms.Button bOK;
		private System.Windows.Forms.Button bCancel;
	}
}