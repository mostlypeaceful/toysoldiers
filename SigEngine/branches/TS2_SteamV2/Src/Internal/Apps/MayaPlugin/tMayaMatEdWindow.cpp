#include "MayaPluginPch.hpp"
#include "tMayaMatEdWindow.hpp"
#include "MayaUtil.hpp"
#include "tMayaDermlMaterial.hpp"
#include <maya/MD3D9Renderer.h>

namespace Sig
{
	namespace
	{
		static tMayaMatEdWindow* gInstance = 0;
	}
	tMayaMatEdWindow* tMayaMatEdWindow::fInstance( )
	{
		return gInstance;
	}

	tMayaMatEdWindow::tMayaMatEdWindow( wxWindow* parent )
		: tMatEdMainWindow( parent, mActionStack, ToolsPaths::fGetSignalRegistryKeyName( ) + "\\MayaMatEd", true )
		, mSelectedMaterial( 0 )
	{
		// store maya's hwnd
		fSetRawParent( M3dView::applicationShell( ) );

		// create the maya event callbacks
		mOnMayaIdle.fReset( new tMayaEvent( 
			tMayaEvent::fEventNameIdle( ), 
			make_delegate_memfn( tMayaEvent::tCallback, tMayaMatEdWindow, fOnMayaIdle ) ) );
		mOnMayaSelChanged.fReset( new tMayaEvent( 
			tMayaEvent::fEventNameSelChanged( ),
			make_delegate_memfn( tMayaEvent::tCallback, tMayaMatEdWindow, fOnMayaSelChanged ) ) );

		fUpdateDevice( );

		fSyncToMaterialNode( );

		gInstance = this;
	}

	tMayaMatEdWindow::~tMayaMatEdWindow( )
	{
		gInstance = 0;
		fSave( );
	}

	void tMayaMatEdWindow::fSyncToMaterialNode( tMayaDermlMaterial* dermlMtl )
	{
		if( dermlMtl == mSelectedMaterial )
			return; // nothing changed

		wxString title = wxString( "MatEd in Maya" );
		mSelectedMaterial = dermlMtl;

		if( dermlMtl )
		{
			title += " ~ ";
			title += dermlMtl->name( ).asChar( );
			fEnableShaderBrowser( true );
			mSelectedMaterial->fUpdateAgainstShaderFile( );
			fSetPreviewBundle( mSelectedMaterial->fPreviewBundle( ) );
			fFromDermlMtlFile( mSelectedMaterial->fMaterialFile( ) );
		}
		else
		{
			fClear( );
			fEnableShaderBrowser( false );
		}

		Freeze( );
		SetTitle( title );
		Thaw( );
	}

	void tMayaMatEdWindow::fOnShaderSelected( const tFilePathPtr& shaderPath )
	{
		if( mSelectedMaterial )
		{
			const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( shaderPath );
			Derml::tFile dermlFile;
			if( !dermlFile.fLoadXml( absolutePath ) )
			{
				wxMessageBox( "The shader could not load - the old shader will be kept.", "Invalid Shader", wxOK | wxCENTRE | wxICON_EXCLAMATION );
				return;
			}

			if( dermlFile.mGeometryStyle != HlslGen::cVshMeshModel )
			{
				wxMessageBox( "The selected shader has a geometry style that is not compatible with models (select Geometry Type: 'MeshModel' in SigShade).", "Invalid Shader", wxOK | wxCENTRE | wxICON_EXCLAMATION );
				return;
			}

			if( mSelectedMaterial->fMaterialFile( ).mShaderPath.fLength( ) > 0 )
			{
				wxMessageDialog msgBox( this, "Are you sure you want to change shaders? Selecting 'yes' will wipe out any material properties that are different.", "Change Shader?", wxCENTRE | wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION );
				const int retVal = msgBox.ShowModal( );
				if( retVal != wxID_YES )
					return;
			}

			mSelectedMaterial->fChangeShader( dermlFile, shaderPath );
			fFromDermlMtlFile( mSelectedMaterial->fMaterialFile( ) );
		}
	}

	void tMayaMatEdWindow::fUpdateDevice( )
	{
		Gfx::tDevicePtr mayaDevice = tMayaDermlMaterial::fGetMayaDevice( );
		if( mayaDevice )
		{
			if( !tMatEdMainWindow::fHasSameDevice( mayaDevice ) )
			{
				log_line( 0, "Creating material preview with maya d3d device..." );
				tMatEdMainWindow::fSetupPreviewWindow( mayaDevice );
			}
		}
		else
		{
			log_warning( 0, "Creating material preview WITHOUT maya d3d device!" );
			tMatEdMainWindow::fSetupPreviewWindow( );
		}
	}

	void tMayaMatEdWindow::fOnMayaIdle( )
	{
		if( fAutoHandleTopMost( ) )
		{
			fSave( );
			tMatEdMainWindow::fOnTick( );
		}
	}

	void tMayaMatEdWindow::fOnMayaSelChanged( )
	{
		MSelectionList list;
		MGlobal::getActiveSelectionList( list );
		const u32 numSel = list.length( );

		tMayaDermlMaterial* dermlMtl = 0;
		b32 foundOnObject = false;
		for( u32 index = 0; index < numSel; ++index )
		{
			MDagPath dagPath;
			MObject dependNode;
			list.getDagPath( index, dagPath );
			list.getDependNode( index, dependNode );

			// try and acquire maya derml material from depend node
			dermlMtl = tMayaDermlMaterial::fFromMayaObject( dependNode, &dagPath, &foundOnObject );
			if( dermlMtl )
				break;
		}

		fSyncToMaterialNode( dermlMtl );
		if( dermlMtl && !foundOnObject ) // if we actually selected a derml mtl, then show window
		{
			if( fIsMinimized( ) )
				fRestoreFromMinimized( );
			else if( !IsShown( ) )
				Show( true );
		}

	}
}
