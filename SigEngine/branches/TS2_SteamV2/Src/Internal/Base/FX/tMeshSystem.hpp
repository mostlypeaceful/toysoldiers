#ifndef __tMeshSystem__
#define __tMeshSystem__

#include "tFxGraph.hpp"
#include "Gfx/tCamera.hpp"
#include "tEntityDef.hpp"
#include "tMeshEntity.hpp"
#include "tSgFileRefEntity.hpp"
#include "tSceneRefEntity.hpp"
#include "tEmitters.hpp"
#include "tParticleAttractor.hpp"

namespace Sig
{
namespace FX
{

	enum tMeshSystemGraphs
	{
		cSpawnRate,
		cSpawnSize,
		cSpawnSpeed,
		cEmitterScale,
		cMeshLife,
		cMeshScale,
		cMeshSpeedScale,
		cMeshSpinRate,
		cMeshTint,
		cFxMeshSystemGraphCount,
	};

	enum tMeshSystemFlags
	{
		cBurstEmit				= ( 1 << 0 ),
		cCollideWithGround		= ( 1 << 1 ),
		cRandomOrientation		= ( 1 << 2 ),
		cMeshSystemFlagsCount	= 3,
	};

	class base_export tToolFxMeshSystemData : public tRefCounter
	{
	public:

		tToolFxMeshSystemData( tToolFxMeshSystemData* rhs );
		tToolFxMeshSystemData( );
		~tToolFxMeshSystemData( );

		template<class t>
		t fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const { return mGraphs[ idx ]->fSample< t >( random, x ); }

		void fUpdate( );

	public:
		static const tStringPtr mGraphNames[ cFxMeshSystemGraphCount ];
		static const Math::tVec3f mColours[ cFxMeshSystemGraphCount ];
		static const tStringPtr mSystemFlagStrings[ cMeshSystemFlagsCount ];
		
		u32							mSystemFlags;
		tEmitterType				mEmitterType;
		tGrowableArray< tGraphPtr > mGraphs;
		tGrowableArray< u32 >		mAttractorIgnoreIds;
	};

	typedef tRefCounterPtr< tToolFxMeshSystemData > tToolFxMeshSystemDataPtr;


	class base_export tBinaryFxMeshSystemData
	{
		declare_reflector( );
	public:

		tBinaryFxMeshSystemData( );
		tBinaryFxMeshSystemData( tNoOpTag );
		~tBinaryFxMeshSystemData( );

		template<class t>
		t fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const { return mGraphs[ idx ]->fSample< t >( random, x ); }
		
	public:		
		u32							mSystemFlags;
		tEnum< tEmitterType, u8 >	mEmitterType;
		u8							mPad[ 3 ];
		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mGraphs;
		tDynamicArray< u32 >									mAttractorIgnoreIds;
	};



	template< class ToolData, class BinaryData >
	class tMeshSystemData : public tRefCounter
	{
		ToolData* mToolData;
		BinaryData* mBinaryData;
	public:

		tMeshSystemData( )
			: mToolData( 0 ), mBinaryData( 0 )
		{
		}

		tMeshSystemData( ToolData* tooldata, BinaryData* binarydata )
			: mToolData( tooldata ), mBinaryData( binarydata )
		{
		}

		tMeshSystemData( const tMeshSystemData< ToolData, BinaryData >* data )
			: mBinaryData( data->mBinaryData )
		{
			// only the tool version needs to be copyable like this!
			mToolData = NEW ToolData( data->mToolData );
		}

		~tMeshSystemData( )
		{
			if( mToolData )		delete mToolData;
			//if( mBinaryData )	delete mBinaryData;
		}

		ToolData* fToolData( ) const { return mToolData; }
		BinaryData* fBinaryData( ) const { return mBinaryData; }

		
		u32 fSystemFlags( ) const
		{
			if( mToolData )		return mToolData->mSystemFlags;
			if( mBinaryData )	return mBinaryData->mSystemFlags;
			return 0;
		}
		void fSetSystemFlags( u32 flags )
		{
			if( mToolData )
				mToolData->mSystemFlags = flags;
		}

