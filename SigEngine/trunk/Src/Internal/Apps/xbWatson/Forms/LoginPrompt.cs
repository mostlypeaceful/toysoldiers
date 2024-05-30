using System;
using System.Windows.Forms;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class LoginPrompt : Form
    {
        public LoginPrompt()
        {
            InitializeComponent();
        }

        private void loginButton_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        public string username
        {
            get
            {
                return usernameTextBox.Text;
            }

            set
            {
                usernameTextBox.Text = value;
            }
        }

        public string password
        {
            get
            {
                return passwordTextBox.Text;
            }

            set
            {
                passwordTextBox.Text = value;
            }
        }

        public bool rememberMyInfo
        {
            get
            {
                return rememberMyInfoCheckBox.Checked;
            }

            set
            {
                rememberMyInfoCheckBox.Checked = value;
            }
        }
    }
}
