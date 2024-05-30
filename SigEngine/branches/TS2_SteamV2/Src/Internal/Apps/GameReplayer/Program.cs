using System;
using System.IO;
using System.Xml;
using XDevkit;
using System.Collections.Generic;
using System.Diagnostics;

namespace GameReplayer
{
	public class CmdLine
	{
		Arg[] Args { get { return mArgs; } }

		public class Arg
		{
			public string Name { get; private set; }
			public string StringValue { get; private set; }

			public int IntValue( int onFail )
			{
				int value;
				if( int.TryParse( StringValue, out value ) )
					return value;

				return onFail;
			}

			public float FloatValue( float onFail )
			{
				float value;
				if( float.TryParse( StringValue, out value ) )
					return value;

				return onFail;
			}

			public Arg( string name, string value )
			{
				Name = name;
				StringValue = value;
			}
		}

		public CmdLine( string cmdLine )
			: this( cmdLine.Split( ' ' ) )
		{

		}

		public CmdLine( string[] cmds )
		{
			var args = new List<Arg>( );
			for( int c = 0; c < cmds.Length; ++c )
			{
				// Is this a command?
				if( cmds[ c ][ 0 ] != '-' )
					continue;

				string cmd = cmds[c].Substring( 1 );
				string argValue = null;
				if( c < cmds.Length - 1 && cmds[ c + 1 ][ 0 ] != '-' )
					argValue = cmds[ ++c ];

				args.Add( new Arg( cmd, argValue ) );
			}

			mArgs = args.ToArray( );
		}

		public Arg FindArg( string name )
		{
			foreach( Arg arg in mArgs )
			{
				if( String.Compare(arg.Name, name, true ) == 0 )
					return arg;
			}

			return null;
		}

		private Arg[] mArgs;
	}

	/// <summary>
	/// Represents a set of replay files and associated replay data
	/// </summary>
	class Replay
	{
		public int ConsoleCount { get { return mFiles.Count; } }
		public string SessionId { get { return mFiles[0].SessionId;} }

		public static Replay[] BuildAll( List<ReplayFile> files )
		{
			var replaySets = new Dictionary<string, List<ReplayFile>>( );
			foreach( ReplayFile file in files )
			{
				if( !replaySets.ContainsKey( file.SessionId ) )
					replaySets[ file.SessionId ] = new List<ReplayFile>( );

				replaySets[ file.SessionId ].Add( file );
			}

			var replays = new List<Replay>( ); 
			foreach( var kv in replaySets )
			{
				bool execute = true;

				int? consoleCount = null;
				foreach( ReplayFile file in kv.Value )
				{
					var cmdLine = new CmdLine( file.GetCmdLine( ) );
					CmdLine.Arg arg = cmdLine.FindArg( "machineCount" );
					if( arg == null )
					{
						Console.WriteLine( "Skipping replay ({0}): Invalid cmd line args", file.SessionId );
						execute = false;
						break;
					}

					if( consoleCount == null )
					{
						consoleCount = arg.IntValue( 0 );
						if( consoleCount == 0 )
						{
							Console.WriteLine( "Skipping replay ({0}): Invalid machine count", file.SessionId );
							execute = false;
							break;
						}
					}
					else if( consoleCount != arg.IntValue( 0 ) )
					{
						Console.WriteLine( "Skipping replay ({0}): Inconsistent machine count", file.SessionId );
						execute = false;
						break;
					}
				}

				if( execute && consoleCount != kv.Value.Count )
				{
					Console.WriteLine( "Skipping replay ({0}): Not enough files for machine count", kv.Value[ 0 ].SessionId );
					execute = false;
				}

				if( execute )
					replays.Add( new Replay( kv.Value ) );
			}

			return replays.ToArray( );
		}

