#ifndef __tParticleSystemStates__
#define __tParticleSystemStates__
#include "tParticle.hpp"
#include "Gfx/tParticleMaterial.hpp"
#include "tFxGraph.hpp"

namespace Sig { namespace FX
{
	// If you add another flag here, make sure to flip over to the .cpp and add the appropriate string into mParticleSystemFlagStrings
	// and then the tool interface will update properly.
	enum tParticleSystemFlags
	{
		cBurstMode					= ( 1 << 0 ),
		cAlignWithVelocity			= ( 1 << 1 ),
		cVelocityStretch			= ( 1 << 2 ),
		cGhostImage					= ( 1 << 3 ),
		cKeepYUp					= ( 1 << 4 ),
		cMoveGhostParticles			= ( 1 << 5 ),
		cVolumeEmit					= ( 1 << 6 ),
		cParticleSystemFlagsCount	= 7,
	};

	class base_export tToolParticleSystemState : public Rtti::tSerializableBaseClass, public tRefCounter
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tToolParticleSystemState, 0x8D5127B3 );
	public:

		tToolParticleSystemState( );
		tToolParticleSystemState( const tToolParticleSystemState& rhs );
		~tToolParticleSystemState( );

		static const tStringPtr mParticleSystemFlagStrings[ cParticleSystemFlagsCount ];

		static const tStringPtr mEmissionGraphNames[ cEmissionGraphCount ];
		static const tStringPtr mPerParticleGraphNames[ cParticleGraphCount ];
		static const tStringPtr mMeshGraphNames[ cMeshGraphCount ];

		static const Math::tVec3f mEmissionColours[ cEmissionGraphCount ];
		static const Math::tVec3f mParticleColours[ cParticleGraphCount ];
		static const Math::tVec3f mMeshColours[ cMeshGraphCount ];

		static const Math::tVec3f mEmissionHighlights[ cEmissionGraphCount ];
		static const Math::tVec3f mParticleHighlights[ cParticleGraphCount ];
		static const Math::tVec3f mMeshHighlights[ cMeshGraphCount ];

		static const Math::tVec3f mEmissionSelects[ cEmissionGraphCount ];
		static const Math::tVec3f mParticleSelects[ cParticleGraphCount ];
		static const Math::tVec3f mMeshSelects[ cMeshGraphCount ];

		template< class t >
		t fSampleEmissionGraph( const tMersenneGenerator* random, const u32 idx, f32 x ) const { return mEmissionGraphs[ idx ]->fSample< t >( random, x ); }
		
		template< class t >
		t fSamplePerParticleGraph( const tMersenneGenerator* random, const u32 idx, f32 x ) const{ return mPerParticleGraphs[ idx ]->fSample< t >( random, x ); }

		template< class t >
		t fSampleMeshGraph( const tMersenneGenerator* random, const u32 idx, f32 x ) const{ return mMeshGraphs[ idx ]->fSample< t >( random, x ); }

