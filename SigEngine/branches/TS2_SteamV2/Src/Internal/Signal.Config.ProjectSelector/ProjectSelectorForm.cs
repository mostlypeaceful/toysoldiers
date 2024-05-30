//------------------------------------------------------------------------------
// \file ProjectSelectorForm.cs - 30 Apr 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using Signal.Config.ProjectSelector.Properties;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using ProfileEntry = System.Collections.Generic.KeyValuePair<string, Signal.Config.ProjectSelector.ProfileData>;

namespace Signal.Config.ProjectSelector
{
	/// <summary>
	/// Displays a list of project profiles on your computer.
	/// </summary>
	public partial class ProjectSelectorForm : Form
	{
		static readonly Dictionary<bool,Bitmap> ActiveIcons = new Dictionary<bool,Bitmap>( )
		{
			{ false,	Resources.NotActive	},
			{ true,		Resources.Active	},
		};

		// Associate Profiles with DataGridViewRow s
		static ProfileEntry ProfileOf( DataGridViewRow dgvr ) { return (ProfileEntry)dgvr.Tag; }
		static void SetProfileOf( DataGridViewRow dgvr, ProfileEntry entry ) { dgvr.Tag = entry; }

		DataGridViewRow ContextMenuRow;
		ProfileEntry ContextMenuProfileEntry
		{
			get { return ProfileOf( ContextMenuRow ); }
			set { SetProfileOf( ContextMenuRow, value ); }
		}


		public ProjectSelectorForm( )
		{
			InitializeComponent( );
			RefreshProfiles( );
		}

		void RefreshProfiles( )
		{
			var activeProfileName = Profiles.ActiveProfileName;
			var selectedProfileNames
				= dgvProfiles
				.SelectedRows
				.Cast<DataGridViewRow>()
				.Select( row => ProfileOf(row).Key )
				.ToArray( )
				;

			dgvProfiles.Rows.Clear( );
			foreach( var profile in Profiles.LoadAll( ) )
			{
				var name = profile.Key;
				var data = profile.Value;

				dgvProfiles.Rows.Add( name == activeProfileName ? Resources.Active : Resources.NotActive, name, data.EnginePath, data.ProjectPath );
				var row = dgvProfiles.Rows.Cast<DataGridViewRow>().Last();
				SetProfileOf( row, profile );
				row.Selected = selectedProfileNames.Contains( name );
			}

			lblActiveProfileValue.Text = activeProfileName;
		}

		void NewProfile( )
		{
			using( var editProfileForm = new EditProfileForm( ) )
				if( editProfileForm.ShowDialog( ) == DialogResult.OK )
					RefreshProfiles( );
		}

		void EditProfile( ProfileEntry entry )
		{
			using( var editProfileForm = new EditProfileForm( entry.Key, entry.Value ) )
				if( editProfileForm.ShowDialog( ) == DialogResult.OK )
					RefreshProfiles( );
		}

		void RollBack( ProfileEntry entry )
		{
			using( var rollBackBuildForm = new RollBackBuildForm( entry.Value.PullFromPath ) )
				if( rollBackBuildForm.ShowDialog( ) == DialogResult.OK )
					RefreshProfiles( );
		}

		protected override void OnFormClosed( FormClosedEventArgs e )
		{
			foreach( var icon in ActiveIcons.Values ) icon.Dispose( );
			base.OnFormClosed( e );
		}

		private void aboutToolStripMenuItem_Click( object sender, EventArgs e )
		{
			MessageBox.Show( this, "This application allows you to create and edit project profiles, as well as set the active project.", "About ProjectSelector", MessageBoxButtons.OK, MessageBoxIcon.Information );
		}

		private void exitToolStripMenuItem_Click( object sender, EventArgs e )
		{
			Close( );
		}

		private void bNewProfile_Click( object sender, EventArgs e )
		{
			NewProfile( );
		}

		private void dgvProfiles_CellMouseDown( object sender, DataGridViewCellMouseEventArgs e )
		{
			switch( e.Button )
			{
			case MouseButtons.Right:
				var row = dgvProfiles.Rows[ e.RowIndex ];
				row.Selected = true;
				ContextMenuRow = row;
				foreach( var item in new ToolStripItem[] { modifyGameSettingsToolStripMenuItem, compileGameSettingsToolStripMenuItem, activeProjectSeperatorMenuItem } )
					item.Visible = ContextMenuProfileEntry.Key == Profiles.ActiveProfileName;
				break;
			default:
				ContextMenuRow = null;
				break;
			}
		}

		private void dgvProfiles_CellDoubleClick( object sender, DataGridViewCellEventArgs e )
		{
			Profiles.ActiveProfileName = dgvProfiles.Rows[ e.RowIndex ].Cells[ colProfileName.Index ].Value as string;
			RefreshProfiles( );
		}



		private void rollBackToolStripMenuItem_Click( object sender, EventArgs e )
		{
			RollBack( ContextMenuProfileEntry );
		}

		private void setAsActiveProjectToolStripMenuItem_Click( object sender, EventArgs e )
		{
			Profiles.ActiveProfileName = ContextMenuProfileEntry.Key;
		}

		private void newToolStripMenuItem_Click( object sender, EventArgs e )
		{
			NewProfile( );
		}

		private void editToolStripMenuItem_Click( object sender, EventArgs e )
		{
			EditProfile( ContextMenuProfileEntry );
		}

		private void deleteToolStripMenuItem_Click( object sender, EventArgs e )
		{
			if( MessageBox.Show( this, "Are you sure you want to delete that project profile?", "Delete Project Profile?", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1 ) == DialogResult.Yes )
			{
				Profiles.Delete( ContextMenuProfileEntry.Key );
				RefreshProfiles( );
			}
		}

		private void modifyGameSettingsToolStripMenuItem_Click( object sender, EventArgs e )
		{
			var psi = new ProcessStartInfo( Environment.ExpandEnvironmentVariables( @"%SigEngine%\Bin\EditGameFlags.exe" ) );
			var p = Process.Start( psi );
			p.WaitForExit( );
		}

		private void compileGameSettingsToolStripMenuItem_Click( object sender, EventArgs e )
		{
			var psi = new ProcessStartInfo( Environment.ExpandEnvironmentVariables( @"%SigEngine%\Bin\BuildGameFlags.exe" ) );
			var p = Process.Start( psi );
			p.WaitForExit( );
		}
	}
}
