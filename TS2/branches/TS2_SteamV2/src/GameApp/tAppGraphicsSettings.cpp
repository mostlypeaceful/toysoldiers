//------------------------------------------------------------------------------
// \file tAppGraphicsSettings.cpp - 15 Apr 2013
// \author pwilliams
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "GameAppPch.hpp"
#include "tAppGraphicsSettings.hpp"
#include "tGameApp.hpp"
#include "tXmlSerializer.hpp"
#include "tXmlDeserializer.hpp"
#include "Gfx/tDevice.hpp"

#ifdef platform_pcdx9
#include "tSteamManager.hpp"

namespace Sig
{
	tAppGraphicsSettings tAppGraphicsSettings::gDefaultSettings;  // these are the default settings on running the app - these are never changed
	tAppGraphicsSettings tAppGraphicsSettings::gCapturedSettings;  // these are the initial settings for when opening the settings menu

	const tFilePathPtr cGfxSettingsFileName( "gfxSettings.xml" );
	const tStringPtr cSettings( "Settings" );
	const tStringPtr cDisplay( "display" );
	const tStringPtr cXRes( "xres" );
	const tStringPtr cYRes( "yres" );
	const tStringPtr cFullscreen( "fullscreen" );
	const tStringPtr cVSync( "vsync" );
	const tStringPtr cRendering( "rendering" );
	const tStringPtr cBrightness( "brightness" );
	const tStringPtr cTextureDetail( "textureDetail" );
	const tStringPtr cShadowQuality( "shadowQuality" );
	const tStringPtr cAntiAlias( "antiAlias" );

	tLocalizedString cEmptyString;

	enum tSetting
	{
		cSetting_Resolution,
		cSetting_Fullscreen,
		cSetting_Vsync,
		cSetting_Brightness,
		cSetting_TextureDetail, // high/low
		cSetting_ShadowQuality, // off/low/medium/high
		cSetting_AntiAliasing,  // off/2x/4x

		cSetting_Count
	};

	enum tSettingType { cSettingType_Slider, cSettingType_Options, cSettingType_Count };

	static const tStringPtr cLowHigh[2] =
	{
		tStringPtr( "Graphics_Low" ),
		tStringPtr( "Graphics_High" ),
	};

	static const tStringPtr cOffOn[2] =
	{
		tStringPtr( "Graphics_Off" ),
		tStringPtr( "Graphics_On" ),
	};

	static const tStringPtr cResolutionNames[4] =
	{ 
		// need to get enums from graphics device
		tStringPtr( "Graphics_Resolution_1" ),
		tStringPtr( "Graphics_Resolution_2" ),
		tStringPtr( "Graphics_Resolution_3" ),
		tStringPtr( "Graphics_Resolution_4" ),
	};

	static const tStringPtr cVsyncNames[3] =
	{ 
		tStringPtr( "Graphics_Vsync_Off" ),
		tStringPtr( "Graphics_Vsync_60" ),
		tStringPtr( "Graphics_Vsync_30" ),
	};

	static const tStringPtr cFullscreenOptionNames[2] =
	{
		tStringPtr( "Graphics_Off" ),
		tStringPtr( "Graphics_On" ),
		// TODO: windowed fullscreen would be good too. Maybe change name to Window mode or something.
	};

	static const tStringPtr cFullscreenOptionDescs[2] =
	{
		tStringPtr( "Graphics_Fullscreen_Off_Desc" ),
		tStringPtr( "Graphics_Fullscreen_On_Desc" ),
	};
		
	static const tStringPtr cShadowQualityNames[3] =
	{
		tStringPtr( "Graphics_ShadowQuality_Low" ),
		tStringPtr( "Graphics_ShadowQuality_Medium" ),
		tStringPtr( "Graphics_ShadowQuality_High" )
	};

	static const tStringPtr cShadowQualityDescs[3] =
	{
		tStringPtr( "Graphics_ShadowQuality_Low_Desc" ),
		tStringPtr( "Graphics_ShadowQuality_Medium_Desc" ),
		tStringPtr( "Graphics_ShadowQuality_High_Desc" ),
	};

