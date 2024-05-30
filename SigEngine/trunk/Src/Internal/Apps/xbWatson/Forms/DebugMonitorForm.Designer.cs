namespace Atg.Samples.xbWatson.Forms
{
    partial class DebugMonitorForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(DebugMonitorForm));
            this.menuStrip = new System.Windows.Forms.MenuStrip();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copySelectionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyAllContentsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.selectAllToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.clearWindowToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.clearWindowAfterRebootToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.addTimestampsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.scrollToRecentMessagesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusStrip = new System.Windows.Forms.StatusStrip();
            this.statusLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.runningProcessLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.rebootToolStripSplitButton = new System.Windows.Forms.ToolStripSplitButton();
            this.rebootToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.coldRebootToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statusImageList = new System.Windows.Forms.ImageList(this.components);
            this.contextMenuStrip = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.copySelectionToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.copyAllContentsToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.selectAllToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.clearWindowToolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.filteringToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.log = new Atg.Samples.xbWatson.Log();
            this.saveFileDialog = new System.Windows.Forms.SaveFileDialog();
            this.menuStrip.SuspendLayout();
            this.statusStrip.SuspendLayout();
            this.contextMenuStrip.SuspendLayout();
            this.SuspendLayout();
            // 
            // menuStrip
            // 
            this.menuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.editToolStripMenuItem});
            this.menuStrip.Location = new System.Drawing.Point(0, 0);
            this.menuStrip.Name = "menuStrip";
            this.menuStrip.Size = new System.Drawing.Size(560, 24);
            this.menuStrip.TabIndex = 0;
            this.menuStrip.Text = "menuStrip1";
            this.menuStrip.Visible = false;
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.copySelectionToolStripMenuItem,
            this.copyAllContentsToolStripMenuItem,
            this.selectAllToolStripMenuItem,
            this.toolStripMenuItem1,
            this.clearWindowToolStripMenuItem,
            this.clearWindowAfterRebootToolStripMenuItem,
            this.toolStripMenuItem2,
            this.addTimestampsToolStripMenuItem,
            this.scrollToRecentMessagesToolStripMenuItem});
            this.editToolStripMenuItem.MergeAction = System.Windows.Forms.MergeAction.Insert;
            this.editToolStripMenuItem.MergeIndex = 1;
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.editToolStripMenuItem.Text = "&Edit";
            // 
            // copySelectionToolStripMenuItem
            // 
            this.copySelectionToolStripMenuItem.Name = "copySelectionToolStripMenuItem";
            this.copySelectionToolStripMenuItem.Size = new System.Drawing.Size(218, 22);
            this.copySelectionToolStripMenuItem.Text = "&Copy Selection";
            this.copySelectionToolStripMenuItem.Click += new System.EventHandler(this.copySelectionToolStripMenuItem_Click);
            // 
            // copyAllContentsToolStripMenuItem
            // 
            this.copyAllContentsToolStripMenuItem.Name = "copyAllContentsToolStripMenuItem";
            this.copyAllContentsToolStripMenuItem.Size = new System.Drawing.Size(218, 22);
            this.copyAllContentsToolStripMenuItem.Text = "Copy &All Contents";
            this.copyAllContentsToolStripMenuItem.Click += new System.EventHandler(this.copyAllContentsToolStripMenuItem_Click);
            // 
            // selectAllToolStripMenuItem
            // 
            this.selectAllToolStripMenuItem.Name = "selectAllToolStripMenuItem";
            this.selectAllToolStripMenuItem.Size = new System.Drawing.Size(218, 22);
            this.selectAllToolStripMenuItem.Text = "&Select All";
            this.selectAllToolStripMenuItem.Click += new System.EventHandler(this.selectAllToolStripMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(215, 6);
            // 
            // clearWindowToolStripMenuItem
            // 
            this.clearWindowToolStripMenuItem.Name = "clearWindowToolStripMenuItem";
            this.clearWindowToolStripMenuItem.Size = new System.Drawing.Size(218, 22);
            this.clearWindowToolStripMenuItem.Text = "Clear &Window";
            this.clearWindowToolStripMenuItem.Click += new System.EventHandler(this.clearWindowToolStripMenuItem_Click);
            // 
            // clearWindowAfterRebootToolStripMenuItem
            // 
            this.clearWindowAfterRebootToolStripMenuItem.CheckOnClick = true;
            this.clearWindowAfterRebootToolStripMenuItem.Name = "clearWindowAfterRebootToolStripMenuItem";
            this.clearWindowAfterRebootToolStripMenuItem.Size = new System.Drawing.Size(218, 22);
            this.clearWindowAfterRebootToolStripMenuItem.Text = "Clear Window After &Reboot";
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(215, 6);
            // 
            // addTimestampsToolStripMenuItem
            // 
            this.addTimestampsToolStripMenuItem.CheckOnClick = true;
            this.addTimestampsToolStripMenuItem.Name = "addTimestampsToolStripMenuItem";
            this.addTimestampsToolStripMenuItem.Size = new System.Drawing.Size(218, 22);
            this.addTimestampsToolStripMenuItem.Text = "Add &Timestamps";
            // 
            // scrollToRecentMessagesToolStripMenuItem
            // 
            this.scrollToRecentMessagesToolStripMenuItem.CheckOnClick = true;
            this.scrollToRecentMessagesToolStripMenuItem.Name = "scrollToRecentMessagesToolStripMenuItem";
            this.scrollToRecentMessagesToolStripMenuItem.Size = new System.Drawing.Size(218, 22);
            this.scrollToRecentMessagesToolStripMenuItem.Text = "Scroll to Recent &Messages";
            this.scrollToRecentMessagesToolStripMenuItem.CheckedChanged += new System.EventHandler(this.scrollToRecentMessagesToolStripMenuItem_CheckedChanged);
            // 
            // statusStrip
            // 
            this.statusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.statusLabel,
            this.runningProcessLabel,
            this.rebootToolStripSplitButton});
            this.statusStrip.Location = new System.Drawing.Point(0, 302);
            this.statusStrip.Name = "statusStrip";
            this.statusStrip.Size = new System.Drawing.Size(560, 22);
            this.statusStrip.SizingGrip = false;
            this.statusStrip.TabIndex = 1;
            this.statusStrip.Text = "statusStrip1";
            // 
            // statusLabel
            // 
            this.statusLabel.ImageScaling = System.Windows.Forms.ToolStripItemImageScaling.None;
            this.statusLabel.Name = "statusLabel";
            this.statusLabel.Size = new System.Drawing.Size(54, 17);
            this.statusLabel.Text = "<status>";
            // 
            // runningProcessLabel
            // 
            this.runningProcessLabel.Name = "runningProcessLabel";
            this.runningProcessLabel.Size = new System.Drawing.Size(414, 17);
            this.runningProcessLabel.Spring = true;
            this.runningProcessLabel.Text = "<runningProcess>";
            this.runningProcessLabel.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // rebootToolStripSplitButton
            // 
            this.rebootToolStripSplitButton.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.rebootToolStripMenuItem,
            this.coldRebootToolStripMenuItem});
            this.rebootToolStripSplitButton.Image = ((System.Drawing.Image)(resources.GetObject("rebootToolStripSplitButton.Image")));
            this.rebootToolStripSplitButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.rebootToolStripSplitButton.Name = "rebootToolStripSplitButton";
            this.rebootToolStripSplitButton.Size = new System.Drawing.Size(77, 20);
            this.rebootToolStripSplitButton.Text = "&Reboot";
            this.rebootToolStripSplitButton.ButtonClick += new System.EventHandler(this.rebootToolStripSplitButton_ButtonClick);
            // 
            // rebootToolStripMenuItem
            // 
            this.rebootToolStripMenuItem.Font = new System.Drawing.Font("Segoe UI", 9F, System.Drawing.FontStyle.Bold);
            this.rebootToolStripMenuItem.Name = "rebootToolStripMenuItem";
            this.rebootToolStripMenuItem.Size = new System.Drawing.Size(148, 22);
            this.rebootToolStripMenuItem.Text = "&Reboot";
            this.rebootToolStripMenuItem.Click += new System.EventHandler(this.rebootToolStripSplitButton_ButtonClick);
            // 
            // coldRebootToolStripMenuItem
            // 
            this.coldRebootToolStripMenuItem.Name = "coldRebootToolStripMenuItem";
            this.coldRebootToolStripMenuItem.Size = new System.Drawing.Size(148, 22);
            this.coldRebootToolStripMenuItem.Text = "Reboot (&Cold)";
            this.coldRebootToolStripMenuItem.Click += new System.EventHandler(this.coldRebootToolStripMenuItem_Click);
            // 
            // statusImageList
            // 
            this.statusImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("statusImageList.ImageStream")));
            this.statusImageList.TransparentColor = System.Drawing.Color.Fuchsia;
            this.statusImageList.Images.SetKeyName(0, "Disconnected.bmp");
            this.statusImageList.Images.SetKeyName(1, "Retry.bmp");
            this.statusImageList.Images.SetKeyName(2, "Stop.bmp");
            this.statusImageList.Images.SetKeyName(3, "Run.bmp");
            this.statusImageList.Images.SetKeyName(4, "Pause.bmp");
            this.statusImageList.Images.SetKeyName(5, "RolledBack.bmp");
            // 
            // contextMenuStrip
            // 
            this.contextMenuStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.copySelectionToolStripMenuItem1,
            this.copyAllContentsToolStripMenuItem1,
            this.selectAllToolStripMenuItem1,
            this.saveAsToolStripMenuItem1,
            this.toolStripSeparator1,
            this.clearWindowToolStripMenuItem1,
            this.toolStripSeparator2,
            this.filteringToolStripMenuItem});
            this.contextMenuStrip.Name = "contextMenuStrip";
            this.contextMenuStrip.Size = new System.Drawing.Size(171, 170);
            // 
            // copySelectionToolStripMenuItem1
            // 
            this.copySelectionToolStripMenuItem1.Name = "copySelectionToolStripMenuItem1";
            this.copySelectionToolStripMenuItem1.Size = new System.Drawing.Size(170, 22);
            this.copySelectionToolStripMenuItem1.Text = "Copy Selection";
            this.copySelectionToolStripMenuItem1.Click += new System.EventHandler(this.copySelectionToolStripMenuItem1_Click);
            // 
            // copyAllContentsToolStripMenuItem1
            // 
            this.copyAllContentsToolStripMenuItem1.Name = "copyAllContentsToolStripMenuItem1";
            this.copyAllContentsToolStripMenuItem1.Size = new System.Drawing.Size(170, 22);
            this.copyAllContentsToolStripMenuItem1.Text = "Copy All Contents";
            this.copyAllContentsToolStripMenuItem1.Click += new System.EventHandler(this.copyAllContentsToolStripMenuItem1_Click);
            // 
            // selectAllToolStripMenuItem1
            // 
            this.selectAllToolStripMenuItem1.Name = "selectAllToolStripMenuItem1";
            this.selectAllToolStripMenuItem1.Size = new System.Drawing.Size(170, 22);
            this.selectAllToolStripMenuItem1.Text = "Select All";
            this.selectAllToolStripMenuItem1.Click += new System.EventHandler(this.selectAllToolStripMenuItem1_Click);
            // 
            // saveAsToolStripMenuItem1
            // 
            this.saveAsToolStripMenuItem1.Name = "saveAsToolStripMenuItem1";
            this.saveAsToolStripMenuItem1.Size = new System.Drawing.Size(170, 22);
            this.saveAsToolStripMenuItem1.Text = "Save As...";
            this.saveAsToolStripMenuItem1.Click += new System.EventHandler(this.saveAsToolStripMenuItem1_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(167, 6);
            // 
            // clearWindowToolStripMenuItem1
            // 
            this.clearWindowToolStripMenuItem1.Name = "clearWindowToolStripMenuItem1";
            this.clearWindowToolStripMenuItem1.Size = new System.Drawing.Size(170, 22);
            this.clearWindowToolStripMenuItem1.Text = "Clear Window";
            this.clearWindowToolStripMenuItem1.Click += new System.EventHandler(this.clearWindowToolStripMenuItem1_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(167, 6);
            // 
            // filteringToolStripMenuItem
            // 
            this.filteringToolStripMenuItem.Name = "filteringToolStripMenuItem";
            this.filteringToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
            this.filteringToolStripMenuItem.Text = "Filtering...";
            // 
            // log
            // 
            this.log.AutoWordSelection = true;
            this.log.BackColor = System.Drawing.Color.White;
            this.log.ContextMenuStrip = this.contextMenuStrip;
            this.log.DetectUrls = false;
            this.log.Dock = System.Windows.Forms.DockStyle.Fill;
            this.log.ForeColor = System.Drawing.Color.Black;
            this.log.Location = new System.Drawing.Point(0, 0);
            this.log.Name = "log";
            this.log.ReadOnly = true;
            this.log.ScrollBars = System.Windows.Forms.RichTextBoxScrollBars.ForcedBoth;
            this.log.ScrollToRecentMessages = false;
            this.log.ShowSelectionMargin = true;
            this.log.Size = new System.Drawing.Size(560, 302);
            this.log.TabIndex = 2;
            this.log.Text = "";
            this.log.WordWrap = false;
            // 
            // DebugMonitorForm
            // 
            this.ClientSize = new System.Drawing.Size(560, 324);
            this.Controls.Add(this.log);
            this.Controls.Add(this.statusStrip);
            this.Controls.Add(this.menuStrip);
            this.Font = new System.Drawing.Font("Tahoma", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip;
            this.Name = "DebugMonitorForm";
            this.Text = "<title>";
            this.Load += new System.EventHandler(this.DebugMonitorForm_Load);
            this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.DebugMonitorForm_FormClosed);
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.DebugMonitorForm_FormClosing);
            this.menuStrip.ResumeLayout(false);
            this.menuStrip.PerformLayout();
            this.statusStrip.ResumeLayout(false);
            this.statusStrip.PerformLayout();
            this.contextMenuStrip.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copySelectionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyAllContentsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem selectAllToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem clearWindowToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.ToolStripMenuItem addTimestampsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem clearWindowAfterRebootToolStripMenuItem;
        private System.Windows.Forms.StatusStrip statusStrip;
        private System.Windows.Forms.ToolStripStatusLabel statusLabel;
        private System.Windows.Forms.ToolStripMenuItem scrollToRecentMessagesToolStripMenuItem;
        private xbWatson.Log log;
        private System.Windows.Forms.ImageList statusImageList;
        private System.Windows.Forms.ToolStripStatusLabel runningProcessLabel;
        private System.Windows.Forms.ToolStripSplitButton rebootToolStripSplitButton;
        private System.Windows.Forms.ToolStripMenuItem coldRebootToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem rebootToolStripMenuItem;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip;
        private System.Windows.Forms.ToolStripMenuItem copySelectionToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem copyAllContentsToolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem selectAllToolStripMenuItem1;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem clearWindowToolStripMenuItem1;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem filteringToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem1;
        private System.Windows.Forms.SaveFileDialog saveFileDialog;
    }
}