		public void Execute( string root, IXboxConsole[] consoles )
		{
			if( consoles.Length != mFiles.Count )
				throw new ArgumentException( "Console count is incorrect" );

			Console.WriteLine( "Executing replay for session {0}:", SessionId );

			try
			{
				// Copy
				for( int i = 0; i < consoles.Length; ++i )
				{
					string targetPath = Path.Combine( root, mFiles[ i ].FileName );
					Console.WriteLine( "\t{0} >> {1}:{2}", mFiles[ i ].FilePath, consoles[ i ].Name, targetPath );

					consoles[ i ].SendFile( mFiles[ i ].FilePath, targetPath );

					// Crack the sync file and send it as well if available
					CmdLine cmdLine = new CmdLine( mFiles[ i ].GetCmdLine( ) );
					CmdLine.Arg arg = cmdLine.FindArg( "syncFile" );
					if( arg != null )
					{
						string syncFileName = Path.GetFileName( arg.StringValue );
						targetPath = Path.Combine( root, syncFileName );

						string sourcePath = Path.Combine( Path.GetDirectoryName( mFiles[ i ].FilePath ), syncFileName );
						consoles[ i ].SendFile( sourcePath, targetPath );
					}
				}

				// Execute
				for( int i = 0; i < consoles.Length; ++i )
				{
					// Do a copy of the resources and exe
					Process process = new Process( );
					process.StartInfo.UseShellExecute = false;
					process.StartInfo.CreateNoWindow = true;
					process.StartInfo.RedirectStandardOutput = true;
					process.StartInfo.FileName = "GameLauncher.exe";
					process.StartInfo.Arguments = 
						"-q -x " + consoles[ i ].Name +
						" -internal -platform xbox360 -nowatson -options \"-replay " + 
						Path.Combine("game:\\", mFiles[ i ].FileName ) + "\"";

					Program.XboxMgr.DefaultConsole = consoles[ i ].Name;

					Console.WriteLine( "\tStarting {0}", consoles[ i ].Name );
					process.Start( );

					string output = process.StandardOutput.ReadToEnd( );
					process.WaitForExit( );

					Console.WriteLine( "\t\tGameLauncher output - {0}", output );
				}

			}
			catch( System.Runtime.InteropServices.COMException e )
			{
				Console.WriteLine( "Error executing replay: {0}", Program.XboxMgr.TranslateError( e.ErrorCode ) );
			}
		}

		private Replay( List<ReplayFile> files )
		{
			mFiles = files;
		}

		
		private List<ReplayFile> mFiles;
	}

	/// <summary>
	/// Represents a replay file
	/// </summary>
	class ReplayFile
	{
		public string FilePath { get; private set; }
		public string FileName { get { return Path.GetFileName( FilePath ); } }

		public string SessionId { get; private set; }
		public string UserId { get; private set; }

		public static ReplayFile Build( string fullPath )
		{
			string name = Path.GetFileNameWithoutExtension( fullPath );
			string[] tags = name.Split( new char[] { '_' } );

			if( tags.Length != 3 )
				return null;

			if( tags[ 0 ] != "r" )
				return null;

			return new ReplayFile( fullPath, tags[ 1 ], tags[ 2 ] );
		}

		public string GetCmdLine( )
		{
			BinaryReader reader = new BinaryReader( File.OpenRead( FilePath ) );
			
			int length = reader.ReadInt32( );
			string cmdLine = System.Text.ASCIIEncoding.ASCII.GetString( reader.ReadBytes( length ) );
			reader.Close( );

			return cmdLine;
		}

		private ReplayFile( string filePath, string sessionId, string userId )
		{
			FilePath = filePath;
			SessionId = sessionId;
			UserId = userId;
		}
	}

    class Program
    {
        const string gProjectEnvVarName = "SigCurrentProject";
		const string gProjectNameEnvVarName = "SigCurrentProjectName";
        const string gConfigFileName = "Test/Replay.config";
		const string gReplayFolderName = "Test/Replays";
		const string gReplayFileExt = ".rply";
		const string gSyncFileExt = ".sync";

		
        static IXboxManager mXboxMgr = new XboxManagerClass();
		static string mProjectDir = null;
		static string mProjectName = null;
		static string mLocalReplayFolder = null;
		static string mRemoteReplayFolder = null;
		static List<IXboxConsole> mSourceConsoles = null;
		static List<IXboxConsole> mTargetConsoles = null;
		static int mStartTime;
		static int mRunTime;

		public static IXboxManager XboxMgr { get { return mXboxMgr; } }

