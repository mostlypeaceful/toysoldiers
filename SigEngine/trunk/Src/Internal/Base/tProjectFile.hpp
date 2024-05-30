#ifndef __tProjectFile__
#define __tProjectFile__

#include "Gfx/tCameraEntity.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "Gfx/tLenseFlareData.hpp"

namespace Sig
{

	///
	/// \brief Stores project specific settings for all aspects of a game project
	/// using the Sig engine, including paths, optional plugins, etc.
	class base_export tProjectFile
	{
	public:
		struct base_export tAssetGenConfig
		{
			tAssetGenConfig( );

			b32					mUseEnginePlugins;
			tFilePathPtrList	mAdditionalPluginPaths;

			f32					mLodFirstPassTargetCost;
			f32					mMeshHighLodRatioBias;
			f32					mMeshMediumLodRatioBias;
			f32					mMeshLowLodRatioBias;

			f32					mRayCastMeshReduce;

			u32					mMaxMipCount; // maximum number of mips to generate for a texture. TODO, there may be other metrics to limit, such as lowest mip size.
		};

		// Default camera stuff, for previewing in the editor.
		struct base_export tEditorDefaults
		{
			tEditorDefaults( );

			Math::tVec2u	mScreenResolution;
			tLensProperties mDefaultLensProperties;
		};

		struct base_export tLenseFlare
		{
			u32						mKey;	// used to uniquely identify the flare through the engine, regardless of human readable name.
			std::string				mName;	// only used for human readability
			Gfx::tLenseFlareData	mData;
		};

		struct base_export tCommonShadowParameters
		{
			tCommonShadowParameters( );
			tCommonShadowParameters( b32 enable, f32 magnitude, const char* debugChannel );

			b32			mEnable;		///<			If this sampling technique should be used.
			f32			mMagnitude;		///<			How much this shadow map sampling interpretation should contribute to shadows.
			tStringPtr	mDebugChannel;	///< {R,G,B}	For debug shaders, what color channel should we be writing.
		};

		/// \brief Parameters for figuring out how much the surface normal vs light normal angle should contribute.
		///   "Edge" effects will start applying at mAngleStart, reach a maximum at their midpoint, and then stop
		/// applying at mAngleEnd.  For example, Bluring/Moving the shadow map sampling points.
		///   "Fade In" effects will start applying at mAngleStart, and then reach a maximum at mAngleEnd, staying
		/// applied for the entire range.  For example, direct value contributions.
		struct base_export tNormalShadowParameters : tCommonShadowParameters
		{
			tNormalShadowParameters( );
			tNormalShadowParameters( b32 enable, f32 angleStart, f32 angleEnd, f32 magnitude, const char* debugChannel );

			f32			mAngleStart;		///< [+90..-90]	Degrees.  Positive faces the light, negative away.
			f32			mAngleEnd;			///< [+90..-90]	When this effect should finish fading in or out (depends on specific calc.) (degrees, +90 = facing light, -90 = facing away from light)
		};

		/// \brief Parameters for figuring out how a given sampling interpretation should be considered.
		struct base_export tSamplingShadowMapParameters : tCommonShadowParameters
		{
			tSamplingShadowMapParameters( );
			tSamplingShadowMapParameters( b32 enable, f32 magnitude, const char* debugChannel );
		};

		struct base_export tShadowRuntimeSettings
		{
			tShadowRuntimeSettings( );

			f32 mHighResShadowDist;													///< distance from "focus pt" that high resolution shadows cover
			f32 mLowResShadowDist;													///< remaining distance that the low resolution shadows cover. after this, there will be no shadows.
			f32 mShadowAmount;														///< Shadow amount
		};

		struct base_export tShadowShaderGenerationSettings
		{
			tShadowShaderGenerationSettings( );

			// Misc. Parameters
			u32 mShadowMapLayerCount;												///< How many maps per light do we use
			b32 mShadowBack;														///< If the shadow term should be applied to backColor as well.

			// Simple nDotL based calculations
			tNormalShadowParameters			mNormalShadowValue;						///< "Fade In" effect -- shadow based purely on the normal
			tNormalShadowParameters			mNormalShadowBlurSample;				///< "Edge" effect -- spread sampling points based on normal
			tNormalShadowParameters			mNormalShadowMoveSample;				///< "Edge" effect -- move sampling points directionally based on normal

