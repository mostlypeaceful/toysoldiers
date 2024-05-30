using System;
using System.Collections.Generic;
using System.Linq;
using System.Windows.Forms;
using Perforce.P4;
using Microsoft.Win32;

namespace ResourceMover
{
	static class Program
	{
		static Server mP4Serv = null;
		static Repository mP4Repo = null;

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main( )
		{
			// Connect to perforce
			{
				string root = "HKEY_CURRENT_USER\\Software\\Perforce\\Environment";
				string uri = (string)Registry.GetValue( root, "P4PORT", null );
				string user = (string)Registry.GetValue( root, "P4USER", null );
				string client = (string)Registry.GetValue( root, "P4CLIENT", null );

				mP4Serv = new Server( new ServerAddress( uri ) );
				mP4Repo = new Repository( mP4Serv );

				Connection conn = mP4Repo.Connection;

				conn.UserName = user;
				conn.Client = new Client( );
				conn.Client.Name = client;

				conn.Connect( null );
			}

			// Start Excel
			ReferenceFixer.LaunchExcel( );

			Application.EnableVisualStyles( );
			Application.SetCompatibleTextRenderingDefault( false );
			Application.Run( new MainForm( mP4Repo ) );

			// Disconnect from perforce
			{
				mP4Repo.Connection.Disconnect( );
			}

			// Quit Excel
			ReferenceFixer.QuitExcel( );
		}
	}
}
