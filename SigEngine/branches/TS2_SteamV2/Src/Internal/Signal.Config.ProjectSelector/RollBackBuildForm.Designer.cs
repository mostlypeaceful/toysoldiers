//------------------------------------------------------------------------------
// \file RollBackBuildForm.Designer.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

namespace Signal.Config.ProjectSelector
{
	partial class RollBackBuildForm
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
			if( disposing && (components != null) )
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
			this.lblExistingBuilds = new System.Windows.Forms.Label();
			this.dgvExistingBuilds = new System.Windows.Forms.DataGridView();
			this.colActive = new System.Windows.Forms.DataGridViewImageColumn();
			this.colBuildName = new System.Windows.Forms.DataGridViewTextBoxColumn();
			this.bClose = new System.Windows.Forms.Button();
			((System.ComponentModel.ISupportInitialize)(this.dgvExistingBuilds)).BeginInit();
			this.SuspendLayout();
			// 
			// lblExistingBuilds
			// 
			this.lblExistingBuilds.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.lblExistingBuilds.Location = new System.Drawing.Point(12, 9);
			this.lblExistingBuilds.Name = "lblExistingBuilds";
			this.lblExistingBuilds.Size = new System.Drawing.Size(496, 17);
			this.lblExistingBuilds.TabIndex = 0;
			this.lblExistingBuilds.Text = "Existing Builds";
			this.lblExistingBuilds.TextAlign = System.Drawing.ContentAlignment.TopCenter;
			// 
			// dgvExistingBuilds
			// 
			this.dgvExistingBuilds.AllowUserToAddRows = false;
			this.dgvExistingBuilds.AllowUserToDeleteRows = false;
			this.dgvExistingBuilds.AllowUserToResizeRows = false;
			this.dgvExistingBuilds.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.dgvExistingBuilds.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.Fill;
			this.dgvExistingBuilds.BackgroundColor = System.Drawing.SystemColors.Window;
			this.dgvExistingBuilds.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
			this.dgvExistingBuilds.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.colActive,
            this.colBuildName});
			this.dgvExistingBuilds.Location = new System.Drawing.Point(12, 29);
			this.dgvExistingBuilds.MultiSelect = false;
			this.dgvExistingBuilds.Name = "dgvExistingBuilds";
			this.dgvExistingBuilds.RowHeadersVisible = false;
			this.dgvExistingBuilds.Size = new System.Drawing.Size(496, 252);
			this.dgvExistingBuilds.TabIndex = 1;
			this.dgvExistingBuilds.CellDoubleClick += new System.Windows.Forms.DataGridViewCellEventHandler(this.dgvExistingBuilds_CellDoubleClick);
			// 
			// colActive
			// 
			this.colActive.FillWeight = 1E-05F;
			this.colActive.HeaderText = "Active";
			this.colActive.MinimumWidth = 48;
			this.colActive.Name = "colActive";
			this.colActive.ReadOnly = true;
			// 
			// colBuildName
			// 
			this.colBuildName.FillWeight = 75.63452F;
			this.colBuildName.HeaderText = "Build";
			this.colBuildName.Name = "colBuildName";
			this.colBuildName.ReadOnly = true;
			// 
			// bClose
			// 
			this.bClose.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
			this.bClose.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.bClose.Location = new System.Drawing.Point(433, 287);
			this.bClose.Name = "bClose";
			this.bClose.Size = new System.Drawing.Size(75, 23);
			this.bClose.TabIndex = 2;
			this.bClose.Text = "Close";
			this.bClose.UseVisualStyleBackColor = true;
			this.bClose.Click += new System.EventHandler(this.bClose_Click);
			// 
			// RollBackBuildForm
			// 
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.bClose;
			this.ClientSize = new System.Drawing.Size(520, 322);
			this.Controls.Add(this.bClose);
			this.Controls.Add(this.dgvExistingBuilds);
			this.Controls.Add(this.lblExistingBuilds);
			this.Name = "RollBackBuildForm";
			this.Text = "Roll Back Build";
			this.Load += new System.EventHandler(this.RollBackBuildForm_Load);
			((System.ComponentModel.ISupportInitialize)(this.dgvExistingBuilds)).EndInit();
			this.ResumeLayout(false);

		}

		#endregion

		private System.Windows.Forms.Label lblExistingBuilds;
		private System.Windows.Forms.DataGridView dgvExistingBuilds;
		private System.Windows.Forms.Button bClose;
		private System.Windows.Forms.DataGridViewImageColumn colActive;
		private System.Windows.Forms.DataGridViewTextBoxColumn colBuildName;
	}
}