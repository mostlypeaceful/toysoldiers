namespace PlatformMonitor
{
    partial class PlatformMonitorForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(PlatformMonitorForm));
            this.ClientOutputLabel = new System.Windows.Forms.Label();
            this.ClientListView = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.clientIPContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.forceDisconnectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.clientLogContextMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.clearLogToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.scrollToRecentToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.filteringToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.notifyIcon1 = new System.Windows.Forms.NotifyIcon(this.components);
            this.ClientLog = new PlatformMonitor.Log();
            this.clientIPContextMenu.SuspendLayout();
            this.clientLogContextMenu.SuspendLayout();
            this.SuspendLayout();
            // 
            // ClientOutputLabel
            // 
            this.ClientOutputLabel.AutoSize = true;
            this.ClientOutputLabel.Location = new System.Drawing.Point(12, 9);
            this.ClientOutputLabel.Name = "ClientOutputLabel";
            this.ClientOutputLabel.Size = new System.Drawing.Size(68, 13);
            this.ClientOutputLabel.TabIndex = 4;
            this.ClientOutputLabel.Text = "Client Output";
            // 
            // ClientListView
            // 
            this.ClientListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.ClientListView.ContextMenuStrip = this.clientIPContextMenu;
            this.ClientListView.Dock = System.Windows.Forms.DockStyle.Right;
            this.ClientListView.FullRowSelect = true;
            this.ClientListView.GridLines = true;
            this.ClientListView.Location = new System.Drawing.Point(680, 0);
            this.ClientListView.MultiSelect = false;
            this.ClientListView.Name = "ClientListView";
            this.ClientListView.Size = new System.Drawing.Size(104, 411);
            this.ClientListView.TabIndex = 6;
            this.ClientListView.UseCompatibleStateImageBehavior = false;
            this.ClientListView.View = System.Windows.Forms.View.Details;
            this.ClientListView.SelectedIndexChanged += new System.EventHandler(this.ClientListView_SelectedIndexChanged);
            this.ClientListView.Click += new System.EventHandler(this.ClientListView_Click);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "IP Address";
            this.columnHeader1.Width = 96;
            // 
            // clientIPContextMenu
            // 
            this.clientIPContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.forceDisconnectToolStripMenuItem});
            this.clientIPContextMenu.Name = "clientIPContextMenu";
            this.clientIPContextMenu.Size = new System.Drawing.Size(166, 48);
            // 
            // forceDisconnectToolStripMenuItem
            // 
            this.forceDisconnectToolStripMenuItem.Name = "forceDisconnectToolStripMenuItem";
            this.forceDisconnectToolStripMenuItem.Size = new System.Drawing.Size(165, 22);
            this.forceDisconnectToolStripMenuItem.Text = "Force Disconnect";
            this.forceDisconnectToolStripMenuItem.MouseUp += new System.Windows.Forms.MouseEventHandler(this.forceDisconnectToolStripMenuItem_MouseUp);
            // 
            // clientLogContextMenu
            // 
            this.clientLogContextMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.clearLogToolStripMenuItem,
            this.scrollToRecentToolStripMenuItem,
            this.filteringToolStripMenuItem});
            this.clientLogContextMenu.Name = "contextMenuStrip";
            this.clientLogContextMenu.Size = new System.Drawing.Size(160, 70);
            // 
            // clearLogToolStripMenuItem
            // 
            this.clearLogToolStripMenuItem.Name = "clearLogToolStripMenuItem";
            this.clearLogToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.clearLogToolStripMenuItem.Text = "Clear Log";
            this.clearLogToolStripMenuItem.Click += new System.EventHandler(this.clearLogToolStripMenuItem_Click);
            // 
            // scrollToRecentToolStripMenuItem
            // 
            this.scrollToRecentToolStripMenuItem.CheckOnClick = true;
            this.scrollToRecentToolStripMenuItem.Name = "scrollToRecentToolStripMenuItem";
            this.scrollToRecentToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.scrollToRecentToolStripMenuItem.Text = "Scroll To Recent";
            this.scrollToRecentToolStripMenuItem.CheckedChanged += new System.EventHandler(this.scrollToRecentToolStripMenuItem_CheckedChanged);
            // 
            // filteringToolStripMenuItem
            // 
            this.filteringToolStripMenuItem.Name = "filteringToolStripMenuItem";
            this.filteringToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.filteringToolStripMenuItem.Text = "Filtering...";
            // 
            // notifyIcon1
            // 
            this.notifyIcon1.BalloonTipIcon = System.Windows.Forms.ToolTipIcon.Info;
            this.notifyIcon1.BalloonTipText = "Double-Click to reopen";
            this.notifyIcon1.Icon = ((System.Drawing.Icon)(resources.GetObject("notifyIcon1.Icon")));
            this.notifyIcon1.Text = "PlatformMonitor";
            this.notifyIcon1.Visible = true;
            this.notifyIcon1.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.notifyIcon1_MouseDoubleClick);
            // 
            // ClientLog
            // 
            this.ClientLog.AutoWordSelection = true;
            this.ClientLog.BackColor = System.Drawing.Color.White;
            this.ClientLog.ContextMenuStrip = this.clientLogContextMenu;
            this.ClientLog.DetectUrls = false;
            this.ClientLog.Dock = System.Windows.Forms.DockStyle.Left;
            this.ClientLog.ForeColor = System.Drawing.Color.Black;
            this.ClientLog.Location = new System.Drawing.Point(0, 0);
            this.ClientLog.Name = "ClientLog";
            this.ClientLog.ReadOnly = true;
            this.ClientLog.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.ForcedBoth;
            this.ClientLog.ScrollToRecentMessages = true;
            this.ClientLog.ShowSelectionMargin = true;
            this.ClientLog.Size = new System.Drawing.Size(674, 411);
            this.ClientLog.TabIndex = 2;
            this.ClientLog.Text = "";
            this.ClientLog.WordWrap = false;
            // 
            // PlatformMonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(784, 411);
            this.Controls.Add(this.ClientLog);
            this.Controls.Add(this.ClientListView);
            this.Controls.Add(this.ClientOutputLabel);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximumSize = new System.Drawing.Size(800, 960);
            this.MinimumSize = new System.Drawing.Size(800, 260);
            this.Name = "PlatformMonitorForm";
            this.Text = "PlatformMonitor";
            this.Load += new System.EventHandler(this.PlatformMonitorForm_Load);
            this.Resize += new System.EventHandler(this.PlatformMonitorForm_Resize);
            this.clientIPContextMenu.ResumeLayout(false);
            this.clientLogContextMenu.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label ClientOutputLabel;
        private System.Windows.Forms.ListView ClientListView;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private Log ClientLog;
        private System.Windows.Forms.ContextMenuStrip clientLogContextMenu;
        private System.Windows.Forms.ToolStripMenuItem filteringToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem clearLogToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem scrollToRecentToolStripMenuItem;
        private System.Windows.Forms.NotifyIcon notifyIcon1;
        private System.Windows.Forms.ContextMenuStrip clientIPContextMenu;
        private System.Windows.Forms.ToolStripMenuItem forceDisconnectToolStripMenuItem;
    }
}

