using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using Perforce;
using Perforce.P4;
using System.IO;
using System.Threading;

namespace ResourceMover
{
	public partial class MainForm : Form
	{
		bool mLockSelectionChanges = false;
		List<string> mFilesUnderRes = new List<string>( );

		List<string> mFilesSrc = new List<string>( );
		List<string> mFilesTgt = new List<string>( );
		List<ReferenceFixer> mFixers = new List<ReferenceFixer>( );

		string mFolderSrc = string.Empty;
		string mFolderTgt = string.Empty;

		string mResDirectory;
		Repository mP4Repo;
		ResourceFinder mResourceFinder;

		public MainForm( Repository p4Repo )
		{
			InitializeComponent( );

			mP4Repo = p4Repo;
			mResDirectory = Environment.GetEnvironmentVariable( "SigCurrentProject" ) + "\\Res\\";

			mResourceFinder = new ResourceFinder( 
				ReferenceExtensionTable.SupportedExtensions.Select( ( ext ) => "*" + ext ), 
				mResDirectory );

			ThreadPool.QueueUserWorkItem( mResourceFinder.DoWork );

			bgWorker.DoWork += new DoWorkEventHandler(bgWorker_DoWork);
			bgWorker.ProgressChanged += new ProgressChangedEventHandler(bgWorker_ProgressChanged);
			bgWorker.RunWorkerCompleted += new RunWorkerCompletedEventHandler( bgWorker_RunWorkerCompleted );

			openFileDialog1.Title = "Select file(s) to move...";
			saveFileDialog1.Title = "Select destination to move file to...";

			workText.Text = string.Empty;

			targetFileBrowse.Enabled = false;
			tgtFileRemove.Enabled = false;
		}

		private void bgWorker_DoWork( object sender, DoWorkEventArgs e )
		{
			mResourceFinder.Event.WaitOne( ); // Make sure we've found all the resources

			BackgroundWorker worker = (BackgroundWorker)sender;
			IList<string>[] pathLists = (IList<string>[])e.Argument;
			IList<string> clientSrcPaths = pathLists[ 0 ];
			IList<string> clientTgtPaths = pathLists[ 1 ];

			string[] relSrcPaths = clientSrcPaths.Select( p => MakeResRelative( p ) ).ToArray( );
			string[] relTgtPaths = clientTgtPaths.Select( p => MakeResRelative( p ) ).ToArray( );

			int progressLock = 0;
			int workComplete = 0;

			const int cConcurrentRefFixes = 128;
			Semaphore semaphore = new Semaphore( 0, cConcurrentRefFixes );

			mFixers.Capacity = mResourceFinder.Results.Count;

			worker.ReportProgress( workComplete, "Building reference fixers..." );
			foreach( string resPath in mResourceFinder.Results )
			{
				ReferenceFixer fixer = new ReferenceFixer(
					semaphore, mP4Repo, resPath, relSrcPaths, relTgtPaths );
				mFixers.Add( fixer );

				fixer.OnComplete += ( f ) =>
				{
					// Everyone updates the work complete
					Interlocked.Increment( ref workComplete );

					// Only signal the progress if no one else is currently
					if( Interlocked.CompareExchange( ref progressLock, 1, 0 ) == 0 )
					{
						worker.ReportProgress(
							workComplete, "Finished fixing " + MakeResRelative( f.Path ) );

						Interlocked.Exchange( ref progressLock, 0 );
					}
				};

				fixer.RegisterForWork( );
			}

			// Let the work begin
			semaphore.Release( cConcurrentRefFixes );

			// Wait for all the workers to finish
			ReferenceFixer.WorkCompleteEvent.WaitOne( );

			// Build out the operations
			var toMove = new List<string>( );
			var toMoveTo = new List<string>( );
			var toDelete = new List<string>( );
			for( int i = 0; i < clientTgtPaths.Count; ++i )
			{
				if( clientTgtPaths[ i ].Length > 0 )
				{
					toMove.Add( clientSrcPaths[ i ] );
					toMoveTo.Add( clientTgtPaths[ i ] );
				}
				else // attempt to delete
				{
					// Ensure that no references were found before doing the delete
					bool hasRef = false;
					foreach( var fixer in mFixers )
					{
						if( fixer.References.ContainsKey( relSrcPaths[ i ] ) )
						{
							hasRef = true;
							break;
						}
					}
					
					if( !hasRef )
						toDelete.Add( clientSrcPaths[ i ] );
				}
			}

			// Move all the files to move
			if( toMove.Count > 0 )
			{
				// Check them all out first
				P4Command editCmd = new P4Command( mP4Repo, "edit", false, toMove.ToArray( ) );
				P4CommandResult result = editCmd.Run( );

				// Rename them
				for( int i = 0; i < toMove.Count; ++i )
				{
					P4Command moveCmd = new P4Command(
						mP4Repo, "move", false, new[] { toMove[ i ], toMoveTo[ i ] } );
					result = moveCmd.Run( );

					worker.ReportProgress( workComplete++, "Moving " + MakeResRelative( toMove[ i ] ) );
				}
			}

			// Delete all the files to delete
			if( toDelete.Count > 0 )
			{
				P4Command delCmd = new P4Command( mP4Repo, "delete", false, toDelete.ToArray( ) );
				P4CommandResult result = delCmd.Run( );
				worker.ReportProgress( workComplete += toDelete.Count, "Deleted files!" );
			}

			int workLeft = clientSrcPaths.Count - toMove.Count - toDelete.Count;
			worker.ReportProgress( workComplete += workLeft, "Finished!" );
		}

