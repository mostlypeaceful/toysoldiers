using System;
using System.Windows.Forms;
using AutoTest;

namespace PlatformDebuggingClientControlPanel
{
    public partial class PlatformDebuggingTester : Form
    {
        PlatformDebuggingClient debuggingClient;

        public PlatformDebuggingTester( )
        {
            InitializeComponent( );

            serverIPTextBox.Text = "127.0.0.1";

            debuggingClient = new PlatformDebuggingClient( serverIPTextBox.Text );
        }

        private void sendButton_Click( object sender, EventArgs e )
        {
            debuggingClient.Send( stringTextBox.Text.Replace( @"\0", "\0" ) );
        }

        private void reconnectButton_Click( object sender, EventArgs e )
        {
            debuggingClient.Reconnect( serverIPTextBox.Text );
        }
    }
}
