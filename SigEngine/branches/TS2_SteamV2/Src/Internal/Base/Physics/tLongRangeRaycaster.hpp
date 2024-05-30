#ifndef __tLongRangeRaycaster__
#define __tLongRangeRaycaster__
#include "tSceneGraph.hpp"
#include "tSpatialEntity.hpp"

namespace Sig { namespace Physics
{

	// For raycasting a long distance, like all the way across a level.
	//  It tries to do as short of a raycast as possible. If it fails it does the rest of the raycast.
	struct tLongRangeRaycaster
	{
	public:
		Math::tRayf mRay;
		f32			mMaxDistance;
		f32			mLastHitDistance;

		// For last hit distance, include some overage to make sure we at least get the last hit and any close farther ones.
		//  +25 or so, max distance the entity could move + farther stragglers.
		explicit tLongRangeRaycaster( const Math::tRayf& ray, f32 maxLevelExtentDistance, f32 lastHitDistance )
			: mRay( ray )
			, mMaxDistance( maxLevelExtentDistance )
			, mLastHitDistance( lastHitDistance ) 
		{
			mRay.mExtent.fSetLength( mLastHitDistance );
		}

		template< typename callbackT >
		f32 fRayCast( tSceneGraph& sg, const callbackT& callback )
		{
			sg.fRayCastAgainstRenderable( mRay, callback );
			if( callback.mHit.fHit( ) )
			{
				return mLastHitDistance * callback.mHit.mT;
			}
			else
			{
				// compute remainder of ray and try again.
				callback.mHit = tRayCastHit( );

				f32 newLen = mMaxDistance - mLastHitDistance;
				mRay.mOrigin += mRay.mExtent;
				mRay.mExtent.fSetLength( newLen );

				sg.fRayCastAgainstRenderable( mRay, callback );
				if( callback.mHit.fHit( ) )
				{
					return mLastHitDistance + newLen * callback.mHit.mT;
				}
			}

			return 1.f; //safe number for ray length
		}
	};

}}

#endif//__tLongRangeRaycaster__
