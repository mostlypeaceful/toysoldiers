#ifndef __tGameEffects__
#define __tGameEffects__

#include "tDataTableFile.hpp"
#include "Audio/tSource.hpp"
#include "tLogic.hpp"

// for tEffectPlayer
#include "Gfx/tCameraController.hpp"
#include "Input/tGamepad.hpp"

namespace Sig
{
	class tGameAppBase;

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

	struct tEffectProperties
	{
		tEntityPtr			mOwner;
		tEntityPtr			mTarget;
		b32					mFrustumCull;
		Input::tRumbleEvent mRumble;
		tScreenShakeParams	mScreenShake;

		tEffectProperties( )
			: mFrustumCull( true )
		{ }
	};

	struct tPersistentEffect : public tRefCounter
	{
		tEffectProperties	mEffect;
		f32					mUserScale;

		tPersistentEffect( const tEffectProperties& effect )
			: mEffect( effect )
			, mUserScale( 1.f )
		{ }
	};

	typedef tRefCounterPtr< tPersistentEffect > tPersistentEffectPtr;

	// This is the interface that tGameEffects uses to communicate to each player.
	//  the game app players should inherit and implement fGetEffectsData.
	class tEffectPlayer : public tLogic
	{
		define_dynamic_cast( tEffectPlayer, tLogic );
	public:
		virtual ~tEffectPlayer( ) { }

		// List of persistent effects we're aware of.
		tGrowableArray< tPersistentEffectPtr > mPersistent;

		// This structure tells the game effects system how to respond to an effect.
		//  From the players perspective.
		struct tEffectData
		{
			Gfx::tCameraController* mCamera;
			const Input::tGamepad*	mGamePad;
			Math::tVec3f			mEffectRefPt;
			b32						mFullStrength;

			tEffectData( Gfx::tCameraController* cam = NULL,
				const Input::tGamepad*	pad = NULL,
				const Math::tVec3f& refPt = Math::tVec3f::cZeroVector
				, b32 fullStrength = false )
				: mCamera( cam )
				, mGamePad( pad )
				, mEffectRefPt( refPt )
				, mFullStrength( fullStrength )
			{ }
		};

		virtual tEffectData fGetEffectsData( tEntity* owner ) const { return tEffectData( ); }

		void fClearPersistentEffects( ) { mPersistent.fSetCount( 0 ); }
		void fStepPersistentEffects( f32 dt );
		static void fProcessEvent( tEffectPlayer::tEffectData& playerData, const tEffectProperties& effect, const Math::tVec3f& pos, b32 persistent, f32 scale );
	};

	typedef tRefCounterPtr< tEffectPlayer > tEffectPlayerPtr;

	struct tEffectArgs
	{
		const Math::tMat3f* mTransformOverride;
		tEntity* mParentOverride;
		tEntity* mInsertParent; // Insert parent will be inserted between the effect entity and what ever it was going to be spawned under. to store additional data above the effect.

		f32 mBlendStrength;
		b32 mDontSpawnEffect;
		u32 mSurfaceType;

		// These must all be set together
		const Math::tVec3f* mSurfaceNormal;
		const Math::tVec3f* mInputNormal;
		const Math::tVec3f* mXDir;
		//

		tEffectArgs( )
			: mTransformOverride( NULL )
			, mParentOverride( NULL )
			, mInsertParent( NULL )
			, mBlendStrength( 2.f )
			, mDontSpawnEffect( false )
			, mSurfaceType( ~0 )
			, mSurfaceNormal( NULL )
			, mInputNormal( NULL )
			, mXDir( NULL )
		{
		}
	};

	// This is not safe to hold on to after the fHandleLogicEvent call
	struct tEffectLogicEventContext : public Logic::tEventContext
	{
		define_dynamic_cast( tEffectLogicEventContext, Logic::tEventContext );

		tEffectArgs&			mArgs;
		Audio::tSourcePtr&		mAudio;			//release this if you dont want the base to play audio
		tStringPtr&				mAudioEvent;	//you may change this but you should release mAudio if you dont want to play any event.
		tPersistentEffectPtr&	mPersistent;

		tEffectLogicEventContext( tEffectArgs& args, Audio::tSourcePtr& audio, tStringPtr& audioEvent, tPersistentEffectPtr& persistent )
			: mArgs( args )
			, mAudio( audio )
			, mAudioEvent( audioEvent )
			, mPersistent( persistent )
		{ }
	};

	class tGameEffects
	{
		declare_singleton_define_own_ctor_dtor( tGameEffects );

		tGameEffects( );
		~tGameEffects( );

	public:
		// Game app must keep this list up to date.
		tGrowableArray< tEffectPlayerPtr > gPlayers;

		// Call this from your permaloaded resource function.
		void fLoadResourcesAndValidate( const tResourcePtr& res, const tGameEnum& surfaceEnum, u32 logicEventID = ~0 );
		void fSetLogicEventID( u32 id ) { mLogicEventID = id; }

		tEntity* fPlayEffect( tEntity* ownerEntity, const tStringPtr& effect, tEffectArgs& args = tEffectArgs( ) );
		tEntity* fPlayEffectForScript( tEntity* ownerEntity, const tStringPtr& effect );

		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tStringHashDataTableFile	mEffectsTable;
		tGameEnum					mSurfaceTypeEnum;
		u32							mLogicEventID;

		tStringPtr fSurfaceLookup( const tStringPtr& tableName, u32 surfaceType );
	};

}

#endif//__tGameEffects__
