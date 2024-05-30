using System;
using System.Windows.Forms;

namespace CheckIn
{
    public partial class InputBox : Form
    {
        public bool okClicked = false;
        public string data;

        public InputBox()
        {
            InitializeComponent();
        }

        private void cancelButton_Click(object sender, EventArgs e)
        {
            Close();
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            if (inputTextBox.Text.Replace(" ", "") == "")
            {
                MessageBox.Show("There is nothing to add.");
                return;
            }

            data = inputTextBox.Text;
            okClicked = true;

            Close();
        }
    }
}