	static const tStringPtr cAntiAliasingNames[3] =
	{
		tStringPtr( "Graphics_AntiAliasing_Off" ),
		tStringPtr( "Graphics_AntiAliasing_Low" ),
		tStringPtr( "Graphics_AntiAliasing_High" )
	};

	struct tDisplayMode
	{
		tLocalizedString mName;
		u32 mWidth;
		u32 mHeight;
	};

	struct tSettingInfo
	{
		tSettingInfo( ) : mType( cSettingType_Count ) { }
		tSettingInfo
			( const tStringPtr& name
			, const tStringPtr optionNames[]
			, const tStringPtr optionDescs[]
			, u32 optionCount )
			: mType( cSettingType_Options )
			, mName( name )
			, mOptionNames( optionNames, optionCount )
			, mOptionDescs( optionDescs, optionCount )
		{
			sigassert( mName.fLength( ) );
			sigassert( optionCount > 0 );
		}

		tSettingInfo
			( const tStringPtr& name
			, const tStringPtr& desc )
			: mType( cSettingType_Slider )
			, mName( name )
			, mDesc( desc )
		{
			sigassert( mName.fLength( ) );
			sigassert( mDesc.fLength( ) );
		}

		u32 mType;
			
		tStringPtr mName;
		tStringPtr mDesc;
		tArraySleeve<const tStringPtr> mOptionNames;
		tArraySleeve<const tStringPtr> mOptionDescs;
	};

	static tGrowableArray<tDisplayMode> gDisplayModes;
	static u32 fFindClosestDisplayMode( u32 width, u32 height )
	{
		// look for closest match
		u32 matchIdx = 0;
		s32 matchDist = 10000;
		for( u32 i=0; i<gDisplayModes.fCount( ); i++ )
		{
			s32 dist = fAbs((s32)(gDisplayModes[i].mWidth-width)) + fAbs((s32)(gDisplayModes[i].mHeight-height));
			if (dist < matchDist)
			{
				matchDist = dist;
				matchIdx = i;
			}
		}
		return matchIdx;
	}


	static b32 SortDisplayModesLowestToHighest( const tDisplayMode& a, const tDisplayMode& b )
	{
		if( b.mWidth == a.mWidth )
		{
			return b.mHeight > a.mHeight;
		}

		return b.mWidth > a.mWidth;
	}


	static tFixedArray<tSettingInfo, cSetting_Count> gSettingInfo;
	define_static_function( fBuildSettingInfo )
	{			
		gSettingInfo[ cSetting_Resolution ] = tSettingInfo
			( tStringPtr( "Settings_Resolution" ), cResolutionNames, NULL, array_length( cResolutionNames ) );

		gSettingInfo[ cSetting_Fullscreen ] = tSettingInfo
			( tStringPtr( "Graphics_Fullscreen" ), cFullscreenOptionNames, cFullscreenOptionDescs, array_length( cFullscreenOptionNames ) );

		gSettingInfo[ cSetting_Vsync ] = tSettingInfo
			( tStringPtr( "Graphics_Vsync" ), cVsyncNames, NULL, array_length( cVsyncNames ) );

		gSettingInfo[ cSetting_Brightness ] = tSettingInfo
			( tStringPtr( "Settings_Brightness" ), tStringPtr( "Settings_Brightness_Desc" ) );

		gSettingInfo[ cSetting_AntiAliasing ] = tSettingInfo
			( tStringPtr( "Graphics_AntiAliasing" ), cAntiAliasingNames, NULL, array_length( cAntiAliasingNames ) );

		gSettingInfo[ cSetting_TextureDetail ] = tSettingInfo
			( tStringPtr( "Graphics_TextureDetail" ), cLowHigh, NULL, array_length( cLowHigh ) );

		gSettingInfo[ cSetting_ShadowQuality ] = tSettingInfo
			( tStringPtr( "Graphics_ShadowQuality" ), cShadowQualityNames, cShadowQualityDescs, array_length( cShadowQualityNames ) );
	}

