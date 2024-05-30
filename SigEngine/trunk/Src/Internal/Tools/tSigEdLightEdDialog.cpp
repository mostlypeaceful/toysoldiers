#include "ToolsPch.hpp"
#include "tSigEdLightEdDialog.hpp"
#include "gfx/tRenderToTextureAgent.hpp"
#include "tApplication.hpp"
#include "Editor/tEditableLightProbeEntity.hpp"
#include "Gfx/tRenderContext.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	namespace
	{
		tEditablePropertyPtr cNullProperty( new tEditablePropertyString( "cDummy" ) );
	}

	tSigEdLightEdDialog* tSigEdLightEdDialog::gDialog = NULL;

	tSigEdLightEdDialog::tSigEdLightEdDialog( wxWindow* parent, Gfx::tDevicePtr device, tEditorActionStack& actionStack, const std::string& regKeyName )
		: tMatEdMainWindow( parent, actionStack, regKeyName, true, true, true )
		, mDefaultTextureBrowserPathSet( false )
	{
		gDialog = this;

		SetIcon( wxIcon( "appicon" ) );

		fSetupPreviewWindow( device );

		mPreviewBundle.fReset( new tMaterialPreviewBundle( fDevice( ), HlslGen::cVshMeshModel ) );
		fSetPreviewBundle( mPreviewBundle );
		fPreviewPanel( )->fSetCurrentPreviewGeometry( tMaterialPreviewBundle::cPreviewGeometryModeSphereAndShadowedPlane );

		fSetDefaultShader( );

		// disable real lighting.
		fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeFront, Math::tVec4f::cZeroVector );
		fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeBack, Math::tVec4f::cZeroVector );
		fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeAmbient, Math::tVec4f::cZeroVector );
		fDefaultLight( )->fSetColor( Gfx::tLight::cColorTypeRim, Math::tVec4f::cZeroVector );
	}

	tSigEdLightEdDialog::~tSigEdLightEdDialog( )
	{
		gDialog = NULL;
	}

	b32 tSigEdLightEdDialog::fOnTick( )
	{
		//fAutoHandleTopMost( ( HWND )mSigEd->GetHWND( ) );
		const b32 o = tMatEdMainWindow::fOnTick( );
		if( fIsActive( ) )
		{
			//mSigEd->fSetDialogInputActive( );
			if( !mDefaultTextureBrowserPathSet )
				mDefaultTextureBrowserPathSet = fSetDefaultBrowseDirectory( tFilePathPtr( "tools/lighting/" ) );
		}

		return o;
	}

	void tSigEdLightEdDialog::fSetDefaultShader( )
	{
		// todo get this from the tools res folder.
		const tFilePathPtr cFileName( "shaders/standard/basic.derml" );
		fOnShaderSelected( cFileName );
	}

	void tSigEdLightEdDialog::fOnShaderSelected( const tFilePathPtr& shaderPath )
	{
		Derml::tFile dermlFile;
		dermlFile.fLoadXml( ToolsPaths::fMakeResAbsolute( shaderPath ) );

		Derml::tMtlFile mtlFile;
		mtlFile.fFromShaderFile( dermlFile, shaderPath );

		mPreviewBundle->fGenerateShaders( dermlFile, HlslGen::cToolTypeDefault );
		mPreviewBundle->fUpdateMaterial( mtlFile, *fTextureCache( ) );

		fFromDermlMtlFile( mtlFile );
	}

	void tSigEdLightEdDialog::fAddProperty( tEditableProperty& prop, const tEditorSelectionList* selection )
	{
		if( !gDialog )
			return;

		tSigEdLightEdDialog& diag = *gDialog;

		// Find owner of property.
		diag.mEditingEnt.fRelease( );

		sigassert( selection );
		for( u32 i = 0; i < selection->fCount( ); ++i )
		{
			tEditableObject* obj = (*selection)[ i ]->fStaticCast<tEditableObject>( );

			if( obj->fGetEditableProperties( ).fContainsProperty( prop ) )
			{
				diag.mEditingEnt.fReset( obj );
				break;
			}
		}

		sigassert( diag.mEditingEnt );	


		// Populate Dialog.
		prop.fGetData( diag.mEditingData );
		diag.mEditingProperty.fReset( &prop );

		diag.fSetUserProperties( diag.mEditingData.mUserOptions );

		if( diag.mEditingData.fCubeMapFileName( ).length( ) )
			diag.fRefreshCubemapFile( );
		else if( !diag.mEditingData.mComputed )
		{
			// No data yet, so render some initial values.
			diag.fRender( );
			diag.mEditingData.mComputed = true;
			diag.fOnUserPropertyChanged( *cNullProperty );
		}

		diag.Show( );
	}

	void tSigEdLightEdDialog::fRefreshProperty( tEditableProperty& prop )
	{
		//TODO ?
	}

	void tSigEdLightEdDialog::fRemoveProperty( tEditableProperty& prop )
	{
		if( !gDialog )
			return;

		tSigEdLightEdDialog& diag = *gDialog;
		diag.fSetUserProperties( tEditablePropertyTable( ) );

		diag.mEditingProperty.fRelease( );
		diag.Hide( );
	}

	// Will render a cube map in the scene graph at the given center point.
	static Dx9Util::tBaseTexturePtr fRenderProbe( tSceneGraph& sg, Gfx::tScreen& screen, const Math::tVec3f& center )
	{
		const u32 cWidth = 256;
		const u32 cHeight = 256;

		Gfx::tLens lens;
		lens.fSetPerspective( 0.1f, 10000.f, 1.0f, Math::cPiOver2 );

		Gfx::tCamera cam;
		cam.fSetLens( lens );

		Gfx::tTripod tripod;
		tripod.mEye = center;

		Gfx::tRenderToTextureAgentPtr rtt( NEW Gfx::tRenderToTextureAgent( ) );
		rtt->mRoot.fReset( &sg.fRootEntity( ) );

		Gfx::tLight desc;
		desc.fSetTypeDirection( );

		const Math::tVec3f shadowMapLightDir = Math::tVec3f( -0.5f, -0.5f, 0 ).fNormalize( );
		const f32 shadowMapDistFromOrigin = -1.f; // means use default
		const Math::tVec3f shadowMapTarget = Math::tVec3f::cZeroVector; // targets the world origin by default
		const Math::tMat3f m = Gfx::tLightEntity::fCreateDirectionalMatrix( shadowMapLightDir, shadowMapDistFromOrigin, shadowMapTarget );

		Gfx::tLightEntity* lightEntity = new Gfx::tLightEntity( m, desc, "DefaultLight" );

		rtt->mLight.fReset( lightEntity );

		rtt->mClearColor = Math::tVec4f( 0,0,0.2f,1.0f );

		IDirect3DTexture9* masterTexture = screen.fGetDevice( )->fCreateSolidColorTexture( cWidth * 4, cHeight * 3, Math::tVec4f(0,0,0,1) );

		tFixedArray<Gfx::tRenderToTexturePtr, 6 > renders;
		tFixedArray< IDirect3DTexture9*, 6 > textures;

		for( u32 i = 0; i < 6; ++i )
		{
			renders[ i ].fReset( NEW Gfx::tRenderToTexture( screen.fGetDevice( ), cWidth, cHeight, Gfx::tRenderTarget::cFormatRGBA8, Gfx::tRenderTarget::cFormatD24FS8, 0 ) );
			
			rtt->mRtt = renders[ i ];
			//textures[ i ] = screen.fGetDevice( )->fCreateSolidColorTexture( cWidth, cHeight, Math::tVec4f(1) );
			//rtt->mOutTexture = (Gfx::tTextureFile::tPlatformHandle)textures[ i ];

			//http://msdn.microsoft.com/en-us/library/windows/desktop/bb204881(v=vs.85).aspx
			// except our x axis is reversed.
			switch( i )
			{
			case 0:
				{
					tripod.mLookAt = center - Math::tVec3f::cXAxis;
					tripod.mUp = Math::tVec3f::cYAxis;
				}
				break;
			case 1:
				{
					tripod.mLookAt = center + Math::tVec3f::cXAxis;
					tripod.mUp = Math::tVec3f::cYAxis;
				}
				break;
			case 2:
				{
					tripod.mLookAt = center + Math::tVec3f::cYAxis;
					tripod.mUp = -Math::tVec3f::cZAxis;
				}
				break;
			case 3:
				{
					tripod.mLookAt = center - Math::tVec3f::cYAxis;
					tripod.mUp = Math::tVec3f::cZAxis;
				}
				break;
			case 4:
				{
					tripod.mLookAt = center + Math::tVec3f::cZAxis;
					tripod.mUp = Math::tVec3f::cYAxis;
				}
				break;
			case 5:
				{
					tripod.mLookAt = center - Math::tVec3f::cZAxis;
					tripod.mUp = Math::tVec3f::cYAxis;
				}
				break;
			}

			// Apply camera for this face
			cam.fSetTripod( tripod );
			rtt->mCamera = cam;

			// do an immediate rendering.
			screen.fRenderRttAgent( *rtt, true );	

			//// Copy render target to a dynamic texture.
			HRESULT hr = screen.fGetDevice( )->fGetDevice( )->CreateTexture( cWidth, cHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &textures[ i ], 0 );
			sigassert( !FAILED( hr ) && textures[ i ] );

			IDirect3DSurface9* destSurface = NULL;
			textures[ i ]->GetSurfaceLevel( 0, &destSurface );

			HRESULT result = screen.fGetDevice( )->fGetDevice( )->GetRenderTargetData( rtt->mRtt->fRenderTarget( )->fGetSurface( ), destSurface );
			sigassert( result == D3D_OK );

			//std::string path = ( "c:/testimg" ) + StringUtil::fToString( i ) + ".png";
			//D3DXSaveSurfaceToFile( path.c_str( ), D3DXIFF_PNG, destSurface, NULL, NULL );
		}

		Dx9Util::fAssembleCubeMapFromTextures( textures, masterTexture );

		//HRESULT hr = D3DXSaveTextureToFile( "c:\\combined.png", D3DXIFF_PNG, masterTexture, NULL );
		//sigassert( hr == D3D_OK );

		return Dx9Util::tBaseTexturePtr( masterTexture );
	}

	void tSigEdLightEdDialog::fRender( )
	{
		if( mEditingEnt )
		{
			mCurrentCubeMap = fRenderProbe( *tApplication::fInstance( ).fSceneGraph( ), *fPreviewPanel( )->fScreen( ), mEditingEnt->fObjectToWorld( ).fGetTranslation( ) );
			mPreviewBundle->fSetCubeMapTexture( mCurrentCubeMap );

			fRefreshHarmonics( );
		}
	}

	void tSigEdLightEdDialog::fLoad( )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			this, 
			"Open cubemap",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled.png" ),
			wxString( "*.png" ),
			wxFD_OPEN ) );

		if( openFileDialog->ShowModal( ) != wxID_OK )
			return; // cancelled

		std::string path = ToolsPaths::fMakeResRelative( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ), true ).fCStr( );

		if( path.length( ) == 0 )
		{
			const int result = wxMessageBox( "Path must be under the current project's res directory.",
				"Oops.", wxOK | wxICON_WARNING );
			return;
		}

		Dx9Util::tBaseTexturePtr newCube = fTextureCache( )->fFindLoad2D( tFilePathPtr( path.c_str( ) ) );

		if( newCube )
		{
			mCurrentCubeMap = newCube;
			mPreviewBundle->fSetCubeMapTexture( mCurrentCubeMap );
			mEditingData.fSetCubeMapFileName( path );
			fRefreshHarmonics( );
			fOnUserPropertyChanged( *cNullProperty );
		}

	}

	void tSigEdLightEdDialog::fSave( )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			this, 
			"Save cubemap As",
			wxString( ToolsPaths::fGetCurrentProjectResFolder( ).fCStr( ) ),
			wxString( "untitled.png" ),
			wxString( "*.png" ),
			wxFD_SAVE | wxFD_OVERWRITE_PROMPT ) );

		if( openFileDialog->ShowModal( ) != wxID_OK )
			return; // cancelled

		std::string absPath = openFileDialog->GetPath( );
		std::string relpath = ToolsPaths::fMakeResRelative( tFilePathPtr( openFileDialog->GetPath( ).c_str( ) ), true ).fCStr( );

		if( relpath.length( ) == 0 )
		{
			const int result = wxMessageBox( "Path must be under the current project's res directory.",
				"Oops.", wxOK | wxICON_WARNING );
			return;
		}
		
		std::string folder = StringUtil::fDirectoryFromPath( absPath.c_str( ) );
		if( !FileSystem::fFolderExists( tFilePathPtr( folder.c_str( ) ) ) )
			FileSystem::fCreateDirectory( tFilePathPtr( folder.c_str( ) ) );

		HRESULT hr = D3DXSaveTextureToFile( absPath.c_str( ), D3DXIFF_PNG, (IDirect3DTexture9*)mCurrentCubeMap.fGetRawPtr( ), NULL );
		sigassert( hr == D3D_OK );

		mEditingData.fSetCubeMapFileName( relpath );
		fOnUserPropertyChanged( *cNullProperty );
	}

	void tSigEdLightEdDialog::fRefreshCubemapFile( )
	{
		tFilePathPtr filename = ToolsPaths::fMakeResAbsolute( tFilePathPtr( mEditingData.fCubeMapFileName( ).c_str( ) ) );		

		fTextureCache( )->fRefresh( );
		Dx9Util::tBaseTexturePtr newCube =  fTextureCache( )->fFindLoad2D( filename );

		if( newCube )
		{
			mCurrentCubeMap = newCube;
			mPreviewBundle->fSetCubeMapTexture( mCurrentCubeMap );
			fRefreshHarmonics( );
		}
	}

	void tSigEdLightEdDialog::fRefreshHarmonics( )
	{
		if( mCurrentCubeMap )
		{
			Gfx::tShBasisWeights weights;

			for( u32 i = 0; i < Gfx::tShBasisWeights::cEditableWeightCount; ++i )
				weights.mWeights[ i ] = mEditingData.mUserOptions.fGetValue<f32>( tEditableLightProbeData::fEditablePropHarmonicsEq( i ), 1.f );

			mEditingData.mHarmonics.mFactors.fFill( Math::tVec4f::cZeroVector );
			Dx9Util::tTextureCache::fComputeSphericalHarmonics( mCurrentCubeMap, mEditingData.mHarmonics, weights );

			// testing
			Gfx::tRenderContext::gSphericalHarmonics = mEditingData.mHarmonics;
		}
	}

	void tSigEdLightEdDialog::fOnUserPropertyChanged( tEditableProperty& property )
	{
		tMatEdMainWindow::fOnUserPropertyChanged( property );
		
		if( property.fGetName( ) == tEditableLightProbeData::fEditablePropCubeMapButtons( ) )
		{
			tEditablePropertyButtons& prop = static_cast<tEditablePropertyButtons&>( property );
			std::string val;
			prop.fGetData( val );
			if( val == tEditableLightProbeData::cCommandRender )
				fRender( );
			else if( val == tEditableLightProbeData::cCommandSave )
				fSave( );
			else if( val == tEditableLightProbeData::cCommandLoad )
				fLoad( );
			else if( val == tEditableLightProbeData::cCommandRefresh )
				fRefreshCubemapFile( );
		}
		else if( property.fGetName( ) == tEditableLightProbeData::fEditablePropCubeMapFile( ) )
			fRefreshCubemapFile( );

		fRefreshHarmonics( );

		sigassert( mEditingProperty );
		mEditingProperty->fSetData( mEditingData );
		
	}

}

