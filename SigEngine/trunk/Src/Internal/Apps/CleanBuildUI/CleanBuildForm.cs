using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Forms;

namespace CleanBuildUI
{
    public partial class CleanBuildForm : Form
    {
        static string cleanbuildPath = Environment.ExpandEnvironmentVariables( @"%SigCurrentProject%\cleanbuild" );

        static Dictionary<CheckBox, string[ ]> groupExtensions = new Dictionary<CheckBox, string[ ]>( );

        public CleanBuildForm( )
        {
            InitializeComponent( );

            InitializeGroupExtensions( );

            InitializeGuiState( );
        }

        private void InitializeGroupExtensions( )
        {
            groupExtensions.Add( tablesGroupCheckBox, new string[ ] { ".tabb" } );
            groupExtensions.Add( texturesGroupCheckBox, new string[ ] { ".bmpb", ".ddsb", ".jpgb", ".pngb", ".tgab"} );
            groupExtensions.Add( shadersGroupCheckBox, new string[ ] { ".derb" } );
            groupExtensions.Add( levelMeshesGroupCheckBox, new string[ ] { ".sigb", ".texb", ".geob" } );
            groupExtensions.Add( skeletonGroupCheckBox, new string[ ] { ".sklb" } );
            groupExtensions.Add( flashMoviesGroupCheckBox, new string[ ] { ".swfb" } );
        }

        private void InitializeGuiState( )
        {
            RefreshExtensionList( );

            RefreshExtensionListState( );

            RefreshGroupListState( );
        }

        private void cancelButton_Click( object sender, EventArgs e )
        {
            ExitApplication( );
        }

        private void okButton_Click( object sender, EventArgs e )
        {
            DialogResult result;
            if( fullCleanButton.Checked )
            {
                result = MessageBox.Show( "WARNING: You have selected a full clean and rebuild of all code and assets. This can take a long time. Are you sure?",
                                                         "Full Clean Selected!", MessageBoxButtons.OKCancel );
            }
            else if( extensionCleanButton.Checked && CheckedExtensionsCount > 0 )
            {
                string message = "The following extensions will be added to the default changelist for cleaning:\n";
                foreach( string extension in CheckedExtensions )
                    message += "\t\n" + extension;
                message += "\n\nAre you sure?";

                result = MessageBox.Show( message, "Extension-Specific Cleaning Selected!", MessageBoxButtons.OKCancel );
            }
            else
            {
                result = MessageBox.Show( "No extensions selected! Revert cleanbuild file?", "Nothing Selected!", MessageBoxButtons.OKCancel );
                if( result == DialogResult.OK )
                {
                    bool reverted = P4Process.RevertFile( cleanbuildPath );
                    ExitApplication( );
                }

                return;
            }

            if( result != DialogResult.OK )
                return;

            bool checkedOut = P4Process.CheckOutFile( cleanbuildPath );

            if( fullCleanButton.Checked )
                File.WriteAllText( cleanbuildPath, "1" );
            else
            {
                string contents = "";
                foreach( string extension in CheckedExtensions )
                    contents += extension + "\n";
                File.WriteAllText( cleanbuildPath, contents );
            }

            ExitApplication( );
        }

        private void fullCleanButton_CheckedChanged( object sender, EventArgs e )
        {
            extensionFlowControlPanel.Enabled = fullCleanButton.Checked ? false : true;
            fileGroupsBox.Enabled = fullCleanButton.Checked ? false : true;
        }

        public void RefreshExtensionList(  )
        {
            List<string> extensionList = new List<string>( );
            GetExtensionsFromDirectory( Environment.ExpandEnvironmentVariables( "%SigCurrentProject%\\game" ), ref extensionList );
            extensionList.Sort( );

            extensionFlowControlPanel.Controls.Clear( );
            foreach( string extension in extensionList )
            {
                CheckBox checkbox = new CheckBox( );
                checkbox.Text = extension;

                extensionFlowControlPanel.Controls.Add( checkbox );
            }
        }

