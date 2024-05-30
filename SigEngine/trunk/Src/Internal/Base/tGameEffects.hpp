#ifndef __tGameEffects__
#define __tGameEffects__

#include "tDataTableFile.hpp"
#include "tLogicEvent.hpp"
#include "Input/tRumbleManager.hpp"

namespace Sig
{
	struct tEffectProperties;
	struct tPersistentEffect;
	class tResourceLoadList2;
	namespace Audio { class tSource; }
	namespace Gfx { class tCameraController; }
	namespace Input { class tGamepad; }

	// this is a test and should not be used elsewhere
	class tGameEnum
	{
	public:
		typedef u32 (*tStrToEnumPtr)( const tStringPtr& );
		typedef const tStringPtr& (*tEnumToStrPtr)( u32 );

		tGameEnum( const tStrToEnumPtr& toEnum = NULL, const tEnumToStrPtr& toStr = NULL, u32 count = 0 )
			: mToEnum( toEnum )
			, mToStr( toStr )
			, mCount( count )
		{ }

		u32					fCount( ) const { return mCount; }
		u32					fStringToEnum( const tStringPtr& str ) const	{ sigassert( mToEnum ); return mToEnum( str ); }
		const tStringPtr&	fEnumToStr( u32 enumVal ) const					{ sigassert( mToStr ); return mToStr( enumVal ); }

	private:
		u32				mCount;
		tStrToEnumPtr	mToEnum;
		tEnumToStrPtr	mToStr;
	};

	//------------------------------------------------------------------------------
	// tScreenShakeParams
	//------------------------------------------------------------------------------
	struct tScreenShakeParams
	{
		f32 mMagnitude;
		f32 mTime;
		f32 mMaxDistance;

		tScreenShakeParams( )
			: mMagnitude( 0.f )
			, mTime( 0.f )
			, mMaxDistance( 1.f )
		{ }
	};

	//------------------------------------------------------------------------------
	// tEffectProperties
	//------------------------------------------------------------------------------
	struct tEffectProperties
	{
		tEntityWeakPtr		mOwner;
		tEntityWeakPtr		mTarget;
		b32					mFrustumCull;
		Input::tRumbleEvent mRumble;
		tScreenShakeParams	mScreenShake;

		tEffectProperties( )
			: mFrustumCull( true )
		{ }
	};

	//------------------------------------------------------------------------------
	// tPersistentEffect
	//------------------------------------------------------------------------------
	struct tPersistentEffect : public tRefCounter
	{
		declare_uncopyable( tPersistentEffect );
	public:
		tEffectProperties	mEffect;
		f32					mUserScale;

		tPersistentEffect( const tEffectProperties& effect )
			: mEffect( effect )
			, mUserScale( 1.0f )
		{ }
	};

	// This is the interface that tGameEffects uses to communicate to each player.
	//  the game app players should inherit and implement fGetEffectsData.
	class base_export tEffectPlayer : public tLogic
	{
		declare_uncopyable( tEffectPlayer );
		define_dynamic_cast( tEffectPlayer, tLogic );
	public:
		// This structure tells the game effects system how to respond to an effect.
		//  From the players perspective.
		struct tEffectData
		{
			tRefCounterPtr<Gfx::tCameraController>	mCamera;
			const Input::tGamepad*					mGamePad;
			Math::tVec3f							mEffectRefPt;
			b32										mFullStrength;

			tEffectData( 
				Gfx::tCameraController* cam = NULL,
				const Input::tGamepad*	pad = NULL,
				const Math::tVec3f& refPt = Math::tVec3f::cZeroVector,
				b32 fullStrength = false );
			void fProcessEvent( const tEffectProperties& effect, const Math::tVec3f& pos, b32 persistent, f32 scale );
		};
	public:
		tEffectPlayer( );
		virtual ~tEffectPlayer( );

		virtual tEffectData fGetEffectsData( tEntity* owner ) const;

		void fAddPersistentEffect( tPersistentEffect* pe );
		void fClearPersistentEffects( );
		void fStepPersistentEffects( f32 dt );

	private:
		tGrowableArray< tRefCounterPtr<tPersistentEffect> > mPersistent;
	};

	typedef tRefCounterPtr< tEffectPlayer > tEffectPlayerPtr;

	struct tEffectArgs
	{
		const Math::tMat3f* mTransformOverride;
		tEntity* mParentOverride;
		tEntity* mInsertParent; // Insert parent will be inserted between the effect entity and what ever it was going to be spawned under. to store additional data above the effect.
		tStringPtr mEffectMapTable;

