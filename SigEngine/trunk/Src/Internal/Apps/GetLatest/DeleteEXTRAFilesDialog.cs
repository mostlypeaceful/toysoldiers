using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace GetLatest
{
    public partial class DeleteEXTRAFilesDialog : Form
    {
        public DeleteEXTRAFilesDialog( )
        {
            InitializeComponent( );
        }

        private void filesListBox_KeyDown( object sender, KeyEventArgs e )
        {
            if( e.KeyCode == Keys.A && e.Control )
            {
                for( int i = 0; i < filesListBox.Items.Count; ++i )
                    filesListBox.SetSelected( i, true );
            }
        }

        private void deleteButton_Click( object sender, EventArgs e )
        {
            if( filesListBox.SelectedItems.Count < 1 )
            {
                MessageBox.Show( "Nothing selected to delete!" );
                return;
            }

            string message = "Really delete";
            if( filesListBox.SelectedItems.Count > 1 )
                message += " " + filesListBox.SelectedItems.Count + " files";
            message += "?";

            if( MessageBox.Show( message, "Confirm Delete", MessageBoxButtons.YesNo ) != DialogResult.Yes )
                return;

            while( filesListBox.SelectedItems.Count > 0 )
            {
                string file = ( string ) filesListBox.SelectedItems[ 0 ];

                File.Delete( file );
                filesListBox.Items.Remove( file );
            }

            if( filesListBox.Items.Count <= 0 )
                Close( );
        }

        private void closeButton_Click( object sender, EventArgs e )
        {
            Close( );
        }

        private void filesListBox_MouseDoubleClick( object sender, MouseEventArgs e )
        {
            int itemIndex = filesListBox.IndexFromPoint( e.Location );
            if( itemIndex == ListBox.NoMatches )
                return;

            string file = ( string ) filesListBox.Items[ itemIndex ];
            if( file != String.Empty )
                Process.Start( "explorer.exe", Path.GetDirectoryName( file ) );
        }
    }
}
