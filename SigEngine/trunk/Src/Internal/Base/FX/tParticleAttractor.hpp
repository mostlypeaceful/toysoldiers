#ifndef __tParticleAttractor__
#define __tParticleAttractor__

#include "tParticle.hpp"
#include "tFxGraph.hpp"
#include "tEntityDef.hpp"


namespace Sig { namespace FX
{
	class tMeshParticle;
	class tOldMeshParticle;

	enum tForceType
	{
		cAttract,
		cRepel,
		cGravity,
		cPlaneCollide,
		cForceCount,
	};
	enum tAttractorGraphs
	{
		cAttractorMassGraph,
		cAttractorScaleGraph,
		cAttractorGravityGraph,
		cAttractorPositionGraph,		
		cAttractorParticleColorGraph,
		cAttractorParticleSizeGraph,
		cAttractorGraphCount,
	};
	enum tAttractorFlags
	{
		cUseWorldSpaceGravity,
		cAttractorFlagCount,
	};


	class base_export tBinaryAttractorData
	{
		declare_reflector( );
	public:

		tBinaryAttractorData( );
		tBinaryAttractorData( tNoOpTag );
		~tBinaryAttractorData( );

		template< class GraphType >
		typename GraphType::tType fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const GraphType* >( mGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ); 
		}

		template<>
		typename f32 fSampleGraph<tBinaryF32Graph>( const tMersenneGenerator* random, u32 idx, f32 x ) const
		{ 
			return static_cast< const tBinaryF32Graph* >( mGraphs[ idx ].fTypedPtr( ) )->fSampleGraphInternal( random, x ).x; 
		}

		tEnum< tForceType, u8 >									mType;
		u8														mPad[ 3 ];
		b32														mParticleMustBeInRadius;
		b32														mAffectParticlesDirection;
		u32														mId;
		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mGraphs;
		u32														mFlags;

		b32 fHasFlag( u32 flag ) const
		{
			return mFlags & flag;
		}

		u32 fId( ) const
		{
			return mId;
		}

		u32 fFlags( ) const
		{
			return mFlags;
		}

		b32 fAffectParticlesDirection( ) const
		{
			return mAffectParticlesDirection;
		}

		b32 fParticleMustBeInRadius( ) const
		{
			return mParticleMustBeInRadius;
		}

		tForceType fForceType( ) const
		{
			return mType;
		}
	};

	class base_export tToolAttractorData : public tRefCounter
	{
		// Why design any system where anything is hidden? Everything should be
		// open and public just like the internet!
	public:
		tForceType	mType;
		b32			mParticleMustBeInRadius;
		b32			mAffectParticlesDirection;
		u32			mId;
		u32			mFlags;
		tGrowableArray< tGraphPtr > mGraphs;

		static const tStringPtr mGraphNames[ cAttractorGraphCount ];
		static const tStringPtr mAttractorFlagNames[ cAttractorFlagCount ];
		static const Math::tVec3f mColours[ cAttractorGraphCount ];

		tToolAttractorData( tToolAttractorData* rhs, u32 id );
		tToolAttractorData( );
		~tToolAttractorData( );

		template<class t>
		t fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const { return mGraphs[ idx ]->fSample< t >( random, x ); }

		u32 fNumGraphs( ) const { return mGraphs.fCount( ); }
		tGraphPtr& fGraph( u32 idx ) { return mGraphs[ idx ]; }
		const tGraphPtr& fGraph( u32 idx ) const { return mGraphs[ idx ]; }

		void fUpdate( );

		tBinaryAttractorData fCreateBinaryData( );

		template< class tSerializer >
		void fSerializeXmlObject( tSerializer& s )
		{

			s( "Graphs", mGraphs );
			
			u32 type = ( u32 ) mType;
			s( "Type", type );
			mType = ( tForceType )type;

			s( "ParticleMustBeInRadius", mParticleMustBeInRadius );
			s( "AffectParticlesDirection", mAffectParticlesDirection );
			s( "ID", mId );
			s( "Flags", mFlags);
		}

		u32 fId( ) const { return mId; }
		u32 fFlags( ) const { return mFlags; }

		// It's like we have this functionality somewhere else in code???????
		b32	fHasFlag( u32 flag ) const { return mFlags & flag; }
		void fRemoveFlag( u32 flag ) { mFlags &= ~flag; }
		void fAddFlag( u32 flag ) { mFlags |= flag; }

		b32 fAffectParticlesDirection( ) const{ return mAffectParticlesDirection; }
		b32 fParticleMustBeInRadius( ) const { return mParticleMustBeInRadius; }

		void fSetForceType( FX::tForceType type ) { mType = type; }	
		void fSetAffectParticlesDirection( b32 b ) { mAffectParticlesDirection = b; }
		void fSetParticleMustBeInRadius( b32 b ) { mParticleMustBeInRadius = b; }

		tForceType fForceType( ) const { return mType; }
	};

	typedef tRefCounterPtr< tToolAttractorData > tToolAttractorDataPtr;


	class tAttractorData : public tRefCounter
	{
	public:
		tToolAttractorData* mToolData;
		tBinaryAttractorData* mBinaryData;

		tAttractorData( )
			: mToolData( 0 ), mBinaryData( 0 )
		{
		}

