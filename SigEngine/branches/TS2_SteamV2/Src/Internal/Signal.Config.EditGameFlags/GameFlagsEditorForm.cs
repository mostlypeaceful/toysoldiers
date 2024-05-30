//------------------------------------------------------------------------------
// \file GameFlagsEditorForm.cs - 1 May 2014
// \author mrickert
//
// Copyright Signal Studios 2014, All Rights Reserved
//------------------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Media;
using System.Text;
using System.Windows.Forms;
using System.Xml;

namespace Signal.Config.EditGameFlags
{
	public partial class GameFlagsEditorForm : Form
	{
		static readonly HashSet< string > HasSubcategories = new HashSet<string>( ) { "Enums" };

		readonly string ProjectXmlPath;
		ProjectXml ProjectXml;

		List<NameKeyPair> SelectedItems { get
		{
			switch( cbCategory.Text )
			{
			case "Flags":			return ProjectXml.GameFlags;
			case "Game Events":		return ProjectXml.GameEvents;
			case "Keyframe Events":	return ProjectXml.KeyframeEvents;
			case "AI Flags":		return ProjectXml.AIFlags;
			case "Enums":
				{
					var selectedGameEnum = ProjectXml.GameEnums.FirstOrDefault( ge => ge.Name == cbSubCategory.Text );
					if( selectedGameEnum != null )
						return selectedGameEnum.Values;
					else
						return new List<NameKeyPair>();
				}
			default:
				Debug.Fail( "Invalid category: " + cbCategory.Text );
				return new List<NameKeyPair>();
			}
		}}

		IEnumerable<NameKeyPair> ConsumeNewItems( )
		{
			var nextKey = SelectedItems.Count > 0 ? SelectedItems.Max( nk => nk.Key ) + 1 : 0;

			var items = tbNewItems
				.Text
				.Split( "\n\r".ToCharArray( ) )
				.Select( line => line.Trim( ) )
				.Where( line => !string.IsNullOrEmpty( line ) )
				.Select( line => new NameKeyPair( line, nextKey++ ) )
				.ToArray( )
				;

			tbNewItems.Text = "";

			return items;
		}

		public GameFlagsEditorForm( ): this( Environment.ExpandEnvironmentVariables( "%SigCurrentProject%\\res\\project.xml" ) ) { }
		public GameFlagsEditorForm( string projectXmlPath )
		{
			InitializeComponent( );
			cbCategory.SelectedIndex = 0;
			ProjectXmlPath = projectXmlPath;
			RefreshDocument( );
		}

		IEnumerable<Control> SubcategoryControls { get {
			return new Control[] { cbSubCategory, bAddSubCategory, bRemoveSubCategory, bRenameSubCategory };
		}}

		IEnumerable<Control> ItemControls { get {
			return new Control[] { lbItems, bAddItems, bInsertItems, bRemoveItems, bMoveItemsUp, bMoveItemsDown, bRenameItem, tbNewItems };
		}}

		void RefreshDocument( )
		{
			ProjectXml = ProjectXml.LoadFrom( ProjectXmlPath );
			RefreshSubcategories( );
		}

		void RefreshSubcategories( )
		{
			var enableSubcategories = HasSubcategories.Contains( cbCategory.Text );
			foreach( var subcategoryControl in SubcategoryControls )
				subcategoryControl.Enabled = enableSubcategories;

			switch( cbCategory.Text )
			{
			case "Enums":
				cbSubCategory.Items.Clear( );
				cbSubCategory.Items.AddRange( ProjectXml.GameEnums.Cast<object>().ToArray( ) );
				break;
			default:
				cbSubCategory.Items.Clear( );
				break;
			}

			RefreshItems( );
		}

		void RefreshItems( )
		{
			lbItems.Items.Clear( );

			if( ProjectXml == null )
			{
				foreach( var ic in ItemControls )
					ic.Enabled = false;
				return;
			}

			foreach( var ic in ItemControls )
				ic.Enabled = true;

			var selectedItems = SelectedItems;
			if( selectedItems != null )
				lbItems.Items.AddRange( selectedItems.Cast<object>( ).ToArray( ) );
		}

		void AddSubCategory( string name )
		{
			switch( cbCategory.Text )
			{
			case "Enums":	ProjectXml.CreateGameEnum( cbSubCategory.Text ); break;
			default:		SystemSounds.Beep.Play( ); break;
			}
			RefreshSubcategories( );
		}

		void RemoveSubCategory( string name )
		{
			switch( cbCategory.Text )
			{
			case "Enums":	ProjectXml.GameEnums.RemoveAll( ge => ge.Name == name ); break;
			default:		SystemSounds.Beep.Play( ); break;
			}
			RefreshSubcategories( );
		}

		void RenameSubCategory( string from, string to )
		{
			switch( cbCategory.Text )
			{
			case "Enums":
				{
					var gameEnum = ProjectXml.GameEnums.FirstOrDefault( ge => ge.Name == from );
					if( gameEnum == null )
					{
						SystemSounds.Beep.Play( ); // ... nothing to rename
					}
					else if( to == null )
					{
						using( var rename = new RenameSomethingForm( from ) )
							if( rename.ShowDialog( ) == DialogResult.OK )
								RenameSubCategory( from, rename.Result );
					}
					else if( from != to )
					{
						gameEnum.Name = to;
						RefreshSubcategories( );
					}
				}
				break;
			default:
				SystemSounds.Beep.Play( ); 
				break;
			}
			RefreshSubcategories( );
		}

