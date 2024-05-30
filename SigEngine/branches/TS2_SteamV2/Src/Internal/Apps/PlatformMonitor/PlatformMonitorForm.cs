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
    public partial class PlatformMonitorForm : Form
    {
        PlatformMonitorServer server;
        
        public PlatformMonitorForm()
        {
            InitializeComponent();

            server = new PlatformMonitorServer(this);

            // First add the text and color to a dictionary so it's easy
            // to add more of them later without changing much
            Dictionary<string, Color> filtersList = new Dictionary<string, Color>();
            filtersList.Add("None", Color.Black);
            filtersList.Add("Rtti", Color.Brown);
            filtersList.Add("File", Color.BurlyWood);
            filtersList.Add("Thread", Color.Peru);
            filtersList.Add("Physics", Color.BlueViolet);
            filtersList.Add("Input", Color.Chocolate);
            filtersList.Add("Graphics", Color.DarkBlue);
            filtersList.Add("Resource", Color.DarkGray);
            filtersList.Add("Script", Color.DarkKhaki);
            filtersList.Add("Simulation", Color.DarkSlateGray);
            filtersList.Add("SceneGraph", Color.DodgerBlue);
            filtersList.Add("DevMenu", Color.Firebrick);
            filtersList.Add("Audio", Color.Gold);
            filtersList.Add("Network", Color.Green);
            filtersList.Add("Memory", Color.IndianRed);
            filtersList.Add("Animation", Color.Sienna);
            filtersList.Add("Session", Color.Salmon);
            filtersList.Add("Warning", Color.DarkRed);
            filtersList.Add("Assert", Color.DarkOrange);

            // All all the filtering sub-menu items to the right-click menu
            ToolStripItem filteringToolStripItem = clientLogContextMenu.Items.Find("filteringToolStripMenuItem", false)[0];
            foreach (KeyValuePair<string, Color> filter in filtersList)
            {
                ClientLog.AddToListOfAllFilters(filter.Key);

                ToolStripMenuItem toolStripItem = new ToolStripMenuItem();
                toolStripItem.Text = filter.Key;
                toolStripItem.ForeColor = filter.Value;
                toolStripItem.CheckOnClick = true;
                
                // By linking the event handler first and then setting the Checked property,
                // we cause the event to fire and add this to the log filter list for us too
                toolStripItem.CheckStateChanged += new EventHandler(toolStripItem_CheckStateChanged);
                toolStripItem.Checked = true;
                
                filteringToolStripMenuItem.DropDownItems.Add(toolStripItem);
            }
        }

        void toolStripItem_CheckStateChanged(object sender, EventArgs e)
        {
            ToolStripMenuItem menuItem = (ToolStripMenuItem)sender;

            if (menuItem.Checked)
                ClientLog.AddFilter(menuItem.Text, menuItem.ForeColor);
            else
                ClientLog.RemoveFilter(menuItem.Text);
        }

        private void PlatformMonitorForm_FormClosed(object sender, FormClosedEventArgs e)
        {
        }

        public delegate void OnClientConnectedDelegate(string clientIP);
        public void OnClientConnected(string clientIP)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new OnClientConnectedDelegate(OnClientConnected), clientIP);
                return;
            }
            
            ListViewItem item = new ListViewItem(clientIP);
            item.Name = clientIP; 
            ClientListView.Items.Add(item);
        }

        public delegate void OnClientDisconnectedDelegate(string clientIP);
        public void OnClientDisconnected(string clientIP)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new OnClientDisconnectedDelegate(OnClientDisconnected), clientIP);
                return;
            }
            ClientListView.Items.RemoveByKey(clientIP);

            string line = "----------------------";
            string msg = "CLIENT " + clientIP + " DISCONNECTED";
            ClientLog.AppendLine(LogFormatting.Output, line + msg + line + "\n");
        }

        public delegate void OnClientOutputDelegate(string output);
        public void OnClientOutput(string output)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new OnClientOutputDelegate(OnClientOutput), output);
                return;
            }
            ClientLog.AppendLine(LogFormatting.Output, output);
        }

        public delegate void OnClientAssertDelegate(string output);
        public void OnClientAssert(string output)
        {
            if(this.InvokeRequired)
            {
                this.Invoke(new OnClientAssertDelegate(OnClientAssert), output);
                return;
            }

            // do we need to do this?
            ClientLog.AppendLine(LogFormatting.Output, output);

            AssertResponseDialog dialog = new AssertResponseDialog( server, output );
            dialog.Show( );
        }

        private void PlatformMonitorForm_Load(object sender, EventArgs e)
        {
        }

        private void ClientListView_SelectedIndexChanged(object sender, EventArgs e)
        {
            Console.WriteLine( "ClientListView_SelectedIndexChanged" );
        }

        private void filteringToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Console.WriteLine( "filteringToolStripMenuItem_Click" );
        }

        private void clearLogToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ClientLog.Text = "";
        }

        private void ClientListView_ItemActivate(object sender, EventArgs e)
        {
            Console.WriteLine( "ItemActivated on ClientListView" );
            ListView lv = (ListView)sender;
        }

        private void ClientListView_Click(object sender, EventArgs e)
        {
            ListView list = sender as ListView;
            ListViewItem item = list.SelectedItems[0];
            if( item != null )
            {          
                bool changed = server.SetCurrentClient( item.Name );
                if( changed )
                {
                    string line = "----------------------";
                    string msg = "NOW REPORTING THE LOG OF ";
                    ClientLog.AppendLine(LogFormatting.Output, line + msg + item.Name + line + "\n");
                }
            }            
        }

        private void scrollToRecentToolStripMenuItem_CheckedChanged(object sender, EventArgs e)
        {
            ClientLog.ScrollToRecentMessages = scrollToRecentToolStripMenuItem.Checked;
        }

        private void PlatformMonitorForm_Resize(object sender, EventArgs e)
        {
            if( WindowState == FormWindowState.Minimized )
            {
                this.Hide();
                notifyIcon1.ShowBalloonTip( 3000 );
            }
        }

        private void notifyIcon1_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            this.Show( );
            this.WindowState = FormWindowState.Normal;
        }

        private void forceDisconnectToolStripMenuItem_MouseUp(object sender, MouseEventArgs e)
        {
            Point pt = e.Location;
            ListViewItem item = ClientListView.GetItemAt(pt.X, pt.Y);
            if( item != null )
            {
                Console.WriteLine( "Trying to force disconnect client at " + item.Name );
                if( server.ForceDisconnect( item.Name ) )
                {
                    OnClientDisconnected( item.Name );
                }
            }
        }
    }
}