		private void bgWorker_ProgressChanged( object sender, ProgressChangedEventArgs e )
		{
			// Clamp the progress
			int progress = e.ProgressPercentage;
			progress = Math.Max( progress, moveProgress.Minimum );
			progress = Math.Min( progress, moveProgress.Maximum );

			moveProgress.Value = progress;
			workText.Text = (string)e.UserState;
		}

		private void bgWorker_RunWorkerCompleted( object sender, RunWorkerCompletedEventArgs e )
		{
			ExecFileMove.Enabled = true;
			ExecFolderMove.Enabled = true;

			refLog.BeginUpdate( );
			{
				refLog.Nodes.Clear( );
				var fixerQuery = mFixers
					.Where( f => { return f.References.Count > 0; } )
					;

				foreach( var fixer in fixerQuery )
				{
					foreach( var entry in fixer.References )
					{
						TreeNode node = refLog.Nodes[ entry.Key ];
						if( node == null )
						{
							node = new TreeNode( entry.Key ) { Name = entry.Key };
							refLog.Nodes.Add( node );
						}

						var childNode = new TreeNode( MakeResRelative( fixer.Path ), 
							entry.Value.Select( r => { return new TreeNode( r.ToString( ) ); } ).ToArray( ) );
						node.Nodes.Add( childNode );
					}
				}
			}
			refLog.EndUpdate( );

			mFixers.Clear( );

			// Files have moved - we need to update the global resource list
			mResourceFinder.Event.Reset( );
			ThreadPool.QueueUserWorkItem( mResourceFinder.DoWork );

			moveProgress.Value = 0;
			workText.Text = "Finished!!!";
		}

		private string MakeResRelative( string path )
		{
			if( path.Length == 0 )
				return string.Empty;

			System.Uri uri1 = new Uri( mResDirectory );
			System.Uri uri2 = new Uri( path );

			Uri relativeUri = uri1.MakeRelativeUri( uri2 );

			return relativeUri.ToString( ).ToLower( ).Replace( "/", "\\" );
		}

		private void SrcFileBrowse_Click( object sender, EventArgs e )
		{
			if( openFileDialog1.ShowDialog( ) == DialogResult.OK )
			{
				int selection = srcFileBox.SelectedIndex;
				foreach( string fileName in openFileDialog1.FileNames )
				{
					string lowerFileName = fileName.ToLower( );
					selection = mFilesSrc.IndexOf( lowerFileName );
					if( selection < 0 )
					{
						selection = mFilesSrc.Count;

						mFilesSrc.Add( lowerFileName );
						mFilesTgt.Add( lowerFileName );

						srcFileBox.Items.Add( MakeResRelative( lowerFileName ) );
						tgtFileBox.Items.Add( MakeResRelative( lowerFileName ) );
					}
				}

				tgtFileRemove.Enabled = targetFileBrowse.Enabled = mFilesTgt.Count > 0;
				srcFileBox.SelectedIndex = selection;
			}
		}

		private void srcFileRemove_Click( object sender, EventArgs e )
		{
			int selectedIndex = srcFileBox.SelectedIndex;
			if( selectedIndex >= 0 )
			{
				mFilesSrc.RemoveAt( selectedIndex );
				mFilesTgt.RemoveAt( selectedIndex );
				srcFileBox.Items.RemoveAt( selectedIndex );
				tgtFileBox.Items.RemoveAt( selectedIndex );
			}

			tgtFileRemove.Enabled = targetFileBrowse.Enabled = mFilesTgt.Count > 0;
		}

