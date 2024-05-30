//------------------------------------------------------------------------------
// \file Program.cs - 30 Apr 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System;
using System.Windows.Forms;

namespace Signal.Config.ProjectSelector
{
	static class Program
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main( string[] args )
		{
			for( int iArg = 0; iArg < args.Length; ++iArg )
			{
				switch( args[ iArg ].ToLowerInvariant( ) )
				{
				case "-setcurrentproject":
					if( iArg+1 < args.Length )
						SetCurrentProject( args[ iArg+1 ] );
					else
						SetCurrentProjectUsage( );
					return;
				case "-externalinstall":
					ExternalInstall( );
					return;
				}
			}
			RunNormally( );
		}

		static void SetCurrentProject( string profile )
		{
			Profiles.ActiveProfileName = profile;
		}

		static void SetCurrentProjectUsage( )
		{
			Console.WriteLine( "Usage: -setCurrentProject [profile name]" );
		}

		static void ExternalInstall( )
		{
			MessageBox.Show( "-externalInstall not yet implemented!", "Missing functionality", MessageBoxButtons.OK, MessageBoxIcon.Error );
			Environment.Exit( 1 );
		}

		static void RunNormally( )
		{
			Application.EnableVisualStyles( );
			Application.SetCompatibleTextRenderingDefault( false );
			Application.Run( new ProjectSelectorForm( ) );
		}
	}
}
