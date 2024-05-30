#include "ToolsGuiPch.hpp"
#include "tWxRenderPanelContainer.hpp"
#include "tToolsGuiApp.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tWxRenderPanelGridSettings.hpp"

// graphics
#include "Gui/tFont.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tFontMaterial.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tParticleMaterial.hpp"
#include "Gfx/tFullBrightMaterial.hpp"
#include "Gfx/tDecalMaterial.hpp"

namespace Sig
{
	static const std::string cRenderPanelNames[ ] =
	{
		"\\RenderPanelTL",
		"\\RenderPanelTR",
		"\\RenderPanelBL",
		"\\RenderPanelBR",
	};

	tWxRenderPanelContainer::tWxRenderPanelContainer( tToolsGuiMainWindow* parent, const std::string& regKeyName, b32 createToolBar )
		: wxPanel( parent )
		, mRegKeyName( regKeyName )
		, mMainWindow( parent )
		, mToolBar( 0 )
		, mSinglePaneView( false )
		, mFocusPanel( cMainSinglePanel )
	{
		fZeroOut( mRenderPanels );

		// create main panel sizer
		wxBoxSizer* vertSizer = new wxBoxSizer( wxVERTICAL );

		// create main panel
		SetSizer( vertSizer );

		// create tool bar (optional)
		if( createToolBar )
		{
			mToolBar = new wxToolBar( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHORIZONTAL );
			vertSizer->Add( mToolBar, 0, wxEXPAND | wxALL, 0 );
		}

		mMainGrid = new wxFlexGridSizer( 2, 2, 0, 0 );
		vertSizer->Add( mMainGrid, 1, wxEXPAND );

		for( u32 i = 0; i < cPanelCount; ++i )
		{
			mSelectionPanels[ i ] = new wxPanel( this );
			mSelectionPanels[ i ]->SetSizer( new wxBoxSizer( wxVERTICAL ) );
			mRenderPanels[ i ] = new tWxRenderPanel( this, mSelectionPanels[ i ], regKeyName + cRenderPanelNames[ i ] );
			mSelectionPanels[ i ]->GetSizer( )->Add( mRenderPanels[ i ], 1, wxEXPAND | wxALL, 2 );
			mMainGrid->Add( mSelectionPanels[ i ], 1, wxEXPAND );

			if( i == mFocusPanel )
				mSelectionPanels[ i ]->SetBackgroundColour( wxColour( "YELLOW" ) );
		}

		mMainGrid->AddGrowableCol( 0 );
		mMainGrid->AddGrowableCol( 1 );
		mMainGrid->AddGrowableRow( 0 );
		mMainGrid->AddGrowableRow( 1 );

		fShowOne( );
	}

	tWxRenderPanelContainer::~tWxRenderPanelContainer( )
	{
		Gfx::tDefaultAllocators::fInstance( ) = Gfx::tDefaultAllocators( );

		if( !mSolidColorMaterialFile.fNull( ) )
			mSolidColorMaterialFile->fUnload( this );
		if( !mDefaultFont.fNull( ) )
			mDefaultFont->fUnload( this );
		if( !mTextGeometryAllocator.fNull( ) )
			mTextGeometryAllocator->fDeallocate( );
		if( !mIndexAllocator.fNull( ) )
			mIndexAllocator->fDeallocate( );
		if( !mSolidColorGeometryAllocator.fNull( ) )
			mSolidColorGeometryAllocator->fDeallocate( );
		if( !mParticleGeomAllocator.fNull( ) )
			mParticleGeomAllocator->fDeallocate( );
		if( !mQuadGeometryAllocator.fNull( ) )
			mQuadGeometryAllocator->fDeallocate( );
		if( !mFullBrightMaterialFile.fNull( ) )
			mFullBrightMaterialFile->fUnload( this );
		if( !mDecalGeometryAllocator.fNull( ) )
			mDecalGeometryAllocator->fDeallocate( );
		if( !mDecalMaterialFile.fNull( ) )
			mDecalMaterialFile->fUnload( this );
	}

