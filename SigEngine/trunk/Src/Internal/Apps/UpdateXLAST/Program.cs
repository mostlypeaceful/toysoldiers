using System.Xml;
using System.Xml.XPath;
using System.IO;
using System.Text;
using System;
using System.Xml.Linq;

namespace UpdateXLAST
{
    class Program
    {
        static void Main( string[ ] args )
        {
            if( args.Length < 3 )
            {
                Console.WriteLine( "Usage: UpdateXLAST.exe <Audio Bank Directory> <Input File> <Output File>" );
                Environment.Exit( 1 );
            }

            string audioDir = Environment.ExpandEnvironmentVariables( args[ 0 ] );
            string inputFile = Environment.ExpandEnvironmentVariables( args[ 1 ] );
            string outputFile = Environment.ExpandEnvironmentVariables( args[ 2 ] );

            if( !File.Exists( inputFile ) )
            {
                Console.WriteLine( "The input file doesn't exist." );
                Environment.Exit( 1 );
            }

            // Load the XML file and create the object used to navigate through it
            XmlDocument document = new XmlDocument( );
            document.Load( inputFile );
            XPathNavigator navigator = document.CreateNavigator( );

            if( !UpdateAudioFileList( ref audioDir, ref document, ref navigator ) )
            {
                Console.WriteLine( "The audio file list could not be updated." );
                Environment.Exit( 1 );
            }

            document.Save( outputFile );
        }

        static bool UpdateAudioFileList( ref string audioDir, ref XmlDocument document, ref XPathNavigator navigator )
        {
            // Start right at the top of the XML tree
            navigator.MoveToRoot( );

            // Find the audio banks section
            if( !FindNodeWithTargetName( ref document, ref navigator, "banks" ) )
            {
                Console.WriteLine( "The audio file list could not be found." );
                return false;
            }

            // Delete any current children - after DeleteSelf() is called,
            // the navigator returns to the parent
            while( navigator.MoveToFirstChild( ) )
                navigator.DeleteSelf( );

            // Add new files
            string[] fileList = Directory.GetFiles( audioDir );
            foreach( string file in fileList )
            {
                // The file paths need to be relative so that the XLAST will stay valid on any system
                // that it's copied to. The real question is - it needs to be relative TO WHAT
                // DIRECTORY?! I have no idea what folder this is relative to exactly, but it is
                // apparently two levels below the Package directory. If I ever discover what directory
                // this is actually relative to, I would like to properly use the Uri.MakeRelativeUri
                // function
                int pathIndex = file.IndexOf( "Package" );
                if( pathIndex < 0 )
                {
                    Console.WriteLine( "Couldn't make a relative path out of this: " + file );
                    continue;
                }
                string relativePath = "..\\..\\" + file.Substring( pathIndex );

                navigator.AppendChild( "<File clsid=\"{8A0BC3DD-B402-4080-8E34-C22144FC1ECE}\" SourceName=\"" + relativePath + "\" TargetName=\"" + Path.GetFileName( file ) + "\"/>" );
            }

            return true;
        }

        static bool FindNodeWithTargetName( ref XmlDocument document, ref XPathNavigator navigator, string targetName )
        {
            do
            {
                if( navigator.HasChildren )
                {
                    navigator.MoveToFirstChild( );
                    if( FindNodeWithTargetName( ref document, ref navigator, targetName ) == true )
                        return true;
                }

                if( navigator.HasAttributes )
                {
                    string currentNodeTargetName = navigator.GetAttribute( "TargetName", "" );
                    if( currentNodeTargetName == targetName )
                        return true;
                }
            } while( navigator.MoveToNext( ) );

            navigator.MoveToParent( );

            return false;
        }
    }
}
