//------------------------------------------------------------------------------
// \file RollBackForm.cs - 30 Apr 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using Signal.Config.ProjectSelector.Properties;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Windows.Forms;

namespace Signal.Config.ProjectSelector
{
	public partial class RollBackBuildForm : Form
	{
		readonly string BuildsPath;

		public RollBackBuildForm( )
		{
			InitializeComponent( );
		}

		public RollBackBuildForm( string buildsPath )
		{
			InitializeComponent( );
			BuildsPath = buildsPath;
		}

		private void RollBackBuildForm_Load( object sender, EventArgs e )
		{
			if( !RefreshBuilds( ) )
				Close( );
		}

		static readonly Dictionary<bool,Bitmap> ActiveIcons = new Dictionary<bool,Bitmap>( )
		{
			{ false,	Resources.NotActive	},
			{ true,		Resources.Active	},
		};

		IEnumerable<string> AvailableBuilds
		{
			get { return Directory.GetDirectories( BuildsPath ).Select( dir => Path.GetFileName(dir) ); }
		}

		string LatestBuildTxtPath
		{
			get { return Path.Combine( BuildsPath, "latest_build.txt" ); }
		}

		string LatestBuild
		{
			get { return File.ReadAllText( LatestBuildTxtPath ).Trim( ); }
			set { File.WriteAllText( LatestBuildTxtPath, value ); }
		}

		bool RefreshBuilds( )
		{
			try
			{
				var latestBuild = LatestBuild;

				dgvExistingBuilds.Rows.Clear( );
				foreach( var build in AvailableBuilds )
					dgvExistingBuilds.Rows.Add( ActiveIcons[ build == latestBuild ], build );

				return true;
			}
			catch( UnauthorizedAccessException uae )	{ NotifyBadPullPath( uae ); }
			catch( DirectoryNotFoundException dnfe )	{ NotifyBadPullPath( dnfe ); }
			catch( FileNotFoundException fnfe )			{ NotifyBadPullPath( fnfe ); }
			catch( IOException ie )						{ NotifyBadPullPath( ie ); }

			return false;
		}

		void NotifyBadPullPath( Exception e )
		{
			MessageBox.Show
				( this
				, string.Format( "{0}.  Your Pull From path \"{1}\" may be invalid.", e.GetType( ).Name, BuildsPath )
				, "Can't roll back build"
				, MessageBoxButtons.OK
				, MessageBoxIcon.Error
				);
		}

		void NotifyRollbackFailed( Exception e )
		{
			MessageBox.Show
				( this
				, string.Format( "{0}.  The share path \"{1}\" may not be writable.", e.GetType( ).Name, LatestBuildTxtPath )
				, "Can't roll back build"
				, MessageBoxButtons.OK
				, MessageBoxIcon.Error
				);
		}

		private void bClose_Click( object sender, System.EventArgs e )
		{
			Close( );
		}

		private void dgvExistingBuilds_CellDoubleClick( object sender, DataGridViewCellEventArgs e )
		{
			try
			{
				LatestBuild = dgvExistingBuilds.Rows[ e.RowIndex ].Cells[ colBuildName.Index ].Value as string;
			}
			catch( UnauthorizedAccessException uae )	{ NotifyRollbackFailed( uae ); }
			catch( DirectoryNotFoundException dnfe )	{ NotifyRollbackFailed( dnfe ); }
			catch( IOException ie )						{ NotifyRollbackFailed( ie ); }

			RefreshBuilds( );
		}
	}
}
