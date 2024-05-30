using System;
using System.Windows.Forms;

namespace DownResRes
{
	partial class ConfigureParametersForm : Form, IProgressListener
	{
		public Action<Parameters> Build;

		public ConfigureParametersForm( ): this( Parameters.Default )
		{
		}

		public ConfigureParametersForm( Parameters parameters )
		{
			InitializeComponent( );
			tbBackupsFolder.Text		= parameters.BackupPath;
			cbCreateBackups.Checked		= parameters.EnableBackup;
			numMaximumDivisor.Value		= parameters.MaximumDivisor;
			numMinimumResolution.Value	= parameters.MinimumResolution;
			cbOverwriteBackups.Checked	= parameters.OverwriteBackup;
			tbSourceFolder.Text			= parameters.ResPath;
			tbSourcePattern.Text		= string.Join( ";", parameters.Patterns );
		}

		private void cbCreateBackups_CheckedChanged( object sender, EventArgs e )
		{
			foreach( var ctrl in new Control[] { cbCheckinBackups, cbOverwriteBackups, lblBackupsFolder, tbBackupsFolder } )
				ctrl.Enabled = cbCreateBackups.Checked;
		}

		public Parameters Parameters { get {
			return new Parameters()
			{
				BackupFilenamePostfix	= "",
				BackupFilenamePrefix	= "",
				BackupPath				= tbBackupsFolder.Text,
				EnableBackup			= cbCreateBackups.Checked,
				MaximumDivisor			= (int)numMaximumDivisor.Value,
				MinimumResolution		= (int)numMinimumResolution.Value,
				OverwriteBackup			= cbOverwriteBackups.Checked,
				ResPath					= tbSourceFolder.Text,
				Patterns				= tbSourcePattern.Text.Split(';'),
			};
		}}

		private void buttonProcess_Click( object sender, EventArgs e )
		{
			if( Build == null )
			{
				DialogResult = DialogResult.OK;
				Close( );
			}
			else
			{
				Build( Parameters );
			}
		}

		private void buttonCancel_Click( object sender, EventArgs e )
		{
			DialogResult = DialogResult.Cancel;
			Close( );
		}

		private void Write( string format, params object[] parameters )
		{
			rtConsole.AppendText( string.Format( format, parameters ) );
			rtConsole.ScrollToCaret( );
		}

		private void AsyncInvoke( Action a )
		{
			BeginInvoke( a );
		}

		public void On( StartProcessingEvent e )
		{
			AsyncInvoke( () =>
			{
				Write( "\nSimplifying texture {0} of {1}: {2} ...", e.FileNumber, e.FilesCount, e.File );
				progressBarConversion.Minimum	= 0;
				progressBarConversion.Value		= e.FileNumber;
				progressBarConversion.Maximum	= e.FilesCount;
			});
		}

		public void On( ErrorProcessingEvent e )
		{
			AsyncInvoke( () => Write( " Error: {0}", e.Description ) );
		}

		public void On( InfoProcessingEvent e )
		{
			AsyncInvoke( () => Write( " {0} ...", e.Message ) );
		}

		public void On( DoneProcessingEvent e )
		{
			AsyncInvoke( () => Write( " Done." ) );
		}
	}
}
