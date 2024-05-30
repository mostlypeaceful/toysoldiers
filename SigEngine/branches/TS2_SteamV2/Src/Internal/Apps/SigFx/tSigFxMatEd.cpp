#include "SigFxPch.hpp"
#include "tSigFxMatEd.hpp"
#include "tSigFxMainWindow.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"

namespace Sig
{
	tSigFxMatEd::tSigFxMatEd( tSigFxMainWindow* sigFx, tEditorActionStack& actionStack, const std::string& regKeyName )
		: tMatEdMainWindow( sigFx, actionStack, regKeyName, true )
		, mSigFx( sigFx )
		, mDefaultTextureBrowserPathSet( false )
	{
		SetIcon( wxIcon( "appicon" ) );
	}

	b32 tSigFxMatEd::fOnTick( )
	{
		fAutoHandleTopMost( ( HWND )mSigFx->GetHWND( ) );
		const b32 o = tMatEdMainWindow::fOnTick( );
		if( fIsActive( ) )
		{
			mSigFx->fSetDialogInputActive( );
			if( !mDefaultTextureBrowserPathSet )
				mDefaultTextureBrowserPathSet = fSetDefaultBrowseDirectory( tFilePathPtr( "effects/textures/particles" ) );
		}

		tEditorSelectionList& selList = mSigFx->fGuiApp( ).fSelectionList( );

		if( selList.fCount( ) > 0 )
			fSetSelected( selList[ 0 ] );
		else
			fSetSelected( tEntityPtr( ) );

		return o;
	}

	void tSigFxMatEd::fSetDefaultShader( const tEntityPtr& sel )
	{
		fSetSelected( sel );
		fOnShaderSelected( tFilePathPtr( "shaders\\particles\\standard.derml" ) );
	}

	void tSigFxMatEd::fOnShaderSelected( const tFilePathPtr& shaderPath )
	{
		if( mSelected )
		{
			const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( shaderPath );
			Derml::tFile dermlFile;
			if( !dermlFile.fLoadXml( absolutePath ) )
			{
				wxMessageBox( "The shader could not load - the old shader will be kept.", "Invalid Shader", wxOK | wxCENTRE | wxICON_EXCLAMATION );
				return;
			}

			if( dermlFile.mGeometryStyle != HlslGen::cVshFacingQuads )
			{
				wxMessageBox( "The selected shader has a geometry style that is not compatible with particles (select Geometry Type: 'FacingParticleQuad' in SigShade).", "Invalid Shader", wxOK | wxCENTRE | wxICON_EXCLAMATION );
				return;
			}

			tSigFxParticleSystem* psys = mSelected ? mSelected->fDynamicCast<tSigFxParticleSystem>( ) : 0;
			if( psys->fMaterialFile( ).mShaderPath.fLength( ) > 0 )
			{
				wxMessageDialog msgBox( this, "Are you sure you want to select a different shader? Selecting 'yes' will wipe out your current material properties.", "Change Shader?", wxCENTRE | wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION );
				const int retVal = msgBox.ShowModal( );
				if( retVal != wxID_YES )
					return;
			}

			if( psys )
			{
				psys->fChangeShader( dermlFile, shaderPath );
				fFromDermlMtlFile( psys->fMaterialFile( ) );
			}
		}
	}

	void tSigFxMatEd::fSetSelected( const tEntityPtr& sel )
	{
		if( mSelected == sel )
			return;
		mSelected = sel;

		tSigFxParticleSystem* psys = mSelected ? mSelected->fDynamicCast<tSigFxParticleSystem>( ) : 0;
		if( psys )
		{
			fEnableShaderBrowser( true );
			psys->fUpdateAgainstShaderFile( );
			fSetPreviewBundle( psys->fPreviewBundle( ) );
			fFromDermlMtlFile( psys->fMaterialFile( ) );
		}
		else
		{
			// nothing is selected
			fClear( );
			fEnableShaderBrowser( false );
		}
	}

}

