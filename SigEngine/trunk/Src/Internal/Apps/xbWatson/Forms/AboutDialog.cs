using System.Diagnostics;
using System.Windows.Forms;
using System;

namespace Atg.Samples.xbWatson.Forms
{
    public partial class AboutDialog : Form
    {
        public AboutDialog()
        {
            InitializeComponent();
            
            // Update version information.
            string filename = typeof(AboutDialog).Assembly.Location;
            FileVersionInfo info = FileVersionInfo.GetVersionInfo(filename);
            versionLabel.Text = string.Format("Version {0}\n{1}\n\nSignal Studios Special Edition\nExtended by Keith Canzoneri",
                info.FileVersion, info.LegalCopyright);
        }
    }
}