		static void Main( string[] args )
		{
			// This will result in one iteration
			mStartTime = System.Environment.TickCount;
			mRunTime = 0;

			// Command line args
			{
				CmdLine cmdLine = new CmdLine( args );

				CmdLine.Arg hrsArg = cmdLine.FindArg( "hrsToRun" );
				if( hrsArg != null )
				{
					float hrsTime = hrsArg.FloatValue( 0 );
					mRunTime = (int)( hrsTime * 60 * 60 * 1000 );

					Console.WriteLine( "Info: Found cmd line option hrsToRun {0}", hrsTime );
				}
			}

			// Gather environment variables
			if( !GetProjectVars( ) )
			{
				WaitForExit( );
				return;
			}

			// Parse the config file
			if( !ParseConfigFile( ) )
			{
				WaitForExit( );
				return;
			}

			// Set up our paths
			mLocalReplayFolder = Path.Combine( mProjectDir, gReplayFolderName );
			mRemoteReplayFolder = Path.Combine( "Devkit:\\", mProjectName );
			Console.WriteLine( "Info: Local replay folder set to \"{0}\"", mLocalReplayFolder );
			Console.WriteLine( "Info: Remote replay folder set to \"{0}\"", mRemoteReplayFolder );

			// Create our local replay folder for copying replays into
			if( !CreateReplayFolder( ) )
			{
				WaitForExit( );
				return;
			}

			// Loop forever 
			while( true )
			{
				// Copy new replay files from the console to the replay folder
				AbsorbReplayFiles( );

				// Execute replays
				IXboxConsole[] used = ExecuteReplays( );

				// Clean replay files from target consoles used
				CleanConsoles( used );

				// Handle run time
				int tickCount = System.Environment.TickCount;
				if( tickCount > mStartTime )
				{
					if( tickCount - mStartTime > mRunTime )
						break;
				}
				else if (tickCount < mStartTime )
				{
					if( ( int.MaxValue - mStartTime ) + (tickCount - int.MinValue ) > mRunTime )
						break;
				}
			}
		}

		static void CleanConsoles( IXboxConsole[] consoles )
		{
			Console.WriteLine( "Info: Deleting replay files from consoles:" );

			var files = new List<IXboxFile>( );
			foreach( IXboxConsole console in consoles )
			{
				// Wait for it to come back to us
				while( !WaitForConsole( console, 0 ) ) ;

				GatherRemoteFiles( console, mRemoteReplayFolder, files, gReplayFileExt );
				GatherRemoteFiles( console, mRemoteReplayFolder, files, gSyncFileExt );

				Console.WriteLine( "\tConsole {0} has {1} files:", console.Name, files.Count );

				try
				{
					foreach( IXboxFile file in files )
					{
						console.DeleteFile( file.Name );
						Console.WriteLine("\t\t{0} >> Deleted", file.Name);
					}
				}
				catch
				{
					// Do nothing
				}
			}
		}

		/// <summary>
		/// Returns a list of consoles used in replays
		/// </summary>
		static IXboxConsole[] ExecuteReplays( )
		{
			var files = new List<ReplayFile>( );
			var consolesUsed = new List<IXboxConsole>( );

			// Grab the replay files
			GatherLocalReplayFiles( mLocalReplayFolder, files );

			Console.WriteLine( "Info: Executing replays" );

			// Build and execute the replays
			foreach( Replay replay in Replay.BuildAll( files ) )
			{
				var consolesToUse = new List<IXboxConsole>( );

				// Work till we finish all the replays
				while( consolesToUse.Count < replay.ConsoleCount )
				{
					// Find available consoles for the replay
					foreach( IXboxConsole console in mTargetConsoles )
					{
						// We already have it
						if( consolesToUse.Contains( console ) )
							continue;

						// Test/Wait for it to become available
						if( !WaitForConsole( console ) )
							continue;

						// Add the console to our use List
						consolesToUse.Add( console );

						// We have em' all!
						if( consolesToUse.Count == replay.ConsoleCount )
							break;
					}
				}

				// Tag these consoles as used
				foreach( IXboxConsole c in consolesToUse )
				{
					if( !consolesUsed.Contains( c ) )
						consolesUsed.Add( c );
				}

				replay.Execute( mRemoteReplayFolder, consolesToUse.ToArray( ) );

				// Give enough time for the consoles to reboot out of the shell
				System.Threading.Thread.Sleep( 1000 );
			}

			return consolesUsed.ToArray( );
		}