	void tWxRenderPanelContainer::fSetupRendering( tToolsGuiApp& guiApp )
	{
		Gfx::tDevicePtr device = guiApp.fGfxDevice( );
		sigassert( !device.fNull( ) );

		// create graphics data pools for text display
		{
			const u32 numTextQuads = 1024;
			const u32 numVerts = 4 * numTextQuads;
			const Gfx::tVertexFormat& vtxFormat = Gfx::tFontMaterial::cVertexFormat;
			const f32 vertsMegs = numVerts * vtxFormat.fVertexSize( ) / ( 1024.f * 1024.f );
			mTextGeometryAllocator.fReset( new Gfx::tGeometryBufferVRamAllocator );
			mTextGeometryAllocator->fAllocate( device, vtxFormat, numVerts, Gfx::tGeometryBufferVRam::cAllocDynamic );
		}

		// create graphics data pools for solid color geometry
		{
			const u32 num3dLines = 128 * 1024;
			const u32 numVerts = 2 * num3dLines;
			const Gfx::tVertexFormat& vtxFormat = Gfx::tSolidColorMaterial::cVertexFormat;
			const f32 vertsMegs = numVerts * vtxFormat.fVertexSize( ) / ( 1024.f * 1024.f );
			mSolidColorGeometryAllocator.fReset( new Gfx::tGeometryBufferVRamAllocator );
			mSolidColorGeometryAllocator->fAllocate( device, vtxFormat, numVerts, Gfx::tGeometryBufferVRam::cAllocDynamic );
		}

		// create allocator for particle geometry
		{
			const Gfx::tVertexFormat& vtxFormat = Gfx::tParticleMaterial::cVertexFormat;
			const u32 numTris = 256 * 1024;
			const u32 numVerts = 3 * numTris;
			const f32 vertsMegs = numVerts * vtxFormat.fVertexSize( ) / ( 1024.f * 1024.f );
			mParticleGeomAllocator.fReset( new Gfx::tGeometryBufferVRamAllocator );
			mParticleGeomAllocator->fAllocate( device, vtxFormat, numVerts, Gfx::tGeometryBufferVRam::cAllocDynamic );
			log_line( 0, "Using [" << vertsMegs << "] megs for particle geometry." );
		}	

		// create allocators for quad geometry
		{
			const Gfx::tVertexFormat& vtxFormat = Gfx::tFullBrightMaterial::cVertexFormat;
			const Gfx::tIndexFormat idxFormat( Gfx::tIndexFormat::cStorageU16, Gfx::tIndexFormat::cPrimitiveTriangleList );
			const u32 numQuads = 256 * 1024;
			const u32 numVerts = 4 * numQuads;
			const f32 vertsMegs = numVerts * vtxFormat.fVertexSize( ) / ( 1024.f * 1024.f );			
			mQuadGeometryAllocator.fReset( new Gfx::tGeometryBufferVRamAllocator );
			mQuadGeometryAllocator->fAllocate( device, vtxFormat, numVerts, Gfx::tGeometryBufferVRam::cAllocDynamic );
		}

		// create allocators for decal geometry
		{
			const Gfx::tVertexFormat& vtxFormat = Gfx::tDecalMaterial::cVertexFormat;
			const u32 numTris = 256 * 512;
			const u32 numVerts = 3 * numTris;
			const f32 vertsMegs = numVerts * vtxFormat.fVertexSize( ) / ( 1024.f * 1024.f );			
			mDecalGeometryAllocator.fReset( new Gfx::tGeometryBufferVRamAllocator );
			mDecalGeometryAllocator->fAllocate( device, vtxFormat, numVerts, Gfx::tGeometryBufferVRam::cAllocDynamic );
		}

		{
			const Gfx::tIndexFormat idxFormat( Gfx::tIndexFormat::cStorageU16, Gfx::tIndexFormat::cPrimitiveTriangleList );
			const u32 numIds = 1024*1024;
			const f32 idsMegs = numIds * idxFormat.mSize / ( 1024.f * 1024.f );
			mIndexAllocator.fReset( new Gfx::tIndexBufferVRamAllocator );
			mIndexAllocator->fAllocate( device, idxFormat, numIds, 0/*numPrims*/, Gfx::tGeometryBufferVRam::cAllocDynamic );
		}

		// load full bright material
		mFullBrightMaterialFile = guiApp.fResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tFullBrightMaterial::fMaterialFilePath( ) ), this );

		// load solid color material
		mSolidColorMaterialFile = guiApp.fResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tSolidColorMaterial::fMaterialFilePath( ) ), this );
		mSolidColorMaterial.fReset( new Gfx::tSolidColorMaterial( ) );
		mSolidColorMaterial->fSetMaterialFileResourcePtrOwned( *guiApp.fResourceDepot( ), mSolidColorMaterialFile->fGetPath( ) );

		// load decal material
		mDecalMaterialFile = guiApp.fResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tDecalMaterial::fMaterialFilePath( ) ), this );

		// load default font // TODO parameterize font resource
		mDefaultFont = guiApp.fResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<Gui::tFont>( Gui::tFont::fDevFontPath( ) ), this );

		Gfx::tDefaultAllocators& defGuiAllocators = Gfx::tDefaultAllocators::fInstance( );
		defGuiAllocators.mResourceDepot				= guiApp.fResourceDepot( );
		defGuiAllocators.mFullBrightGeomAllocator	= mQuadGeometryAllocator;
		defGuiAllocators.mSolidColorGeomAllocator	= mSolidColorGeometryAllocator;
		defGuiAllocators.mTextGeometryAllocator		= mTextGeometryAllocator;
		defGuiAllocators.mParticleGeomAllocator		= fGetParticleGeometryAllocator( );
		defGuiAllocators.mDecalGeomAllocator		= mDecalGeometryAllocator;
		defGuiAllocators.mDecalMaterialFile			= mDecalMaterialFile;
		defGuiAllocators.mIndexAllocator			= mIndexAllocator;
		defGuiAllocators.mFullBrightMaterialFile	= mFullBrightMaterialFile;
		defGuiAllocators.mSolidColorMaterialFile	= mSolidColorMaterialFile;
		defGuiAllocators.mSolidColorMaterial		= mSolidColorMaterial;

		// setup rendering on all render panels
		for( u32 i = 0; i < mRenderPanels.fCount( ); ++i )
		{
			if( mRenderPanels[ i ] )
				mRenderPanels[ i ]->fSetupRendering( guiApp );
		}

		mRenderPanels[ cPanelBotLeft ]->fSetOrthoAndLookPos( Math::tVec3f::cZeroVector, Math::tVec3f::cYAxis, -Math::tVec3f::cZAxis );
		mRenderPanels[ cPanelTopLeft ]->fSetOrthoAndLookPos( Math::tVec3f::cZeroVector, Math::tVec3f::cZAxis, Math::tVec3f::cYAxis );
		mRenderPanels[ cPanelBotRight ]->fSetOrthoAndLookPos( Math::tVec3f::cZeroVector, Math::tVec3f::cXAxis, Math::tVec3f::cYAxis );

		// Disable rotation and ortho toggle for the ortho panels.
		mRenderPanels[ cPanelBotLeft ]->fDisableRotation( );
		mRenderPanels[ cPanelTopLeft ]->fDisableRotation( );
		mRenderPanels[ cPanelBotRight ]->fDisableRotation( );
		mRenderPanels[ cPanelBotLeft ]->fDisableOrthoToggle( );
		mRenderPanels[ cPanelTopLeft ]->fDisableOrthoToggle( );
		mRenderPanels[ cPanelBotRight ]->fDisableOrthoToggle( );

		mRenderPanels[ cPanelTopLeft ]->fGetGridSettings( )->fSetGridAxes( Math::tVec3f::cXAxis, Math::tVec3f::cYAxis );
		mRenderPanels[ cPanelTopLeft ]->fGetGridSettings( )->fUpdateGrid( );
		mRenderPanels[ cPanelBotRight ]->fGetGridSettings( )->fSetGridAxes( Math::tVec3f::cZAxis, Math::tVec3f::cYAxis );
		mRenderPanels[ cPanelBotRight ]->fGetGridSettings( )->fUpdateGrid( );

		fShowOne( );
	}

	void tWxRenderPanelContainer::fOnTick( )
	{
		if( fGetActiveScreen( ) )
		{
			const Math::tVec3f shadowTarget = fGetActiveScreen( )->fViewport( 0 )->fRenderCamera( ).fGetTripod( ).mLookAt;
			mMainWindow->fGuiApp( ).fDefaultLight( )->fUpdateShadowMapTarget( shadowTarget );
		}

		if( mSinglePaneView )
		{
			mRenderPanels[ mFocusPanel ]->fOnTick( );
			return;
		}

		for( u32 i = 0; i < mRenderPanels.fCount( ); ++i )
		{
			if( mRenderPanels[ i ] )
			{
				mRenderPanels[ i ]->fOnTick( );

				if( mRenderPanels[ i ]->fGetMouse( ).fButtonDown( Input::tMouse::cButtonLeft )
					&& mRenderPanels[ i ]->fGetMouse( ).fCursorInClientArea( ) )
				{
					fSetPanelFocus( i );
				}
			}
		}
	}

	void tWxRenderPanelContainer::fRender( const Gfx::tDisplayStats* selectedDisplayStats )
	{
		if( mSinglePaneView )
		{
			mRenderPanels[ mFocusPanel ]->fRender( selectedDisplayStats );
			return;
		}

		for( u32 i = 0; i < mRenderPanels.fCount( ); ++i )
		{
			if( mRenderPanels[ i ] )
				mRenderPanels[ i ]->fRender( selectedDisplayStats );
		}
	}

	tWxRenderPanel* tWxRenderPanelContainer::fGetActiveRenderPanel( )
	{
		if( mSinglePaneView )
		{
			if( mRenderPanels[ mFocusPanel ]->fGetMouse( ).fCursorInClientArea( ) )
				return mRenderPanels[ mFocusPanel ];

			return 0;
		}

		for( u32 i = 0; i < mRenderPanels.fCount( ); ++i )
		{
			if( mRenderPanels[ i ] && mRenderPanels[ i ]->fGetMouse( ).fCursorInClientArea( ) )
				return mRenderPanels[ i ];
		}

		return 0;
	}

	Gfx::tScreen* tWxRenderPanelContainer::fGetActiveScreen( )
	{
		tWxRenderPanel* panel = fGetActiveRenderPanel( );
		return panel ? panel->fGetScreen( ).fGetRawPtr( ) : 0;
	}

	tWxRenderPanel*	tWxRenderPanelContainer::fGetFocusRenderPanel( )
	{
		return mRenderPanels[ mFocusPanel ];
	}

	b32 tWxRenderPanelContainer::fPanelIsMain( tWxRenderPanel* panel )
	{
		return panel == mRenderPanels[ mFocusPanel ];
	}

	void tWxRenderPanelContainer::fShowFour( )
	{
		Freeze( );
		for( u32 i = 0; i < cPanelCount; ++i )
		{
			if( i == mFocusPanel )
				continue;

			mRenderPanels[ i ]->SetSize( -1, -1 );
			mMainGrid->Show( mRenderPanels[ i ] );
			mMainGrid->Show( mSelectionPanels[ i ] );
		}

		mSelectionPanels[ mFocusPanel ]->SetBackgroundColour( wxColour( "YELLOW" ) );
		mSelectionPanels[ mFocusPanel ]->Refresh( );

		Layout( );

		mSinglePaneView = false;

		Thaw( );
	}

	void tWxRenderPanelContainer::fShowOne( )
	{
		Freeze( );

		mSelectionPanels[ mFocusPanel ]->SetBackgroundColour( GetBackgroundColour( ) );
		mSelectionPanels[ mFocusPanel ]->Refresh( );

		for( u32 i = 0; i < cPanelCount; ++i )
		{
			if( i == mFocusPanel )
			{
				mRenderPanels[ i ]->SetSize( -1, -1 );
				continue;
			}

			mRenderPanels[ i ]->SetSize( 0, 0 );
			mMainGrid->Hide( mRenderPanels[ i ] );
			mMainGrid->Hide( mSelectionPanels[ i ] );
		}

		Layout( );

		mSinglePaneView = true;

		Thaw( );
	}

	void tWxRenderPanelContainer::fToggleViewMode( )
	{
		if( mSinglePaneView )
		{
			fShowFour( );
		}
		else
		{
			fShowOne( );
		}
	}

	void tWxRenderPanelContainer::fFrameAllViewports( const Math::tAabbf& frameBox )
	{
		for( u32 i = 0; i < mRenderPanels.fCount( ); ++i )
		{
			if( mRenderPanels[ i ] )
				mRenderPanels[ i ]->fFrame( frameBox );
		}
	}

	void tWxRenderPanelContainer::fSetPanelFocus( u32 newFocus )
	{
		if( mFocusPanel == newFocus )
			return;

		mFocusPanel = newFocus;

		for( u32 i = 0; i < cPanelCount; ++i )
		{
			if( i == newFocus )
			{
				mSelectionPanels[ i ]->SetBackgroundColour( wxColour( "YELLOW" ) );
				mSelectionPanels[ i ]->Refresh( );
			}
			else
			{
				mSelectionPanels[ i ]->SetBackgroundColour( GetBackgroundColour( ) );
				mSelectionPanels[ i ]->Refresh( );
			}
		}
	}
}

