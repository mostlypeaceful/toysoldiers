//------------------------------------------------------------------------------
// \file GameFlagsEditorForm.Designer.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

namespace Signal.Config.EditGameFlags
{
	partial class GameFlagsEditorForm
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
			System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(GameFlagsEditorForm));
			this.lblCategory = new System.Windows.Forms.Label();
			this.cbCategory = new System.Windows.Forms.ComboBox();
			this.cbSubCategory = new System.Windows.Forms.ComboBox();
			this.lbItems = new System.Windows.Forms.ListBox();
			this.bAddItems = new System.Windows.Forms.Button();
			this.bInsertItems = new System.Windows.Forms.Button();
			this.bRemoveItems = new System.Windows.Forms.Button();
			this.bMoveItemsUp = new System.Windows.Forms.Button();
			this.bMoveItemsDown = new System.Windows.Forms.Button();
			this.bRenameItem = new System.Windows.Forms.Button();
			this.tbNewItems = new System.Windows.Forms.TextBox();
			this.bCancel = new System.Windows.Forms.Button();
			this.bSave = new System.Windows.Forms.Button();
			this.bAddSubCategory = new System.Windows.Forms.Button();
			this.bRemoveSubCategory = new System.Windows.Forms.Button();
			this.bRenameSubCategory = new System.Windows.Forms.Button();
			this.SuspendLayout();
			// 
			// lblCategory
			// 
			this.lblCategory.AutoSize = true;
			this.lblCategory.Location = new System.Drawing.Point(12, 15);
			this.lblCategory.Name = "lblCategory";
			this.lblCategory.Size = new System.Drawing.Size(52, 13);
			this.lblCategory.TabIndex = 0;
			this.lblCategory.Text = "Category:";
			// 
			// cbCategory
			// 
			this.cbCategory.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
			this.cbCategory.FormattingEnabled = true;
			this.cbCategory.Items.AddRange(new object[] {
            "Flags",
            "Game Events",
            "Keyframe Events",
            "AI Flags",
            "Enums"});
			this.cbCategory.Location = new System.Drawing.Point(70, 12);
			this.cbCategory.Name = "cbCategory";
			this.cbCategory.Size = new System.Drawing.Size(121, 21);
			this.cbCategory.TabIndex = 1;
			this.cbCategory.SelectedIndexChanged += new System.EventHandler(this.cbCategory_SelectedIndexChanged);
			// 
			// cbSubCategory
			// 
			this.cbSubCategory.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.cbSubCategory.AutoCompleteMode = System.Windows.Forms.AutoCompleteMode.SuggestAppend;
			this.cbSubCategory.AutoCompleteSource = System.Windows.Forms.AutoCompleteSource.ListItems;
			this.cbSubCategory.FormattingEnabled = true;
			this.cbSubCategory.Location = new System.Drawing.Point(197, 12);
			this.cbSubCategory.Name = "cbSubCategory";
			this.cbSubCategory.Size = new System.Drawing.Size(234, 21);
			this.cbSubCategory.TabIndex = 2;
			this.cbSubCategory.SelectedIndexChanged += new System.EventHandler(this.cbSubCategory_SelectedIndexChanged);
			// 
			// lbItems
			// 
			this.lbItems.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left)));
			this.lbItems.FormattingEnabled = true;
			this.lbItems.IntegralHeight = false;
			this.lbItems.Location = new System.Drawing.Point(12, 40);
			this.lbItems.Name = "lbItems";
			this.lbItems.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
			this.lbItems.Size = new System.Drawing.Size(295, 168);
			this.lbItems.TabIndex = 3;
			// 
			// bAddItems
			// 
			this.bAddItems.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bAddItems.Location = new System.Drawing.Point(313, 39);
			this.bAddItems.Name = "bAddItems";
			this.bAddItems.Size = new System.Drawing.Size(90, 24);
			this.bAddItems.TabIndex = 4;
			this.bAddItems.Text = "<-- Add --<";
			this.bAddItems.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bAddItems.UseVisualStyleBackColor = true;
			this.bAddItems.Click += new System.EventHandler(this.bAddItems_Click);
			// 
			// bInsertItems
			// 
			this.bInsertItems.Image = ((System.Drawing.Image)(resources.GetObject("bInsertItems.Image")));
			this.bInsertItems.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bInsertItems.Location = new System.Drawing.Point(313, 68);
			this.bInsertItems.Name = "bInsertItems";
			this.bInsertItems.Size = new System.Drawing.Size(90, 24);
			this.bInsertItems.TabIndex = 5;
			this.bInsertItems.Text = "<-Insert-<";
			this.bInsertItems.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bInsertItems.UseVisualStyleBackColor = true;
			this.bInsertItems.Click += new System.EventHandler(this.bInsertItems_Click);
			// 
			// bRemoveItems
			// 
			this.bRemoveItems.Image = ((System.Drawing.Image)(resources.GetObject("bRemoveItems.Image")));
			this.bRemoveItems.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bRemoveItems.Location = new System.Drawing.Point(313, 97);
			this.bRemoveItems.Name = "bRemoveItems";
			this.bRemoveItems.Size = new System.Drawing.Size(90, 24);
			this.bRemoveItems.TabIndex = 6;
			this.bRemoveItems.Text = "Remove";
			this.bRemoveItems.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bRemoveItems.UseVisualStyleBackColor = true;
			this.bRemoveItems.Click += new System.EventHandler(this.bRemoveItems_Click);
			// 
			// bMoveItemsUp
			// 
			this.bMoveItemsUp.Image = ((System.Drawing.Image)(resources.GetObject("bMoveItemsUp.Image")));
			this.bMoveItemsUp.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bMoveItemsUp.Location = new System.Drawing.Point(313, 126);
			this.bMoveItemsUp.Name = "bMoveItemsUp";
			this.bMoveItemsUp.Size = new System.Drawing.Size(90, 24);
			this.bMoveItemsUp.TabIndex = 7;
			this.bMoveItemsUp.Text = "Move Up";
			this.bMoveItemsUp.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bMoveItemsUp.UseVisualStyleBackColor = true;
			this.bMoveItemsUp.Click += new System.EventHandler(this.bMoveItemsUp_Click);
			// 
			// bMoveItemsDown
			// 
			this.bMoveItemsDown.Image = ((System.Drawing.Image)(resources.GetObject("bMoveItemsDown.Image")));
			this.bMoveItemsDown.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bMoveItemsDown.Location = new System.Drawing.Point(313, 155);
			this.bMoveItemsDown.Name = "bMoveItemsDown";
			this.bMoveItemsDown.Size = new System.Drawing.Size(90, 24);
			this.bMoveItemsDown.TabIndex = 8;
			this.bMoveItemsDown.Text = "Move Down";
			this.bMoveItemsDown.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bMoveItemsDown.UseVisualStyleBackColor = true;
			this.bMoveItemsDown.Click += new System.EventHandler(this.bMoveItemsDown_Click);
			// 
			// bRenameItem
			// 
			this.bRenameItem.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bRenameItem.Location = new System.Drawing.Point(313, 184);
			this.bRenameItem.Name = "bRenameItem";
			this.bRenameItem.Size = new System.Drawing.Size(90, 24);
			this.bRenameItem.TabIndex = 9;
			this.bRenameItem.Text = "Rename";
			this.bRenameItem.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bRenameItem.UseVisualStyleBackColor = true;
			this.bRenameItem.Click += new System.EventHandler(this.bRenameItem_Click);
			// 
			// tbNewItems
			// 
			this.tbNewItems.AcceptsReturn = true;
			this.tbNewItems.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
			this.tbNewItems.Location = new System.Drawing.Point(409, 39);
			this.tbNewItems.Multiline = true;
			this.tbNewItems.Name = "tbNewItems";
			this.tbNewItems.Size = new System.Drawing.Size(224, 168);
			this.tbNewItems.TabIndex = 10;
			// 
			// bCancel
			// 
			this.bCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.bCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.bCancel.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bCancel.Location = new System.Drawing.Point(12, 213);
			this.bCancel.Name = "bCancel";
			this.bCancel.Size = new System.Drawing.Size(75, 24);
			this.bCancel.TabIndex = 11;
			this.bCancel.Text = "Cancel";
			this.bCancel.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bCancel.UseVisualStyleBackColor = true;
			this.bCancel.Click += new System.EventHandler(this.bCancel_Click);
			// 
			// bSave
			// 
			this.bSave.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
			this.bSave.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bSave.Location = new System.Drawing.Point(93, 213);
			this.bSave.Name = "bSave";
			this.bSave.Size = new System.Drawing.Size(75, 24);
			this.bSave.TabIndex = 12;
			this.bSave.Text = "Save && Build";
			this.bSave.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bSave.UseVisualStyleBackColor = true;
			this.bSave.Click += new System.EventHandler(this.bSave_Click);
			// 
			// bAddSubCategory
			// 
			this.bAddSubCategory.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.bAddSubCategory.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bAddSubCategory.Location = new System.Drawing.Point(437, 9);
			this.bAddSubCategory.Name = "bAddSubCategory";
			this.bAddSubCategory.Size = new System.Drawing.Size(55, 24);
			this.bAddSubCategory.TabIndex = 13;
			this.bAddSubCategory.Text = "Add";
			this.bAddSubCategory.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bAddSubCategory.UseVisualStyleBackColor = true;
			this.bAddSubCategory.Click += new System.EventHandler(this.bAddSubCategory_Click);
			// 
			// bRemoveSubCategory
			// 
			this.bRemoveSubCategory.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.bRemoveSubCategory.Image = ((System.Drawing.Image)(resources.GetObject("bRemoveSubCategory.Image")));
			this.bRemoveSubCategory.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bRemoveSubCategory.Location = new System.Drawing.Point(498, 9);
			this.bRemoveSubCategory.Name = "bRemoveSubCategory";
			this.bRemoveSubCategory.Size = new System.Drawing.Size(72, 24);
			this.bRemoveSubCategory.TabIndex = 14;
			this.bRemoveSubCategory.Text = "Remove";
			this.bRemoveSubCategory.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bRemoveSubCategory.UseVisualStyleBackColor = true;
			this.bRemoveSubCategory.Click += new System.EventHandler(this.bRemoveSubCategory_Click);
			// 
			// bRenameSubCategory
			// 
			this.bRenameSubCategory.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
			this.bRenameSubCategory.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
			this.bRenameSubCategory.Location = new System.Drawing.Point(576, 9);
			this.bRenameSubCategory.Name = "bRenameSubCategory";
			this.bRenameSubCategory.Size = new System.Drawing.Size(57, 24);
			this.bRenameSubCategory.TabIndex = 15;
			this.bRenameSubCategory.Text = "Rename";
			this.bRenameSubCategory.TextImageRelation = System.Windows.Forms.TextImageRelation.ImageBeforeText;
			this.bRenameSubCategory.UseVisualStyleBackColor = true;
			this.bRenameSubCategory.Click += new System.EventHandler(this.bRenameSubCategory_Click);
			// 
			// GameFlagsEditorForm
			// 
			this.AcceptButton = this.bSave;
			this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
			this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
			this.CancelButton = this.bCancel;
			this.ClientSize = new System.Drawing.Size(645, 249);
			this.Controls.Add(this.bRenameSubCategory);
			this.Controls.Add(this.bRemoveSubCategory);
			this.Controls.Add(this.bAddSubCategory);
			this.Controls.Add(this.bSave);
			this.Controls.Add(this.bCancel);
			this.Controls.Add(this.tbNewItems);
			this.Controls.Add(this.bRenameItem);
			this.Controls.Add(this.bMoveItemsDown);
			this.Controls.Add(this.bMoveItemsUp);
			this.Controls.Add(this.bRemoveItems);
			this.Controls.Add(this.bInsertItems);
			this.Controls.Add(this.bAddItems);
			this.Controls.Add(this.lbItems);
			this.Controls.Add(this.cbSubCategory);
			this.Controls.Add(this.cbCategory);
			this.Controls.Add(this.lblCategory);
			this.Name = "GameFlagsEditorForm";
			this.Text = "Project XML";
			this.ResumeLayout(false);
			this.PerformLayout();

		}

		#endregion

		private System.Windows.Forms.Label lblCategory;
		private System.Windows.Forms.ComboBox cbCategory;
		private System.Windows.Forms.ComboBox cbSubCategory;
		private System.Windows.Forms.ListBox lbItems;
		private System.Windows.Forms.Button bAddItems;
		private System.Windows.Forms.Button bInsertItems;
		private System.Windows.Forms.Button bRemoveItems;
		private System.Windows.Forms.Button bMoveItemsUp;
		private System.Windows.Forms.Button bMoveItemsDown;
		private System.Windows.Forms.Button bRenameItem;
		private System.Windows.Forms.TextBox tbNewItems;
		private System.Windows.Forms.Button bCancel;
		private System.Windows.Forms.Button bSave;
		private System.Windows.Forms.Button bAddSubCategory;
		private System.Windows.Forms.Button bRemoveSubCategory;
		private System.Windows.Forms.Button bRenameSubCategory;

	}
}