        private void RefreshExtensionListState( )
        {
            string[ ] lines = File.ReadAllLines( cleanbuildPath );
            if( lines.Length < 1 )
                return;

            if( lines[ 0 ] == "1" )
                fullCleanButton.Checked = true;
            else
            {
                extensionCleanButton.Checked = true;

                for( int i = 0; i < lines.Length; ++i )
                {
                    string line = lines[ i ];

                    // Remove star at the beginning
                    if( line[ 0 ] == '*' )
                        line = line.Substring( 1 );

                    // Add leading dot
                    if( line[ 0 ] != '.' )
                        line = "." + line;

                    for( int j = 0; j < extensionFlowControlPanel.Controls.Count; ++j )
                    {
                        string extension = ( string ) extensionFlowControlPanel.Controls[ j ].Text;
                        if( line == extension )
                        {
                            CheckBox checkbox = ( CheckBox ) extensionFlowControlPanel.Controls[ j ];
                            checkbox.Checked = true;
                        }
                    }
                }
            }
        }

        public void RefreshGroupListState( )
        {
            // If all of the associated extensions for a group checkbox are
            // checked, also check the group checkbox
            List<string> checkedExtensions = CheckedExtensions;
            foreach( KeyValuePair<CheckBox, string[]> group in groupExtensions )
            {
                bool allChecked = true;
                foreach( string extension in group.Value )
                {
                    if( Extensions.Contains( extension) && !CheckedExtensions.Contains( extension ) )
                    {
                        allChecked = false;
                        break;
                    }
                }
                if( allChecked )
                    group.Key.Checked = true;
            }
        }

        public void GetExtensionsFromDirectory( string topDirectory, ref List<string> extensionList )
        {
            // Process the list of files found in the directory
            string[ ] fileEntries = Directory.GetFiles( topDirectory );
            foreach( string fileName in fileEntries )
            {
                string extension = Path.GetExtension( fileName );
                if( !extensionList.Contains( extension ) )
                    extensionList.Add( extension );
            }

            // Recurse into subdirectories of this directory.
            string[ ] subdirectoryEntries = Directory.GetDirectories( topDirectory );
            foreach( string subdirectory in subdirectoryEntries )
                GetExtensionsFromDirectory( subdirectory, ref extensionList );
        }

        private void UpdatedExtensionCheckedState( string extension, bool check )
        {
            foreach( Control control in extensionFlowControlPanel.Controls )
            {
                if( control.Text == extension )
                {
                    ( ( CheckBox ) control ).Checked = check;
                    break;
                }
            }
        }

        private void UpdateGroupExtensionStates( CheckBox groupCheckBox )
        {
            foreach( string extension in groupExtensions[ groupCheckBox ] )
                UpdatedExtensionCheckedState( extension, groupCheckBox.Checked );
        }

        private void tablesGroupCheckBox_CheckedChanged( object sender, EventArgs e )
        {
            UpdateGroupExtensionStates( tablesGroupCheckBox );
        }

        private void texturesGroupCheckBox_CheckedChanged( object sender, EventArgs e )
        {
            UpdateGroupExtensionStates( texturesGroupCheckBox );
        }

        private void shadersGroupCheckBox_CheckedChanged( object sender, EventArgs e )
        {
            UpdateGroupExtensionStates( shadersGroupCheckBox );
        }

        private void levelMeshesGroupCheckBox_CheckedChanged( object sender, EventArgs e )
        {
            UpdateGroupExtensionStates( levelMeshesGroupCheckBox );
        }

        private void skeletonGroupCheckBox_CheckedChanged( object sender, EventArgs e )
        {
            UpdateGroupExtensionStates( skeletonGroupCheckBox );
        }

        private void flashMoviesGroupCheckBox_CheckedChanged( object sender, EventArgs e )
        {
            UpdateGroupExtensionStates( flashMoviesGroupCheckBox );
        }

        private int CheckedExtensionsCount
        {
            get
            {
                int count = 0;
                foreach( Control control in extensionFlowControlPanel.Controls )
                {
                    if( ( ( CheckBox ) control ).Checked )
                        ++count;
                }

                return count;
            }
        }

        private List<string> Extensions
        {
            get
            {
                List<string> extensionList = new List<string>( );

                foreach( Control control in extensionFlowControlPanel.Controls )
                        extensionList.Add( control.Text );

                return extensionList;
            }
        }

        private List<string> CheckedExtensions
        {
            get
            {
                List<string> extensionList = new List<string>( );

                foreach( Control control in extensionFlowControlPanel.Controls )
                {
                    if( ( ( CheckBox ) control ).Checked )
                        extensionList.Add( control.Text );
                }

                return extensionList;
            }
        }

        private void ExitApplication( )
        {
            Application.Exit( );
        }
    }
}