		void fAddFlag( u32 flag )
		{
			if( mToolData )
				mToolData->mSystemFlags |= flag;
		}

		void fRemoveFlag( u32 flag )
		{
			if( mToolData )
				mToolData->mSystemFlags &= ~flag;
		}

		b32 fHasFlag( u32 flag ) const
		{
			if( mBinaryData )
				return mBinaryData->mSystemFlags& flag;
			if( mToolData )
				return mToolData->mSystemFlags & flag;
			return false;
		}

		tEmitterType fEmitterType( ) const
		{
			if( mToolData )		return mToolData->mEmitterType;
			if( mBinaryData )	return mBinaryData->mEmitterType;
			return cSphere;
		}

		void fSetEmitterType( tEmitterType type )
		{
			if( mToolData )
				mToolData->mEmitterType = type;
		}
		
		template< class t >
		t fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{
			if( mToolData )		return mToolData->fSampleGraph< t >( random, idx, x );
			if( mBinaryData )	return mBinaryData->fSampleGraph< t >( random, idx, x );
			return t( 0.f );
		}

		tGraphPtr& fGraph( u32 idx )
		{
			return mToolData->mGraphs[ idx ];
		}

		void fClearAttractorIgnoreIds( )
		{
			if( mToolData )	return mToolData->mAttractorIgnoreIds.fSetCount( 0 );
			// Can't modify binary state
		}

		void fAddAttractorIgnoreId( u32 id )
		{
			if( mToolData )	return mToolData->mAttractorIgnoreIds.fPushBack( id );
			// Can't modify binary state
		}

		u32 fAttractorIgnoreListCount( ) const
		{
			if( mToolData )	return mToolData->mAttractorIgnoreIds.fCount( );
			if( mBinaryData )	return mBinaryData->mAttractorIgnoreIds.fCount( );
			return 0;
		}

		u32 fAttractorIgnoreId( u32 idx ) const
		{
			if( mToolData )	return mToolData->mAttractorIgnoreIds[ idx ];
			if( mBinaryData )	return mBinaryData->mAttractorIgnoreIds[ idx ];
			return ~0;
		}

		template< class tSerializer >
		void fSerializeXmlObject( tSerializer& s )
		{
			if( !mToolData )
				mToolData = NEW ToolData( );

			s( "Graphs", mToolData->mGraphs );
			s( "SystemFlags", mToolData->mSystemFlags );

			u32 emitterType = ( u32 ) mToolData->mEmitterType;
			s( "EmitterType", emitterType );
			mToolData->mEmitterType = ( tEmitterType )emitterType;

			s( "AttractorIgnoreIds", mToolData->mAttractorIgnoreIds );
		}
	};

	namespace FxMeshSystem
	{
		typedef tMeshSystemData< tToolFxMeshSystemData, tBinaryFxMeshSystemData > tData;
		typedef tRefCounterPtr< tData > tMeshSystemDataPtr;
		const int tNullBinaryData = 0;
		const int tNullToolData = 0;
	}


	class base_export tMeshSystemDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tMeshSystemDef, 0x2AE8074 );
	
	public:
		tLoadInPlacePtrWrapper< tBinaryFxMeshSystemData >	mBinaryData;
		tLoadInPlaceStringPtr*								mFxMeshSystemName;
		tLoadInPlaceStringPtr*								mParticleSystemToSyncWith;
		tLoadInPlaceStringPtr*								mMeshResourceFile;

	public:
		tMeshSystemDef( );
		tMeshSystemDef( tNoOpTag );
		~tMeshSystemDef( );

		virtual b32 fHasRenderableBounds( ) const { return true; }
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;

