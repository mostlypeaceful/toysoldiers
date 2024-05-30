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

	///
	/// \class tBinaryParticleSystemState
	/// \brief Binary state info for particles.
	class base_export tBinaryParticleSystemState
	{
		declare_reflector( );
	public:

		tBinaryParticleSystemState( );
		tBinaryParticleSystemState( tNoOpTag );
		~tBinaryParticleSystemState( );

		template< class GraphType >
		typename GraphType::tType fSampleEmissionGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const GraphType* >( mEmissionGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ); 
		}

		template<>
		typename f32 fSampleEmissionGraph<tBinaryF32Graph>( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const tBinaryF32Graph* >( mEmissionGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ).x; 
		}

		template< class GraphType >
		typename GraphType::tType fSamplePerParticleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const GraphType* >( mPerParticleGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ); 
		}

		template<>
		typename f32 fSamplePerParticleGraph<tBinaryF32Graph>( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const tBinaryF32Graph* >( mPerParticleGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ).x; 
		}

		template< class GraphType >
		typename GraphType::tType fSampleMeshGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const GraphType* >( mMeshGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ); 
		}

		template<>
		typename f32 fSampleMeshGraph<tBinaryF32Graph>( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const tBinaryF32Graph* >( mMeshGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ).x; 
		}

		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mEmissionGraphs;
		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mPerParticleGraphs;
		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mMeshGraphs;
		tDynamicArray< u32 >									mAttractorIgnoreIds;
		u32														mSystemFlags;

		u32 fAttractorIgnoreListCount( ) const
		{
			return mAttractorIgnoreIds.fCount( );
		}

		u32 fAttractorIgnoreId( u32 idx ) const
		{
			return mAttractorIgnoreIds[ idx ];
		}

		b32 fHasFlag( u32 flag ) const
		{
			return mSystemFlags & flag;
		}
	};

	///
	/// \class tToolParticleSystemState
	/// \brief Editor-side info for particles
	class base_export tToolParticleSystemState : public tRefCounter
	{
	public:
		tGrowableArray< tGraphPtr > mEmissionGraphs;
		tGrowableArray< tGraphPtr > mPerParticleGraphs;
		tGrowableArray< tGraphPtr > mMeshGraphs;
		tGrowableArray< u32 >		mAttractorIgnoreIds;
		u32							mSystemFlags;

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

		tToolParticleSystemState( );
		tToolParticleSystemState( const tToolParticleSystemState& rhs );
		~tToolParticleSystemState( );

		tBinaryParticleSystemState fCreateBinaryState( ) const;

		template< class t >
		t fSampleEmissionGraph( const tMersenneGenerator* random, const u32 idx, f32 x ) const { return mEmissionGraphs[ idx ]->fSample< t >( random, x ); }
		
		template< class t >
		t fSamplePerParticleGraph( const tMersenneGenerator* random, const u32 idx, f32 x ) const{ return mPerParticleGraphs[ idx ]->fSample< t >( random, x ); }

		template< class t >
		t fSampleMeshGraph( const tMersenneGenerator* random, const u32 idx, f32 x ) const{ return mMeshGraphs[ idx ]->fSample< t >( random, x ); }

		void fClearAttractorIgnoreIds( )
		{
			return mAttractorIgnoreIds.fSetCount( 0 );
		}

		void fAddAttractorIgnoreId( u32 id )
		{
			mAttractorIgnoreIds.fPushBack( id );
		}

		u32 fAttractorIgnoreListCount( ) const
		{
			return mAttractorIgnoreIds.fCount( );
		}

		u32 fAttractorIgnoreId( u32 idx ) const
		{
			return mAttractorIgnoreIds[ idx ];
		}

		template< class tSerializer >
		void fSerializeXmlObject( tSerializer& s )
		{
			s( "EmissionGraphs", mEmissionGraphs );
			s( "PerParticleGraphs", mPerParticleGraphs );
			s( "MeshGraphs", mMeshGraphs );
			s( "SystemFlags", mSystemFlags );
			s( "AttractorIgnoreIds", mAttractorIgnoreIds );
		}

		void fAddFlag( u32 flag )
		{
			mSystemFlags |= flag;
		}
		void fRemoveFlag( u32 flag )
		{
			mSystemFlags &= ~flag;
		}
		b32 fHasFlag( u32 flag ) const
		{
			return mSystemFlags & flag;
		}
	};

	typedef tRefCounterPtr< tToolParticleSystemState > tToolParticleSystemStatePtr;


	//------------------------------------------------------------------------------
	// Deprecated but supported
	//------------------------------------------------------------------------------
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
	private:
		void fFixupAfterXmlLoad( );
	};

	typedef tRefCounterPtr< tState > tStatePtr;
}}

#endif //__tParticleSystemStates__

