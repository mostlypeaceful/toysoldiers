//------------------------------------------------------------------------------
// \file EditProfileForm.cs - 30 Apr 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System;
using System.Windows.Forms;

namespace Signal.Config.ProjectSelector
{
	/// <summary>
	/// Edits a project profile on your computer.
	/// </summary>
	partial class EditProfileForm : Form
	{
		readonly ProfileData ProfileData;

		public EditProfileForm( ): this( "", new ProfileData( ) ) { }
		public EditProfileForm( string name, ProfileData profile )
		{
			InitializeComponent( );
			ProfileData = profile;

			tbProfileName.Text					= name;
			tbWorkspaceOverride.Text			= profile.WorkSpaceOverride;
			tbDeployToDir.Text					= profile.DeployToDir;
			tbPullFromPath.Text					= profile.PullFromPath;
			tbPerforcePort.Text					= profile.PerforcePort;
			tbEnginePath.Text					= profile.EnginePath;
			tbProjectPath.Text					= profile.ProjectPath;
			cbActiveProject.Checked				= name == Profiles.ActiveProfileName;
			cbSyncChangelistToBuild.Checked		= profile.SyncChangelist;
		}

		void ShowFolderBrowser( TextBox textbox, string description )
		{
			using( var dialog = new FolderBrowserDialog( )
			{
				Description			= description,
				RootFolder			= Environment.SpecialFolder.MyComputer,
				ShowNewFolderButton	= true,
			})
			{
				var result = dialog.ShowDialog( this );
				if( result == DialogResult.OK )
					textbox.Text = dialog.SelectedPath;
			}
		}

		private void bEnginePathBrowse_Click( object sender, EventArgs e )
		{
			ShowFolderBrowser( tbEnginePath, "Select your SigEngine path..." );
		}

		private void bProjectPathBrowse_Click( object sender, EventArgs e )
		{
			ShowFolderBrowser( tbProjectPath, "Select your Project path..." );
		}

		private void bOK_Click( object sender, EventArgs e )
		{
			DialogResult = DialogResult.OK;

			var name	= tbProfileName.Text;
			var profile	= ProfileData;

			profile.WorkSpaceOverride	= tbWorkspaceOverride.Text;
			profile.DeployToDir			= tbDeployToDir.Text;
			profile.PullFromPath		= tbPullFromPath.Text;
			profile.PerforcePort		= tbPerforcePort.Text;
			profile.EnginePath			= tbEnginePath.Text;
			profile.ProjectPath			= tbProjectPath.Text;
			if( cbActiveProject.Checked )
				Profiles.ActiveProfileName	= name;
			profile.SyncChangelist		= cbSyncChangelistToBuild.Checked;

			Profiles.Save( name, profile );

			Close( );
		}

		private void bCancel_Click( object sender, EventArgs e )
		{
			DialogResult = DialogResult.Cancel;
			Close( );
		}
	}
}
