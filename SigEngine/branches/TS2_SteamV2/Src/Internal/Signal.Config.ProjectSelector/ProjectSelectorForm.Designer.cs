//------------------------------------------------------------------------------
// \file ProjectSelectorForm.Designer.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

namespace Signal.Config.ProjectSelector
{
	partial class ProjectSelectorForm
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
			this.components = new System.ComponentModel.Container();
			this.menuStrip = new System.Windows.Forms.MenuStrip();
			this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
			this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.lblExistingProfiles = new System.Windows.Forms.Label();
			this.statusStrip = new System.Windows.Forms.StatusStrip();
			this.lblActiveProfileHeader = new System.Windows.Forms.ToolStripStatusLabel();
			this.lblActiveProfileValue = new System.Windows.Forms.ToolStripStatusLabel();
			this.dgvProfiles = new System.Windows.Forms.DataGridView();
			this.colActive = new System.Windows.Forms.DataGridViewImageColumn();
			this.colProfileName = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.colSigEngine = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.colProjectPath = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.profilesContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
			this.setAsActiveProjectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
			this.rollBackToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
			this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
			this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.bNewProfile = new System.Windows.Forms.Button();
			this.modifyGameSettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.compileGameSettingsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
			this.activeProjectSeperatorMenuItem = new System.Windows.Forms.ToolStripSeparator();
			this.menuStrip.SuspendLayout();
			this.statusStrip.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)(this.dgvProfiles)).BeginInit();
			this.profilesContextMenu.SuspendLayout();
			this.SuspendLayout();
			// 
			// menuStrip
			// 
			this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
			this.menuStrip.Location = new System.Drawing.Point(0, 0);
			this.menuStrip.Name = "menuStrip";
			this.menuStrip.Size = new System.Drawing.Size(665, 24);
			this.menuStrip.TabIndex = 0;
			this.menuStrip.Text = "menuStrip1";
			// 
			// fileToolStripMenuItem
			// 
			this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem,
            this.toolStripMenuItem1,
            this.exitToolStripMenuItem});
			this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
			this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
			this.fileToolStripMenuItem.Text = "&File";
			// 
			// aboutToolStripMenuItem
			// 
			this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
			this.aboutToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
			this.aboutToolStripMenuItem.Text = "&About...";
			this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
			// 
			// toolStripMenuItem1
			// 
			this.toolStripMenuItem1.Name = "toolStripMenuItem1";
			this.toolStripMenuItem1.Size = new System.Drawing.Size(113, 6);
			// 
			// exitToolStripMenuItem
			// 
			this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
			this.exitToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
			this.exitToolStripMenuItem.Text = "E&xit";
			this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
			// 
			// lblExistingProfiles
			// 
			this.lblExistingProfiles.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.lblExistingProfiles.Location = new System.Drawing.Point(12, 27);
			this.lblExistingProfiles.Margin = new System.Windows.Forms.Padding(3);
			this.lblExistingProfiles.Name = "lblExistingProfiles";
			this.lblExistingProfiles.Size = new System.Drawing.Size(641, 13);
			this.lblExistingProfiles.TabIndex = 1;
			this.lblExistingProfiles.Text = "Existing Profiles";
			this.lblExistingProfiles.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// statusStrip
			// 
			this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.lblActiveProfileHeader,
            this.lblActiveProfileValue});
			this.statusStrip.Location = new System.Drawing.Point(0, 363);
			this.statusStrip.Name = "statusStrip";
			this.statusStrip.Size = new System.Drawing.Size(665, 22);
			this.statusStrip.TabIndex = 3;
			this.statusStrip.Text = "statusStrip1";
			// 
			// lblActiveProfileHeader
			// 
			this.lblActiveProfileHeader.Name = "lblActiveProfileHeader";
			this.lblActiveProfileHeader.Size = new System.Drawing.Size(80, 17);
			this.lblActiveProfileHeader.Text = "Active Profile:";
			// 
			// lblActiveProfileValue
			// 
			this.lblActiveProfileValue.Name = "lblActiveProfileValue";
			this.lblActiveProfileValue.Size = new System.Drawing.Size(58, 17);
			this.lblActiveProfileValue.Text = "TS3_DX11";
			// 
			// dgvProfiles
			// 
			this.dgvProfiles.AllowUserToAddRows = false;
			this.dgvProfiles.AllowUserToDeleteRows = false;
			this.dgvProfiles.AllowUserToResizeRows = false;
			this.dgvProfiles.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.dgvProfiles.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
			this.dgvProfiles.BackgroundColor = System.Drawing.SystemColors.Window;
			this.dgvProfiles.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			this.dgvProfiles.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colActive,
            this.colProfileName,
            this.colSigEngine,
            this.colProjectPath});
			this.dgvProfiles.ContextMenuStrip = this.profilesContextMenu;
			this.dgvProfiles.Location = new System.Drawing.Point(15, 46);
			this.dgvProfiles.MultiSelect = false;
			this.dgvProfiles.Name = "dgvProfiles";
			this.dgvProfiles.RowHeadersVisible = false;
			this.dgvProfiles.SelectionMode = System.Windows.Forms.DataGridViewSelectionMode.FullRowSelect;
			this.dgvProfiles.Size = new System.Drawing.Size(641, 285);
			this.dgvProfiles.TabIndex = 5;
			this.dgvProfiles.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.dgvProfiles_CellDoubleClick);
			this.dgvProfiles.CellMouseDown += new System.Windows.Forms.DataGridViewCellMouseEventHandler(this.dgvProfiles_CellMouseDown);
			// 
			// colActive
			// 
			this.colActive.FillWeight = 1E-05F;
			this.colActive.HeaderText = "Active";
			this.colActive.MinimumWidth = 48;
			this.colActive.Name = "colActive";
			this.colActive.ReadOnly = true;
			// 
			// colProfileName
			// 
			this.colProfileName.FillWeight = 20F;
			this.colProfileName.HeaderText = "Name";
			this.colProfileName.Name = "colProfileName";
			this.colProfileName.ReadOnly = true;
			// 
			// colSigEngine
			// 
			this.colSigEngine.FillWeight = 40F;
			this.colSigEngine.HeaderText = "Engine Path";
			this.colSigEngine.Name = "colSigEngine";
			this.colSigEngine.ReadOnly = true;
			// 
			// colProjectPath
			// 
			this.colProjectPath.FillWeight = 40F;
			this.colProjectPath.HeaderText = "Project Path";
			this.colProjectPath.Name = "colProjectPath";
			this.colProjectPath.ReadOnly = true;
			// 
			// profilesContextMenu
			// 
			this.profilesContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.setAsActiveProjectToolStripMenuItem,
            this.toolStripMenuItem2,
            this.rollBackToolStripMenuItem,
            this.activeProjectSeperatorMenuItem,
            this.modifyGameSettingsToolStripMenuItem,
            this.compileGameSettingsToolStripMenuItem,
            this.toolStripMenuItem3,
            this.newToolStripMenuItem,
            this.editToolStripMenuItem,
            this.toolStripMenuItem4,
            this.deleteToolStripMenuItem});
			this.profilesContextMenu.Name = "profilesContextMenu";
			this.profilesContextMenu.Size = new System.Drawing.Size(199, 204);
			// 
			// setAsActiveProjectToolStripMenuItem
			// 
			this.setAsActiveProjectToolStripMenuItem.Name = "setAsActiveProjectToolStripMenuItem";
			this.setAsActiveProjectToolStripMenuItem.Size = new System.Drawing.Size(198, 22);
			this.setAsActiveProjectToolStripMenuItem.Text = "Set as Active Project";
			this.setAsActiveProjectToolStripMenuItem.Click += new System.EventHandler(this.setAsActiveProjectToolStripMenuItem_Click);
			// 
			// toolStripMenuItem2
			// 
			this.toolStripMenuItem2.Name = "toolStripMenuItem2";
			this.toolStripMenuItem2.Size = new System.Drawing.Size(195, 6);
			// 
			// rollBackToolStripMenuItem
			// 
			this.rollBackToolStripMenuItem.Name = "rollBackToolStripMenuItem";
			this.rollBackToolStripMenuItem.Size = new System.Drawing.Size(198, 22);
			this.rollBackToolStripMenuItem.Text = "Roll Back";
			this.rollBackToolStripMenuItem.Click += new System.EventHandler(this.rollBackToolStripMenuItem_Click);
			// 
			// toolStripMenuItem3
			// 
			this.toolStripMenuItem3.Name = "toolStripMenuItem3";
			this.toolStripMenuItem3.Size = new System.Drawing.Size(195, 6);
			// 
			// newToolStripMenuItem
			// 
			this.newToolStripMenuItem.Name = "newToolStripMenuItem";
			this.newToolStripMenuItem.Size = new System.Drawing.Size(198, 22);
			this.newToolStripMenuItem.Text = "New";
			this.newToolStripMenuItem.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
			// 
			// editToolStripMenuItem
			// 
			this.editToolStripMenuItem.Name = "editToolStripMenuItem";
			this.editToolStripMenuItem.Size = new System.Drawing.Size(198, 22);
			this.editToolStripMenuItem.Text = "Edit";
			this.editToolStripMenuItem.Click += new System.EventHandler(this.editToolStripMenuItem_Click);
			// 
			// toolStripMenuItem4
			// 
			this.toolStripMenuItem4.Name = "toolStripMenuItem4";
			this.toolStripMenuItem4.Size = new System.Drawing.Size(195, 6);
			// 
			// deleteToolStripMenuItem
			// 
			this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
			this.deleteToolStripMenuItem.Size = new System.Drawing.Size(198, 22);
			this.deleteToolStripMenuItem.Text = "Delete";
			this.deleteToolStripMenuItem.Click += new System.EventHandler(this.deleteToolStripMenuItem_Click);
			// 
			// bNewProfile
			// 
			this.bNewProfile.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.bNewProfile.Location = new System.Drawing.Point(570, 337);
			this.bNewProfile.Name = "bNewProfile";
			this.bNewProfile.Size = new System.Drawing.Size(86, 23);
			this.bNewProfile.TabIndex = 6;
			this.bNewProfile.Text = "New Profile...";
			this.bNewProfile.UseVisualStyleBackColor = true;
			this.bNewProfile.Click += new System.EventHandler(this.bNewProfile_Click);
			// 
			// modifyGameSettingsToolStripMenuItem
			// 
			this.modifyGameSettingsToolStripMenuItem.Name = "modifyGameSettingsToolStripMenuItem";
			this.modifyGameSettingsToolStripMenuItem.Size = new System.Drawing.Size(198, 22);
			this.modifyGameSettingsToolStripMenuItem.Text = "Modify Game Settings";
			this.modifyGameSettingsToolStripMenuItem.Click += new System.EventHandler(this.modifyGameSettingsToolStripMenuItem_Click);
			// 
			// compileGameSettingsToolStripMenuItem
			// 
			this.compileGameSettingsToolStripMenuItem.Name = "compileGameSettingsToolStripMenuItem";
			this.compileGameSettingsToolStripMenuItem.Size = new System.Drawing.Size(198, 22);
			this.compileGameSettingsToolStripMenuItem.Text = "Compile Game Settings";
			this.compileGameSettingsToolStripMenuItem.Click += new System.EventHandler(this.compileGameSettingsToolStripMenuItem_Click);
			// 
			// activeProjectSeperatorMenuItem
			// 
			this.activeProjectSeperatorMenuItem.Name = "activeProjectSeperatorMenuItem";
			this.activeProjectSeperatorMenuItem.Size = new System.Drawing.Size(195, 6);
			// 
			// ProjectSelectorForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.ClientSize = new System.Drawing.Size(665, 385);
			this.Controls.Add(this.bNewProfile);
			this.Controls.Add(this.dgvProfiles);
			this.Controls.Add(this.statusStrip);
			this.Controls.Add(this.lblExistingProfiles);
			this.Controls.Add(this.menuStrip);
			this.MainMenuStrip = this.menuStrip;
			this.Name = "ProjectSelectorForm";
			this.Text = "SigEngine ProjectSelector";
			this.menuStrip.ResumeLayout(false);
			this.menuStrip.PerformLayout();
			this.statusStrip.ResumeLayout(false);
			this.statusStrip.PerformLayout();
			((System.ComponentModel.ISupportInitialize)(this.dgvProfiles)).EndInit();
			this.profilesContextMenu.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.MenuStrip menuStrip;
		private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
		private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
		private System.Windows.Forms.Label lblExistingProfiles;
		private System.Windows.Forms.StatusStrip statusStrip;
		private System.Windows.Forms.ToolStripStatusLabel lblActiveProfileHeader;
		private System.Windows.Forms.ToolStripStatusLabel lblActiveProfileValue;
		private System.Windows.Forms.DataGridView dgvProfiles;
		private System.Windows.Forms.Button bNewProfile;
		private System.Windows.Forms.DataGridViewImageColumn colActive;
		private System.Windows.Forms.DataGridViewTextBoxColumn colProfileName;
		private System.Windows.Forms.DataGridViewTextBoxColumn colSigEngine;
		private System.Windows.Forms.DataGridViewTextBoxColumn colProjectPath;
		private System.Windows.Forms.ContextMenuStrip profilesContextMenu;
		private System.Windows.Forms.ToolStripMenuItem setAsActiveProjectToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
		private System.Windows.Forms.ToolStripMenuItem rollBackToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
		private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
		private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
		private System.Windows.Forms.ToolStripSeparator activeProjectSeperatorMenuItem;
		private System.Windows.Forms.ToolStripMenuItem modifyGameSettingsToolStripMenuItem;
		private System.Windows.Forms.ToolStripMenuItem compileGameSettingsToolStripMenuItem;
	}
}

