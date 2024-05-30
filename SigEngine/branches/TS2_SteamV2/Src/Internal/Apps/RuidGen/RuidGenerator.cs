using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace RUID
{
    public partial class RuidGenerator : Form
    {
        private void justDoIt()
        {
            int ruid = System.Guid.NewGuid().ToString().GetHashCode();
            ruidTextBox.Text = "0x" + string.Format("{0:X}", ruid);
            ruidTextBox.Select();
        }

        public RuidGenerator()
        {
            InitializeComponent();
            justDoIt();
        }

        private void generateRuidButton_Click(object sender, EventArgs e)
        {
            justDoIt();
        }

        private void RuidGenerator_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Escape)
                Close();
        }

        private void ruidTextBox_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Escape)
                Close();
        }
    }
}