		static void AbsorbReplayFiles( )
		{
			Console.WriteLine( "Info: Absorbing replay files" );

			// Gather new replay files
			var files = new List<IXboxFile>( );

			// Look for replay files on the source consoles
			foreach( IXboxConsole console in mSourceConsoles )
			{
				if( !ConsoleIsAvailable( console ) )
				{
					Console.WriteLine( "Info: Console ({0}) was not available, no files copied", console.Name );
					continue;
				}

				GatherRemoteFiles( console, mRemoteReplayFolder, files, gReplayFileExt );
				GatherRemoteFiles( console, mRemoteReplayFolder, files, gSyncFileExt );

				Console.WriteLine( "Absorbing {0} files from console \"{1}\"", files.Count, console.Name );

				// Download and delete all the new files
				foreach( IXboxFile file in files )
				{
					string fileName = Path.GetFileName( file.Name );

					try
					{
						string targetPath = Path.Combine( mLocalReplayFolder, fileName );
						Console.WriteLine( "Info: ({0}) << ({1})", targetPath, file.Name );
						console.ReceiveFile( targetPath, file.Name );

						// Try to delete the file off the console
						if( !console.GetFileObject( file.Name ).IsReadOnly )
							console.DeleteFile( file.Name );						
					}
					catch( System.Runtime.InteropServices.COMException e )
					{
						Console.WriteLine(
							"Absorb Error: File ({0}) from console ({1}): {2}",
							file.Name,
							console.Name,
							mXboxMgr.TranslateError( e.ErrorCode ) );
					}
					catch( Exception e )
					{
						Console.WriteLine( "Absorb Error: Unrecognized exception: {0}", e.Message );
					}
				}

				files.Clear( );
			}
		}

		static bool CreateReplayFolder( )
		{
			try
			{
				if( !Directory.Exists( mLocalReplayFolder ) )
				{
					Console.Write( "Info: Creating local replay folder" );
					Directory.CreateDirectory( mLocalReplayFolder );
				}
			}
			catch( Exception e )
			{
				Console.Write( "Error creating local replay folder ({0}): {1}", mLocalReplayFolder, e.Message );
				return false;
			}

			return true;
		}

		static bool ParseConfigFile( )
		{
			// Config file
			try
			{
				Console.WriteLine( "Info: Parsing config file" );
				string configPath = Path.Combine( mProjectDir, gConfigFileName );

				XmlDocument config = new XmlDocument( );
				config.Load( configPath );

				// Gather source consoles
				mSourceConsoles = BuildConsoleList( config, "Sources" );
				if( mSourceConsoles == null )
				{
					Console.WriteLine( "Config error: No valid Source consoles" );
					return false;
				}

				Console.WriteLine( "Source Consoles: " );
				foreach( IXboxConsole console in mSourceConsoles )
					Console.WriteLine( "\t{0}", console.Name );

				// Gather target machines
				mTargetConsoles = BuildConsoleList( config, "Targets" );
				if( mTargetConsoles == null )
				{
					Console.WriteLine( "Config error: No valid Target consoles" );
					return false;
				}

				Console.WriteLine( "Target Consoles: " );
				foreach( IXboxConsole console in mTargetConsoles )
					Console.WriteLine( "\t{0}", console.Name );

				// Set dump settings
				foreach( IXboxConsole console in mTargetConsoles )
				{
					XBOX_DUMP_SETTINGS ds = new XBOX_DUMP_SETTINGS();
					ds.Flags = XboxDumpReportFlags.AlwaysReport | XboxDumpReportFlags.FormatFullHeap | XboxDumpReportFlags.LocalDestination;

					console.SetDumpSettings( ref ds );
				}
			}
			catch( Exception e )
			{
				Console.WriteLine( "Error parsing config file: {0}", e.Message );
				return false;
			}

			return true;
		}

		static bool GetProjectVars( )
		{
			try
			{
				Console.WriteLine( "Info: Pulling project environment vars" );

				// Current project
				mProjectDir = System.Environment.GetEnvironmentVariable( gProjectEnvVarName );
				if( mProjectDir == null )
				{
					Console.WriteLine( "Error: Could not find project EnvVar" );
					return false;
				}
				Console.WriteLine( "Info: Got project directory ({0})", mProjectDir );

				// Current project name
				mProjectName = System.Environment.GetEnvironmentVariable( gProjectNameEnvVarName );
				if( mProjectName == null )
				{
					Console.WriteLine( "Error: Could not find project name EnvVar" );
					return false;
				}
				Console.WriteLine( "Info: Got project name ({0})", mProjectName );
			}
			catch( Exception e )
			{
				Console.WriteLine( "Error accessing EnvVar: {0}", e.Message );
				return false;
			}

			return true;
		}

		static bool WaitForConsole( IXboxConsole console )
		{
			return WaitForConsole( console, 5 );
		}

