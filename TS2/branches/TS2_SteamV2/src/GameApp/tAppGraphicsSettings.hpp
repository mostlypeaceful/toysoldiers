//------------------------------------------------------------------------------
// \file tAppGraphicsSettings.hpp - 15 Apr 2013
// \author pwilliams
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tAppGraphicsSettings__
#define __tAppGraphicsSettings__

#include "tUser.hpp"
#include "Gui/tSaveUI.hpp"
#include "Threads/tThread.hpp"

#ifdef platform_pcdx9
#include "Gfx/tDevice_pcdx9.hpp"
namespace Sig
{
	class tLocalizedString;
	class tXmlDeserializer;
	class tXmlSerializer;

	///
	/// \class tAppGraphicsSettings
	/// \brief 
	class tAppGraphicsSettings
	{
	public:

		tAppGraphicsSettings( );

		f32 fBrightness( ) const { return mBrightness; }
		void fSetBrightness( f32 brightness );

		u32 fResolution( ) const { return mResolution; }
		void fSetResolution( u32 resolution );

		u32 fDisplayWidth( ) const { return mResolutionX; }
		u32 fDisplayHeight( ) const { return mResolutionY; }

		Gfx::tVsync fVsync( ) const { return (Gfx::tVsync)mVsync; }
		void fSetVsync( Gfx::tVsync vsync );

		b32 fIsFullscreen( ) const;
		void fSetFullscreen( b32 fullscreen );

		b32 fTextureDetail( ) const { return mTextureDetail; }
		void fSetTextureDetail( b32 textureDetail );

		u32 fShadowQuality( ) const { return mShadowQuality; }
		void fSetShadowQuality( u32 shadowQuality );

		// these are derived from shadow quality - so read-only
		b32 fShadowsEnabled( ) const;
		u32 fShadowLayers( ) const;
		u32 fShadowResolution( ) const;
		f32 fShadowRange( ) const;

		b32 fShaderQuality( ) const { return mShaderQuality; }
		void fSetShaderQuality( b32 shaderQuality );

		u32 fAntiAliasLevel( ) const { return mAntiAliasLevel; }
		void fSetAntiAliasLevel( u32 antiAliasLevel );

		void fApply( );

		// reset the device to the latest width/height/vsync/fullscreen settings
		void fApplyDisplayMode( );

		static void fExportFuiInterface( );

		// load the settings from disk
		// also captures these settings to the default
		void fLoad( );

		// save these settings to disk
		void fSave();

		// these callbacks used by the serializer
		void fSerializeXml( tXmlDeserializer& s );
		void fSerializeXml( tXmlSerializer& s );


		// register device callbacks for when device params change (ie. Alt Enter)
		void fRegisterDeviceCallbacks();

		void fOnDeviceParamsChanged( const Gfx::tDevicePtr &device );

		static void fExportScriptInterface( tScriptVm& vm );
		void fExportScriptInstance( tScriptVm& vm, const char *name );

		// capture these settings to the default settings object
		u32 fCaptureSettings( );

	private:

		//------------------------------------------------------------------------------
		// Script Helpers
		//------------------------------------------------------------------------------

		// reset the settings the default settings and apply them if necessary
		u32 fSetToDefault( );

		// capture settings if required to keep them, otherwise reset to default
		u32 fTestChanges( );

		// capture settings if required to keep them, otherwise reset to default
		u32 fResolveChanges( b32 keep );

		u32 fGetSettingCount( ) const;
		u32 fGetSettingType( u32 setting ) const;
		u32 fGetSettingOptionCount( u32 setting ) const;
		const tLocalizedString& fGetSettingName( u32 setting ) const;
		const tLocalizedString& fGetSettingDesc( u32 setting ) const;
		const tLocalizedString& fGetSettingOptionName( u32 setting, u32 option ) const;
		const tLocalizedString& fGetSettingOptionDesc( u32 setting, u32 option ) const;

		f32 fGetSettingSlider( u32 setting ) const;
		u32 fSetSettingSlider( u32 setting, f32 slider );

		u32 fGetSettingOption( u32 setting ) const;
		u32 fSetSettingOption( u32 setting, u32 option );

		b32 fGetSettingEnabled( u32 setting ) const;

	private:

		u32 mResolution;		// index into display modes array
		u32 mResolutionX;		// x resolution in pixels
		u32 mResolutionY;		// y resolution in pixels

		u32 mVsync;					  // vsync modes - off, 60fps, 30fps
		u32 mShadowQuality;		// shadow quality - high,low,off
		u32 mAntiAliasLevel;	// high,low,off

		b32 mIsFullscreen;		// fullscreen or windowed
		b32 mShaderQuality;		// ?? per pixel lighting / post effects ??
		b32 mTextureDetail;		// ?? texture mipmapping bias or actually load smaller textures into vram...

		f32 mBrightness;			// 0..1 brightness - not sure if we support this on PC...
		b32 mAllowSave;			// If false, don't save during params changed event

		// enumerate the windows display modes and set the resolution options to match
		// called once on init
		void fEnumerateDisplayModes( );

		// keep a static 'default' version of the settings that we can always reset to
		// these are effectively the last 'saved' version of the settings
		static tAppGraphicsSettings gDefaultSettings;

		// initial settings from the last fCapture - taken when first opening the UI and reverted to on a false Resolve call
		static tAppGraphicsSettings gCapturedSettings;

		// last applied display mode
		void fGatherDisplaySettings( );
		u32 mAppliedDisplayWidth;
		u32 mAppliedDisplayHeight;
		u8 mAppliedDisplayFullscreen;
		u8 mAppliedDisplayVSync;
		u8 mAppliedShadowQuality;
		u8 mAppliedAntiAliasLevel;
	};


} // ::Sig
#endif//platform_pcdx9

#endif//__tAppGraphicsSettings__
