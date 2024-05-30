using System.Windows.Forms;

namespace GetLatest
{
    public partial class MemoryMessageBox : Form
    {
        public MemoryMessageBox( string message, string checkboxLabel )
        {
            InitializeComponent( );

            messageLabel.Text = message;
            memoryCheckBox.Text = checkboxLabel;
        }

        public bool Checked
        {
            get { return memoryCheckBox.Checked; }
        }
    }
}
