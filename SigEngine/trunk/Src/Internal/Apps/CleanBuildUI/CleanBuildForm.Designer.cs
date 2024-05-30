namespace CleanBuildUI
{
    partial class CleanBuildForm
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(CleanBuildForm));
            this.fullCleanButton = new System.Windows.Forms.RadioButton();
            this.extensionCleanButton = new System.Windows.Forms.RadioButton();
            this.cancelButton = new System.Windows.Forms.Button();
            this.okButton = new System.Windows.Forms.Button();
            this.cleaningTypeGroupBox = new System.Windows.Forms.GroupBox();
            this.extensionFlowControlPanel = new System.Windows.Forms.FlowLayoutPanel();
            this.fileGroupsBox = new System.Windows.Forms.GroupBox();
            this.flashMoviesGroupCheckBox = new System.Windows.Forms.CheckBox();
            this.skeletonGroupCheckBox = new System.Windows.Forms.CheckBox();
            this.levelMeshesGroupCheckBox = new System.Windows.Forms.CheckBox();
            this.shadersGroupCheckBox = new System.Windows.Forms.CheckBox();
            this.texturesGroupCheckBox = new System.Windows.Forms.CheckBox();
            this.tablesGroupCheckBox = new System.Windows.Forms.CheckBox();
            this.cleaningTypeGroupBox.SuspendLayout();
            this.fileGroupsBox.SuspendLayout();
            this.SuspendLayout();
            // 
            // fullCleanButton
            // 
            this.fullCleanButton.AutoSize = true;
            this.fullCleanButton.Location = new System.Drawing.Point(6, 19);
            this.fullCleanButton.Name = "fullCleanButton";
            this.fullCleanButton.Size = new System.Drawing.Size(71, 17);
            this.fullCleanButton.TabIndex = 0;
            this.fullCleanButton.Text = "Full Clean";
            this.fullCleanButton.UseVisualStyleBackColor = true;
            this.fullCleanButton.CheckedChanged += new System.EventHandler(this.fullCleanButton_CheckedChanged);
            // 
            // extensionCleanButton
            // 
            this.extensionCleanButton.AutoSize = true;
            this.extensionCleanButton.Checked = true;
            this.extensionCleanButton.Location = new System.Drawing.Point(6, 42);
            this.extensionCleanButton.Name = "extensionCleanButton";
            this.extensionCleanButton.Size = new System.Drawing.Size(152, 17);
            this.extensionCleanButton.TabIndex = 1;
            this.extensionCleanButton.TabStop = true;
            this.extensionCleanButton.Text = "Specific Binary Extensions:";
            this.extensionCleanButton.UseVisualStyleBackColor = true;
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(448, 372);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 5;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            this.cancelButton.Click += new System.EventHandler(this.cancelButton_Click);
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(367, 372);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 6;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cleaningTypeGroupBox
            // 
            this.cleaningTypeGroupBox.Controls.Add(this.extensionFlowControlPanel);
            this.cleaningTypeGroupBox.Controls.Add(this.fileGroupsBox);
            this.cleaningTypeGroupBox.Controls.Add(this.fullCleanButton);
            this.cleaningTypeGroupBox.Controls.Add(this.extensionCleanButton);
            this.cleaningTypeGroupBox.Location = new System.Drawing.Point(12, 12);
            this.cleaningTypeGroupBox.Name = "cleaningTypeGroupBox";
            this.cleaningTypeGroupBox.Size = new System.Drawing.Size(511, 350);
            this.cleaningTypeGroupBox.TabIndex = 9;
            this.cleaningTypeGroupBox.TabStop = false;
            this.cleaningTypeGroupBox.Text = "Cleaning Type";
            // 
            // extensionFlowControlPanel
            // 
            this.extensionFlowControlPanel.AutoScroll = true;
            this.extensionFlowControlPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.extensionFlowControlPanel.Location = new System.Drawing.Point(6, 65);
            this.extensionFlowControlPanel.Name = "extensionFlowControlPanel";
            this.extensionFlowControlPanel.Size = new System.Drawing.Size(388, 279);
            this.extensionFlowControlPanel.TabIndex = 11;
            // 
            // fileGroupsBox
            // 
            this.fileGroupsBox.Controls.Add(this.flashMoviesGroupCheckBox);
            this.fileGroupsBox.Controls.Add(this.skeletonGroupCheckBox);
            this.fileGroupsBox.Controls.Add(this.levelMeshesGroupCheckBox);
            this.fileGroupsBox.Controls.Add(this.shadersGroupCheckBox);
            this.fileGroupsBox.Controls.Add(this.texturesGroupCheckBox);
            this.fileGroupsBox.Controls.Add(this.tablesGroupCheckBox);
            this.fileGroupsBox.Location = new System.Drawing.Point(400, 65);
            this.fileGroupsBox.Name = "fileGroupsBox";
            this.fileGroupsBox.Size = new System.Drawing.Size(105, 159);
            this.fileGroupsBox.TabIndex = 10;
            this.fileGroupsBox.TabStop = false;
            this.fileGroupsBox.Text = "File Groups";
            // 
            // flashMoviesGroupCheckBox
            // 
            this.flashMoviesGroupCheckBox.AutoSize = true;
            this.flashMoviesGroupCheckBox.Location = new System.Drawing.Point(7, 135);
            this.flashMoviesGroupCheckBox.Name = "flashMoviesGroupCheckBox";
            this.flashMoviesGroupCheckBox.Size = new System.Drawing.Size(88, 17);
            this.flashMoviesGroupCheckBox.TabIndex = 5;
            this.flashMoviesGroupCheckBox.Text = "Flash Movies";
            this.flashMoviesGroupCheckBox.UseVisualStyleBackColor = true;
            this.flashMoviesGroupCheckBox.CheckedChanged += new System.EventHandler(this.flashMoviesGroupCheckBox_CheckedChanged);
            // 
            // skeletonGroupCheckBox
            // 
            this.skeletonGroupCheckBox.AutoSize = true;
            this.skeletonGroupCheckBox.Location = new System.Drawing.Point(7, 112);
            this.skeletonGroupCheckBox.Name = "skeletonGroupCheckBox";
            this.skeletonGroupCheckBox.Size = new System.Drawing.Size(73, 17);
            this.skeletonGroupCheckBox.TabIndex = 4;
            this.skeletonGroupCheckBox.Text = "Skeletons";
            this.skeletonGroupCheckBox.UseVisualStyleBackColor = true;
            this.skeletonGroupCheckBox.CheckedChanged += new System.EventHandler(this.skeletonGroupCheckBox_CheckedChanged);
            // 
            // levelMeshesGroupCheckBox
            // 
            this.levelMeshesGroupCheckBox.AutoSize = true;
            this.levelMeshesGroupCheckBox.Location = new System.Drawing.Point(7, 89);
            this.levelMeshesGroupCheckBox.Name = "levelMeshesGroupCheckBox";
            this.levelMeshesGroupCheckBox.Size = new System.Drawing.Size(92, 17);
            this.levelMeshesGroupCheckBox.TabIndex = 3;
            this.levelMeshesGroupCheckBox.Text = "Level Meshes";
            this.levelMeshesGroupCheckBox.UseVisualStyleBackColor = true;
            this.levelMeshesGroupCheckBox.CheckedChanged += new System.EventHandler(this.levelMeshesGroupCheckBox_CheckedChanged);
            // 
            // shadersGroupCheckBox
            // 
            this.shadersGroupCheckBox.AutoSize = true;
            this.shadersGroupCheckBox.Location = new System.Drawing.Point(7, 66);
            this.shadersGroupCheckBox.Name = "shadersGroupCheckBox";
            this.shadersGroupCheckBox.Size = new System.Drawing.Size(65, 17);
            this.shadersGroupCheckBox.TabIndex = 2;
            this.shadersGroupCheckBox.Text = "Shaders";
            this.shadersGroupCheckBox.UseVisualStyleBackColor = true;
            this.shadersGroupCheckBox.CheckedChanged += new System.EventHandler(this.shadersGroupCheckBox_CheckedChanged);
            // 
            // texturesGroupCheckBox
            // 
            this.texturesGroupCheckBox.AutoSize = true;
            this.texturesGroupCheckBox.Location = new System.Drawing.Point(7, 43);
            this.texturesGroupCheckBox.Name = "texturesGroupCheckBox";
            this.texturesGroupCheckBox.Size = new System.Drawing.Size(67, 17);
            this.texturesGroupCheckBox.TabIndex = 1;
            this.texturesGroupCheckBox.Text = "Textures";
            this.texturesGroupCheckBox.UseVisualStyleBackColor = true;
            this.texturesGroupCheckBox.CheckedChanged += new System.EventHandler(this.texturesGroupCheckBox_CheckedChanged);
            // 
            // tablesGroupCheckBox
            // 
            this.tablesGroupCheckBox.AutoSize = true;
            this.tablesGroupCheckBox.Location = new System.Drawing.Point(7, 20);
            this.tablesGroupCheckBox.Name = "tablesGroupCheckBox";
            this.tablesGroupCheckBox.Size = new System.Drawing.Size(58, 17);
            this.tablesGroupCheckBox.TabIndex = 0;
            this.tablesGroupCheckBox.Text = "Tables";
            this.tablesGroupCheckBox.UseVisualStyleBackColor = true;
            this.tablesGroupCheckBox.CheckedChanged += new System.EventHandler(this.tablesGroupCheckBox_CheckedChanged);
            // 
            // CleanBuildForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(535, 407);
            this.Controls.Add(this.cleaningTypeGroupBox);
            this.Controls.Add(this.okButton);
            this.Controls.Add(this.cancelButton);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "CleanBuildForm";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Clean Build";
            this.cleaningTypeGroupBox.ResumeLayout(false);
            this.cleaningTypeGroupBox.PerformLayout();
            this.fileGroupsBox.ResumeLayout(false);
            this.fileGroupsBox.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.RadioButton fullCleanButton;
        private System.Windows.Forms.RadioButton extensionCleanButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.GroupBox cleaningTypeGroupBox;
        private System.Windows.Forms.GroupBox fileGroupsBox;
        private System.Windows.Forms.CheckBox flashMoviesGroupCheckBox;
        private System.Windows.Forms.CheckBox skeletonGroupCheckBox;
        private System.Windows.Forms.CheckBox levelMeshesGroupCheckBox;
        private System.Windows.Forms.CheckBox shadersGroupCheckBox;
        private System.Windows.Forms.CheckBox texturesGroupCheckBox;
        private System.Windows.Forms.CheckBox tablesGroupCheckBox;
        private System.Windows.Forms.FlowLayoutPanel extensionFlowControlPanel;
    }
}