			// Shadow map sample based calculations
			tSamplingShadowMapParameters	mShadowMapNaieveSingleRangeCheck;		///< Uses 1 sample -- are we in front of that sample?
			tSamplingShadowMapParameters	mShadowMapNaieveBoundingRangesCheck;	///< Uses 4 samples -- are we in front of any sample?
			tSamplingShadowMapParameters	mShadowMapPercentageCloserFiltering;	///< Uses 4 samples -- lerp between the individual 'are we in front of that sample' values
			tSamplingShadowMapParameters	mShadowMapEstimatedNormal;				///< Uses 4 samples -- I don't even know man.
		};

		struct base_export tRendererSettings
		{
			tRendererSettings( );

			b32								mEnableLOD;								///< If true, will try to use other LODs (if they exist)
			b32								mBuildLOD;								///< If true, assetgen will build other LODs (doesn't necessarily mean they will be used in game.)

			f32								mPointLightIntensityMultiplier;			///< Multiplier for all point light intensities
			f32								mPointLightSizeMultiplier;				///< Multiplier for all point light sizes

			tShadowRuntimeSettings			mShadowRuntimeSettings;
			tShadowShaderGenerationSettings	mShadowShaderGenerationSettings;

			tFixedArray<f32, Gfx::tRenderableEntity::cFadeSettingCount>		mRenderableFarFadeSettings;
			tFixedArray<f32, Gfx::tRenderableEntity::cFadeSettingCount>		mRenderableNearFadeSettings;

			tGrowableArray< tLenseFlare > mLenseFlares;

			tLenseFlare* fAddFlare( const std::string& name );
			void fEraseFlare( u32 key );
		};

		struct base_export tEngineConfig
		{
			tRendererSettings	mRendererSettings;
			tEditorDefaults		mEditorDefaults;
			tAssetGenConfig		mAssetGenConfig;
		};

		struct base_export tGameTag
		{
			std::string mName;
			u32			mKey;
			tGameTag( ) : mKey( ~0 ) { }
			tGameTag( const std::string& name, u32 key ) : mName( name ), mKey( key ) { }
			inline b32 operator==( const std::string& name ) const { return mName == name; }
		};

		struct base_export tEvent
		{
			std::string mName;
			u32			mKey;
			tEvent( ) : mKey( ~0 ) { }
			tEvent( const std::string& name, u32 key ) : mName( name ), mKey( key ) { }
			inline b32 operator==( const std::string& name ) const { return mName == name; }

			static std::string fCppName( const std::string& name );
		};

		struct base_export tGameEnumeratedValue
		{
			std::string mName;
			u32			mKey;
			tGameEnumeratedValue( ) : mKey( ~0 ) { }
			tGameEnumeratedValue( const std::string& name, u32 key ) : mName( name ), mKey( key ) { }
			inline b32 operator==( const std::string& name ) const { return mName == name; }
		};

		struct base_export tGameEnumeratedTypeAlias
		{
			std::string								mName;
			u32										mKey;
			b32										mHide;
			tGameEnumeratedTypeAlias( ) : mKey( ~0 ), mHide( false ) { }
			inline b32 operator==( const std::string& name ) const { return mName == name; }
		};

		struct base_export tGameEnumeratedType
		{
			std::string									mName;
			u32											mKey;
			tGrowableArray<tGameEnumeratedTypeAlias>	mAliases;
			tGrowableArray<tGameEnumeratedValue>		mValues;
			b32											mHide;

			tGameEnumeratedType( ) : mKey( ~0 ), mHide( false ) { }
			tGameEnumeratedType( const std::string& name, u32 key ) : mName( name ), mKey( key ), mHide( false ) { }
			inline b32 operator==( const std::string& name ) const { return mName == name; }
			inline b32 operator==( u32 key ) const { return mKey == key; }

			u32 fFindValueIndexByKey( u32 key ) const;
			u32 fFindValueIndexByName( const std::string& name ) const;
			void fInsertValue( const std::string& name, u32 index );
		};


