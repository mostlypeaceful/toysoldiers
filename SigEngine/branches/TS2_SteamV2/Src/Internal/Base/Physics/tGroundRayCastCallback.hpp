#ifndef __tGroundRayCastCallback__
#define __tGroundRayCastCallback__
#include "tSceneGraph.hpp"
#include "tSpatialEntity.hpp"

namespace Sig { namespace Physics
{
	struct tGroundRayCastCallback
	{
	public:
		static b32 cShapesEnabledAsGround;
	public:
		mutable Math::tRayCastHit	mHit;
		mutable tEntity*			mFirstEntity;
		tEntity*					mIgnoreEntity;
		const tEntityTagMask		mGroundMask;

		explicit tGroundRayCastCallback( tEntity& ignore, tEntityTagMask groundMask ) 
			: mFirstEntity( 0 )
			, mIgnoreEntity( &ignore )
			, mGroundMask( groundMask )
		{
		}

		inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
		{
			if( i->fQuickRejectByFlags( ) )
				return;
			tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );
			if( !spatial->fHasGameTagsAny( mGroundMask ) )
				return;
			if( spatial == mIgnoreEntity || spatial->fIsAncestorOfMine( *mIgnoreEntity ) )
				return;
			if( i->fQuickRejectByBox( ray ) )
				return;

			Math::tRayCastHit hit;
			spatial->fRayCast( ray, hit );
			if( hit.fHit( ) && hit.mT < mHit.mT )
			{
				mHit			= hit;
				mFirstEntity	= spatial;
			}
		}
		void fRayCast( tSceneGraph& sg, const Math::tRayf& ray );
	};

	struct tGroundRayCastCallbackMultipleIgnore
	{
	public:
		static b32 cShapesEnabledAsGround;
	public:
		mutable Math::tRayCastHit	mHit;
		mutable tEntity*			mFirstEntity;
		tGrowableArray<tEntity*>	mIgnoreEntities;
		const tEntityTagMask		mGroundMask;

		explicit tGroundRayCastCallbackMultipleIgnore( tEntityTagMask groundMask ) 
			: mFirstEntity( 0 )
			, mGroundMask( groundMask )
		{
		}

		inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
		{
			if( i->fQuickRejectByFlags( ) )
				return;
			tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );
			if( !spatial->fHasGameTagsAny( mGroundMask ) )
				return;

			for( u32 ig = 0; ig < mIgnoreEntities.fCount( ); ++ig )
			{
				if( spatial == mIgnoreEntities[ ig ] || spatial->fIsAncestorOfMine( *mIgnoreEntities[ ig ] ) )
					return;
			}

			if( i->fQuickRejectByBox( ray ) )
				return;

			Math::tRayCastHit hit;
			spatial->fRayCast( ray, hit );
			if( hit.fHit( ) && hit.mT < mHit.mT )
			{
				mHit			= hit;
				mFirstEntity	= spatial;
			}
		}
		void fRayCast( tSceneGraph& sg, const Math::tRayf& ray );
	};
}}

#endif//__tGroundRayCastCallback__
