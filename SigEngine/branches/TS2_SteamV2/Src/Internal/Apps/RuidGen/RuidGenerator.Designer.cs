namespace RUID
{
    partial class RuidGenerator
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
            this.ruidTextBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.generateRuidButton = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // ruidTextBox
            // 
            this.ruidTextBox.Location = new System.Drawing.Point(55, 16);
            this.ruidTextBox.Name = "ruidTextBox";
            this.ruidTextBox.Size = new System.Drawing.Size(154, 20);
            this.ruidTextBox.TabIndex = 0;
            this.ruidTextBox.KeyDown += new System.Windows.Forms.KeyEventHandler(this.ruidTextBox_KeyDown);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(14, 19);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "RUID";
            // 
            // generateRuidButton
            // 
            this.generateRuidButton.Location = new System.Drawing.Point(215, 14);
            this.generateRuidButton.Name = "generateRuidButton";
            this.generateRuidButton.Size = new System.Drawing.Size(75, 23);
            this.generateRuidButton.TabIndex = 2;
            this.generateRuidButton.Text = "Generate";
            this.generateRuidButton.UseVisualStyleBackColor = true;
            this.generateRuidButton.Click += new System.EventHandler(this.generateRuidButton_Click);
            // 
            // RuidGenerator
            // 
            this.AcceptButton = this.generateRuidButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(306, 55);
            this.Controls.Add(this.generateRuidButton);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.ruidTextBox);
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(322, 91);
            this.MinimumSize = new System.Drawing.Size(322, 91);
            this.Name = "RuidGenerator";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "RttiUniqueID Generator";
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.RuidGenerator_KeyDown);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox ruidTextBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button generateRuidButton;
    }
}

