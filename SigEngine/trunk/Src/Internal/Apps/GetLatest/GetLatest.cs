using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using Microsoft.Win32;
using System.Text.RegularExpressions;

namespace GetLatest
{
    public partial class GetLatest : Form
    {
        private string mProjNameVar;
        private string mProjPathVar;
        private string mEngPathVar;
        private string mBuildMachineName;
        private string mLatestBuildPath;
        private string mLatestChangelist;
        private bool mSyncChangelist = false;

        private BackgroundWorker mProcessWorker = new BackgroundWorker( );
        
        private List<Process> mProcessList = new List<Process>( );
        private List<string> mProcessLog = new List<string>( );
        private List<string> mSyncErrors = new List<string>( );
        private List<string> mEXTRAFiles = new List<string>( );
        
        private DateTime mCleanGameLastWriteTime;
        
        private bool mQuickUpdate = false;

        public GetLatest( string[ ] args )
        {
            if( args.Contains( "quick" ) )
                mQuickUpdate = true;

            InitializeComponent( );
        }

        private void fGetRegistryOptions( )
        {
            // Determine if the user has locked their Perforce sync to the latest green build changelist
            RegistryKey projectSettingsKey = fGetRegistryKey( Registry.CurrentUser, @"Software\SignalStudios\ProjectProfiles\" + mProjNameVar );
            if( projectSettingsKey == null )
                throw new Exception( "Could not find project settings for " + mProjNameVar + " in the registry." );
            string syncString = ( string ) projectSettingsKey.GetValue( "SyncChangelist", "0" );
            projectSettingsKey.Close( );
            mSyncChangelist = syncString == "1" ? true : false;
        }

        private delegate void fUpdateOutputLogDelegate( string message );
        private void fUpdateOutputLog( string message )
        {
            if( message == null || message == "" )
                return;

            if( OutputLog.InvokeRequired )
                OutputLog.Invoke( new fUpdateOutputLogDelegate( fUpdateOutputLog ), message );
            else
                OutputLog.AppendText( message );
        }

        private delegate void fShowDialogDelegate( IWin32Window owner );
        private void fBackgroundProcessWorker( object sender, DoWorkEventArgs e )
        {
            fUpdateOutputLog( "\r\n" );

            fGetRegistryOptions( );

            if( mSyncChangelist && mLatestChangelist != null && mLatestChangelist != String.Empty )
                fUpdateOutputLog( "NOTICE: You have enabled synchronization between Perforce changelists and build numbers.\r\n\r\n" );

            fUpdateOutputLog( "====================== Updating Crash Dump Settings...\r\n" );
            fUpdateDumpConfigSettings( );

            fUpdateOutputLog( "====================== Synchronizing SignalShared Install Scripts...\r\n" );
            fSyncSignalShared( );

            fUpdateOutputLog( "====================== Clean Game Required?\r\n" );
            fCleanGame( );

            fUpdateOutputLog( "====================== Synchronizing " + mProjNameVar + " Workspace...\r\n" );
            string arguments = "sync //...";
            if( mSyncChangelist && mLatestChangelist != null && mLatestChangelist != String.Empty )
                arguments += "@" + mLatestChangelist;

            fRunProcessAndDisplayOutput( "p4", arguments, "" );

            fUpdateOutputLog( "====================== Updating timestamps...\r\n" );
            fUpdateOpenedTimestamps( );

            try
            {
                fUpdateOutputLog( "\r\n====================== Copying Converted Assets from Build Server..." );
                fRunProcessAndDisplayOutput( "robocopy", mLatestBuildPath + "Game " + mProjPathVar + @"\Game /S /XO /NS /NP /NDL /NJH /NJS", mProjPathVar );

                // Find files listed as "EXTRA" from Robocopy - this means they exist on the
                // local machine, but not on the server
                foreach( string line in mProcessLog )
                {
                    if( String.IsNullOrEmpty( line ) )
                        continue;

                    Regex pattern = new Regex( @"\s+\*EXTRA File\s+(?<filename>.+)" );
                    string filename = pattern.Match( line ).Groups[ "filename" ].ToString( );
                    if( !String.IsNullOrEmpty( filename ) )
                    {
                        // Skip files in directories which start with a tilde or underscore
                        if( Regex.Match( filename, @".*\\~.*" ).Success || Regex.Match( filename, @".*\\_.*" ).Success )
                            continue;

                        mEXTRAFiles.Add( filename );
                    }
                }

                fUpdateOutputLog( "\r\n====================== Copying Game Bins from Build Server..." );
                fRunProcessAndDisplayOutput( "robocopy", mLatestBuildPath + "Bin " + mProjPathVar + @"\Bin /S /XO /NS /NP /NDL /NJH /NJS", mProjPathVar );

                fUpdateOutputLog( "\r\n====================== Copying Engine Bins from Build Server..." );
                fRunProcessAndDisplayOutput( "robocopy", mLatestBuildPath + "SigEngine\\Bin " + mEngPathVar + @"\Bin /S /XO /NS /NP /NDL /NJH /NJS", mEngPathVar );
                fRunProcessAndDisplayOutput( "robocopy", mLatestBuildPath + @"SigEngine\Src\External\lib " + mEngPathVar + @"\Src\External\lib /S /XO /NS /NP /NDL /NJH /NJS", mEngPathVar );
            }
            catch(Exception ex)
            {
                throw new Exception( fCopyError( ) + "\n\n" + ex.Message + "\n\n" );
            }

            fUpdateOutputLog( "\r\n====================== Building Missing and Outdated Assets...\r\n" );
            if(!mQuickUpdate)
                fRunProcessAndDisplayOutput( "AssetGen.exe", "-np -m -r -pcdx9 -xbox360 -game2xbox -fullsync -log 1", mProjPathVar + @"\Res" );
            else
            {
                fUpdateOutputLog( "\r\nWARNING: Quick Update - No Assets are Being Rebuilt!\r\n\r\n" );
                fRunProcessAndDisplayOutput( "game2xbox.cmd", "", mProjPathVar + @"\Res" );
            }

            fUpdateOutputLog( "\r\n====================== GetLatest is complete!\r\n" );

            string conflicts = fCheckForConflicts( );
            if( conflicts != null && conflicts != "" )
                throw new Exception( conflicts );
        }

        private void fBackgroundProcessWorkerCompleted( object sender, RunWorkerCompletedEventArgs e )
        {
            // If there were errors, display them, otherwise exit application
            if( e.Error == null && mSyncErrors.Count < 1 )
                Application.Exit( );
            else
            {
                string message = "ERROR(S): \r\n";
                if( mSyncErrors.Count > 0 )
                {
                    foreach( string syncError in mSyncErrors )
                        message += syncError + "\r\n";
                }

                if( e != null && e.Error != null )
                    message += e.Error.Message;

                MessageBox.Show( message, "Error!", MessageBoxButtons.OK );
            }
        }

        private void fSyncSignalShared( )
        {
            // Find out the Perforce user and host name
            P4TaggedData data = fGetP4TaggedData( "info" );
            string userName = data[ "userName" ];
            string clientHost = data[ "clientHost" ];
            if( userName.Length < 1 || clientHost.Length < 1 )
            {
                if( userName.Length < 1 )
                    fUpdateOutputLog( "WARNING: Could not find user name.\r\n" );

                if( clientHost.Length < 1 )
                    fUpdateOutputLog( "WARNING: Could not find host name.\r\n" );

                return;
            }

            // Search for the correct SignalShared workspace
            List<string> clientNames = new List<string>( );
            List<string> clientHostNames = new List<string>( );
            int hostClientCount = 0;

            data = fGetP4TaggedData( "clients -u " + userName + " -e *signalshared*" );
            for( int i = 0; i < data.Tags.Count; ++i )
            {
                if( data.Tags[ i ] == "client" )
                    clientNames.Add( data.Values[ i ] );
                else if( data.Tags[ i ] == "Host" )
                {
                    if( data.Values[ i ].ToLower( ) == clientHost )
                        ++hostClientCount;

                    clientHostNames.Add( data.Values[ i ] );
                }
            }
            if( clientNames.Count != clientHostNames.Count )
            {
                fUpdateOutputLog( "WARNING: Client/Host count mismatch: " + clientNames.Count + "/" + clientHostNames.Count + "!\r\n(Not all signalshared workspaces have hostnames?)\r\n" );
                return;
            }

            if( hostClientCount != 1 )
            {
                fUpdateOutputLog( "WARNING: " + hostClientCount + " clients were found!\r\n" );
                return;
            }

            string clientHostInList = clientHostNames.FirstOrDefault( clientName => string.Compare(clientName,clientHost,StringComparison.CurrentCultureIgnoreCase) == 0 );
            if ( clientHostInList == null )
            {
                fUpdateOutputLog( "WARNING: Couldn't find a signalshared workspace for host \""+clientHost+"\" for user \""+userName+"\" in perforce workspace list" );
                return;
            }

            string client = clientNames[ clientHostNames.IndexOf(clientHostInList) ];

            fRunProcessAndDisplayOutput( "p4", "-c " + client + " sync //SignalShared/Tools/Install/*.cmd", "" );
        }

        private void fProcessCleanBuildFile( string gameDirectory, List<string> cleanbuildFileLines )
        {
            // Either process the message automatically or present the user with a prompt
            DialogResult result;
            MemoryMessageBox messageBox = null;
            RegistryKey registryEntries = Application.UserAppDataRegistry;
            bool alwaysYes = Convert.ToBoolean( registryEntries.GetValue( "AlwaysYes", false ) );
            if( alwaysYes )
                result = DialogResult.Yes;
            else
            {
                string message;
                if( cleanbuildFileLines[ 0 ] == "1" )
                    message = "WARNING: Your project needs to be cleaned. This will also reboot your dev kit. Continue?";
                else
                {
                    message = "WARNING: The following file types need to be cleaned:\n\n";

                    // Assume the file is full of extensions; get rid of any preceding dot
                    for( int j = 0; j < cleanbuildFileLines.Count; ++j )
                    {
                        cleanbuildFileLines[ j ] = cleanbuildFileLines[ j ].ToLower( );

                        if( cleanbuildFileLines[ j ].Substring( 0, 1 ) != "." )
                            cleanbuildFileLines[ j ] = "." + cleanbuildFileLines[ j ];

                        message += cleanbuildFileLines[ j ] + "\n";
                    }

                    message += "\nThis will also reboot your dev kit. Continue?";
                }

                messageBox = new MemoryMessageBox( message, "Always Answer Yes" );
                result = messageBox.ShowDialog( );
            }

            if( result == DialogResult.No )
                throw new Exception( "Clean Game Canceled" );
            else
            {
                if( messageBox != null && messageBox.Checked )
                    registryEntries.SetValue( "AlwaysYes", true );

                if( fXboxConnected( ) )
                {
                    // Reboot the dev kit
                    Process process = fLaunchXEDKProcess( "xbreboot.exe", "" );
                    process.WaitForExit( );

                    // Wait for it to reconnect
                    while( !fXboxConnected( ) )
                        Thread.Sleep( 1000 );
                }
            }

            if( cleanbuildFileLines[ 0 ] == "1" )
            {
                // Delete game folder
                fUpdateOutputLog( "\r\nCleaning Game Folder...\r\n" );
                fRunProcessAndDisplayOutput( Environment.ExpandEnvironmentVariables( "\"%SigEngine%\\Bin\\CleanGame.cmd\"" ), "", "" );
            }
            else
            {
                fUpdateOutputLog( "Cleaning Extensions...\r\n" );
                List<string> filesToDelete = new List<string>( );
                fProcessDirectory( gameDirectory, ref filesToDelete, ref cleanbuildFileLines );
                foreach( string file in filesToDelete )
                {
                    fUpdateOutputLog( "\r\nDeleting " + file + "...\r\n" );
                    File.Delete( file );
                }
            }
        }

        private void fSyncP4File( string file, int revision )
        {
            P4Process process = new P4Process( "sync " + file + "#" + revision );
            process.Start( );
            process.WaitForExit( );
        }

        private void fCleanGame( )
        {
            string cleangameFile = mProjPathVar + "\\cleangame";
            string cleanbuildFile = mProjPathVar + "\\cleanbuild";
            string cleanGameDirectory = mProjPathVar + "\\game";

            // Find which revision of cleanbuild we currently have
            P4TaggedData data = fGetP4TaggedData( "have " + cleanbuildFile );
            if( data[ "haveRev" ] == "" )
            {
                fUpdateOutputLog( "Could not get have revision of " + cleanbuildFile + "!" );
                return;
            }
            int haveRev = Convert.ToInt32( data[ "haveRev" ] );

            // Find the latest revision of cleanbuild for the revision to which we're syncing the project
            data = mSyncChangelist ? fGetP4TaggedData( "files " + cleanbuildFile + "@" + mLatestChangelist ) : fGetP4TaggedData( "files " + cleanbuildFile );
            if( data[ "rev" ] == "" )
            {
                fUpdateOutputLog( "Could not get head revision of " + cleanbuildFile + "!" );
                return;
            }
            int headRev = Convert.ToInt32( data[ "rev" ] );

            if( haveRev == headRev )
                fUpdateOutputLog( "No cleaning required!\r\n" );
            else
            {
                // Search for a full clean - if there is one, execute that and
                // be done with the whole thing at once
                for( int i = haveRev; i <= headRev; ++i )
                {
                    fSyncP4File( cleanbuildFile, i );

                    string fileContents = fReadFile( cleanbuildFile );
                    List<string> cleanbuildLines = new List<string>( fileContents.Split( new string[ ] { "\r\n" }, StringSplitOptions.RemoveEmptyEntries ) );
                    
                    if( cleanbuildLines.Count <= 0 )
                        continue;

                    if( cleanbuildLines[ 0 ] == "1" )
                    {
                        fProcessCleanBuildFile( cleanGameDirectory, cleanbuildLines );
                        fSyncP4File( cleanbuildFile, headRev );
                        haveRev = headRev + 1;
                        break;
                    }
                }

                // Sync and process each revision of cleanbuild that has
                // occurred since the last time it was synced locally
                for( int i = haveRev; i <= headRev; ++i )
                {
                    fSyncP4File( cleanbuildFile, i );

                    string fileContents = fReadFile( cleanbuildFile );
                    List<string> cleanbuildLines = new List<string>( fileContents.Split( new string[]{"\r\n"}, StringSplitOptions.RemoveEmptyEntries ) );
                    if( cleanbuildLines.Count <= 0 || cleanbuildLines[ 0 ] == "0" )
                        continue;

                    fProcessCleanBuildFile( cleanGameDirectory, cleanbuildLines );
                }
            }
        }

        private RegistryKey fGetOrCreateRegistryKey( RegistryKey parent, string keyPath, string keyName )
        {
            RegistryKey key = parent.OpenSubKey( keyPath, true );
            if( key == null )
            {
                fUpdateOutputLog( "Creating " + keyName + " key...\r\n" );
                key = parent.CreateSubKey(keyPath);
                if( key == null )
                    throw new Exception( "Could not create sub-key for " + keyPath + "!\r\n" );
            }

            return key;
        }

        private RegistryKey fGetRegistryKey( RegistryKey parent, string keyPath )
        {
            return parent.OpenSubKey( keyPath, true );
        }

        private void fUpdateDumpConfigValue( RegistryKey rootKey, string valueName, string value, string nameForOutput )
        {
            string valueInRegistry = ( string ) rootKey.GetValue( valueName );
            if( valueInRegistry != Environment.ExpandEnvironmentVariables( value ) )
            {
                fUpdateOutputLog( "Updating default " + nameForOutput + "...\r\n" );
                rootKey.SetValue( valueName, value, RegistryValueKind.ExpandString );
            }
        }

        private void fUpdateDumpConfigValue( RegistryKey rootKey, string valueName, int value, string nameForOutput )
        {
            int valueInRegistry = ( int ) rootKey.GetValue( valueName, -1 );
            if( valueInRegistry != value )
            {
                fUpdateOutputLog( "Updating default " + nameForOutput + "...\r\n" );
                rootKey.SetValue( valueName, value, RegistryValueKind.DWord );
            }
        }

        private void fUpdateDumpConfigSettings( )
        {
            RegistryKey dumpsKey = fGetOrCreateRegistryKey( Registry.LocalMachine,
                                                            @"SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps",
                                                            "dumps configuration" );

            fUpdateDumpConfigValue( dumpsKey, "DumpFolder", @"%LOCALAPPDATA%\CrashDumps", "default dump path" );
            fUpdateDumpConfigValue( dumpsKey, "DumpType", 1, "default dump type" );
            fUpdateDumpConfigValue( dumpsKey, "DumpCount", 3, "default dump count" );

            // Get list of EXEs in SigEngine\Bin
            string[] exeList = Directory.GetFiles( Environment.ExpandEnvironmentVariables( "%SigEngine%\\Bin" ), "*.exe" );
            foreach( string exe in exeList )
            {
                string exeName = Path.GetFileName( exe );

                RegistryKey exeKey = fGetOrCreateRegistryKey( dumpsKey, exeName, exeName );

                fUpdateDumpConfigValue( exeKey, "DumpFolder", @"\\Shares\Shared\QA\PC\%COMPUTERNAME%\", exeName + " dump path" );
                fUpdateDumpConfigValue( exeKey, "DumpType", 2, exeName + " dump type" );

                exeKey.Close( );
            }

            dumpsKey.Close( );

            fUpdateOutputLog( "\r\n" );
        }

        private string fCopyError( )
        {
            // First check that the actual project folder exists on the remote machine
            // because if it doesn't, something is wrong with the build machine
            bool isBuilding = false;
            if( Directory.Exists( @"\\" + mBuildMachineName + @"\" + mProjNameVar ) )
                isBuilding = !Directory.Exists( @"\\" + mBuildMachineName + @"\" + mProjNameVar + @"\Project\Builds\Current\Game" );

            if( isBuilding )
                return "A clean build is likely being done on the build machine. Try to obtain assets again after the current build completes.";
            else
                return "The Build Server is probably down. Ask for help.";
        }

        private string fCheckForConflicts( )
        {
            string conflicts = "";

            // Check for unresolved conflicts manually, because if the conflict
            // has already been reported by P4 once, it won't be reported again
            Process process = fCreateStandardProcess( "p4", "resolve -n", "" );
            process.Start( );
            while( !process.StandardOutput.EndOfStream )
            {
                string output = process.StandardOutput.ReadLine( );
                conflicts += "\r\n" + output;
            }

            return conflicts;
        }

        private Process fCreateStandardProcess( string cmd, string args, string workingDir )
        {
            Process process = new Process( );
            process.StartInfo.FileName = cmd;
            process.StartInfo.Arguments = args;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardInput = true;
            process.StartInfo.RedirectStandardError = true;
            process.StartInfo.CreateNoWindow = true;
            process.StartInfo.WorkingDirectory = workingDir;

            return process;
        }

        private int fRunProcessAndDisplayOutput( string cmd, string args, string workingDir )
        {
            mProcessLog.Clear( );

            if( workingDir == null || workingDir == "" || workingDir == String.Empty )
                workingDir = mProjPathVar;

            Process process = fCreateStandardProcess( cmd, args, workingDir );

            process.OutputDataReceived += fOutputReceived;
            process.ErrorDataReceived += fErrorOutputReceived;

            process.Start( );

            mProcessList.Add( process );

            process.BeginOutputReadLine( );
            process.BeginErrorReadLine( );

            process.WaitForExit( );

            return process.ExitCode;
        }

        private void fGetLatest_FormClosing( object sender, FormClosingEventArgs e )
        {
            if( mProcessWorker != null && mProcessWorker.IsBusy )
                mProcessWorker = null;

            // Any external process that's created and is still
            // running should be killed
            foreach( Process process in mProcessList )
            {
                if( !process.HasExited )
                {
                    // We don't need the program to crash when we get the
                    // occasional "Access is denied." message
                    try
                    {
                        ProcessUtility.fKillProcessTree( process.Id );
                    }
                    catch( System.Exception ex )
                    {
                        fUpdateOutputLog( ex.Message );
                    }
                }
            }

            Application.Exit( );
        }

        private void fGetLatest_Shown( object sender, EventArgs e )
        {
            // Check for any open processes that could conflict with this.
            List<string> procsOpen = new List<string>( );

            Process[ ] foundProcs = null;

            foundProcs = Process.GetProcessesByName( "maya" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigEd" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigScript" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigAI" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigAnim" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigFx" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigTile" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigAtlas" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "SigShade" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "ProjectSelector" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            foundProcs = Process.GetProcessesByName( "AssetGen" );
            foreach( Process proc in foundProcs )
                procsOpen.Add( proc.ProcessName );

            if( procsOpen.Count > 0 )
            {
                const string cMessageBoxTitle = "Close Conflicting Applications";
                string display = "Please close the following applications before updating:\n";
                foreach( string proc in procsOpen )
                    display += proc + "\n";

                MessageBox.Show( display, cMessageBoxTitle );
                Application.Exit( );
                return;
            }

            // Force xbWatson to be closed
            Process[ ] watsonProcs = Process.GetProcessesByName( "xbWatson" );
            foreach( Process proc in watsonProcs )
                proc.Kill( );

            FormClosing += new System.Windows.Forms.FormClosingEventHandler( fGetLatest_FormClosing );

            string envErrors = String.Empty;

            mProjNameVar = System.Environment.GetEnvironmentVariable( "SigCurrentProjectName" );
            if( mProjNameVar == null )
                envErrors += "SigCurrentProjectName environment variable is not valid!";

            mProjPathVar = System.Environment.GetEnvironmentVariable( "SigCurrentProject" );
            if( mProjPathVar == null || !Directory.Exists( mProjPathVar ) )
                envErrors += "\nSigCurrentProject environment variable is not valid!";

            mEngPathVar = System.Environment.GetEnvironmentVariable( "SigEngine" );
            if( mEngPathVar == null || !Directory.Exists( mEngPathVar ) )
                envErrors += "\nSigEngine environment variable is not valid!";

            if( envErrors != String.Empty )
            {
                MessageBox.Show( envErrors, "Error!", MessageBoxButtons.OK );
                return;
            }

            // DeployTo needs to be checked here as well. Since Xbox transfer happens in over a thousand places.
            string deploy = System.Environment.GetEnvironmentVariable("SigDeployTo");
            if (deploy == null)
            {
                System.Environment.SetEnvironmentVariable("SigDeployTo", mProjNameVar, EnvironmentVariableTarget.User);
            }

            mBuildMachineName = ( "build-" + mProjNameVar ).ToLower( );
            mLatestBuildPath = @"\\" + mBuildMachineName + @"\" + mProjNameVar + @"\Project\Builds\";

            try
            {
                System.IO.FileInfo fileInfo = new System.IO.FileInfo( mProjPathVar + "\\cleangame" );
                fileInfo.Refresh( );
                mCleanGameLastWriteTime = fileInfo.LastWriteTime;
            }
            catch
            {
                mCleanGameLastWriteTime = new System.DateTime( );
            }

            fUpdateOutputLog( "********************************************\r\n" );
            fUpdateOutputLog( "Update Server:\t" + mBuildMachineName + "\r\n" );

            string latestBuild = fGetLatestBuildNumber( );
            mLatestBuildPath += latestBuild + "\\";
            mLatestChangelist = fGetLatestChangelistNumber( );

            fUpdateOutputLog( "Latest Build:\t" + latestBuild + "\r\n" );

            fUpdateOutputLog( "Latest Changelist:\t" + mLatestChangelist + "\r\n" );
            fUpdateOutputLog( "********************************************\r\n" );

            mProcessWorker.DoWork += new DoWorkEventHandler( fBackgroundProcessWorker );
            mProcessWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler( fBackgroundProcessWorkerCompleted );
            mProcessWorker.WorkerSupportsCancellation = true;
            mProcessWorker.RunWorkerAsync( );
        }

        private Process fLaunchXEDKProcess( string fileName, string arguments )
        {
            string xedkDir = Environment.GetEnvironmentVariable( "XEDK" );
            if( xedkDir == "" )
                return null;

            string bin = xedkDir + @"\bin\win32\";

            Process process = fCreateStandardProcess( bin + fileName, arguments, "" );
            process.Start( );

            return process;
        }

        private bool fXboxConnected( )
        {
            Process process = fLaunchXEDKProcess( "xbconsoletype.exe", "" );
            if( process == null )
                return false;

            process.WaitForExit( );

            if( process.StandardOutput.ReadToEnd( ) == "" )
                return false;

            return true;
        }

        private void fProcessDirectory( string targetDirectory, ref List<string> fileList, ref List<string> extList )
        {
            if( !Directory.Exists( targetDirectory ) )
                return;

            // Process the list of files found in the directory
            string[ ] fileEntries = Directory.GetFiles( targetDirectory );
            foreach( string fileName in fileEntries )
            {
                string ext = Path.GetExtension( fileName ).ToLower( );
                if( extList.Contains( ext ) )
                    fileList.Add( fileName );
            }

            // Recurse into subdirectories of this directory.
            string[ ] subdirectoryEntries = Directory.GetDirectories( targetDirectory );
            foreach( string subdirectory in subdirectoryEntries )
                fProcessDirectory( subdirectory, ref fileList, ref extList );
        }

        private string fGetLatestBuildNumber( )
        {
            string latestBuild = fReadFile( mLatestBuildPath + "latest_build.txt" );
            return latestBuild != "" ? latestBuild : "Current";
        }

        private string fGetLatestChangelistNumber( )
        {
            return fReadFile( mLatestBuildPath + "latest_changelist.txt" );
        }

        private void fUpdateOpenedTimestamps( )
        {
            P4TaggedData data = fGetP4TaggedData( "info" );

            string clientName = data[ "clientName" ];
            if( clientName == "" )
                fUpdateOutputLog( "WARNING: Could not find client name.\r\n" );

            string clientRoot = data[ "clientRoot" ];
            if( clientRoot == "" )
                fUpdateOutputLog( "WARNING: Could not find client root.\r\n" );

            if( clientName == "" || clientRoot == "" )
                return;

            data = fGetP4TaggedData( "opened" );
            for( int i = 0; i < data.Tags.Count; ++i )
            {
                if( data.Tags[ i ] != "clientFile" )
                    continue;

                string path = data.Values[ i ];
                path = path.Replace( "//" + clientName, clientRoot );
                path = path.Replace( "/", "\\" );

                if( path.ToLower( ).Contains( mProjPathVar + "\\res\\" ) &&
                    File.Exists( path ) &&
                    Path.GetExtension( path ) != ".as" ) // ActionScript files have issues with this
                {
                    try
                    {
                        File.SetLastWriteTime( path, DateTime.Now );
                        fUpdateOutputLog( "Updated timestamp for " + path + "...\n" );
                    }
                    catch( Exception e )
                    {
                        fUpdateOutputLog( "Couldn't update timestamp for " + path + ": " + e.Message + "\n" );
                    }
                }
            }
        }

        private string fReadFile( string path )
        {
            string contents = "";
            try
            {
                using( StreamReader file = new StreamReader( path ) )
                {
                    contents = file.ReadToEnd( ).Trim( );
                }
            }
            catch( Exception )
            {
            }

            return contents;
        }

        private void fOutputReceived( object sender, System.Diagnostics.DataReceivedEventArgs e )
        {
            mProcessLog.Add( e.Data );

            fUpdateOutputLog( e.Data + "\r\n" );
        }

        private void fErrorOutputReceived( object sender, System.Diagnostics.DataReceivedEventArgs e )
        {
            string message = ( e.Data != null ) ? e.Data.ToLower( ) : "";
            if( message.Contains( "must resolve" ) || 
                message.Contains( "submit failed" ) ||
                message.Contains("can't clobber") )
                mSyncErrors.Add( e.Data );

            fOutputReceived( sender, e );
        }

        private void DeleteEXTRAFilesButton_Click( object sender, EventArgs e )
        {
            DeleteEXTRAFilesDialog dialog = new DeleteEXTRAFilesDialog( );

            ListBox filesListBox = ( ListBox ) dialog.Controls[ "filesListBox" ];
            filesListBox.Items.AddRange( mEXTRAFiles.ToArray( ) );

            dialog.ShowDialog( this );
            if( filesListBox.Items.Count < 1 )
            {
                Close( );
                return;
            }

            mEXTRAFiles.Clear( );
            foreach( string file in filesListBox.Items )
                mEXTRAFiles.Add( file );
        }

        private P4TaggedData fGetP4TaggedData( string command )
        {
            P4TaggedData data = new P4TaggedData( );

            Process process = new P4Process( command );
            process.Start( );
            while( !process.StandardOutput.EndOfStream || !process.HasExited )
            {
                string line = process.StandardOutput.ReadLine( );

                string[] tagData = P4Process.GetTagData( line );
                if( tagData[ 0 ] != "" )
                    data.Add( tagData[ 0 ], tagData[ 1 ] );
            }

            return data;
        }
    }
}
