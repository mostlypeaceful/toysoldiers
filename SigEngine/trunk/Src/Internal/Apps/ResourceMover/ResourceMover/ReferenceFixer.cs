//------------------------------------------------------------------------------
// \file ReferenceFixer.cs - 25 Sep 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------

using System.Linq;
using System.Threading;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.IO;
using Perforce.P4;
using System.Windows.Forms;
using System.Text;
using System;

using Excel = Microsoft.Office.Interop.Excel;
using System.Runtime.InteropServices; 

namespace ResourceMover
{
	public class ReferenceFixer
	{
		public class Reference
		{
			public Reference( string search, string replace, string text, int lineNumber )
			{
				SearchPath = search.Replace( "/", "\\" );
				Search = search;
				Replace = replace;
				Text = text;
				LineNumber = lineNumber;
			}

			// Same as Search, with guaranteed back slashes
			public string SearchPath { get; private set; }

			public string Search { get; private set; }
			public string Replace { get; private set; }
			public string Text { get; private set; }
			public int LineNumber { get; private set; }

			public override string ToString( )
			{
				return string.Format( "Line {0}: {1}", LineNumber, Text );
			}
		}

		public static AutoResetEvent WorkCompleteEvent { get; private set; }
		private static Excel.Application ExcelApp { get; set; }

		public Repository P4Repo { get; private set; }
		public string Path { get; private set; }
		public string[] Search { get; private set; }
		public string[] Replace { get; private set; }
		public IDictionary<string, List<Reference>> References { get; private set; }

		public delegate void OnCompleteEventHandler( ReferenceFixer fixer );
		public event OnCompleteEventHandler OnComplete;

		static ReferenceFixer( )
		{
			WorkCompleteEvent = new AutoResetEvent( false );
		}

		public static void LaunchExcel( )
		{
			if( ExcelApp == null )
				ExcelApp = new Excel.Application( );
		}

		public static void QuitExcel( )
		{
			if( ExcelApp != null )
				ExcelApp = null;
		}

		public ReferenceFixer( Semaphore semaphore, Repository p4Repo, string path, string[] search, string[] replace )
		{
			P4Repo = p4Repo;
			Path = path;
			Search = search;
			Replace = replace;
			mSemaphore = semaphore;

			References = new Dictionary<string, List<Reference>>( );
		}

		public void RegisterForWork( )
		{
			Interlocked.Increment( ref mWorkerCount );
			ThreadPool.QueueUserWorkItem( DoWork );
		}

		public void DoWork( object ignored )
		{
			if( Search.Length > 0 )
				FixReferences( );

			if( OnComplete != null )
				OnComplete( this );

			if( Interlocked.Decrement( ref mWorkerCount ) == 0 )
				WorkCompleteEvent.Set( );
		}

		private void FixReferences( )
		{
			mSemaphore.WaitOne( );

			string pathExt = System.IO.Path.GetExtension( Path );
		
			string[] culledSearch = ReferenceExtensionTable.CullPaths( pathExt, Search );
			List<string> tests = new List<string>( culledSearch.Length * 2 );
			tests.AddRange( culledSearch );
			tests.AddRange( culledSearch.Select( s => s.Replace( "\\", "/" ) ) );

			string[] culledReplace = ReferenceExtensionTable.CullPaths( pathExt, Replace );
			List<string> replacers = new List<string>( tests.Capacity );
			replacers.AddRange( culledReplace );
			replacers.AddRange( culledReplace.Select( r => r.Replace( "\\", "/" ) ) );

			int minSearch = int.MaxValue;
			foreach( var test in tests )
				minSearch = Math.Min( minSearch, test.Length );


			if( pathExt == ".xlsx" )
			{
				FixExcelFile( tests, replacers, minSearch );
			}
			else
			{
				Encoding encoding = Encoding.Default;
				if( pathExt == ".tab" )
					encoding = Encoding.Unicode;

				FixTextFile( tests, replacers, minSearch, encoding );
			}

			mSemaphore.Release( ); // Let some other worker begin - we're done with our heavy lifting
		}

