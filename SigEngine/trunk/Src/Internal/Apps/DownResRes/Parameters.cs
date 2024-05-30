using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace DownResRes
{
	struct Parameters
	{
		string _ResPath;
		string _BackupPath;

		public string	ResPath		{ get { return _ResPath;	} set { _ResPath	= NormalizeDirectory( value ); } }
		public string	BackupPath	{ get { return _BackupPath;	} set { _BackupPath	= NormalizeDirectory( value ); } }
		public string[]	Patterns;
		public string	BackupFilenamePrefix;
		public string	BackupFilenamePostfix;
		public int		MinimumResolution;
		public int		MaximumDivisor;
		public bool		EnableBackup;
		public bool		OverwriteBackup;

		public IEnumerable<Regex>	PatternsAsRegexps { get {
			return (Patterns ?? new[]{"*.*"}).Select( pattern => new Regex( Regex.Escape( pattern ).Replace( "\\*", "(.*)" ), RegexOptions.IgnoreCase ) );
		}}

		public static readonly Parameters Default = new Parameters()
		{
			ResPath				= Environment.ExpandEnvironmentVariables( @"%SigCurrentProject%\res" ),
			BackupPath			= Environment.ExpandEnvironmentVariables( @"%SigCurrentProject%\res\~backup" ),
			Patterns			= new[] { "*.png", "*.tga", "*.dds" },
			MinimumResolution	= 16,
			MaximumDivisor		= 2,
			EnableBackup		= false,
			OverwriteBackup		= true,
		};

		private static string NormalizeDirectory( string path )
		{
			if( path == null )
				return null;

			path = path.Replace( Path.AltDirectorySeparatorChar, Path.DirectorySeparatorChar );
			if( path.EndsWith( Path.DirectorySeparatorChar.ToString() ) )
				return path;
			else
				return path + Path.DirectorySeparatorChar;
		}

		public string ToBackupPath( string file )
		{
			Debug.Assert( file.StartsWith( ResPath ) );

			var backupPath	= BackupPath ?? ResPath;
			var resPath		= ResPath;
			var prefix		= BackupFilenamePrefix ?? "";
			var postfix		= BackupFilenamePostfix ?? "";

			// Correct directory
			file = backupPath + file.Substring( resPath.Length );

			// Add prefix
			int dir = file.LastIndexOfAny( new[] { Path.AltDirectorySeparatorChar, Path.DirectorySeparatorChar } ) + 1;
			file = file.Insert( dir, prefix );

			// Add postfix
			int ext = file.LastIndexOf( '.' );
			if( ext == -1 )
				ext = file.Length;

			file = file.Insert( ext, postfix );

			return file;
		}
	}
}
