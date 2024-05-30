using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Threading;
using System.Windows.Forms;
using FreeImageAPI;

namespace DownResRes
{
	static class Program
	{
		static bool Cancel = false;

		static Parameters Parameters = Parameters.Default;

		static IProgressListener Listener = new ConsoleProgressListener( );

		static string[] FindImages()
		{
			Listener.On( new InfoProcessingEvent( null, string.Format( "Scanning {0}", Parameters.ResPath ) ) );
			return Directory
				.GetFiles( Parameters.ResPath, "*.*", SearchOption.AllDirectories )
				.Where( path => Parameters.PatternsAsRegexps.Any( pattern => pattern.IsMatch( path ) ) )
				.ToArray( );
		}

		static readonly string AddToSourceControlCmd	= Environment.ExpandEnvironmentVariables( @"%SigEngine%\Bin\AddToSourceControl.cmd" );
		static readonly string OpenForEditCmd			= Environment.ExpandEnvironmentVariables( @"%SigEngine%\Bin\OpenForEdit.cmd" );

		static bool IsReadOnly( string path )
		{
			return (File.GetAttributes( path ) & FileAttributes.ReadOnly) == FileAttributes.ReadOnly;
		}

		static bool CheckOut( string path )
		{
			if( IsReadOnly( path ) )
			{
				try
				{
					var psi = new ProcessStartInfo( )
					{
						FileName = OpenForEditCmd,
						Arguments = string.Format( @"""{0}""", path ),
						CreateNoWindow = true,
						WindowStyle = ProcessWindowStyle.Hidden,
					};
					Listener.On( new InfoProcessingEvent( path, "Checking out" ) );
					Process.Start( psi ).WaitForExit( );
				}
				catch( Exception e )
				{
					Listener.On( new ErrorProcessingEvent( path, "Failed to check file out", e ) );
					return false;
				}
			}

			if( IsReadOnly( path ) )
			{
				Listener.On( new ErrorProcessingEvent( path, "Still read only" ) );
				return false;
			}

			return true;
		}

		static bool Backup( string path )
		{
			if( !Parameters.EnableBackup )
				return true;

			var backupFile = Parameters.ToBackupPath( path );
			if( path == backupFile )
				return true;

			Listener.On( new InfoProcessingEvent( path, "Backing up" ) );
			try
			{
				Directory.CreateDirectory( backupFile );
			}
			catch( Exception e )
			{
				Listener.On( new ErrorProcessingEvent( path, "Couldn't create directory", e ) );
				return false;
			}

			try
			{
				File.Copy( path, backupFile, Parameters.OverwriteBackup );
			}
			catch( Exception e )
			{
				Listener.On( new ErrorProcessingEvent( path, "Couldn't create copy", e ) );
				return false;
			}

			try
			{
				Process.Start( AddToSourceControlCmd, path ).WaitForExit( );
			}
			catch( Exception e )
			{
				Listener.On( new ErrorProcessingEvent( path, "Failed to add to version control", e ) );
			}

			return true;
		}

		static FreeImageBitmap CreateAverageDownsampleFrom( FreeImageBitmap img, int divisor )
		{
			var newW = img.Width / divisor;
			var newH = img.Height / divisor;

			if( img.Width % 4 == 0 && img.Height % 4 == 0 )
			{
				if( newW % 4 != 0 || newH % 4 != 0 )
					Listener.On( new InfoProcessingEvent( "???", "Rounding down image size to preserve multiple of 4 dimensions" ) );

				// Source is multiples of four, preserve that by rounding down by abusing integer truncation.
				newW = newW / 4 * 4;
				newH = newH / 4 * 4;
			}

			var newimg = new FreeImageBitmap( newW, newH, img.PixelFormat );

			for( int y=0; y<newimg.Height; ++y )
			for( int x=0; x<newimg.Width; ++x )
			{
				uint a = 0;
				uint r = 0;
				uint g = 0;
				uint b = 0;

				int subw = Math.Min( divisor, img.Width - x * divisor );
				int subh = Math.Min( divisor, img.Height - y * divisor );
				uint samples = unchecked( (uint)(subw * subh) );

				for( int sdy=0; sdy<subh; ++sdy )
				for( int sdx=0; sdx<subw; ++sdx )
				{
					int sx = x * divisor + sdx;
					int sy = y * divisor + sdy;

					var sample = img.GetPixel( sx, sy );
					a += sample.A;
					r += sample.R;
					g += sample.G;
					b += sample.B;
				}

				a /= samples;
				r /= samples;
				g /= samples;
				b /= samples;

				newimg.SetPixel( x, y, unchecked( Color.FromArgb( (byte)a, (byte)r, (byte)g, (byte)b ) ) );
			}

			return newimg;
		}

		static void BuildProcess( )
		{

			var images = FindImages( );

			int i = 1;
			foreach( var resfile in images )
			{
				if( Cancel )
					break;

				Listener.On( new StartProcessingEvent( resfile, i++, images.Length ) );

				var img = FreeImageBitmap.FromFile( resfile );
				if( img.Width <= Parameters.MinimumResolution || img.Height <= Parameters.MinimumResolution )
					continue;

				int divisor = new[] { Parameters.MaximumDivisor, img.Width / Parameters.MinimumResolution, img.Height / Parameters.MinimumResolution }.Min( );
				if( divisor <= 1 && Parameters.MaximumDivisor != 1 )
					continue;

				if( !CheckOut( resfile ) )
					continue;

				if( !Backup( resfile ) )
					continue;

				using( var newimg = CreateAverageDownsampleFrom( img, divisor ) )
				{
					img.Dispose( );
					File.Delete( resfile );
					newimg.Save( resfile );
				}

				Listener.On( new DoneProcessingEvent( resfile ) );
			}
		}

		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static unsafe void Main( string[] commandArguments )
		{
			Application.EnableVisualStyles( );
			Application.SetCompatibleTextRenderingDefault( false );

			var form = new ConfigureParametersForm( );
			form.Build = (p) =>
			{
				Parameters = p;
				Listener = form;
				new Thread( (args) =>
				{
					BuildProcess( );
					form.BeginInvoke( new Action( () => form.Close( ) ) );
				}).Start( );
			};

			Application.Run( form );
		}
	}
}
