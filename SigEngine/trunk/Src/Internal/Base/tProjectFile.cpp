#include "BasePch.hpp"
#include "tProjectFile.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "FileSystem.hpp"
#include "Win32Util.hpp"
#include <fstream>

namespace Sig
{

	tProjectFile& tProjectFile::fInstance( )
	{
		static tProjectFile sProjectFile;

#if target_tools
		static b32 sInit = false;
		if( !sInit && FileSystem::fFileExists( ToolsPaths::fGetCurrentProjectFilePath( ) ) )
		{
			sInit = true;
			sProjectFile.fLoadXml( ToolsPaths::fGetCurrentProjectFilePath( ) );
		}
#endif

		return sProjectFile;
	}


	///
	/// \section tProjectFile::tAssetGenConfig
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tAssetGenConfig& o )
	{
		s( "UseEnginePlugins", o.mUseEnginePlugins );
		s( "AdditionalPluginPaths", o.mAdditionalPluginPaths );
		s( "LodFirstPassTargetCost", o.mLodFirstPassTargetCost );
		s( "HighLodRatioBias", o.mMeshHighLodRatioBias );
		s( "MediumLodRatioBias", o.mMeshMediumLodRatioBias );
		s( "LowLodRatioBias", o.mMeshLowLodRatioBias );
		s( "RayCastMeshReduce", o.mRayCastMeshReduce );
		s( "MaxMipCount", o.mMaxMipCount );		
	}

	tProjectFile::tAssetGenConfig::tAssetGenConfig( )
		: mUseEnginePlugins( true )
		, mLodFirstPassTargetCost( 0.f )
		, mMeshHighLodRatioBias( 0.f )
		, mMeshMediumLodRatioBias( 0.f )
		, mMeshLowLodRatioBias( 0.f )
		, mRayCastMeshReduce( 1.f )
		, mMaxMipCount( ~0 )
	{
		mAdditionalPluginPaths.fPushBack( tFilePathPtr( "Bin" ) );
	}


	///
	/// \section tProjectFile::tEditorDefaults
	///

	tProjectFile::tEditorDefaults::tEditorDefaults( )
		: mScreenResolution( 1280, 720 )
	{
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tEditorDefaults& o )
	{
		s( "ScreenRes", o.mScreenResolution );
		s( "DefaultLens", o.mDefaultLensProperties );
	}


	///
	/// \section tProjectFile::tCommonShadowParameters
	///

	tProjectFile::tCommonShadowParameters::tCommonShadowParameters( )
		: mEnable( false )
		, mMagnitude( 1.0f )
		, mDebugChannel( "g" )
	{
	}

	tProjectFile::tCommonShadowParameters::tCommonShadowParameters
		( b32 enable, f32 magnitude, const char* debugChannel )
		: mEnable( enable )
		, mMagnitude( magnitude )
		, mDebugChannel( debugChannel )
	{
	}


	///
	/// \section tProjectFile::tNormalShadowParameters
	///

	tProjectFile::tNormalShadowParameters::tNormalShadowParameters( )
		: tCommonShadowParameters( )
		, mAngleStart( +10.0f )
		, mAngleEnd( -10.0f )
	{
	}

	tProjectFile::tNormalShadowParameters::tNormalShadowParameters
		( b32 enable, f32 angleStart, f32 angleEnd, f32 magnitude, const char* debugChannel )
		: tCommonShadowParameters	( enable, magnitude, debugChannel )
		, mAngleStart				( angleStart )
		, mAngleEnd					( angleEnd )
	{
	}

	template< class tSerializer >
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tNormalShadowParameters& o )
	{
		s( "Enable",		o.mEnable );
		s( "AngleStart",	o.mAngleStart );
		s( "AngleEnd",		o.mAngleEnd );
		s( "Magnitude",		o.mMagnitude );
		s( "DebugChannel",	o.mDebugChannel );
	}


	///
	/// \section tProjectFile::tSamplingShadowMapParameters
	///

	tProjectFile::tSamplingShadowMapParameters::tSamplingShadowMapParameters( )
		: tCommonShadowParameters( )
	{
	}

	tProjectFile::tSamplingShadowMapParameters::tSamplingShadowMapParameters
		( b32 enable, f32 magnitude, const char* debugChannel )
		: tCommonShadowParameters( enable, magnitude, debugChannel )
	{
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tSamplingShadowMapParameters& o )
	{
		s( "Enable",		o.mEnable );
		s( "Magnitude",		o.mMagnitude );
		s( "DebugChannel",	o.mDebugChannel );
	}


	///
	/// \section tProjectFile::tShadowRuntimeSettings
	///

	tProjectFile::tShadowRuntimeSettings::tShadowRuntimeSettings( )
		: mHighResShadowDist					( 20 )
		, mLowResShadowDist						( 500 )
		, mShadowAmount							( 0.5f )
	{
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tShadowRuntimeSettings& o )
	{
		s( "HighResShadowDist",	o.mHighResShadowDist );
		s( "LowResShadowDist",	o.mLowResShadowDist );
		s( "ShadowAmount",		o.mShadowAmount );
	}


	///
	/// \section tProjectFile::tShadowShaderGenerationSettings
	///

	tProjectFile::tShadowShaderGenerationSettings::tShadowShaderGenerationSettings( )
		: mShadowMapLayerCount					( 1 )
		, mShadowBack							( true )

		, mNormalShadowValue					( true , +20, +10, 1.0f, "r" )
		, mNormalShadowBlurSample				( false, +20, +10, 1.0f, NULL )
		, mNormalShadowMoveSample				( false, +20, +10, 1.0f, NULL )
		
		, mShadowMapNaieveSingleRangeCheck		( false, 1.0f, NULL )
		, mShadowMapNaieveBoundingRangesCheck	( false, 1.0f, NULL )
		, mShadowMapPercentageCloserFiltering	( true , 1.0f, NULL )
		, mShadowMapEstimatedNormal				( false, 1.0f, NULL )
	{
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tShadowShaderGenerationSettings& o )
	{
		s( "ShadowMapLayerCount",					o.mShadowMapLayerCount );
		s( "ShadowBack",							o.mShadowBack );

		s( "NormalShadowValue",						o.mNormalShadowValue );
		s( "NormalShadowBlurSample",				o.mNormalShadowBlurSample );
		s( "NormalShadowMoveSample",				o.mNormalShadowMoveSample );

		s( "ShadowMapNaieveSingleRangeCheck",		o.mShadowMapNaieveSingleRangeCheck );
		s( "ShadowMapNaieveBoundingRangesCheck",	o.mShadowMapNaieveBoundingRangesCheck );
		s( "ShadowMapPercentageCloserFiltering",	o.mShadowMapPercentageCloserFiltering );
		s( "ShadowMapEstimatedNormal",				o.mShadowMapEstimatedNormal );
	}


	///
	/// \section tProjectFile::tRendererSettings
	///

	tProjectFile::tRendererSettings::tRendererSettings( )
		: mEnableLOD( false )
		, mBuildLOD( false )
		, mPointLightIntensityMultiplier( 1.0f )
		, mPointLightSizeMultiplier( 1.0f )
	{
		for( u32 i = 0; i < mRenderableFarFadeSettings.cDimension; ++i )
			mRenderableFarFadeSettings[ i ] = Gfx::tRenderableEntity::fGetGlobalFarFadeSetting( i );

		for( u32 i = 0; i < mRenderableNearFadeSettings.cDimension; ++i )
			mRenderableNearFadeSettings[ i ] = Gfx::tRenderableEntity::fGetGlobalNearFadeSetting( i );

		// temporary testing, move this to fSetDefaults
		mLenseFlares.fPushBack( tLenseFlare( ) );
		mLenseFlares.fBack( ).mName = "Test";
		mLenseFlares.fBack( ).mKey = 0;
		mLenseFlares.fBack( ).mData.mFlares.fPushBack( Gfx::tLenseFlareData::tFlare( ) );
	}

	tProjectFile::tLenseFlare* tProjectFile::tRendererSettings::fAddFlare( const std::string& name )
	{
		tLenseFlare flare;
		flare.mName = name;
		flare.mKey = fNextKey( mLenseFlares );
		mLenseFlares.fPushBack( flare );
		return &mLenseFlares.fBack( );
	}

	// just not tested/needed yet.
	//void tProjectFile::tRendererSettings::fEraseFlare( u32 key )
	//{
	//	u32 index = fFindItemIndexByKey( key, mLenseFlares );
	//	if( index != ~0 )
	//		mLenseFlares.fEraseOrdered( index );
	//}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tRendererSettings& o )
	{
		s( "EnableLOD",						o.mEnableLOD );
		s( "BuildLOD",						o.mBuildLOD );
		s( "PointLightIntensityMultiplier",	o.mPointLightIntensityMultiplier );
		s( "PointLightSizeMultiplier",		o.mPointLightSizeMultiplier );
		s( "ShadowRuntime",					o.mShadowRuntimeSettings );
		s( "ShadowShaderGeneration",		o.mShadowShaderGenerationSettings );
		s( "FarFadeSettings",				o.mRenderableFarFadeSettings );
		s( "NearFadeSettings",				o.mRenderableNearFadeSettings );
		s( "LenseFlares",					o.mLenseFlares );
	}


	///
	/// \section tProjectFile::tLenseFlare
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tLenseFlare& o )
	{
		s( "Key", o.mKey );
		s( "Name", o.mName );
		s( "Data", o.mData );
	}

	///
	/// \section tProjectFile::tEngineConfig
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tEngineConfig& o )
	{
		s( "Renderer", o.mRendererSettings );
		s( "Editor", o.mEditorDefaults );
		s( "AssetGen", o.mAssetGenConfig );
	}


	///
	/// \section tProjectFile::tGameTag
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameTag& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
	}


	///
	/// \section tProjectFile::tEvent
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tEvent& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
	}

	///
	/// \section tProjectFile::tGameEnumeratedValue
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameEnumeratedValue& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
	}

	///
	/// \section tProjectFile::tGameEnumeratedTypeAlias
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameEnumeratedTypeAlias& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
		s( "Hide", o.mHide );
	}

	///
	/// \section tProjectFile::tGameEnumeratedType
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile::tGameEnumeratedType& o )
	{
		s( "Name", o.mName );
		s( "Key", o.mKey );
		s( "Hide", o.mHide );
		s( "Aliases", o.mAliases );
		s( "Values", o.mValues );
	}

	u32 tProjectFile::tGameEnumeratedType::fFindValueIndexByKey( u32 key ) const
	{
		for( u32 i = 0; i < mValues.fCount( ); ++i )
			if( mValues[ i ].mKey == key )
				return i;

		return ~0;
	}

	u32 tProjectFile::tGameEnumeratedType::fFindValueIndexByName( const std::string& name ) const
	{
		for( u32 i = 0; i < mValues.fCount( ); ++i )
			if( _stricmp( mValues[ i ].mName.c_str( ), name.c_str( ) ) == 0 )
				return i;
		return ~0;
	}

	void tProjectFile::tGameEnumeratedType::fInsertValue( const std::string& name, u32 index )
	{
		mValues.fInsertSafe( index, tGameEnumeratedValue( name, fNextKey( mValues ) ) );
	}

	///
	/// \section tProjectFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tProjectFile& o )
	{
		s( "EngineConfig", o.mEngineConfig );
		s( "GameFlags", o.mGameTags );
		s( "GameEnumeratedValues", o.mGameEnumeratedTypes );
		s( "GameEvents", o.mGameEvents );
		s( "KeyframeEvents", o.mKeyFrameEvents );
		s( "AIFlags", o.mAIFlags );
	}

	b32 tProjectFile::fLoadXml( const char* data )
	{
		tXmlDeserializer des;
		b32 result = des.fParse( data, "SigProj", *this );
		return result;
	}

	const tProjectFile::tGameEnumeratedType* tProjectFile::fFindEnumeratedTypeByName( const std::string& enumName, u32* keyOut ) const
	{
		for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
		{
			if( mGameEnumeratedTypes[ i ].mName == enumName )
			{
				if( keyOut )
					*keyOut = mGameEnumeratedTypes[ i ].mKey;
				return &mGameEnumeratedTypes[ i ];
			}
			else
			{
				const tGameEnumeratedTypeAlias* alias = mGameEnumeratedTypes[ i ].mAliases.fFind( enumName );
				if( alias )
				{
					if( keyOut )
						*keyOut = alias->mKey;
					return &mGameEnumeratedTypes[ i ];
				}
			}
		}
		return 0;
	}
	const tProjectFile::tGameEnumeratedType* tProjectFile::fFindEnumeratedTypeByKey( u32 key ) const
	{
		for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
		{
			if( mGameEnumeratedTypes[ i ].mKey == key )
				return &mGameEnumeratedTypes[ i ];

			for( u32 a = 0; a < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++a )
				if( mGameEnumeratedTypes[ i ].mAliases[ a ].mKey == key )
					return &mGameEnumeratedTypes[ i ];
		}

		return NULL;
	}

