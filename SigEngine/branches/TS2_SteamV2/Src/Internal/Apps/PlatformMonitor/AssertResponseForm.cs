using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PlatformMonitor
{
    public partial class AssertResponseDialog : Form
    {
        PlatformMonitorServer server;

        public AssertResponseDialog( PlatformMonitorServer server, string assertMsg )
        {
            this.server = server;
            InitializeComponent();

            // show the assert in the box
            AssertTextBox.Text = assertMsg;
        }

        private void ContinueButton_Click(object sender, EventArgs e)
        {
            server.SendCommand( "assertresponse", "continue" );
            this.Hide( );
        }

        private void BreakpointButton_Click(object sender, EventArgs e)
        {
            server.SendCommand( "assertresponse", "breakpoint" );
            this.Hide( );
        }

        private void CrashButton_Click(object sender, EventArgs e)
        {
            server.SendCommand( "assertresponse", "crash" );
            this.Hide( );
        }
    }
}