	//------------------------------------------------------------------------------
	// tApplicationGraphicsSettings
	//------------------------------------------------------------------------------
	// note that the constructor values are considered to be 'default' settings, so they are important..
	//
	tAppGraphicsSettings::tAppGraphicsSettings( )
		: mBrightness( 0.4f ) // Found empirically to be closest to xdk default
		, mVsync( Gfx::cVsync60Hz )
		, mResolution( 0 )
		, mIsFullscreen( true )
		, mTextureDetail( 1 )
		, mShadowQuality( 2 )
		, mAntiAliasLevel( 0 )
		, mResolutionX( 1280 )
		, mResolutionY( 720 )
		, mAllowSave( true )
	{
		// enumerate the display modes the first time...
		if ( gDisplayModes.fCount( ) == 0 )
			fEnumerateDisplayModes( );

		// consider the 'default' resolution to the current display mode resolution
		DEVMODE dm;
		EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dm );
		mResolutionX = dm.dmPelsWidth;
		mResolutionY = dm.dmPelsHeight;
	}

	void tAppGraphicsSettings::fEnumerateDisplayModes( )
	{
		DEVMODE dm;				// The structure needed to hold the settings data
		int i = 0;					// Index value that specifies the graphics mode for 
											// which information is to be obtained.
		gDisplayModes.fDeleteArray( );
		while( EnumDisplaySettings( NULL, i++, &dm ) ) 
		{
			log_output( 0, "DRIVER " << dm.dmDeviceName << " Size " << dm.dmPelsWidth << "x" << dm.dmPelsHeight << "x" << dm.dmBitsPerPel << " @ " << dm.dmDisplayFrequency << std::endl );

			if ( dm.dmBitsPerPel == 32 && dm.dmDisplayFixedOutput == 0 )
			{
				// check if this mode already exists...
				bool exists = false;
				for ( u32 i = 0; i < gDisplayModes.fCount( ); i++ )
				{
					if ( gDisplayModes[i].mWidth == dm.dmPelsWidth && gDisplayModes[i].mHeight == dm.dmPelsHeight )
					{
						exists = true;
						break;
					}
				}

				if ( !exists )
				{
					tDisplayMode mode;
					mode.mWidth = dm.dmPelsWidth;
					mode.mHeight = dm.dmPelsHeight;

					std::stringstream ss;
					ss << mode.mWidth << "x" << mode.mHeight;
					mode.mName = tLocalizedString::fFromCString( ss.str( ).c_str( ) );
					gDisplayModes.fPushBack( mode );
				}
			}
		}

		// add full virtual screen if it is bigger than the largest mode
#if 0
		int virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		int actualWidth = GetSystemMetrics(SM_CXSCREEN);
		int actualHeight = GetSystemMetrics(SM_CYSCREEN);
		if (virtualWidth != actualWidth || virtualHeight != actualHeight)
		{
			tDisplayMode mode;
			mode.mWidth = virtualWidth;
			mode.mHeight = virtualHeight;

			std::stringstream ss;
			ss << mode.mWidth << "x" << mode.mHeight;
			mode.mName = tLocalizedString::fFromCString( ss.str( ).c_str( ) );
			gDisplayModes.fPushBack( mode );
		}
#endif

		std::sort( gDisplayModes.fBegin( ), gDisplayModes.fEnd( ), SortDisplayModesLowestToHighest );
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSetResolution( u32 resolution )
	{
		if( mResolution != resolution)
		{
			mResolution = resolution;

			tDisplayMode &mode = gDisplayModes [ resolution ];
			mResolutionX = mode.mWidth;
			mResolutionY = mode.mHeight;
		}
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSetFullscreen( b32 fullscreen )
	{
		mIsFullscreen = fullscreen;
	}

	//------------------------------------------------------------------------------
	b32 tAppGraphicsSettings::fIsFullscreen( ) const
	{
		return tSteamManager::fIsBigPictureMode( ) || mIsFullscreen;
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSetVsync( Gfx::tVsync vsync )
	{
		mVsync = ( u32 )vsync;
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSetBrightness( f32 brightness ) 
	{ 
		if ( brightness != mBrightness )
		{
			mBrightness = brightness;
			tGameApp::fInstance( ).fSetBrightness( fBrightness( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSetTextureDetail( b32 textureDetail )
	{
		if ( textureDetail != mTextureDetail )
		{
			mTextureDetail = textureDetail;
			//TODO: do something?
		}
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSetShadowQuality( u32 shadowQuality )
	{
		sigassert( shadowQuality < 3 );

		if( mShadowQuality != shadowQuality )
		{
			mShadowQuality = shadowQuality;
		}
	}

	static u32 sShadowLayers[3] = { 1,1,1 };
	static u32 sShadowResolutions[3] = { 1024, 2048, 4096 };
	static f32 sShadowSize[3] = { 200, 400, 700 };
	static D3DMULTISAMPLE_TYPE sMultisampleLevels[3] = { D3DMULTISAMPLE_NONE, D3DMULTISAMPLE_4_SAMPLES, D3DMULTISAMPLE_16_SAMPLES };

	b32 tAppGraphicsSettings::fShadowsEnabled( ) const
	{
		return mShadowQuality != 0;
	}

	u32 tAppGraphicsSettings::fShadowLayers( ) const
	{
		return sShadowLayers[ mShadowQuality ];
	}

	u32 tAppGraphicsSettings::fShadowResolution( ) const
	{
		return sShadowResolutions[ mShadowQuality ];
	}

	f32 tAppGraphicsSettings::fShadowRange( ) const
	{
		return sShadowSize[ mShadowQuality ];
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSetAntiAliasLevel( u32 antiAliasLevel )
	{
		if( mAntiAliasLevel != antiAliasLevel )
		{
			mAntiAliasLevel = antiAliasLevel;
		}
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fLoad( )
	{
		// initialise to default settings - in case the load fails for any reason
		if( !tGameApp::cLocalAppData.fNull( ) )
		{
			tXmlDeserializer des;
			if( !des.fLoad( tFilePathPtr::fConstructPath( tGameApp::cLocalAppData, cGfxSettingsFileName ), cSettings.fCStr( ), *this ) )
				fSave( );
		}

		mResolution = fFindClosestDisplayMode( mResolutionX, mResolutionY );
		gCapturedSettings = *this;
	}

	void tAppGraphicsSettings::fSerializeXml( tXmlDeserializer& s )
	{
		tXmlNode child;
		if( s.fCurRoot( ).fFindChild( child, cDisplay.fCStr( ) ) )
		{
			child.fGetAttribute( cXRes.fCStr( ), mResolutionX );
			child.fGetAttribute( cYRes.fCStr( ), mResolutionY );
			child.fGetAttribute( cFullscreen.fCStr( ), mIsFullscreen );
			child.fGetAttribute( cVSync.fCStr( ), mVsync );
		}
		if ( s.fCurRoot( ).fFindChild( child, cRendering.fCStr( ) ) )
		{
			child.fGetAttribute( cBrightness.fCStr( ), mBrightness );
			child.fGetAttribute( cTextureDetail.fCStr( ), mTextureDetail );
			child.fGetAttribute( cShadowQuality.fCStr( ), mShadowQuality );
			child.fGetAttribute( cAntiAlias.fCStr( ), mAntiAliasLevel );
		}

	}

	void tAppGraphicsSettings::fSerializeXml( tXmlSerializer& s )
	{
		tXmlNode displayNode = s.fCurRoot( ).fCreateChild( cDisplay.fCStr( ) );
		displayNode.fSetAttribute( cXRes.fCStr( ), mResolutionX );
		displayNode.fSetAttribute( cYRes.fCStr( ), mResolutionY );
		displayNode.fSetAttribute( cFullscreen.fCStr( ), mIsFullscreen );
		displayNode.fSetAttribute( cVSync.fCStr( ), mVsync );

		tXmlNode renderingNode = s.fCurRoot( ).fCreateChild( cRendering.fCStr( ) );
		renderingNode.fSetAttribute( cBrightness.fCStr( ), mBrightness );
		renderingNode.fSetAttribute( cTextureDetail.fCStr( ), mTextureDetail );
		renderingNode.fSetAttribute( cShadowQuality.fCStr( ), mShadowQuality );
		renderingNode.fSetAttribute( cAntiAlias.fCStr( ), mAntiAliasLevel );
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fSave( )
	{
		// Called explicitly, so allow saves again from the params changed event
		mAllowSave = true;
		// initialise to default settings - in case the load fails for any reason
		if( !tGameApp::cLocalAppData.fNull( ) ) 
		{
			tXmlSerializer des;
			des.fSave( tFilePathPtr::fConstructPath( tGameApp::cLocalAppData, cGfxSettingsFileName ), cSettings.fCStr( ), *this, false, false );
		}

		gCapturedSettings = *this;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fSetToDefault( )
	{
		*this = gDefaultSettings;

		fSetSettingSlider( cSetting_Brightness, fBrightness( ) );
		fSetSettingOption( cSetting_Resolution, fResolution( ) );
		fSetSettingOption( cSetting_Fullscreen, fIsFullscreen( ) ? 1 : 0 );
		fSetSettingOption( cSetting_Vsync, fVsync( ) );
		fSetSettingOption( cSetting_TextureDetail, fTextureDetail( ) ? 1 : 0 );
		fSetSettingOption( cSetting_ShadowQuality, fShadowQuality( ) );
		fSetSettingOption( cSetting_AntiAliasing, fAntiAliasLevel( ) );

		// change Brightness immediately
		tGameApp::fInstance( ).fSetBrightness( fBrightness( ) );

		return 0;
	}

	void tAppGraphicsSettings::fApplyDisplayMode( )
	{
		if ( mAppliedDisplayWidth != mResolutionX || mAppliedDisplayHeight != mResolutionY
			 || mAppliedDisplayFullscreen != mIsFullscreen || mAppliedDisplayVSync != mVsync || mAppliedShadowQuality != mShadowQuality
			 || mAntiAliasLevel != mAppliedAntiAliasLevel )
		{
			tGameApp::fInstance( ).fScreen( )->fUpdateShadowSize( fShadowsEnabled( ), fShadowLayers( ), fShadowResolution( ) );
			Gfx::tDevice::fGetDefaultDevice( )->fSetMultisamplePower( fAntiAliasLevel( ) );
			tGameApp::fInstance( ).fSetBrightness( fBrightness( ) );
			tGameApp::fInstance( ).fScreen( )->fDevice( )->fResetDisplayMode( mResolutionX, mResolutionY, fIsFullscreen( ), mVsync );

			fGatherDisplaySettings( );
		}
	}

	void tAppGraphicsSettings::fGatherDisplaySettings( )
	{
		mAppliedDisplayWidth = mResolutionX;
		mAppliedDisplayHeight = mResolutionY;
		mAppliedDisplayFullscreen = fIsFullscreen( );
		mAppliedDisplayVSync = mVsync;
		mAppliedShadowQuality = mShadowQuality;
		mAppliedAntiAliasLevel = mAntiAliasLevel;

		log_line( 0, "GATHER SETTINGS FULLSCREEN " << mAppliedDisplayFullscreen );
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fCaptureSettings( )
	{
		// pull the screen settings straight from the graphics device (since we may have alt-enter'd to change them without going through settings)
		mResolution = fFindClosestDisplayMode( mResolutionX, mResolutionY );

		// called when entering the graphics menu
		gCapturedSettings = *this;
		fGatherDisplaySettings( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fTestChanges( )
	{
		mAllowSave = false;
		fApplyDisplayMode( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fResolveChanges( b32 keep )
	{
		if( keep )
		{
			//gCapturedSettings = *this;
			//fSave( );
		}
		else
		{
			//*this = gCapturedSettings;
			//fSetBrightness( this->mBrightness );
			//fSetAntiAliasLevel( this->mAntiAliasLevel );
			mAllowSave = true;
		}
		fApplyDisplayMode( );
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fGetSettingCount( ) const
	{
		return cSetting_Count;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fGetSettingType( u32 setting ) const
	{
		return gSettingInfo[ setting ].mType;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fGetSettingOptionCount( u32 setting ) const
	{
		if ( setting == cSetting_Resolution )
			return gDisplayModes.fCount( );
		return gSettingInfo[ setting ].mOptionNames.fCount( );
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tAppGraphicsSettings::fGetSettingName( u32 setting ) const
	{
		return tGameApp::fInstance( ).fLocString( gSettingInfo[ setting ].mName );
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tAppGraphicsSettings::fGetSettingDesc( u32 setting ) const
	{
		const tStringPtr& locId = gSettingInfo[ setting ].mDesc;
		if( !locId.fLength( ) )
			return cEmptyString;
		
		return tGameApp::fInstance( ).fLocString( locId );
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tAppGraphicsSettings::fGetSettingOptionName( u32 setting, u32 option ) const
	{
		if ( setting == cSetting_Resolution )
			return gDisplayModes[ option ].mName;
		return tGameApp::fInstance( ).fLocString( gSettingInfo[ setting ].mOptionNames[ option ] );
	}

	//------------------------------------------------------------------------------
	const tLocalizedString& tAppGraphicsSettings::fGetSettingOptionDesc( u32 setting, u32 option ) const
	{
		if ( setting == cSetting_Resolution || gSettingInfo[ setting ].mOptionDescs.fNull( ) )
			return cEmptyString;

		return tGameApp::fInstance( ).fLocString( gSettingInfo[ setting ].mOptionDescs[ option ] );
	}

	//------------------------------------------------------------------------------
	f32 tAppGraphicsSettings::fGetSettingSlider( u32 setting ) const
	{
		sigassert( gSettingInfo[ setting ].mType == cSettingType_Slider );
		
		switch( setting )
		{
		case cSetting_Brightness:
			return fBrightness( );
		default:
			log_warning( Log::cFlagGraphics, __FUNCTION__ );
			break;
		}

		return 0.f;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fSetSettingSlider( u32 setting, f32 slider )
	{
		sigassert( gSettingInfo[ setting ].mType == cSettingType_Slider );

		switch( setting )
		{
		case cSetting_Brightness:
			fSetBrightness( slider );
			break;
		default:
			log_warning( Log::cFlagGraphics, __FUNCTION__ );
			break;
		}

		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fGetSettingOption( u32 setting ) const
	{
		sigassert( gSettingInfo[ setting ].mType == cSettingType_Options );
		
		switch( setting )
		{
		case cSetting_Resolution:
			return fResolution( );
		case cSetting_Fullscreen:
			return fIsFullscreen( ) ? 1 : 0;
		case cSetting_Vsync:
			return ( u32 )fVsync( );
		case cSetting_TextureDetail:
			return fTextureDetail( ) ? 1 : 0;
		case cSetting_ShadowQuality:
			return fShadowQuality( );
		case cSetting_AntiAliasing:
			return fAntiAliasLevel( );

		default:
			log_warning( Log::cFlagGraphics, __FUNCTION__ );
			break;
		}

		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tAppGraphicsSettings::fSetSettingOption( u32 setting, u32 option )
	{
		sigassert( gSettingInfo[ setting ].mType == cSettingType_Options );
		sigassert( option < (setting == cSetting_Resolution ? gDisplayModes.fCount ( ) : gSettingInfo[ setting ].mOptionNames.fCount( )) );
		
		switch( setting )
		{
		case cSetting_Resolution: 
			fSetResolution( option );
			break;
		case cSetting_Fullscreen: 
			fSetFullscreen( !!option );
			break;
		case cSetting_Vsync: 
			fSetVsync( (Gfx::tVsync)option );
			break;
		case cSetting_TextureDetail: 
			fSetTextureDetail( !!option );
			break;
		case cSetting_ShadowQuality: 
			fSetShadowQuality( option );
			break;
		case cSetting_AntiAliasing: 
			fSetAntiAliasLevel( option );
			break;

		default:
			log_warning( Log::cFlagGraphics, __FUNCTION__ );
			break;
		}

		return 0;
	}

	//------------------------------------------------------------------------------
	b32 tAppGraphicsSettings::fGetSettingEnabled( u32 setting ) const
	{
		switch( setting )
		{
		case cSetting_Fullscreen:
			return !tSteamManager::fIsBigPictureMode( );

		default:
			break;
		}

		return true;
	}

	//------------------------------------------------------------------------------
	void tAppGraphicsSettings::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tAppGraphicsSettings, Sqrat::DefaultAllocator<tAppGraphicsSettings>> classDesc( vm.fSq( ) );
		classDesc
			.Func( "SetToDefault",			&tAppGraphicsSettings::fSetToDefault )
			.Func( "CaptureSettings",		&tAppGraphicsSettings::fCaptureSettings )
			.Func( "ResolveChanges",		&tAppGraphicsSettings::fResolveChanges )
			.Func( "TestChanges",			&tAppGraphicsSettings::fTestChanges )
			.Func( "GetSettingCount",		&tAppGraphicsSettings::fGetSettingCount )
			.Func( "GetSettingType",		&tAppGraphicsSettings::fGetSettingType )
			.Func( "GetSettingOptionCount", &tAppGraphicsSettings::fGetSettingOptionCount )
			.Func( "GetSettingName",		&tAppGraphicsSettings::fGetSettingName )
			.Func( "GetSettingDesc",		&tAppGraphicsSettings::fGetSettingDesc )
			.Func( "GetSettingOptionName",	&tAppGraphicsSettings::fGetSettingOptionName )
			.Func( "GetSettingOptionDesc",	&tAppGraphicsSettings::fGetSettingOptionDesc )
			.Func( "GetSettingSlider",		&tAppGraphicsSettings::fGetSettingSlider )
			.Func( "SetSettingSlider",		&tAppGraphicsSettings::fSetSettingSlider )
			.Func( "GetSettingOption",		&tAppGraphicsSettings::fGetSettingOption )
			.Func( "SetSettingOption",		&tAppGraphicsSettings::fSetSettingOption )
			.Func( "GetSettingEnabled",		&tAppGraphicsSettings::fGetSettingEnabled )
			.Func( "Save",					&tAppGraphicsSettings::fSave )
			;
		vm.fRootTable( ).Bind( _SC("AppGraphicsSettings"), classDesc ); 
		vm.fConstTable( ).Const(_SC("GRAPHICS_SETTING_RESOLUTION"), (int)cSetting_Resolution );
		vm.fConstTable( ).Const(_SC("GRAPHICS_SETTING_FULLSCREEN"), (int)cSetting_Fullscreen );
		vm.fConstTable( ).Const(_SC("GRAPHICS_SETTING_VSYNC"), (int)cSetting_Vsync );
		vm.fConstTable( ).Const(_SC("GRAPHICS_SETTING_BRIGHTNESS"), (int)cSetting_Brightness );
		vm.fConstTable( ).Const(_SC("GRAPHICS_SETTING_SHADOW_QUALITY"), (int)cSetting_ShadowQuality );
		vm.fConstTable( ).Const(_SC("GRAPHICS_SETTING_ANTIALIASING"), (int)cSetting_AntiAliasing );
		vm.fConstTable( ).Const(_SC("GRAPHICS_SETTING_TEXTURE_DETAIL"), (int)cSetting_TextureDetail );
	}
	void tAppGraphicsSettings::fExportScriptInstance( tScriptVm& vm, const char *name )
	{
		vm.fRootTable( ).SetInstance(_SC(name), this);
	}

	void tAppGraphicsSettings::fRegisterDeviceCallbacks()
	{
		Gfx::tDevice::fGetDefaultDevice( )->fAddParamsChangedCallback( make_delegate_memfn( Gfx::tDevice::tCallback, tAppGraphicsSettings, fOnDeviceParamsChanged) );
	}

	void tAppGraphicsSettings::fOnDeviceParamsChanged( const Gfx::tDevicePtr &device )
	{
		const D3DPRESENT_PARAMETERS &params = device->fGetCreationPresentParams( );
		mResolutionX = params.BackBufferWidth;
		mResolutionY = params.BackBufferHeight;
		mResolution = fFindClosestDisplayMode( mResolutionX, mResolutionY );
		mVsync = params.PresentationInterval != D3DPRESENT_INTERVAL_IMMEDIATE;
		mIsFullscreen = !params.Windowed;
		if( mAllowSave )
			fSave( );
	}

} // ::Sig

#endif
