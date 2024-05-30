#ifndef __tParticle__
#define __tParticle__

#include "Memory/tPool.hpp"
#include "Threads/tMutex.hpp"

namespace Sig { namespace FX
{
	enum tEmissionGraphs
	{
		cEmissionRateGraph,
		cSpawnYawGraph,
		cSpawnEnergyGraph,
		cSpawnSizeGraph,
		cEmitterTranslationGraph,
		cRotationGraph,
		cEmitterScaleGraph,
		cParticleLifeGraph,
		cEmissionGraphCount,
	};
	enum tParticleGraphs
	{
		cScaleGraph,
		cSpinGraph,
		cEnergyGraph,
		cColorGraph,
		cParticleGraphCount,
	};
	enum tMeshGraphs
	{
		cMeshInitialAxis,
		cMeshAxisDelta,
		cMeshGraphCount
	};
	enum tParticleSortMode
	{
		cNoSort,
		cDistanceSort,
		cFirstBornSort,
		cLastBornSort,
		cAlphaSort,
		cParticleSortCount,
	};


	class base_export tParticle
	{
		define_class_pool_new_delete_mt( tParticle, 5 * 1024 );

	public:

		tParticle( f32 lifetime, f32 curLife, f32 yaw, const Math::tVec2f& scale, const Math::tVec3f& pos, const Math::tVec3f& dir, const Math::tVec4f& color )
			: mLifetime( lifetime )
			, mCurrentLife( curLife )
			, mPosition( pos )
			, mDirection( dir )
			, mScale0( scale )
			, mYaw0( yaw )
			, mEnergy0( 0.f )
			, mCurrentColor( color )
		{

		}

		tParticle( )
		{
			
		}

		f32				mLifetime;			// 4
		f32				mCurrentLife;		// 4
		f32				mYaw0;				// 4
		f32				mEnergy0;			// 4
		
		Math::tVec2f	mScale0;			// 8

		Math::tVec3f	mPosition;			// 12
		Math::tVec3f	mDirection;			// 12

		Math::tVec4f	mCurrentColor;		// 16
											// 64
	};


		// SORTING MODES!

		struct tDistanceSort
		{
			Math::tVec3f mCameraPosition;
			tDistanceSort( const Math::tVec3f campos )
				: mCameraPosition( campos )		{	}

			inline b32 operator( )( const tParticle* a, const tParticle* b ) const
			{
				const Math::tVec3f sub1 = a->mPosition - mCameraPosition;
				const Math::tVec3f sub2 = b->mPosition - mCameraPosition;
				return sub1.fLengthSquared( ) < sub2.fLengthSquared( );
			}
		};

		struct tAlphaSort
		{
			inline b32 operator( )( const tParticle* a, const tParticle* b ) const
			{
				return a->mCurrentColor.w > b->mCurrentColor.w;
			}
		};

		struct tFirstBornSort
		{
			inline b32 operator( )( const tParticle* a, const tParticle* b ) const
			{
				return a->mCurrentLife > b->mCurrentLife;
			}
		};

		struct tLastBornSort
		{
			inline b32 operator( )( const tParticle* a, const tParticle* b ) const
			{
				return a->mCurrentLife < b->mCurrentLife;
			}
		};
}}

#endif	// __tParticle__