		private void FixTextFile( IList<string> tests, IList<string> replacers, int minSearch, Encoding encoding )
		{
			string tempFileName = null;
			System.IO.StreamWriter writer = null;

			const int cBufferSize = 2 * 1024 * 1024;

			// Read the file and search it line by line
			using( System.IO.StreamReader reader = new System.IO.StreamReader( Path, encoding, false, cBufferSize ) )
			{
				string line;
				int lineCount = 0;

				while( ( line = reader.ReadLine( ) ) != null )
				{
					int replaceStart = -1;
					int testIndex = 0;

					if( line.Length >= minSearch )
					{
						string lowerLine = line.ToLower( );
						for( ; testIndex < tests.Count; ++testIndex )
						{
							replaceStart = lowerLine.IndexOf( tests[ testIndex ] );
							if( replaceStart >= 0 )
								break;
						}
					}

					// If we've found a reference
					if( replaceStart >= 0 )
					{
						// Add the ref to the list
						fAddReference( tests[ testIndex ], replacers[ testIndex], line, lineCount + 1 );

						// If the replacement is real
						if( replacers[ testIndex ].Length > 0 )
						{
							// If we don't have a writer, we'll need to create a file,
							// and write all the lines up to now to the file
							if( writer == null )
							{
								tempFileName = System.IO.Path.GetTempFileName( );
								writer = new System.IO.StreamWriter( tempFileName, false, encoding, cBufferSize );

								// Reset the read stream to the beginning
								reader.BaseStream.Seek( 0, SeekOrigin.Begin );
								reader.DiscardBufferedData( );

								// Write all the lines up till now
								for( int i = 0; i < lineCount; ++i )
									writer.WriteLine( reader.ReadLine( ) );
								reader.ReadLine( ); // Catch up to the last read
							}

							// Replace the line
							string newLine = line.Substring( 0, replaceStart ) + replacers[ testIndex ] + line.Substring( replaceStart + tests[ testIndex ].Length );
							writer.WriteLine( newLine );
						}
					}
					else if( writer != null )
						writer.WriteLine( line );

					lineCount++;
				}
			}

			// If we've created a writer then we've had to fix up some references
			if( writer != null )
			{
				writer.Dispose( );
				writer = null;

				// Edit the original file in perforce
				P4Command editCmd = new P4Command( P4Repo, "edit", false, new[] { Path } );
				P4CommandResult result = editCmd.Run( );

				// Copy over it and delete the temp file
				System.IO.File.Copy( tempFileName, Path, true );
				System.IO.File.Delete( tempFileName );
			}
		}

		private void FixExcelFile( IList<string> tests, IList<string> replacers, int minSearch )
		{	
			try
			{
				lock( typeof( ReferenceFixer ) )
				{
					// Edit the sheet in perforce because we cannot save a sheet that was readonly when opened
					{
						P4Command editCmd = new P4Command( P4Repo, "edit", false, new[] { Path } );
						P4CommandResult result = editCmd.Run( );
					}

					Excel.Workbook workBook = ExcelApp.Workbooks.Open( Path );

					bool bookChanged = false;
					foreach( Excel.Worksheet sheet in workBook.Sheets )
					{
						Excel.Range range = sheet.UsedRange;
						object[ , ] cells = (object[ , ])range.Value;

						if( cells == null )
							continue;

						int xMax = cells.GetUpperBound( 0 );
						int yMax = cells.GetUpperBound( 1 );
						int xMin = cells.GetLowerBound( 0 );
						int yMin = cells.GetLowerBound( 1 );

						bool sheetChanged = false;

						for( int x = xMin; x <= xMax; ++x )
						{
							for( int y = yMin; y <= yMax; ++y )
							{
								object cellObj = cells[ x, y ];
								if( cellObj == null )
									continue;

								// We only operate on strings as we're trying to fix up references
								if( !( cellObj is string ) )
									continue;

								string cell = ( (string)cellObj ).ToLower( );
								for( int z = 0; z < tests.Count; ++z )
								{
									int replaceStart = cell.IndexOf( tests[ z ] );
									if( replaceStart >= 0 )
									{
										// Add the ref to the list
										fAddReference( tests[ z ], replacers[ z ], cell, y + 1 );

										// If it's a real replace
										if( replacers[ z ].Length > 0 )
										{
											string newCell = cell.Substring( 0, replaceStart ) + replacers[ z ] + cell.Substring( replaceStart + tests[ z ].Length );
											cells[ x, y ] = newCell;

											sheetChanged = true;
										}

										break;
									}
								}
							}
						}

						// Reassign the cells
						if( sheetChanged )
						{
							range.Value = cells;
							bookChanged = true;
						}
					}

					if( !bookChanged )
					{
						P4Command revertCmd = new P4Command( P4Repo, "revert", false, new[] { "-a", Path } );
						P4CommandResult result = revertCmd.Run( );
					}

					workBook.Close( bookChanged );
					Marshal.ReleaseComObject( workBook );
				}
			}
			catch( Exception ex )
			{
				MessageBox.Show( "Unable to release the Object " + ex.ToString( ) );
			}
		}

		private void fAddReference( string search, string replace, string line, int lineNumber )
		{
			Reference reference = new Reference( search, replace, line, lineNumber );

			List<Reference> refList;
			if( !References.TryGetValue( reference.SearchPath, out refList ) )
			{
				refList = new List<Reference>( );
				References.Add( reference.SearchPath, refList );
			}

			refList.Add( reference );
		}

		private Semaphore mSemaphore;
		private static long mWorkerCount;
	}
}
