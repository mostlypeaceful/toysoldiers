#ifndef __tParticleAttractor__
#define __tParticleAttractor__

#include "tParticle.hpp"
#include "tFxGraph.hpp"
#include "Gfx/tCamera.hpp"
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
		cUseWorldGravityDirection,
		cAttractorFlagCount,
	};


	class base_export tToolAttractorData : public tRefCounter
	{
	public:

		tToolAttractorData( tToolAttractorData* rhs, u32 id );
		tToolAttractorData( );
		~tToolAttractorData( );

		template<class t>
		t fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const { return mGraphs[ idx ]->fSample< t >( random, x ); }

		void fUpdate( );

	public:
		static const tStringPtr mGraphNames[ cAttractorGraphCount ];
		static const tStringPtr mAttractorFlagNames[ cAttractorFlagCount ];
		static const Math::tVec3f mColours[ cAttractorGraphCount ];
		
		tForceType	mType;
		b32			mParticleMustBeInRadius;
		b32			mAffectParticlesDirection;
		u32			mId;
		u32			mFlags;
		tGrowableArray< tGraphPtr > mGraphs;
	};

	typedef tRefCounterPtr< tToolAttractorData > tToolAttractorDataPtr;


	class base_export tBinaryAttractorData
	{
		declare_reflector( );
	public:

		tBinaryAttractorData( );
		tBinaryAttractorData( tNoOpTag );
		~tBinaryAttractorData( );

		template<class t>
		t fSampleGraph( const tMersenneGenerator* random, u32 idx, f32 x ) const { return mGraphs[ idx ]->fSample< t >( random, x ); }
		
		tEnum< tForceType, u8 >									mType;
		u8														mPad[ 3 ];
		b32														mParticleMustBeInRadius;
		b32														mAffectParticlesDirection;
		u32														mId;
		tDynamicArray< tLoadInPlacePtrWrapper< tBinaryGraph > > mGraphs;
		u32														mFlags;
	};



	template< class ToolData, class BinaryData >
	class tAttractorData : public tRefCounter
	{
		ToolData* mToolData;
		BinaryData* mBinaryData;
	public:

		tAttractorData( )
			: mToolData( 0 ), mBinaryData( 0 )
		{
		}

		tAttractorData( ToolData* tooldata, BinaryData* binarydata )
			: mToolData( tooldata ), mBinaryData( binarydata )
		{
		}

		tAttractorData( const tAttractorData< ToolData, BinaryData >* data, u32 id )
			: mBinaryData( data->mBinaryData )
		{
			// only the tool version needs to be copyable like this!
			mToolData = NEW ToolData( data->mToolData, id );
		}

		~tAttractorData( )
		{
			if( mToolData )		delete mToolData;
			//if( mBinaryData )	delete mBinaryData;
		}

		ToolData* fToolData( ) const { return mToolData; }
		BinaryData* fBinaryData( ) const { return mBinaryData; }

		void fSetForceType( tForceType type )
		{
			if( mToolData )		mToolData->mType = type;
			if( mBinaryData )	mBinaryData->mType = type;
		}
		tForceType fForceType( ) const
		{
			if( mToolData )		return mToolData->mType;
			if( mBinaryData )	return mBinaryData->mType;
			return cAttract;
		}
		void fSetFlags( u32 flags )
		{
			if( mToolData )		mToolData->mFlags = flags;
			if( mBinaryData )	mBinaryData->mFlags = flags;
		}
		void fSetParticleMustBeInRadius( b32 b )
		{
			if( mToolData )		mToolData->mParticleMustBeInRadius = b;
			if( mBinaryData )	mBinaryData->mParticleMustBeInRadius = b;
		}
		b32 fParticleMustBeInRadius( ) const
		{
			if( mToolData )		return mToolData->mParticleMustBeInRadius;
			if( mBinaryData )	return mBinaryData->mParticleMustBeInRadius;
			return false;
		}
		
		void fSetAffectParticlesDirection( b32 b )
		{
			if( mToolData )		mToolData->mAffectParticlesDirection = b;
			if( mBinaryData )	mBinaryData->mAffectParticlesDirection = b;
		}
		b32 fAffectParticlesDirection( ) const
		{
			if( mToolData )		return mToolData->mAffectParticlesDirection;
			if( mBinaryData )	return mBinaryData->mAffectParticlesDirection;
			return false;
		}
		u32 fId( ) const
		{
			if( mToolData )		return mToolData->mId;
			if( mBinaryData )	return mBinaryData->mId;
			return -1;
		}
		u32 fFlags( ) const
		{
			if( mToolData )		return mToolData->mFlags;
			if( mBinaryData )	return mBinaryData->mFlags;
			return 0;
		}
		
		void fAddFlag( u32 flag )
		{
			if( mToolData )
				mToolData->mFlags |= flag;
		}
		void fRemoveFlag( u32 flag )
		{
			if( mToolData )
				mToolData->mFlags &= ~flag;
		}
		b32 fHasFlag( u32 flag ) const
		{
			if( mBinaryData )
				return mBinaryData->mFlags & flag;
			if( mToolData )
				return mToolData->mFlags & flag;
			return false;
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

		template< class tSerializer >
		void fSerializeXmlObject( tSerializer& s )
		{
			if( !mToolData )
				mToolData = NEW ToolData( );

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

	namespace Attractors
	{
		typedef tAttractorData< tToolAttractorData, tBinaryAttractorData > tData;
		typedef tRefCounterPtr< tData > tAttractorDataPtr;
		const int tNullBinaryData = 0;
		const int tNullToolData = 0;
	}

	class base_export tParticleAttractorDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tParticleAttractorDef, 0x6D33C597 );
	
	public:
		tLoadInPlacePtrWrapper< tBinaryAttractorData >	mBinaryData;
		tEnum< tForceType, u8 >							mType;
		u8												mPad[ 3 ];
		u32												mId;
		b32												mParticleMustBeInRadius;
		b32												mAffectParticlesDirection;
		tLoadInPlaceStringPtr*							mAttractorName;
		u32												mFlags;

	public:
		tParticleAttractorDef( );
		tParticleAttractorDef( tNoOpTag );
		~tParticleAttractorDef( );

		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;

		void fFromToolData( const tToolAttractorData* data );
	};


	class base_export tParticleAttractor : public tEntity 
	{
		define_dynamic_cast( tParticleAttractor, tEntity );
	private:

		Attractors::tAttractorDataPtr mData;

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

		void fSetForceType( tForceType type ) { mData->fSetForceType( type ); }	
		void fParticleMustBeInRadius( b32 b ) { mData->fSetParticleMustBeInRadius( b ); }
		void fSetAffectParticlesDirection( b32 b ) { mData->fSetAffectParticlesDirection( b ); }

		const tMersenneGenerator* fRandomNumberGenerator( ) const { return &mRandomGenerator; }

		tForceType fForceType( ) const { return mData->fForceType( ); }
		b32 fParticleMustBeInRadius( ) const { return mData->fParticleMustBeInRadius( ); }
		b32 fAffectParticlesDirection( ) const { return mData->fAffectParticlesDirection( ); }
		u32 fId( ) const { return mData->fId( ); }
		u32 fFlags( ) const { return mData->fFlags( ); }

		void fAddFlag( u32 flag ) { mData->fAddFlag( flag ); }
		void fRemoveFlag( u32 flag ) { mData->fRemoveFlag( flag ); }
		b32	fHasFlag( u32 flag ) const { return mData->fHasFlag( flag ); }

		Attractors::tAttractorDataPtr fGetAttractorData( ) const { return mData; }
		void fSetData( Attractors::tAttractorDataPtr data ) { mData = data; }
		void fUpdateGraphValues( const f32 delta );

		const tStringPtr& fAttractorName( ) const { return mAttractorName; }
		void fSetAttractorName( const tStringPtr& name ) { mAttractorName = name; }

		f32 fCurrentScale( ) const { return mCurrentScale; }
		f32 fCurrentMass( ) const { return mCurrentMass; }
		f32 fCurrentGravity( ) const { return mCurrentGravity; }
		Math::tVec3f fCurrentPosition( ) const { return mCurrentPosition; }
		Math::tVec3f fWorldPosition( ) const { return fObjectToWorld( ).fGetTranslation( ) + fObjectToWorld( ).fXformVector( mCurrentPosition ); }
		Math::tVec4f fCurrentColor( ) const { return mCurrentColor; }

	private:
		void fCommonCtor( );
	};

	typedef tRefCounterPtr< tParticleAttractor > tParticleAttractorPtr;
}}


#endif // __tParticleAttractor__