		void fFromToolData( const tToolFxMeshSystemData* data );
	};


	class base_export tOldMeshParticle : public tSceneRefEntity
	{
	public:
		tOldMeshParticle( tEntity& parent, const tResourcePtr& res );
		f32					mCurLife;
		f32					mMaxLife;
		Math::tVec3f		mInitialScale;
		Math::tVec3f		mDirection;
		Math::tMat3f		mXform;
	};

	class base_export tMeshSystem : public tEntity 
	{
		define_dynamic_cast( tMeshSystem, tEntity );
	private:

		f32										mLifetime;
		f32										mCurrentTime;
		f32										mEmissionRatePerFrame;
		f32										mEmissionPercent;
		u32										mRandomSeed;

		b8										mStopped;
		b8										mFinishUp;
		b8										mQuickReset;
		b8										pad2;

		FxMeshSystem::tMeshSystemDataPtr		mData;
		tStringPtr								mFxMeshSystemName;
		tStringPtr								mParticleSystemToSyncWith;
		tFilePathPtr							mMeshResourceFile;
		tMersenneGenerator						mRandomGenerator;

		tParticleEmitterPtr						mEmitter;

		tResourcePtr							mMeshResource;
		tResource::tOnLoadComplete::tObserver	mOnLoadComplete;
		tGrowableArray< tOldMeshParticle* >		mMeshes;
		tGrowableArray< tOldMeshParticle* >		mMeshRemoval;
		tGrowableArray< tParticleAttractorPtr > mAttractorsList;

	public:
		tMeshSystem( );
		tMeshSystem( const tMeshSystemDef* def );
		virtual ~tMeshSystem( );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fMoveST( f32 dt );
		virtual void fEffectsMT( f32 dt );
		virtual void fThinkST( f32 dt );

		virtual b32 fReadyForRemoval( ) const;

		void fFastUpdate( f32 curTime, b32 fromTheStart = false );
		void fEmitNewMeshes( f32 dt );
		void fSystemUpdate( f32 dt );
		void fUpdateMeshXforms( );

		FxMeshSystem::tMeshSystemDataPtr fFxMeshSystemData( ) const { return mData; }
		void fSetData( FxMeshSystem::tMeshSystemDataPtr data ) { mData = data; }
		
		const tStringPtr& fFxMeshSystemName( ) const { return mFxMeshSystemName; }
		void fSetMeshSystemName( const tStringPtr& name ) { mFxMeshSystemName = name; }

		const tStringPtr& fParticleSystemToSyncWith( ) const { return mParticleSystemToSyncWith; }
		void fSetParticleSystemToSyncWith( const tStringPtr& syncWith ) { mParticleSystemToSyncWith = syncWith; }

		void fSetAttractors( const tGrowableArray< tParticleAttractorPtr >& list );

		void fSetMeshResourceFile( const tFilePathPtr& meshFile, b32 nameOnly = false );
		const tFilePathPtr& fMeshResourceFile( ) const { return mMeshResourceFile; }

		void fSetRandomSeed( u32 seed );
		void fSetLifetime( const f32 life ) { mLifetime = life; }
		void fSetCurrentTime( const f32 curTime ) { mCurrentTime = curTime; }

		void fSetEmitterType( const tEmitterType type );
		tEmitterType fEmitterType( ) const { return mData->fEmitterType( ); }

		void fSetEmissionPercent( f32 percent ) { mEmissionPercent = percent; }


		void fAddFlag( u32 flag ) { mData->fAddFlag( flag ); }
		void fRemoveFlag( u32 flag ) { mData->fRemoveFlag( flag ); }
		b32 fHasFlag( u32 flag ) const { return mData->fHasFlag( flag ); }

		const tMersenneGenerator* fRandomNumberGenerator( ) const { return &mRandomGenerator; }

		const u32	fMeshCount( ) const { return mMeshes.fCount( ); }
		void		fStop( b32 stop ) { mStopped = stop; }
		void		fQuickReset( );
		void		fFinishUp( b32 finish ) { mFinishUp = finish; }

	private:

		void fClearAllMeshes( );
		void fOnLoadComplete( tResource& resource, b32 success );

	};

	typedef tRefCounterPtr< tMeshSystem > tMeshSystemPtr;




	namespace MeshSystem
	{
		struct tFirstBornSort
		{
			inline b32 operator( )( const tOldMeshParticle* a, const tOldMeshParticle* b ) const
			{
				return a->mCurLife > b->mCurLife;
			}
		};

		struct tLastBornSort
		{
			inline b32 operator( )( const tOldMeshParticle* a, const tOldMeshParticle* b ) const
			{
				return a->mCurLife < b->mCurLife;
			}
		};
	}
}
}


#endif // __tMeshSystem__