	public:
		tEngineConfig							mEngineConfig;
		tGrowableArray< tGameTag >				mGameTags;
		tGrowableArray< tGameEnumeratedType >	mGameEnumeratedTypes;
		tGrowableArray< tEvent >				mGameEvents;
		tGrowableArray< tEvent >				mKeyFrameEvents;
		tGrowableArray< tEvent >				mAIFlags; // game events specific to the ai system.

#ifdef target_tools
	// Tools only stuff!
	public:
		static std::string fToCppName( const std::string& in );
		void fSetupProjectDefaults( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		void fCompileGameSettings( b32 autoBuildRes ) const;
		void fValidateKeys( );
		void fWriteGameSettingsHpp( ) const;
		void fWriteGameSettingsCpp( ) const;
#endif
	public:
		// Be verrrryyy careful changing the project file. IE dont do it.
		static tProjectFile& fInstance( );

		b32 fLoadXml( const char* data );

		u32				fToTagBitIndexFromKey( u32 key ) const				{ return fFindItemIndexByKey( key, mGameTags ); }
		const tGameTag* fFindTagByName( const std::string& tagName ) const	{ return fFindItemByName( tagName, mGameTags ); }
		const tGameTag* fFindTagByKey( u32 key ) const						{ return fFindItemByKey( key, mGameTags ); }

		void fInsertTag( const std::string& name, u32 index )				{ sigassert( mGameTags.fCount( ) < 32 && "Too many tags :(" ); mGameTags.fInsertSafe( index, tGameTag( name, fNextKey( mGameTags ) ) ); }
		void fInsertEnum( const std::string& name, u32 index )				{ mGameEnumeratedTypes.fInsertSafe( index, tGameEnumeratedType( name, fNextKey( mGameEnumeratedTypes ) ) ); }
		void fInsertGameEvent( const std::string& name, u32 index )			{ mGameEvents.fInsertSafe( index, tEvent( name, fNextKey( mGameEvents ) ) );  }
		void fInsertKeyframeEvent( const std::string& name, u32 index )		{ mKeyFrameEvents.fInsertSafe( index, tEvent( name, fNextKey( mKeyFrameEvents ) ) );  }
		void fInsertAIFlags( const std::string& name, u32 index )			{ mAIFlags.fInsertSafe( index, tEvent( name, fNextKey( mAIFlags ) ) );  }

		// searches aliases also
		const tGameEnumeratedType* fFindEnumeratedTypeByName( const std::string& enumName, u32* keyOut = NULL ) const;
		const tGameEnumeratedType* fFindEnumeratedTypeByKey( u32 key ) const;

		const tEvent* fFindGameEventByName( const std::string& name ) const { return fFindItemByName( name, mGameEvents ); }
		const tEvent* fFindGameEventByKey( u32 key ) const					{ return fFindItemByKey( key, mGameEvents ); }

		const tEvent* fFindAIFlagByName( const std::string& name ) const	{ return fFindItemByName( name, mAIFlags ); }
		const tEvent* fFindAIFlagByKey( u32 key ) const						{ return fFindItemByKey( key, mAIFlags ); }
		u32 fFindAIFlagIndexByKey( u32 key ) const							{ return fFindItemIndexByKey( key, mAIFlags ); }

		const tEvent* fFindKeyframeEventByName( const std::string& name ) const { return fFindItemByName( name, mKeyFrameEvents ); }
		const tEvent* fFindKeyframeEventByKey( u32 key ) const					{ return fFindItemByKey( key, mKeyFrameEvents ); }
		u32 fFindKeyframeEventIndexByKey( u32 key ) const						{ return fFindItemIndexByKey( key, mKeyFrameEvents ); }

		// Use these helper functions yourself, in order to search all the data.

		template< typename t >
		static const t* fFindItemByName( const std::string& name, const tGrowableArray< t >& list )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				if( list[ i ].mName == name )
					return &list[ i ];
			}

			return NULL;
		}

		template< typename t >
		static t* fFindItemByKey( u32 key, tGrowableArray< t >& list )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				if( list[ i ].mKey == key )
					return &list[ i ];
			}

			return NULL;
		}

		template< typename t >
		static const t* fFindItemByKey( u32 key, const tGrowableArray< t >& list )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				if( list[ i ].mKey == key )
					return &list[ i ];
			}

			return NULL;
		}

		template< typename t >
		static u32 fFindItemIndexByKey( u32 key, const tGrowableArray< t >& list )
		{
			for( u32 i = 0; i < list.fCount( ); ++i )
			{
				if( list[ i ].mKey == key )
					return i;
			}

			return ~0;
		}

		template< typename T >
		static u32 fNextKey( const T& list )
		{
			u32 nextKey = 0;
			for( u32 i = 0; i < list.fCount( ); ++i )
				nextKey = fMax( nextKey, list[ i ].mKey + 1 );

			return nextKey;
		}
	};

}

#endif //__tProjectFile__
