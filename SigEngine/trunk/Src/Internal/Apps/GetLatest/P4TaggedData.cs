using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GetLatest
{
    class P4TaggedData
    {
        List<string> tags = new List<string>( );
        List<string> values = new List<string>( );

        public List<string> Tags
        {
            get { return tags; }
        }

        public List<string> Values
        {
            get { return values; }
        }

        public void Add( string tag, string value )
        {
            tags.Add( tag );
            values.Add( value );
        }

        public string this[ string key ]
        {
            get
            {
                for( int i = 0; i < tags.Count; ++i )
                {
                    if( key == tags[ i ] )
                        return values[ i ];
                }

                return "";
            }
        }

        public string this[ int index ]
        {
            get { return values[ index ]; }
        }

        public int Count
        {
            get { return tags.Count; }
        }
    }
}