/*
	Start tools only code section
*/
#ifdef target_tools

	std::string tProjectFile::tEvent::fCppName( const std::string& name )
	{
		return "GAME_EVENT_" + fToCppName( name );
	}

	namespace tAIFlag
	{
		std::string fCppName( const std::string& name )
		{
			return "AIFLAG_" + tProjectFile::fToCppName( name );
		}
	}

	std::string tProjectFile::fToCppName( const std::string& in )
	{
		std::string out = in;
		for( u32 i = 0; i < out.length( ); ++i )
		{
			if( isalpha( out[ i ] ) )	out[ i ] = toupper( out[ i ] );
			else if( !isdigit( out[ i ] ) ) out[ i ] = '_';
		}
		return out;
	}

	namespace
	{
		template< class tArray >
		void fValidateUniqueKeys( const char* prefix, const tArray& ar, tGrowableArray<u32>& keysOut, std::stringstream& errorsOut )
		{
			for( u32 i = 0; i < ar.fCount( ); ++i )
			{
				if( keysOut.fFind( ar[ i ].mKey ) )
				{
					errorsOut << prefix << " key " << ar[ i ].mKey << " used more than once. ( " + ar[ i ].mName + " )" << std::endl;
				}
				else 
					keysOut.fPushBack( ar[ i ].mKey );
			}
		}
	}

	void tProjectFile::fValidateKeys( )
	{
		//validate keys are unique
		std::stringstream errors;
		tGrowableArray<u32> enumKeys;

		fValidateUniqueKeys( "Enum", mGameEnumeratedTypes, enumKeys, errors );

		for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
		{
			tGameEnumeratedType& e = mGameEnumeratedTypes[ i ];

			std::string errorPrefix = "EnumAlias ";
			errorPrefix += e.mName;
			fValidateUniqueKeys( errorPrefix.c_str( ), e.mAliases, enumKeys, errors );

			errorPrefix = "EnumValue ";
			errorPrefix += e.mName;
			fValidateUniqueKeys( errorPrefix.c_str( ), e.mValues, tGrowableArray<u32>( ), errors );
		}

		fValidateUniqueKeys( "Tag", mGameTags, tGrowableArray<u32>( ), errors );
		fValidateUniqueKeys( "GameEvent", mGameEvents, tGrowableArray<u32>( ), errors );
		fValidateUniqueKeys( "KeyFrameEvent", mKeyFrameEvents, tGrowableArray<u32>( ), errors );
		fValidateUniqueKeys( "AIFlag", mAIFlags, tGrowableArray<u32>( ), errors );

		log_assert( errors.str( ).length( ) == 0, "Project XML file has a key conflict." << std::endl << errors.str( ) );
	}

	void tProjectFile::fSetupProjectDefaults( )
	{
		// these are just some very basic examples that make it easier to start editing the file

		mGameTags.fPushBack( tGameTag( "Edible", 0 ) );
		mGameTags.fPushBack( tGameTag( "Visible", 1 ) );
		mGameTags.fPushBack( tGameTag( "Collidable", 2 ) );

		tGameEnumeratedType damageType;
		damageType.mName = "Damage Type";
		damageType.mKey = 0;
		damageType.mValues.fPushBack( tGameEnumeratedValue( "Bludgeoning", 0 ) );
		damageType.mValues.fPushBack( tGameEnumeratedValue( "Slashing", 1 ) );
		damageType.mValues.fPushBack( tGameEnumeratedValue( "Piercing", 2 ) );

		tGameEnumeratedType iceCreamFlavor;
		iceCreamFlavor.mName = "Ice Cream Flavor";
		damageType.mKey = 1;
		iceCreamFlavor.mValues.fPushBack( tGameEnumeratedValue( "Vanilla", 0 ) );
		iceCreamFlavor.mValues.fPushBack( tGameEnumeratedValue( "Chocolate", 1 ) );
		iceCreamFlavor.mValues.fPushBack( tGameEnumeratedValue( "Pralines 'N Cream", 2 ) );

		mGameEnumeratedTypes.fPushBack( damageType );
		mGameEnumeratedTypes.fPushBack( iceCreamFlavor );

		mGameEvents.fPushBack( tEvent( "Invalid", 0 ) );
		mGameEvents.fPushBack( tEvent( "Fall", 1 ) );
		mGameEvents.fPushBack( tEvent( "Land", 2 ) );
	}

	void tProjectFile::fCompileGameSettings( b32 autoBuildRes ) const
	{
		fWriteGameSettingsHpp( );
		fWriteGameSettingsCpp( );

		if( autoBuildRes )
			system( "BuildRes.cmd" );
	}

	b32 tProjectFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		return ser.fSave( path, "SigProj", *this, promptToCheckout );
	}

	b32 tProjectFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		b32 result = des.fLoad( path, "SigProj", *this );

		if( !result ) 
			return false;


		fValidateKeys( );
		return true;
	}

	void tProjectFile::fWriteGameSettingsHpp( ) const
	{
		const tFilePathPtr filePath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectSrcFolder( ), tFilePathPtr( "GameFlags.hpp" ) );

		if( Win32Util::fIsFileReadOnly( filePath ) )
			ToolsPaths::fCheckout( filePath );

		std::ofstream file( filePath.fCStr( ) );

		file << "#ifndef __GameFlags__" << std::endl;
		file << "#define __GameFlags__" << std::endl;
		file << std::endl;

		file << "// Auto-generated file containing the project's list of game flags and game enumerations." << std::endl;
		file << "// These values are exposed to the editor, script, and code." << std::endl;
		file << "// DO NOT EDIT THIS FILE MANUALLY" << std::endl;
		file << std::endl;
		file << "namespace Sig { namespace GameFlags" << std::endl;
		file << "{" << std::endl;

		// Settings
		{
			file << std::endl;
			file << "\t" << "// Settings..." << std::endl;
			file << std::endl;

			// Renderable fade settings
			file << "\t" << "static const f32 cSETTING_RENDERABLE_FARFADE_NEAR = " << mEngineConfig.mRendererSettings.mRenderableFarFadeSettings[ Gfx::tRenderableEntity::cFadeNear ] << ";" << std::endl;
			file << "\t" << "static const f32 cSETTING_RENDERABLE_FARFADE_MEDIUM = " << mEngineConfig.mRendererSettings.mRenderableFarFadeSettings[ Gfx::tRenderableEntity::cFadeMedium ] << ";" << std::endl;
			file << "\t" << "static const f32 cSETTING_RENDERABLE_FARFADE_FAR = " << mEngineConfig.mRendererSettings.mRenderableFarFadeSettings[ Gfx::tRenderableEntity::cFadeFar ] << ";" << std::endl;

			file << "\t" << "static const f32 cSETTING_RENDERABLE_NEARFADE_NEAR = " << mEngineConfig.mRendererSettings.mRenderableNearFadeSettings[ Gfx::tRenderableEntity::cFadeNear ] << ";" << std::endl;
			file << "\t" << "static const f32 cSETTING_RENDERABLE_NEARFADE_MEDIUM = " << mEngineConfig.mRendererSettings.mRenderableNearFadeSettings[ Gfx::tRenderableEntity::cFadeMedium ] << ";" << std::endl;
			file << "\t" << "static const f32 cSETTING_RENDERABLE_NEARFADE_FAR = " << mEngineConfig.mRendererSettings.mRenderableNearFadeSettings[ Gfx::tRenderableEntity::cFadeFar ] << ";" << std::endl;
		}

		// Flags
		{
			file << std::endl;
			file << "\t" << "// Flags..." << std::endl;
			file << std::endl;

			for( u32 i = 0; i < mGameTags.fCount( ); ++i )
				file << "\t" << "static const u32 cFLAG_" << fToCppName( mGameTags[ i ].mName ) << " = ( 1u << " << i << "u );" << std::endl;
			file << "\t" << "static const u32 cFLAG_COUNT = " << mGameTags.fCount( ) << ";" << std::endl;
		}

		// Game Events
		{
			file << std::endl;
			file << "\t" << "// Game Events..." << std::endl;
			file << std::endl;

			for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
				file << "\t" << "static const u32 cEVENT_" << fToCppName( mGameEvents[ i ].mName ) << " = " << i << "u;" << std::endl;
			file << "\t" << "static const u32 cEVENT_COUNT = " << mGameEvents.fCount( ) << ";" << std::endl;

			file << std::endl;
			file << "\t" << "const tStringPtr& fGameEventToString( u32 eventValue );			// returns \"GAME_EVENT_VALUE_NAME\"" << std::endl;  //extra tab required when built
			file << "\t" << "u32 fGameEventToValue( const tStringPtr& name );				// takes input like GAME_EVENT_VALUE_NAME" << std::endl; 
			file << "\t" << "u32 fGameEventValueStringToEnum( const tStringPtr& name );		// takes input like VALUE_NAME" << std::endl; 
		}

		// KeyFrame Events
		{
			file << std::endl;
			file << "\t" << "// KeyFrame Events..." << std::endl;
			file << std::endl;

			for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
				file << "\t" << "static const u32 cKEYFRAME_EVENT_" << fToCppName( mKeyFrameEvents[ i ].mName ) << " = " << i << "u;" << std::endl;
			file << "\t" << "static const u32 cKEYFRAME_EVENT_COUNT = " << mKeyFrameEvents.fCount( ) << ";" << std::endl;
		}

		// AIFlags
		{
			file << std::endl;
			file << "\t" << "// AI Flags..." << std::endl;
			file << std::endl;

			for( u32 i = 0; i < mAIFlags.fCount( ); ++i )
				file << "\t" << "static const u32 cAIFLAG_" << fToCppName( mAIFlags[ i ].mName ) << " = " << i << "u;" << std::endl;
			file << "\t" << "static const u32 cAIFLAG_COUNT = " << mAIFlags.fCount( ) << ";" << std::endl;

			file << "\t" << "const tStringPtr& fAIFlagToString( u32 eventValue );      // returns \"AIFLAG_VALUE_NAME\"" << std::endl; 
			file << "\t" << "const tStringPtr& fAIFlagToValueString( u32 eventValue );      // returns \"VALUE_NAME\"" << std::endl; 
			file << "\t" << "u32 fAIFlagToValue( const tStringPtr& name );  " << std::endl; 
		}

		// Enums
		{
			file << std::endl;
			file << "\t" << "// Enumerated Types..." << std::endl;
			file << std::endl;

			u32 numEnumTypes = mGameEnumeratedTypes.fCount( );
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				file << "\t" << "static const u32 cENUM_" << fToCppName( mGameEnumeratedTypes[ i ].mName ) << " = " << mGameEnumeratedTypes[ i ].mKey << "u;" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++j )
					file << "\t" << "static const u32 cENUM_" << fToCppName( mGameEnumeratedTypes[ i ].mAliases[ j ].mName ) << " = " << mGameEnumeratedTypes[ i ].mAliases[ j ].mKey << "u;" << std::endl;
				numEnumTypes += mGameEnumeratedTypes[ i ].mAliases.fCount( );
			}
			file << "\t" << "static const u32 cENUM_COUNT = " << numEnumTypes << ";" << std::endl;

			file << std::endl;
			file << "\t" << "// Enumerated Type Values..." << std::endl;

			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				// write the enum
				const std::string cppEnumName = fToCppName( mGameEnumeratedTypes[ i ].mName );
				file << std::endl;
				file << "\t" << "enum t" << cppEnumName << std::endl;
				file << "\t" << "{" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
					file << "\t\t" << "c" << cppEnumName << "_" << fToCppName( mGameEnumeratedTypes[ i ].mValues[ j ].mName ) << " = " << j << "u," << std::endl;
				file << "\t\t" << "c" << cppEnumName << "_COUNT = " << mGameEnumeratedTypes[ i ].mValues.fCount( ) << "," << std::endl;
				file << "\t" << "};" << std::endl;

				// declare conversion funcs
				file << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToString( u32 enumValue );      // returns \"" << cppEnumName << "_VALUE_NAME\"" << std::endl;  
				file << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToValueString( u32 enumValue ); // returns \"VALUE_NAME\"" << std::endl;
				file << "\t" << "u32 f" << cppEnumName << "StringToEnum( const tStringPtr& enumString );" << std::endl;
				file << "\t" << "u32 f" << cppEnumName << "StringToRequiredEnum( const tStringPtr& enumString );" << std::endl;
				file << "\t" << "u32 f" << cppEnumName << "ValueStringToEnum( const tStringPtr& enumValueString );" << std::endl;
				file << "\t" << "u32 f" << cppEnumName << "ValueStringToRequiredEnum( const tStringPtr& enumValueString );" << std::endl;
			}
		}

		file << std::endl;
		file << "}}" << std::endl;
		file << std::endl;

		file << std::endl;

		file << "namespace Sig { namespace GameFlags { namespace Detail" << std::endl;
		file << "{" << std::endl;
		file << std::endl;
		file << "\textern const tStringPtr      cGameFlagNames[];" << std::endl;
		file << "\textern const u32             cGameFlagValues[];" << std::endl;
		file << "\textern const tStringPtr      cGameEnumTypeNames[];" << std::endl;
		file << "\textern const u32             cGameEnumTypeKeys[];" << std::endl;
		file << "\textern const tStringPtr*const cGameEnumValueNames[];" << std::endl;
		file << "\textern const u32*const       cGameEnumValues[];" << std::endl;
		file << "\textern const tStringPtr      cGameEventNames[];" << std::endl;
		file << "\textern const u32             cGameEventValues[];" << std::endl;
		file << "\textern const tStringPtr      cKeyFrameEventNames[];" << std::endl;
		file << "\textern const u32             cKeyFrameEventValues[];" << std::endl;
		file << std::endl;
		file << "}}}" << std::endl;
		file << std::endl;

		file << "#endif//__GameFlags__" << std::endl;
		file << std::endl;
	}
	void tProjectFile::fWriteGameSettingsCpp( ) const
	{
		const tFilePathPtr cppPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectSrcFolder( ), tFilePathPtr( "GameFlags.cpp" ) );
		const tFilePathPtr nutPath = tFilePathPtr::fConstructPath( ToolsPaths::fGetCurrentProjectResFolder( ), tFilePathPtr( "GameFlags.nut" ) );

		if( Win32Util::fIsFileReadOnly( cppPath ) )
			ToolsPaths::fCheckout( cppPath );
		if( Win32Util::fIsFileReadOnly( nutPath ) )
			ToolsPaths::fCheckout( nutPath );

		std::ofstream fcpp( cppPath.fCStr( ) );
		std::ofstream fnut( nutPath.fCStr( ) );

		fcpp << std::endl;
		fcpp << "#include \"GameAppPch.hpp\"" << std::endl;
		fcpp << "#include \"gameflags.hpp\"" << std::endl;
		fcpp << std::endl;
		fcpp << "// Auto-generated file containing the project's list of game flags and game enumerations." << std::endl;
		fcpp << "// These values are exposed to the editor, script, and code." << std::endl;
		fcpp << "// DO NOT EDIT THIS FILE MANUALLY" << std::endl;
		fcpp << std::endl;
		fcpp << "namespace Sig { namespace GameFlags { namespace Detail" << std::endl;
		fcpp << "{" << std::endl;

		fnut << "// Auto-generated file containing the project's list of game flags and game enumerations." << std::endl;
		fnut << "// These values are exposed to the editor, script, and code." << std::endl;
		fnut << "// DO NOT EDIT THIS FILE MANUALLY" << std::endl;
		fnut << std::endl;

		// Flags
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cGameFlagNames[]={" << std::endl;
			for( u32 i = 0; i < mGameTags.fCount( ); ++i )
			{
				const std::string name = "FLAG_" + fToCppName( mGameTags[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
				fnut << "const " << name << " = " << ( 1u << i ) << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst u32 cGameFlagValues[]={" << std::endl;
			for( u32 i = 0; i < mGameTags.fCount( ); ++i )
				fcpp << "\t\t" << "( 1u << " << i << "u )," << std::endl;
			fcpp << "\t\t" << "0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
		}

		// Game Events
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cGameEventNames[]={" << std::endl;
			for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
			{
				const std::string name = tEvent::fCppName( mGameEvents[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
				fnut << "const " << name << " = " << i << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;

			fcpp << "\tconst tStringPtr cGameEventValueNames[]={" << std::endl;
			for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
			{
				const std::string name = fToCppName( mGameEvents[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
			}			
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst u32 cGameEventValues[]={" << std::endl;
			for( u32 i = 0; i < mGameEvents.fCount( ); ++i )
				fcpp << "\t\t" << "( " << i << "u )," << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
		}

		// AI Flags
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cAIFlagNames[]={" << std::endl;
			for( u32 i = 0; i < mAIFlags.fCount( ); ++i )
			{
				const std::string name = tAIFlag::fCppName( mAIFlags[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
				fnut << "const " << name << " = " << i << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cAIFlagValueNames[]={" << std::endl;
			for( u32 i = 0; i < mAIFlags.fCount( ); ++i )
			{
				const std::string name = fToCppName( mAIFlags[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst u32 cAIFlagValues[]={" << std::endl;
			for( u32 i = 0; i < mAIFlags.fCount( ); ++i )
				fcpp << "\t\t" << "( " << i << "u )," << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
		}

		// KeyFrame Events
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cKeyFrameEventNames[]={" << std::endl;
			for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
			{
				const std::string name = "KEYFRAME_EVENT_" + fToCppName( mKeyFrameEvents[ i ].mName );
				fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
				fnut << "const " << name << " = " << i << std::endl;
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			fcpp << std::endl;
			fcpp << "\tconst u32 cKeyFrameEventValues[]={" << std::endl;
			for( u32 i = 0; i < mKeyFrameEvents.fCount( ); ++i )
				fcpp << "\t\t" << "( " << i << "u )," << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
		}

		// Enums
		{
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr cGameEnumTypeNames[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				{
					const std::string name = "ENUM_" + fToCppName( mGameEnumeratedTypes[ i ].mName );
					fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
					fnut << "const " << name << " = " << mGameEnumeratedTypes[ i ].mKey << std::endl;
				}
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++j )
				{
					const std::string name = "ENUM_" + fToCppName( mGameEnumeratedTypes[ i ].mAliases[ j ].mName );
					fcpp << "\t\t" << "tStringPtr( \"" << name << "\" )," << std::endl;
					fnut << "const " << name << " = " << mGameEnumeratedTypes[ i ].mAliases[ j ].mKey << std::endl;
				}
			}
			fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fnut << std::endl;

			u32 numEnumTypes = mGameEnumeratedTypes.fCount( );
			fcpp << std::endl;
			fcpp << "\tconst u32 cGameEnumTypeKeys[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				fcpp << "\t\t" << mGameEnumeratedTypes[ i ].mKey << "," << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mAliases.fCount( ); ++j )
					fcpp << "\t\t" << j << "," << std::endl;
				numEnumTypes += mGameEnumeratedTypes[ i ].mAliases.fCount( );
			}
			fcpp << "\t\t" << "~0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;



			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				fcpp << std::endl;
				fcpp << "\tstatic const tStringPtr cGameEnumValueNames" << i << "[]={" << std::endl;
				const std::string cppEnumName = fToCppName( mGameEnumeratedTypes[ i ].mName );
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
				{
					const std::string name = cppEnumName + "_" + fToCppName( mGameEnumeratedTypes[ i ].mValues[ j ].mName );
					fcpp << "\t\ttStringPtr( \"" << name << "\" )," << std::endl;
					fnut << "const " << name << " = " << j << std::endl;
				}
				fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
				fcpp << "\t};" << std::endl;
				fcpp << std::endl;

				fnut << "const " << cppEnumName + "_COUNT = " << mGameEnumeratedTypes[ i ].mValues.fCount( ) << std::endl;
				fnut << std::endl;

				fcpp << "\tstatic const tStringPtr cGameEnumValueNamesRaw" << i << "[]={" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
					fcpp << "\t\ttStringPtr( \"" << fToCppName( mGameEnumeratedTypes[ i ].mValues[ j ].mName ) << "\" )," << std::endl;
				fcpp << "\t\t" << "tStringPtr::cNullPtr" << std::endl;
				fcpp << "\t};" << std::endl;
			}

			fcpp << std::endl;
			fcpp << "\tconst tStringPtr*const cGameEnumValueNames[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
				fcpp << "\t\tcGameEnumValueNames" << i << "," << std::endl;
			fcpp << "\t\t0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;
			fcpp << "\tconst tStringPtr*const cGameEnumValueNamesRaw[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
				fcpp << "\t\tcGameEnumValueNamesRaw" << i << "," << std::endl;
			fcpp << "\t\t0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;

			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				fcpp << std::endl;
				fcpp << "\tstatic const u32 cGameEnumValues" << i << "[]={" << std::endl;
				for( u32 j = 0; j < mGameEnumeratedTypes[ i ].mValues.fCount( ); ++j )
					fcpp << "\t\t" << j << "u," << std::endl;
				fcpp << "\t\t" << "~0" << std::endl;
				fcpp << "\t};" << std::endl;
			}
			fcpp << std::endl;
			fcpp << "\tconst u32*const cGameEnumValues[]={" << std::endl;
			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
				fcpp << "\t\tcGameEnumValues" << i << "," << std::endl;
			fcpp << "\t\t0" << std::endl;
			fcpp << "\t};" << std::endl;
			fcpp << std::endl;

			fcpp << std::endl;
			fcpp << "}}}" << std::endl;
			fcpp << std::endl;

			fcpp << std::endl;
			fcpp << "namespace Sig { namespace GameFlags" << std::endl;
			fcpp << "{" << std::endl;

			// define conversion funcs
			fcpp << "\tconst tStringPtr& fGameEventToString( u32 eventValue )"  << std::endl; 
			fcpp << "\t{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEventValues ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cGameEventValues[ i ] == eventValue )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cGameEventNames[ i ];" << std::endl;
			fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
			fcpp << "\t}" << std::endl;

			fcpp << "\t" << "u32 fGameEventToValue( const tStringPtr& name )" << std::endl;
			fcpp << "\t" << "{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEventNames ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cGameEventNames[ i ] == name )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cGameEventValues[ i ];" << std::endl;
			fcpp << "\t\t" << "return ~0;" << std::endl;
			fcpp << "\t" << "}" << std::endl;

			fcpp << "\t" << "u32 fGameEventValueStringToEnum( const tStringPtr& name )" << std::endl;
			fcpp << "\t" << "{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEventValueNames ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cGameEventValueNames[ i ] == name )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cGameEventValues[ i ];" << std::endl;
			fcpp << "\t\t" << "return ~0;" << std::endl;
			fcpp << "\t" << "}" << std::endl;			

			// define conversion funcs
			fcpp << "\tconst tStringPtr& fAIFlagToString( u32 eventValue )"  << std::endl; 
			fcpp << "\t{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cAIFlagValues ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cAIFlagValues[ i ] == eventValue )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cAIFlagNames[ i ];" << std::endl;
			fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
			fcpp << "\t}" << std::endl;

			fcpp << "\tconst tStringPtr& fAIFlagToValueString( u32 eventValue )"  << std::endl; 
			fcpp << "\t{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cAIFlagValueNames ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cAIFlagValues[ i ] == eventValue )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cAIFlagValueNames[ i ];" << std::endl;
			fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
			fcpp << "\t}" << std::endl;

			fcpp << "\t" << "u32 fAIFlagToValue( const tStringPtr& name )" << std::endl;
			fcpp << "\t" << "{" << std::endl;
			fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cAIFlagNames ); ++i )" << std::endl;
			fcpp << "\t\t\t" << "if( Detail::cAIFlagNames[ i ] == name )" << std::endl;
			fcpp << "\t\t\t\t" << "return Detail::cAIFlagValues[ i ];" << std::endl;
			fcpp << "\t\t" << "return ~0;" << std::endl;
			fcpp << "\t" << "}" << std::endl;

			for( u32 i = 0; i < mGameEnumeratedTypes.fCount( ); ++i )
			{
				const std::string cppEnumName = fToCppName( mGameEnumeratedTypes[ i ].mName );

				fcpp << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToString( u32 enumValue )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "if( enumValue < array_length( Detail::cGameEnumValueNames" << i << " ) )" << std::endl;
				fcpp << "\t\t\t" << "return Detail::cGameEnumValueNames" << i << "[ enumValue ];" << std::endl;
				fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "const tStringPtr& f" << cppEnumName << "EnumToValueString( u32 enumValue )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "if( enumValue < array_length( Detail::cGameEnumValueNamesRaw" << i << " ) )" << std::endl;
				fcpp << "\t\t\t" << "return Detail::cGameEnumValueNamesRaw" << i << "[ enumValue ];" << std::endl;
				fcpp << "\t\t" << "return tStringPtr::cNullPtr;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "u32 f" << cppEnumName << "StringToEnum( const tStringPtr& enumString )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEnumValueNames" << i << " ); ++i )" << std::endl;
				fcpp << "\t\t" << "{" << std::endl;
				fcpp << "\t\t\t" << "if( Detail::cGameEnumValueNames" << i << "[ i ] == enumString )" << std::endl;
				fcpp << "\t\t\t\t" << "return i;" << std::endl;
				fcpp << "\t\t" << "}" << std::endl;
				fcpp << "\t\t" << "return ~0;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "u32 f" << cppEnumName << "StringToRequiredEnum( const tStringPtr& enumString )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "const u32 value = f" << cppEnumName << "StringToEnum( enumString );" << std::endl;
				fcpp << "\t\t" << "log_assert( value != ~0, \"Could not convert enum string '\" << enumString << \"' to " << "t" << cppEnumName << "\" );" << std::endl;
				fcpp << "\t\t" << "return value;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "u32 f" << cppEnumName << "ValueStringToEnum( const tStringPtr& enumValueString )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "for( u32 i = 0; i < array_length( Detail::cGameEnumValueNamesRaw" << i << " ); ++i )" << std::endl;
				fcpp << "\t\t" << "{" << std::endl;
				fcpp << "\t\t\t" << "if( Detail::cGameEnumValueNamesRaw" << i << "[ i ] == enumValueString )" << std::endl;
				fcpp << "\t\t\t\t" << "return i;" << std::endl;
				fcpp << "\t\t" << "}" << std::endl;
				fcpp << "\t\t" << "return ~0;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

				fcpp << "\t" << "u32 f" << cppEnumName << "ValueStringToRequiredEnum( const tStringPtr& enumValueString )" << std::endl;
				fcpp << "\t" << "{" << std::endl;
				fcpp << "\t\t" << "const u32 value = f" << cppEnumName << "ValueStringToEnum( enumValueString );" << std::endl;
				fcpp << "\t\t" << "log_assert( value != ~0, \"Could not convert enum value string '\" << enumValueString << \"' to " << "t" << cppEnumName << "\" );" << std::endl;
				fcpp << "\t\t" << "return value;" << std::endl;
				fcpp << "\t" << "}" << std::endl;

			}
			fcpp << std::endl;
		}

		fcpp << "}}" << std::endl;
		fcpp << std::endl;
	}
#endif
	
}
