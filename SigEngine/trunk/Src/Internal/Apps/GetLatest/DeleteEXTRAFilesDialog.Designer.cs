namespace GetLatest
{
    partial class DeleteEXTRAFilesDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose( bool disposing )
        {
            if( disposing && ( components != null ) )
            {
                components.Dispose( );
            }
            base.Dispose( disposing );
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent( )
        {
            this.closeButton = new System.Windows.Forms.Button( );
            this.deleteButton = new System.Windows.Forms.Button( );
            this.messageLabel = new System.Windows.Forms.Label( );
            this.filesListBox = new System.Windows.Forms.ListBox( );
            this.SuspendLayout( );
            // 
            // closeButton
            // 
            this.closeButton.Anchor = ( ( System.Windows.Forms.AnchorStyles ) ( ( System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right ) ) );
            this.closeButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.closeButton.Location = new System.Drawing.Point( 447, 227 );
            this.closeButton.Name = "closeButton";
            this.closeButton.Size = new System.Drawing.Size( 75, 23 );
            this.closeButton.TabIndex = 1;
            this.closeButton.Text = "Close";
            this.closeButton.UseVisualStyleBackColor = true;
            this.closeButton.Click += new System.EventHandler( this.closeButton_Click );
            // 
            // deleteButton
            // 
            this.deleteButton.Anchor = ( ( System.Windows.Forms.AnchorStyles ) ( ( System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right ) ) );
            this.deleteButton.Location = new System.Drawing.Point( 366, 227 );
            this.deleteButton.Name = "deleteButton";
            this.deleteButton.Size = new System.Drawing.Size( 75, 23 );
            this.deleteButton.TabIndex = 2;
            this.deleteButton.Text = "Delete";
            this.deleteButton.UseVisualStyleBackColor = true;
            this.deleteButton.Click += new System.EventHandler( this.deleteButton_Click );
            // 
            // messageLabel
            // 
            this.messageLabel.Location = new System.Drawing.Point( 12, 9 );
            this.messageLabel.Name = "messageLabel";
            this.messageLabel.Size = new System.Drawing.Size( 491, 34 );
            this.messageLabel.TabIndex = 3;
            this.messageLabel.Text = "The following binary assets are not on the server, but are present on your local " +
                "machine. You can choose to delete them if you wish.";
            this.messageLabel.TextAlign = System.Drawing.ContentAlignment.TopCenter;
            // 
            // filesListBox
            // 
            this.filesListBox.Anchor = ( ( System.Windows.Forms.AnchorStyles ) ( ( ( ( System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom )
                        | System.Windows.Forms.AnchorStyles.Left )
                        | System.Windows.Forms.AnchorStyles.Right ) ) );
            this.filesListBox.FormattingEnabled = true;
            this.filesListBox.HorizontalScrollbar = true;
            this.filesListBox.Location = new System.Drawing.Point( 12, 46 );
            this.filesListBox.Name = "filesListBox";
            this.filesListBox.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.filesListBox.Size = new System.Drawing.Size( 510, 173 );
            this.filesListBox.TabIndex = 4;
            this.filesListBox.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler( this.filesListBox_MouseDoubleClick );
            this.filesListBox.KeyDown += new System.Windows.Forms.KeyEventHandler( this.filesListBox_KeyDown );
            // 
            // DeleteEXTRAFilesDialog
            // 
            this.AcceptButton = this.deleteButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF( 6F, 13F );
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.closeButton;
            this.ClientSize = new System.Drawing.Size( 534, 262 );
            this.ControlBox = false;
            this.Controls.Add( this.filesListBox );
            this.Controls.Add( this.messageLabel );
            this.Controls.Add( this.deleteButton );
            this.Controls.Add( this.closeButton );
            this.MinimumSize = new System.Drawing.Size( 550, 300 );
            this.Name = "DeleteEXTRAFilesDialog";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Delete EXTRA Files";
            this.ResumeLayout( false );

        }

        #endregion

        private System.Windows.Forms.Button closeButton;
        private System.Windows.Forms.Button deleteButton;
        private System.Windows.Forms.Label messageLabel;
        private System.Windows.Forms.ListBox filesListBox;
    }
}