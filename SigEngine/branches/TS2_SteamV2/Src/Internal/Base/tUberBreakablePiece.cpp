#include "BasePch.hpp"
#include "tUberBreakablePiece.hpp"
#include "tUberBreakableLogic.hpp"
#include "tUberBreakableDebris.hpp"
#include "tSceneGraph.hpp"

namespace Sig
{
	tUberBreakablePiece::tUberBreakablePiece( const Math::tAabbf& objectSpaceBox )
		: tShapeEntity( objectSpaceBox, tShapeEntityDef::cShapeTypeBox )
		, mPositionFlags( 0 )
		, mHit( false )
		, mCollapseTimer( 0.f )
		, mDamageVelocity( Math::tVec3f::cZeroVector )
	{
		fAddGameTags( tUberBreakableLogic::cGameFlags | tUberBreakableLogic::cCollisionFlags | tUberBreakableLogic::cGroundFlags );
	}
	void tUberBreakablePiece::fReleaseReferences( )
	{
		mAdjacent.fSetCount( 0 );
		mAbove.fSetCount( 0 );
		mBelow.fSetCount( 0 );
		mMeshes.fSetCount( 0 );
	}
	void tUberBreakablePiece::fOnHit( const Math::tVec3f& damageVelocity )
	{
		if( mHit )
			return;

		mHit = true;
		mDamageVelocity = damageVelocity;

		for( u32 i = 0; i < mAbove.fCount( ); ++i )
			mAbove[ i ]->mCollapseTimer = 0.25f;
	}
	void tUberBreakablePiece::fHandleHit( f32 dt )
	{
		if( !mHit )
		{
			if( mCollapseTimer > 0.f )
			{
				mCollapseTimer -= dt;
				if( mCollapseTimer <= 0.f )
				{
					mHit = true;
					mCollapseTimer = 0.f;
					for( u32 i = 0; i < mAbove.fCount( ); ++i )
						mAbove[ i ]->mCollapseTimer = 0.25f;
				}
			}
			return;
		}
		for( u32 i = 0; i < mMeshes.fCount( ); ++i )
		{
			if( mMeshes[ i ]->mMesh )
			{
				sigassert( mMeshes[ i ]->mMesh->fStateIndex( ) == 0 );
				tUberBreakableDebris::fSpawnFromMesh( *mMeshes[ i ]->mMesh, fSceneGraph( )->fRootEntity( ), mDamageVelocity );
				mMeshes[ i ]->mMesh.fRelease( );
			}
		}
		mMeshes.fSetCount( 0 );
	}
	b32 tUberBreakablePiece::fWillBreakSoonDueToLackOfSupport( ) const
	{
		if( fIsBroken( ) )
			return false; // we consider this case false, though maybe it's better to consider it as true? ambiguous...

		for( u32 i = 0; i < mBelow.fCount( ); ++i )
		{
			if( mBelow[ i ]->fIsBroken( ) || mBelow[ i ]->fWillBreakSoonDueToLackOfSupport( ) )
				return true;
		}

		return false;
	}
	b32 tUberBreakablePiece::fDeleteIfEmpty( )
	{
		if( mMeshes.fCount( ) > 0 )
			return false; // still have meshes
		fRemoveGameTags( tUberBreakableLogic::cCollisionFlags | tUberBreakableLogic::cGameFlags | tUberBreakableLogic::cGroundFlags );
		fReleaseReferences( );
		fDelete( ); // remove from scene graph, etc
		return true;
	}
	Math::tRayf tUberBreakablePiece::fComputeProbeUp( ) const
	{
		return Math::tRayf( fBox( ).fCenter( ) + fBox( ).mExtents[ 1 ] * fBox( ).mAxes[ 1 ], +0.25f * Math::tVec3f::cYAxis );
	}
	Math::tRayf tUberBreakablePiece::fComputeProbeDown( ) const
	{
		return Math::tRayf( fBox( ).fCenter( ) - fBox( ).mExtents[ 1 ] * fBox( ).mAxes[ 1 ], -0.25f * Math::tVec3f::cYAxis );
	}
}
