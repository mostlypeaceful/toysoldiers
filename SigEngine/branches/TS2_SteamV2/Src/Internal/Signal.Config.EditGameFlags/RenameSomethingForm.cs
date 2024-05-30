//------------------------------------------------------------------------------
// \file RenameSomethingForm.cs - 1 Mar 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System;
using System.Windows.Forms;

namespace Signal.Config.EditGameFlags
{
	public partial class RenameSomethingForm : Form
	{
		public string Result { get; set; }

		public RenameSomethingForm( ): this( "Example Thing being Renamed" ) { }
		public RenameSomethingForm( string name )
		{
			InitializeComponent( );
			tbOriginalName.Text = name;
			tbNewName.Text = name;
		}

		private void RenameSomethingForm_Load( object sender, EventArgs e )
		{
			tbNewName.Focus( );
		}

		private void bRename_Click( object sender, EventArgs e )
		{
			DialogResult = DialogResult.OK;
			Close( );
		}

		private void bCancel_Click( object sender, EventArgs e )
		{
			DialogResult = DialogResult.Cancel;
			Close( );
		}

		private void tbNewName_TextChanged( object sender, EventArgs e )
		{
			Result = tbNewName.Text;
		}
	}
}