		static bool WaitForConsole( IXboxConsole console, int attempts )
		{
			do
			{
				--attempts;

				if( ConsoleIsAvailable( console ) )
					return true;

				System.Threading.Thread.Sleep( 1000 );

			} while( attempts != 0 );

			return false;
		}

		static bool ConsoleIsAvailable( IXboxConsole console )
		{
			return ConsoleIsAvailable( console, true );
		}

		static bool ConsoleIsAvailable( IXboxConsole console, bool testShell )
		{
			try
			{
				console.FindConsole( 0, 0 );

				if( testShell && Path.GetFileNameWithoutExtension( console.RunningProcessInfo.ProgramName ) != "xshell" )
					return false;

				return true;
			}
			catch
			{
				return false;
			}
		}

		/// <summary>
		/// Builds a list of machine names for a list name and node name
		/// </summary>
		static List<IXboxConsole> BuildConsoleList( XmlDocument config, string listName )
		{
			// List nodes
			XmlNodeList lists = config.GetElementsByTagName( listName );
			if( lists.Count == 0 )
			{
				Console.WriteLine( "Config error: Missing {0}", listName );
				return null;
			}

			// Build the source xbox names
			List<IXboxConsole> consoles = null;
			foreach( XmlNode consoleList in lists )
			{
				foreach( XmlNode consoleNode in consoleList.ChildNodes )
				{
					// Unrecognized node name
					if( consoleNode.Name != "Console" )
					{
						Console.WriteLine( "Config error: Child of {0} node not named \"Console\" - skipping", listName );
						continue;
					}

					string name = null;
					foreach( XmlAttribute attr in consoleNode.Attributes )
					{
						// Unrecognized attribute name
						if( String.Compare( attr.Name, "name", true ) != 0 )
						{
							Console.WriteLine( "Config error: Unrecognized attribute on Console node: {0}", attr.Name );
							continue;
						}

						// Too short
						if( attr.Value.Length == 0 )
						{
							Console.WriteLine( "Config error: Console node has empty \"name\" attribute" );
							break;
						}

						name = attr.Value;
						break;
					}

					// No name value
					if( name == null )
					{
						Console.Write( "Config error: Console node has no \"name\" attribute" );
						break;
					}

					try
					{
						IXboxConsole console = mXboxMgr.OpenConsole( name );

						if( consoles == null )
							consoles = new List<IXboxConsole>( );

						consoles.Add( console );
					}
					catch( System.Runtime.InteropServices.COMException e )
					{
						Console.WriteLine( "Console Error: Attempt to find \"{0}\" returned: {1}", name, mXboxMgr.TranslateError( e.ErrorCode ) );
					}
					catch( Exception e )
					{
						Console.WriteLine( "Console Error: Unknown exception: {0}", e.Message );
					}
				}
			}

			return consoles;
		}

		/// <summary>
		/// Gather replay files for the console
		/// </summary>
		static void GatherRemoteFiles( IXboxConsole console, string folderName, List<IXboxFile> files, string extFilter )
		{
			try
			{
				foreach( IXboxFile file in console.DirectoryFiles( folderName ) )
				{
					// We expect the files to be in the base directory
					if( file.IsDirectory )
						continue;

					if( Path.GetExtension( file.Name ) != extFilter )
						continue;

					files.Add( file );
				}

			}
			catch( System.Runtime.InteropServices.COMException e )
			{
				Console.WriteLine( "Source File Error: {0}", mXboxMgr.TranslateError( e.ErrorCode ) );
			}
			catch( Exception e )
			{
				Console.WriteLine( "Unknown exception gathering files: {0}", e.Message );
			}
		}

		static void GatherLocalReplayFiles( string folderName, List<ReplayFile> files )
		{
			try
			{
				string[] fileNames = Directory.GetFiles( folderName );
				foreach( string file in fileNames )
				{
					if( Path.GetExtension( file ) != gReplayFileExt )
						continue;

					ReplayFile replay = ReplayFile.Build( file );
					if( replay == null )
						continue;

					files.Add( replay );
				}
			}
			catch( Exception e )
			{
				Console.WriteLine( "Error gathering local files ({0}): {1}", folderName, e.Message );
			}
		}

		/// <summary>
		/// Simply waits for the enter key
		/// </summary>
		static void WaitForExit( )
		{
			Console.WriteLine( "Press enter key to exit" );
			Console.ReadLine( );
		}
    }
}
