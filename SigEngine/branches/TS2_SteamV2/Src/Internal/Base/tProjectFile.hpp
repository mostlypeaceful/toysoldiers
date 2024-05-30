#ifndef __tProjectFile__
#define __tProjectFile__
#if defined( platform_pcdx9 ) || defined( platform_pcdx10 )

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
			b32					mUseEnginePlugins;
			tFilePathPtrList	mAdditionalPluginPaths;
			tAssetGenConfig( );
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

			u32 fFindValueIndexByKey( u32 key ) const;
			u32 fFindValueIndexByName( const std::string& name ) const;
			void fInsertValue( const std::string& name, u32 index );
		};


	public:
		u32										mShadowMapLayerCount;
		tAssetGenConfig							mAssetGenConfig;
		tGrowableArray< tGameTag >				mGameTags;
		tGrowableArray< tGameEnumeratedType >	mGameEnumeratedTypes;
		tGrowableArray< tEvent >				mGameEvents;
		tGrowableArray< tEvent >				mKeyFrameEvents;

	public:
		static const tProjectFile& fGetCurrentProjectFileCached( f32 maxTimeBetweenCaches = 10.f ); // in seconds
	public:
		tProjectFile( );
		~tProjectFile( );

		static std::string fToCppName( const std::string& in );

		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );

		void fSetupProjectDefaults( );
		void fCompileGameSettings( b32 autoBuildRes ) const;

		u32				fToTagBitIndexFromKey( u32 key ) const;
		const tGameTag* fFindTagByName( const std::string& tagName ) const;
		const tGameTag* fFindTagByKey( u32 key ) const;
		const tGameTag& FindTagByIndex( u32 index ) const { return mGameTags[ index ]; }

		void fInsertTag( const std::string& name, u32 index )				{ sigassert( mGameTags.fCount( ) < 32 && "Too many tags :(" ); mGameTags.fInsertSafe( index, tGameTag( name, fNextKey( mGameTags ) ) ); }
		void fInsertEnum( const std::string& name, u32 index )				{ mGameEnumeratedTypes.fInsertSafe( index, tGameEnumeratedType( name, fNextKey( mGameEnumeratedTypes ) ) ); }
		void fInsertGameEvent( const std::string& name, u32 index )			{ mGameEvents.fInsertSafe( index, tEvent( name, fNextKey( mGameEvents ) ) );  }
		void fInsertKeyframeEvent( const std::string& name, u32 index )		{ mKeyFrameEvents.fInsertSafe( index, tEvent( name, fNextKey( mKeyFrameEvents ) ) );  }

		// searches aliases also
		const tGameEnumeratedType* fFindEnumeratedTypeByName( const std::string& enumName, u32* keyOut = NULL ) const;
		const tGameEnumeratedType* fFindEnumeratedTypeByKey( u32 key ) const;

		const tEvent* fFindGameEventByName( const std::string& name ) const { return fFindEventByName( name, mGameEvents ); }
		const tEvent* fFindGameEventByKey( u32 key ) const					{ return fFindEventByKey( key, mGameEvents ); }

		const tEvent* fFindKeyframeEventByName( const std::string& name ) const { return fFindEventByName( name, mKeyFrameEvents ); }
		const tEvent* fFindKeyframeEventByKey( u32 key ) const					{ return fFindEventByKey( key, mKeyFrameEvents ); }
		u32 fFindKeyframeEventIndexByKey( u32 key ) const;

	private:
		void fWriteGameSettingsHpp( ) const;
		void fWriteGameSettingsCpp( ) const;

		static const tEvent* fFindEventByName( const std::string& name, const tGrowableArray< tEvent >& list );
		static tEvent* fFindEventByKey( u32 key, tGrowableArray< tEvent >& list );
		static const tEvent* fFindEventByKey( u32 key, const tGrowableArray< tEvent >& list );

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

#endif//defined( platform_pcdx9 ) || defined( platform_pcdx10 )
#endif//__tProjectFile__