		private void TargetFileBrowse_Click( object sender, EventArgs e )
		{
			string currTgtPath = mFilesTgt[ tgtFileBox.SelectedIndex ];
			saveFileDialog1.FileName = Path.GetFileName( currTgtPath );

			if( saveFileDialog1.ShowDialog( ) == DialogResult.OK )
			{
				currTgtPath = saveFileDialog1.FileName.ToLower( );
				mFilesTgt[ tgtFileBox.SelectedIndex ] = currTgtPath;
				tgtFileBox.Items[ tgtFileBox.SelectedIndex ] = MakeResRelative( currTgtPath );
			}
		}

		private void SrcFolderBrowse_Click( object sender, EventArgs e )
		{
			folderBrowserDialog1.Reset( );
			folderBrowserDialog1.SelectedPath = mResDirectory;
			folderBrowserDialog1.ShowNewFolderButton = false;
			folderBrowserDialog1.Description = "Select folder to move...";

			if( folderBrowserDialog1.ShowDialog( ) == DialogResult.OK )
			{
				mFolderSrc = folderBrowserDialog1.SelectedPath.ToLower( );
				srcFolderText.Text = MakeResRelative( mFolderSrc );
			}
		}

		private void TargetFolderBrowse_Click( object sender, EventArgs e )
		{
			folderBrowserDialog1.Reset( );
			folderBrowserDialog1.Description = "Select folder to move to...";

			if( mFolderSrc.Length != 0 )
			{
				folderBrowserDialog1.SelectedPath = mFolderSrc;
			}
			else
			{
				folderBrowserDialog1.SelectedPath = mResDirectory;
			}


			if( folderBrowserDialog1.ShowDialog( ) == DialogResult.OK )
			{
				mFolderTgt = folderBrowserDialog1.SelectedPath.ToLower( );
				tgtFolderText.Text = MakeResRelative( mFolderTgt );
			}
		}

		private void ExecFileMove_Click( object sender, EventArgs e )
		{
			if( mFilesSrc.Count > 0 )
				DoMove( mFilesSrc, mFilesTgt );
		}

		private void ExecFolderMove_Click( object sender, EventArgs e )
		{
			if( mFolderSrc.Length != 0 && mFolderTgt.Length != 0 )
			{
				string[] localFiles = Directory.GetFiles( mFolderSrc, "*.*", SearchOption.AllDirectories );
                List<string> clientSrcPaths = new List<string>( localFiles.Select( file => file.ToLower( ) ) );
                List<string> clientTgtPaths = new List<string>(clientSrcPaths.Select(path => path.Replace(mFolderSrc, mFolderTgt)));

				DoMove( clientSrcPaths, clientTgtPaths );
			}
		}

		private void DoMove( IList<string> clientSrcPaths, IList<string> clientTgtPaths )
		{
			moveProgress.Value = 0;
			moveProgress.Maximum = mResourceFinder.Results.Count + clientSrcPaths.Count;

			ExecFileMove.Enabled = false;
			ExecFolderMove.Enabled = false;
			bgWorker.RunWorkerAsync( new [] { clientSrcPaths, clientTgtPaths } );
		}

		private void srcFileBox_SelectedIndexChanged( object sender, EventArgs e )
		{
			if( !mLockSelectionChanges )
			{
				mLockSelectionChanges = true;
				if( srcFileBox.SelectedIndex < 0 && srcFileBox.Items.Count > 0 )
					srcFileBox.SelectedIndex = 0;

				tgtFileBox.SelectedIndex = srcFileBox.SelectedIndex;
				mLockSelectionChanges = false;
			}
		}

		private void tgtFileBox_SelectedIndexChanged( object sender, EventArgs e )
		{
			if( !mLockSelectionChanges )
			{
				mLockSelectionChanges = true;
				if( tgtFileBox.SelectedIndex < 0 && tgtFileBox.Items.Count > 0 )
					tgtFileBox.SelectedIndex = 0;

				srcFileBox.SelectedIndex = tgtFileBox.SelectedIndex;
				mLockSelectionChanges = false;
			}
		}

		private void tgtFileRemove_Click( object sender, EventArgs e )
		{
			mFilesTgt[ tgtFileBox.SelectedIndex ] = string.Empty;
			tgtFileBox.Items[ tgtFileBox.SelectedIndex ] = "---This file will be deleted---";
		}
	}
}
