using System.Diagnostics;
using System.Text.RegularExpressions;

namespace GetLatest
{
    class P4Process : Process
    {
        public P4Process( string arguments )
        {
            StartInfo = base.StartInfo;
            StartInfo.UseShellExecute = false;
            StartInfo.RedirectStandardOutput = true;
            StartInfo.RedirectStandardError = true;
            StartInfo.RedirectStandardInput = true;
            StartInfo.CreateNoWindow = true;
            StartInfo.FileName = "p4";
            StartInfo.Arguments = "-z tag " + arguments;
        }

        public static string[] GetTagData( string line )
        {
            string[] data = new string[ 2 ]{"",""};

            if( line == null || line == "" )
                return data;

            Regex pattern = new Regex( @"^\.\.\. (.+) (.+)\r*$", RegexOptions.Multiline );

            Match match = pattern.Match( line );
            if( match.Groups.Count < 3 )
                return data;

            data[ 0 ] = match.Groups[ 1 ].ToString( ).Trim( );
            data[ 1 ] = match.Groups[ 2 ].ToString( ).Trim( );

            return data;
        }

        public static string GetP4SetVariable( string variableName )
        {
            P4Process process = new P4Process( "set " + variableName );
            process.Start( );
            process.WaitForExit( );
            string value = process.StandardOutput.ReadToEnd( );

            string[ ] splitData = value.Split( "=".ToCharArray( ), 2 );
            if( splitData.Length >= 2 )
                value = splitData[ 1 ];
            value = value.Replace( "(set)", "" ).Trim( );

            return value;
        }
    }
}
