//------------------------------------------------------------------------------
// \file RenameSomethingForm.Designer.cs - 1 Mar 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

namespace Signal.Config.EditGameFlags
{
	partial class RenameSomethingForm
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
			this.lblOriginalName = new System.Windows.Forms.Label();
			this.tbOriginalName = new System.Windows.Forms.TextBox();
			this.lblNewName = new System.Windows.Forms.Label();
			this.tbNewName = new System.Windows.Forms.TextBox();
			this.bRename = new System.Windows.Forms.Button();
			this.bCancel = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// lblOriginalName
			// 
			this.lblOriginalName.AutoSize = true;
			this.lblOriginalName.Location = new System.Drawing.Point(12, 15);
			this.lblOriginalName.Name = "lblOriginalName";
			this.lblOriginalName.Size = new System.Drawing.Size(76, 13);
			this.lblOriginalName.TabIndex = 0;
			this.lblOriginalName.Text = "Original Name:";
			// 
			// tbOriginalName
			// 
			this.tbOriginalName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbOriginalName.Location = new System.Drawing.Point(90, 12);
			this.tbOriginalName.Name = "tbOriginalName";
			this.tbOriginalName.ReadOnly = true;
			this.tbOriginalName.Size = new System.Drawing.Size(182, 20);
			this.tbOriginalName.TabIndex = 1;
			// 
			// lblNewName
			// 
			this.lblNewName.AutoSize = true;
			this.lblNewName.Location = new System.Drawing.Point(12, 41);
			this.lblNewName.Name = "lblNewName";
			this.lblNewName.Size = new System.Drawing.Size(63, 13);
			this.lblNewName.TabIndex = 2;
			this.lblNewName.Text = "New Name:";
			// 
			// tbNewName
			// 
			this.tbNewName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbNewName.Location = new System.Drawing.Point(90, 38);
			this.tbNewName.Name = "tbNewName";
			this.tbNewName.Size = new System.Drawing.Size(182, 20);
			this.tbNewName.TabIndex = 3;
			this.tbNewName.TextChanged += new System.EventHandler(this.tbNewName_TextChanged);
			// 
			// bRename
			// 
			this.bRename.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.bRename.Location = new System.Drawing.Point(116, 64);
			this.bRename.Name = "bRename";
			this.bRename.Size = new System.Drawing.Size(75, 23);
			this.bRename.TabIndex = 4;
			this.bRename.Text = "Rename";
			this.bRename.UseVisualStyleBackColor = true;
			this.bRename.Click += new System.EventHandler(this.bRename_Click);
			// 
			// bCancel
			// 
			this.bCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.bCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.bCancel.Location = new System.Drawing.Point(197, 64);
			this.bCancel.Name = "bCancel";
			this.bCancel.Size = new System.Drawing.Size(75, 23);
			this.bCancel.TabIndex = 5;
			this.bCancel.Text = "Cancel";
			this.bCancel.UseVisualStyleBackColor = true;
			this.bCancel.Click += new System.EventHandler(this.bCancel_Click);
			// 
			// RenameSomethingForm
			// 
			this.AcceptButton = this.bRename;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.bCancel;
			this.ClientSize = new System.Drawing.Size(284, 98);
			this.Controls.Add(this.bCancel);
			this.Controls.Add(this.bRename);
			this.Controls.Add(this.tbNewName);
			this.Controls.Add(this.lblNewName);
			this.Controls.Add(this.tbOriginalName);
			this.Controls.Add(this.lblOriginalName);
			this.Name = "RenameSomethingForm";
			this.Text = "Rename...";
			this.Load += new System.EventHandler(this.RenameSomethingForm_Load);
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label lblOriginalName;
		private System.Windows.Forms.TextBox tbOriginalName;
		private System.Windows.Forms.Label lblNewName;
		private System.Windows.Forms.TextBox tbNewName;
		private System.Windows.Forms.Button bRename;
		private System.Windows.Forms.Button bCancel;
	}
}