		private void cbCategory_SelectedIndexChanged( object sender, EventArgs e )
		{
			RefreshSubcategories( );
		}

		private void cbSubCategory_SelectedIndexChanged( object sender, EventArgs e )
		{
			RefreshItems( );
		}

		private void bAddSubCategory_Click( object sender, EventArgs e )
		{
			AddSubCategory( cbSubCategory.Text );
		}

		private void bRemoveSubCategory_Click( object sender, EventArgs e )
		{
			RemoveSubCategory( cbSubCategory.Text );
		}

		private void bRenameSubCategory_Click( object sender, EventArgs e )
		{
			RenameSubCategory( cbSubCategory.Text, null );
		}

		private void bAddItems_Click( object sender, EventArgs e )
		{
			SelectedItems.AddRange( ConsumeNewItems( ) );
			RefreshItems( );
		}

		private void bInsertItems_Click( object sender, EventArgs e )
		{
			var insertBefore = lbItems.SelectedIndex;
			if( insertBefore == -1 )
				insertBefore = SelectedItems.Count;

			SelectedItems.InsertRange( insertBefore, ConsumeNewItems( ) );
			RefreshItems( );
		}

		private void bRemoveItems_Click( object sender, EventArgs e )
		{
			SelectedItems.RemoveAll( item => lbItems.SelectedItems.Contains( item ) );
			RefreshItems( );
		}

		private void bMoveItemsUp_Click( object sender, EventArgs e )
		{
			var cantMoveI = 0; // Can't move the topmost item.

			var itemIndicies
				= lbItems
				.SelectedIndices
				.Cast<int>( )
				.ToArray( )
				;

			var movedItemIndicies
				= itemIndicies
				.SkipWhile( i => i == cantMoveI++ ) // If we selected that item, it can't move, and neither can the item bellow it.
				.ToArray( )
				;

			foreach( var i in movedItemIndicies )
			{
				var item = SelectedItems[ i ];
				SelectedItems.RemoveAt( i );
				SelectedItems.Insert( i-1, item );
			}

			RefreshItems( );

			foreach( var i in itemIndicies )
				lbItems.SetSelected( i > cantMoveI ? i-1 : i-0, true );
		}

		private void bMoveItemsDown_Click( object sender, EventArgs e )
		{
			var cantMoveI = lbItems.Items.Count - 1; // Can't move the bottommost item.

			var itemIndicies
				= lbItems
				.SelectedIndices
				.Cast<int>( )
				.Reverse( )
				.ToArray( )
				;

			var movedItemIndicies
				= itemIndicies
				.SkipWhile( i => i == cantMoveI-- ) // If we selected that item, it can't move, and neither can the item bellow it.
				.ToArray( )
				;

			foreach( var i in movedItemIndicies )
			{
				var item = SelectedItems[ i ];
				SelectedItems.RemoveAt( i );
				SelectedItems.Insert( i+1, item );
			}

			RefreshItems( );

			foreach( var i in itemIndicies )
				lbItems.SetSelected( i < cantMoveI ? i+1 : i+0, true );
		}

		private void bRenameItem_Click( object sender, EventArgs e )
		{
			var nkp = lbItems.SelectedItem as NameKeyPair;
			if( nkp == null )
			{
				SystemSounds.Beep.Play( );
				return;
			}

			using( var rename = new RenameSomethingForm( nkp.Name ) )
			{
				if( rename.ShowDialog( this ) == DialogResult.OK )
				{
					nkp.Name = rename.Result;
					RefreshItems( );
				}
			}
		}

		private void bCancel_Click( object sender, EventArgs e )
		{
			DialogResult = DialogResult.Cancel;
			Close( );
		}

		static readonly XmlWriterSettings XmlWriterSettings = new XmlWriterSettings( )
		{
			// Preserve existing SigEngine XML format to minimize diffs
			Indent = true,
			IndentChars = "    ",
			Encoding = Encoding.GetEncoding("ISO-8859-1"),
		};

		static void Checkout( string path )
		{
			var psi = new ProcessStartInfo( Environment.ExpandEnvironmentVariables( @"%SigEngine%\bin\OpenForEdit.cmd" ), string.Format( "\"{0}\"", path ) )
			{
				WindowStyle = ProcessWindowStyle.Hidden,
			};
			var p = Process.Start( psi );
			p.WaitForExit( );
			var ec = p.ExitCode;
		}

		private void bSave_Click( object sender, EventArgs e )
		{
			Checkout( ProjectXmlPath );
			ProjectXml.SaveTo( ProjectXmlPath );
			var psi = new ProcessStartInfo( Environment.ExpandEnvironmentVariables( @"%SigEngine%\bin\BuildGameFlags.exe" ), string.Format( "\"{0}\"", ProjectXmlPath ) )
			{
				WindowStyle = ProcessWindowStyle.Hidden,
			};
			var p = Process.Start( psi );
			p.WaitForExit( );
		}
	}
}