		tGrowableArray< tGraphPtr > mEmissionGraphs;
		tGrowableArray< tGraphPtr > mPerParticleGraphs;
		tGrowableArray< tGraphPtr > mMeshGraphs;
		tGrowableArray< u32 >		mAttractorIgnoreIds;
		u32							mSystemFlags;
	};

	typedef tRefCounterPtr< tToolParticleSystemState > tToolParticleSystemStatePtr;


	class base_export tBinaryParticleSystemState
	{
		declare_reflector( );
	public:

		tBinaryParticleSystemState( );
		tBinaryParticleSystemState( tNoOpTag );
		~tBinaryParticleSystemState( );

		template<class t>
		t fSampleEmissionGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const { return mEmissionGraphs[ idx ]->fSample< t >( random, x ); }
		
		template<class t>
		t fSamplePerParticleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const{ return mPerParticleGraphs[ idx ]->fSample< t >( random, x ); }

		template<class t>
		t fSampleMeshGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const{ return mMeshGraphs[ idx ]->fSample< t >( random, x ); }

		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mEmissionGraphs;
		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mPerParticleGraphs;
		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mMeshGraphs;
		tDynamicArray< u32 >									mAttractorIgnoreIds;
		u32														mSystemFlags;
	};

	class base_export tState : public tRefCounter
	{
		tToolParticleSystemState* mToolState;
		const tBinaryParticleSystemState* mBinaryState;
	public:

		tState( )
			: mToolState( NEW tToolParticleSystemState( ) )
			, mBinaryState( 0 )
		{
		}
		explicit tState( tToolParticleSystemState* toolState )
			: mToolState( toolState )
			, mBinaryState( 0 )
		{
		}
		explicit tState( const tBinaryParticleSystemState* binaryState )
			: mToolState( 0 )
			, mBinaryState( binaryState )
		{
		}

		~tState( )
		{
			delete mToolState;
		}

		tToolParticleSystemState& fToolState( ) { sigassert( mToolState ); return *mToolState; }
		const tToolParticleSystemState& fToolState( ) const { sigassert( mToolState ); return *mToolState; }

		template< class t >
		t fSampleEmissionGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{
			if( mBinaryState )	return mBinaryState->fSampleEmissionGraph< t >( random, idx, x );
			if( mToolState )	return mToolState->fSampleEmissionGraph< t >( random, idx, x );
			return t( 0.f );
		}

		template< class t >
		t fSamplePerParticleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{
			if( mBinaryState )	return mBinaryState->fSamplePerParticleGraph< t >( random, idx, x );
			if( mToolState )	return mToolState->fSamplePerParticleGraph< t >( random, idx, x );
			return t( 0.f );
		}

		template< class t >
		t fSampleMeshGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{
			if( mBinaryState )	return mBinaryState->fSampleMeshGraph< t >( random, idx, x );
			if( mToolState )	return mToolState->fSampleMeshGraph< t >( random, idx, x );
			return t( 0.f );
		}

		void fClearAttractorIgnoreIds( )
		{
			if( mToolState )	return mToolState->mAttractorIgnoreIds.fSetCount( 0 );

			// Can't modify binary state
			//if( mBinaryState )	return mBinaryState->mAttractorIgnoreIds.fNewArray( 0 );
		}

		void fAddAttractorIgnoreId( u32 id )
		{
			if( mToolState )	return mToolState->mAttractorIgnoreIds.fPushBack( id );

			// Can't modify binary state
			//if( mBinaryState )	return mBinaryState->mAttractorIgnoreIds.fPushBack( id );
		}

		u32 fAttractorIgnoreListCount( ) const
		{
			if( mBinaryState )	return mBinaryState->mAttractorIgnoreIds.fCount( );
			if( mToolState )	return mToolState->mAttractorIgnoreIds.fCount( );
			return 0;
		}

		u32 fAttractorIgnoreId( u32 idx ) const
		{
			if( mBinaryState )	return mBinaryState->mAttractorIgnoreIds[ idx ];
			if( mToolState )	return mToolState->mAttractorIgnoreIds[ idx ];
			return ~0;
		}

		template< class tSerializer >
		void fSerializeXmlObject( tSerializer& s )
		{
			sigassert( mToolState );

			s( "EmissionGraphs", mToolState->mEmissionGraphs );
			s( "PerParticleGraphs", mToolState->mPerParticleGraphs );
			s( "MeshGraphs", mToolState->mMeshGraphs );
			s( "SystemFlags", mToolState->mSystemFlags );
			s( "AttractorIgnoreIds", mToolState->mAttractorIgnoreIds );

			if( s.fIn( ) )
				fFixupAfterXmlLoad( );
		}

		void fAddFlag( u32 flag )
		{
			if( mToolState )
				mToolState->mSystemFlags |= flag;
		}
		void fRemoveFlag( u32 flag )
		{
			if( mToolState )
				mToolState->mSystemFlags &= ~flag;
		}
		b32 fHasFlag( u32 flag ) const
		{
			if( mBinaryState )
				return mBinaryState->mSystemFlags& flag;
			if( mToolState )
				return mToolState->mSystemFlags & flag;
			return false;
		}
	private:
		void fFixupAfterXmlLoad( );
	};

	typedef tRefCounterPtr< tState > tStatePtr;

}}

#endif //__tParticleSystemStates__

