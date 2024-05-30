#ifndef __tGroundRayCastCallback__
#define __tGroundRayCastCallback__
#include "tSceneGraph.hpp"
#include "tSpatialEntity.hpp"

namespace Sig { namespace Physics
{
	struct tGroundRayCastCallback
	{
	public:
		base_export static b32 cShapesEnabledAsGround;
	public:
		mutable Math::tRayCastHit	mHit;
		mutable tEntity*			mFirstEntity;
		tEntity*					mIgnoreEntity;
		const tEntityTagMask		mGroundMask;
		const tEntityTagMask		mIgnoreMask;

		explicit tGroundRayCastCallback( tEntity& ignore, tEntityTagMask groundMask, tEntityTagMask ignoreMask = 0 ) 
			: mFirstEntity( 0 )
			, mIgnoreEntity( &ignore )
			, mGroundMask( groundMask )
			, mIgnoreMask( ignoreMask )
		{
		}

		inline void operator()( const Math::tRayf& ray, tEntityBVH::tObjectPtr i ) const
		{
			if( i->fQuickRejectByFlags( ) )
				return;
			tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i->fOwner( ) );
			if( spatial->fHasGameTagsAny(  mIgnoreMask ) )
				return;
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
		base_export void fRayCast( tSceneGraph& sg, const Math::tRayf& ray );
	};

	struct base_export tGroundRayCastCallbackMultipleIgnore
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
			tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i->fOwner( ) );
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