		tAttractorData( tToolAttractorData* tooldata, tBinaryAttractorData* binarydata )
			: mToolData( tooldata ), mBinaryData( binarydata )
		{
		}

		tAttractorData( const tAttractorData* data, u32 id )
			: mBinaryData( data->mBinaryData )
		{
			// only the tool version needs to be copyable like this!
			mToolData = NEW_TYPED( tToolAttractorData )( data->mToolData, id );
		}

		~tAttractorData( )
		{
			if( mToolData )		delete mToolData;
			//if( mBinaryData )	delete mBinaryData;
		}

		template< class tSerializer >
		void fSerializeXmlObject( tSerializer& s )
		{
			if( !mToolData )
				mToolData = NEW_TYPED( tToolAttractorData )( );

			s( "Graphs", mToolData->mGraphs );
			
			u32 type = ( u32 ) mToolData->mType;
			s( "Type", type );
			mToolData->mType = ( tForceType )type;

			s( "ParticleMustBeInRadius", mToolData->mParticleMustBeInRadius );
			s( "AffectParticlesDirection", mToolData->mAffectParticlesDirection );
			s( "ID", mToolData->mId );
			s( "Flags", mToolData->mFlags);
		}
	};

	typedef tRefCounterPtr< tAttractorData > tAttractorDataPtr;

	class base_export tParticleAttractorDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tParticleAttractorDef, 0x6D33C597 );
	
	public:
		tBinaryAttractorData		mBinaryData;
		tEnum< tForceType, u8 >		mType;
		u8							mPad[ 3 ];
		u32							mId;
		b32							mParticleMustBeInRadius;
		b32							mAffectParticlesDirection;
		tLoadInPlaceStringPtr*		mAttractorName;
		u32							mFlags;

	public:
		tParticleAttractorDef( );
		tParticleAttractorDef( tNoOpTag );
		~tParticleAttractorDef( );

		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;

		void fFromToolData( const tToolAttractorData* data );
	};


	class base_export tParticleAttractor : public tEntity 
	{
		define_dynamic_cast( tParticleAttractor, tEntity );

		tBinaryAttractorData mData;

		tStringPtr	 mAttractorName;
		Math::tVec4f mCurrentColor;
		f32			 mCurrentMass;
		f32			 mCurrentScale;
		f32			 mCurrentGravity;
		f32			 mCurrentParticleSizeScale;
		f32			 mScaleSquared;
		f32			 mDelta;

		Math::tVec3f mCurrentPosition;
		Math::tVec3f mLastPosition;

		tMersenneGenerator mRandomGenerator; 
		
	public:
		tParticleAttractor( );
		tParticleAttractor( const tParticleAttractorDef* def );
		virtual ~tParticleAttractor( );

		void fActOnParticle( f32 dt, tParticle* particle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const;
	private:
		f32  fActOnParticleInternal( f32 dt, tParticle* particle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const;
	public:
		void fActOnMeshParticle( f32 dt, tMeshParticle* particle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const;
		void fActOnOldMeshParticle( f32 dt, tOldMeshParticle* meshParticle, b32 localSpace, const Math::tMat3f& systemXform, const Math::tMat3f& invSystemXform ) const;

		//void fSetForceType( tForceType type ) { mData.fSetForceType( type ); }	
		//void fParticleMustBeInRadius( b32 b ) { mData.fSetParticleMustBeInRadius( b ); }
		//void fSetAffectParticlesDirection( b32 b ) { mData.fSetAffectParticlesDirection( b ); }

		const tMersenneGenerator* fRandomNumberGenerator( ) const { return &mRandomGenerator; }

		tForceType fForceType( ) const { return mData.fForceType( ); }
		b32 fParticleMustBeInRadius( ) const { return mData.fParticleMustBeInRadius( ); }
		b32 fAffectParticlesDirection( ) const { return mData.fAffectParticlesDirection( ); }
		u32 fId( ) const { return mData.fId( ); }
		u32 fFlags( ) const { return mData.fFlags( ); }

		b32	fHasFlag( u32 flag ) const { return mData.fHasFlag( flag ); }

		tBinaryAttractorData& fGetAttractorData( ) { return mData; }
		void fSetData( tBinaryAttractorData& data ) { mData = data; }
		void fUpdateGraphValues( const f32 delta );

		const tStringPtr& fAttractorName( ) const { return mAttractorName; }
		void fSetAttractorName( const tStringPtr& name ) { mAttractorName = name; }

		f32 fCurrentScale( ) const { return mCurrentScale; }
		f32 fCurrentMass( ) const { return mCurrentMass; }
		f32 fCurrentGravity( ) const { return mCurrentGravity; }
		Math::tVec3f fCurrentPosition( ) const { return mCurrentPosition; }
		Math::tVec3f fWorldPosition( ) const { return fObjectToWorld( ).fGetTranslation( ) + fObjectToWorld( ).fXformVector( mCurrentPosition ); }
		Math::tVec4f fCurrentColor( ) const { return mCurrentColor; }

		Math::tVec3f fGravityDir( ) const;

	private:
		void fCommonCtor( );
		void fSyncToBinary( );
	};

	typedef tRefCounterPtr< tParticleAttractor > tParticleAttractorPtr;
}}


#endif // __tParticleAttractor__