		f32 mBlendStrength;
		b32 mDontSpawnEffect;
		u32 mSurfaceType;
		u32 m3rdDimsension;

		// These must all be set together
		const Math::tVec3f* mSurfaceNormal;
		const Math::tVec3f* mInputNormal;

		// Set this if you want to get back all the entities that were spawned.
		tGrowableArray< tEntity* >* mEntitiesOut;

		tEffectArgs( )
			: mTransformOverride( NULL )
			, mParentOverride( NULL )
			, mInsertParent( NULL )
			, mBlendStrength( 2.f )
			, mDontSpawnEffect( false )
			, mSurfaceType( ~0 )
			, m3rdDimsension( ~0 )
			, mSurfaceNormal( NULL )
			, mInputNormal( NULL )
			, mEntitiesOut( NULL )
		{
		}
	};

	// This is not safe to hold on to after the fHandleLogicEvent call
	struct tEffectLogicEventContext : public Logic::tEventContext
	{
		debug_watch( tEffectLogicEventContext );
		define_dynamic_cast( tEffectLogicEventContext, Logic::tEventContext );

		const tEffectArgs&					mArgs;
		tRefCounterPtr<Audio::tSource>		mAudio;			//release this if you dont want the base to play audio
		tStringPtr							mAudioEvent;	//you may change this but you should release mAudio if you dont want to play any event.
		tRefCounterPtr<tPersistentEffect>	mPersistent;

		tEffectLogicEventContext( const tEffectArgs& args, Audio::tSource* audio, tStringPtr& audioEvent, tPersistentEffect* persistent );
	};

	class base_export tGameEffects
	{
		declare_singleton_define_own_ctor_dtor( tGameEffects );

		struct tTableInfo
		{
			const tStringHashDataTable* mTable;
			u32 mRow;

			tTableInfo( );
		};

		struct tAudioStopEvent
		{
			tStringPtr mStopEvent;
			tEntityPtr mOwner;
			tRefCounterPtr<Audio::tSource> mSource;
			Math::tMat3f mOffset;

			tAudioStopEvent( const tStringPtr& stopEvent = tStringPtr::cNullPtr, tEntity* owner = NULL, Audio::tSource* source = NULL );

			// returns true if finished
			b32 fCheck( );
		};

	private:

		tGameEffects( );
		~tGameEffects( );

	public:
		typedef tDelegate< void ( const tEntity*, const tStringPtr&, tGrowableArray<tEntity*>& ) > tTargetAccumulator;

		// Game app must keep this list up to date.
		tGrowableArray< tEffectPlayerPtr > gPlayers;

		// Call this from your permaloaded resource function.
		void fInitialize( 
			const tResourcePtr& effectsTable, 
			const tResourcePtr& effectsMapTable, 
			const tGameEnum& surfaceEnum, 
			u32 logicEventID = ~0 );
		void fShutdown( );

		void fLoadResourcesAndValidate( );

		void fSetLogicEventID( u32 id ) { mLogicEventID = id; }
		void fSetTargetAccumulator( const tTargetAccumulator& targ ) { mTargetAcculuator = targ; }

		// Will only return you the first effect entity spawned. Use args to get all of them back.
		tEntity* fPlayEffect( tEntity* ownerEntity, const tStringPtr& effect, const tEffectArgs& args = tEffectArgs( ) );

		tEntity* fPlayEffectForScript( tEntity* ownerEntity, const tStringPtr& effect );

		// you must call this if you're using this sytem
		void fTickST( f32 dt );

		static void fExportScriptInterface( tScriptVm& vm );

	private:

		tTableInfo fFindTableInfo( const tStringPtr& tableName, const tStringPtr& rowName );
		tFilePathPtr fTryGetSigmlPath( const tStringHashDataTable& table, u32 row );
		tFilePathPtr fSurfaceLookup( const tStringHashDataTable& table, u32 row, const tEffectArgs& args );

	private:

		tRefCounterPtr< tResourceLoadList2 >	mResources;			///< TODO: Async load effects on demand instead?
		tStringHashDataTableFile				mEffectsTable;
		tStringHashDataTableFile				mEffectsMapTable;
		tGameEnum								mSurfaceTypeEnum;
		u32										mLogicEventID;
		tTargetAccumulator						mTargetAcculuator;
		tGrowableArray<tAudioStopEvent>			mAudioStopEvents;
	};
}//Sig

#endif//__tGameEffects__
