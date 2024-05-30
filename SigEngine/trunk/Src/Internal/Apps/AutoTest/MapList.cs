using System;
using System.Collections.Generic;
using System.IO;

namespace AutoTest
{
    public enum Platforms { xbox360, pcdx9 };

    public class MapList
    {
        private TextReader mapFile;
        private List<Map> pathList = new List<Map>( );
        private Platforms currentPlatform = Platforms.xbox360; // Default to Xbox 360

        public MapList( string mapFilePath = @"\test\bin\map_list.txt" )
        {
            // Fully qualify the file path
            string fullPath = DebugMonitor.ProjectPath + mapFilePath;

            // Open the map list file
            try
            {
                using( mapFile = new StreamReader( fullPath ) )
                {
                    string line = String.Empty;
                    while( ( line = mapFile.ReadLine( ) ) != null )
                    {
                        if( String.IsNullOrWhiteSpace( line ) )
                            continue;

                        switch(line)
                        {
                            case "platform_xbox360":
                                currentPlatform = Platforms.xbox360;
                                break;
                            case "platform_pcdx9":
                                currentPlatform = Platforms.pcdx9;
                                break;
                            default:
                                pathList.Add( new Map( line, currentPlatform ) );
                                break;
                        }
                    }
                }
            }
            catch( System.IO.FileNotFoundException )
            {
                throw new FileNotFoundException( "The map list file could not be found!", fullPath );
            }
        }

        public int MapCount
        {
            get { return pathList.Count; }
        }

        public Map this[ int mapID ]
        {
            get
            {
                if( mapID > MapCount )
                    mapID = 0;

                return pathList[ mapID ];
            }
        }

        public Platforms CurrentPlatform
        {
            get { return currentPlatform; }
        }
    }
}
