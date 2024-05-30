#include "BasePch.hpp"
#include "tUberBreakableClump.hpp"
#include "tUberBreakableLogic.hpp"
#include "tSceneGraph.hpp"

namespace Sig
{
	tUberBreakableClump::tUberBreakableClump( tEntity& ownerEntity, const Math::tAabbf& objSpaceBounds, const Math::tVec3u& numSegments )
		: mOwnerEntity( &ownerEntity )
		, mObjSpaceBounds( objSpaceBounds )
		, mNumSegments( numSegments )
		, mCurrentColorizedPiece( 0 )
		, mHasBroke( false )
	{
	}
	void tUberBreakableClump::fComputeSubdivision( const tGrowableArray<tBreakMeshRefPtr>& meshes )
	{
		fCollectPieces( );
		fComputeAdjacentPieces( );
		fAssignMeshesToPieces( meshes );
		fRemoveEmptyPieces( );
	}
	void tUberBreakableClump::fLinkClumps( const tGrowableArray<tUberBreakableClumpPtr>& clumps )
	{
		for( u32 i = 0; i < mPieces.fCount( ); ++i )
		{
			if( mPieces[ i ]->mPositionFlags & tUberBreakablePiece::cPosTop )
				fLinkClumps( *mPieces[ i ], true, clumps );
			if( mPieces[ i ]->mPositionFlags & tUberBreakablePiece::cPosBottom )
				fLinkClumps( *mPieces[ i ], false, clumps );
		}
	}
	void tUberBreakableClump::fOnDelete( )
	{
		for( u32 i = 0; i < mPieces.fCount( ); ++i )
			mPieces[ i ]->fReleaseReferences( );
		mPieces.fSetCount( 0 );
		mOwnerEntity.fRelease( );
	}
	void tUberBreakableClump::fThinkST( f32 dt )
	{
		if( mPieces.fCount( ) )
			fReleaseHitPieces( dt );
	}
	void tUberBreakableClump::fReleaseHitPieces( f32 dt )
	{
		tUberBreakableLogic* uberBreakableOwner = mOwnerEntity->fLogicDerived< tUberBreakableLogic >( );
		sigassert( uberBreakableOwner );

		for( u32 i = 0; i < mPieces.fCount( ); ++i )
		{
			tUberBreakablePiece& piece = *mPieces[ i ];
			if( piece.fSceneGraph( ) )
			{
				if( piece.mHit )
				{
					uberBreakableOwner->fOnBreak( );

					if( uberBreakableOwner->fDamageable( ) )
					{
						uberBreakableOwner->fDamageable( )->fOnCellBroken( *uberBreakableOwner, piece );
						if( !mHasBroke )
							uberBreakableOwner->fDamageable( )->fOnClumpBroken( *uberBreakableOwner, *this );
					}

					mHasBroke = true;
				}
				piece.fHandleHit( dt );
				piece.fDeleteIfEmpty( );
			}
			else
			{
				mPieces[ i ]->fReleaseReferences( );
				mPieces[ i ]->fDelete( );
				mPieces.fErase( i );
				--i;
			}
		}
	}
	void tUberBreakableClump::fIntersect( const Math::tSpheref& worldSphere, tGrowableArray<tUberBreakablePiece*>& pieces )
	{
		for( u32 i = 0; i < mPieces.fCount( ); ++i )
		{
			if( mPieces[ i ]->fIsBroken( ) )
				continue; // dead piece
			if( mPieces[ i ]->fIntersects( worldSphere ) )
				pieces.fPushBack( mPieces[ i ].fGetRawPtr( ) );
		}
	}
	void tUberBreakableClump::fQueryBrokenPieceCounts( u32& numPiecesBroken, u32& numPiecesTotal, b32 includePiecesThatWillBreak )
	{
		numPiecesTotal += mPieces.fCount( );
		for( u32 i = 0; i < mPieces.fCount( ); ++i )
		{
			if( mPieces[ i ]->fIsBroken( ) )
				++numPiecesBroken;
			else if( includePiecesThatWillBreak )
			{
				// go through pieces below me to see if anyone is broken or breaking
				if( mPieces[ i ]->fWillBreakSoonDueToLackOfSupport( ) )
					++numPiecesBroken;
			}
		}
	}
	u32 tUberBreakableClump::fLinearPieceIndex( u32 x, u32 y, u32 z ) const
	{
		return x * ( mNumSegments.y * mNumSegments.z ) + y * ( mNumSegments.z ) + z;
	}
	void tUberBreakableClump::fCollectPieces( )
	{
		const Math::tMat3f objToWorld = mOwnerEntity->fObjectToWorld( );

		for( u32 x = 0; x < mNumSegments.x; ++x )
		{
			const f32 tx = ( x / ( mNumSegments.x - 0.f ) );
			const f32 dx = ( 1 / ( mNumSegments.x - 0.f ) );
			for( u32 y = 0; y < mNumSegments.y; ++y )
			{
				const f32 ty = ( y / ( mNumSegments.y - 0.f ) );
				const f32 dy = ( 1 / ( mNumSegments.y - 0.f ) );
				for( u32 z = 0; z < mNumSegments.z; ++z )
				{
					const f32 tz = ( z / ( mNumSegments.z - 0.f ) );
					const f32 dz = ( 1 / ( mNumSegments.z - 0.f ) );

					const Math::tAabbf pieceAabb = mObjSpaceBounds.fSubAabb( Math::tVec3f( tx, ty, tz ), Math::tVec3f( tx + dx, ty + dy, tz + dz ) );
					tUberBreakablePiecePtr piece( NEW tUberBreakablePiece( pieceAabb ) );

					if( tUberBreakableLogic::cAttachPieceLogic )
					{
						tUberBreakblePieceLogic *dl = NEW tUberBreakblePieceLogic;
						tLogicPtr *dlp = NEW tLogicPtr( dl );
						piece->fAcquireLogic( dlp );
					}

					if( y == 0 )
						piece->mPositionFlags |= tUberBreakablePiece::cPosBottom;
					if( y == mNumSegments.y - 1 )
						piece->mPositionFlags |= tUberBreakablePiece::cPosTop;

					piece->fSpawnImmediate( *mOwnerEntity );
					if( tUberBreakableLogic::cAcquireProperties )
						piece->fAcquirePropertiesFromAncestors( );

					mPieces.fPushBack( piece );
				}
			}
		}
	}
	void tUberBreakableClump::fComputeAdjacentPieces( )
	{
		const Math::tMat3f objToWorld = mOwnerEntity->fObjectToWorld( );

		// for now, hard-coding sub-division
		for( u32 x = 0; x < mNumSegments.x; ++x )
		{
			for( u32 y = 0; y < mNumSegments.y; ++y )
			{
				for( u32 z = 0; z < mNumSegments.z; ++z )
				{
					const u32 pieceIndex = fLinearPieceIndex( x, y, z );
					tUberBreakablePiecePtr piece = mPieces[ pieceIndex ];

					if( mNumSegments.y > 1 )
					{
						if( !fTestBits( piece->mPositionFlags, tUberBreakablePiece::cPosBottom ) )
							piece->mBelow.fPushBack( mPieces[ fLinearPieceIndex( x, y - 1, z ) ] );
						if( !fTestBits( piece->mPositionFlags, tUberBreakablePiece::cPosTop ) )
							piece->mAbove.fPushBack( mPieces[ fLinearPieceIndex( x, y + 1, z ) ] );
					}

					if( mNumSegments.x > 1 )
					{
						if( x == 0 )
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x + 1, y, z ) ] );
						else if( x == mNumSegments.x - 1 )
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x - 1, y, z ) ] );
						else
						{
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x + 1, y, z ) ] );
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x - 1, y, z ) ] );
						}
					}

					if( mNumSegments.z > 1 )
					{
						if( z == 0 )
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x, y, z + 1 ) ] );
						else if( z == mNumSegments.z - 1 )
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x, y, z - 1 ) ] );
						else
						{
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x, y, z + 1 ) ] );
							piece->mAdjacent.fPushBack( mPieces[ fLinearPieceIndex( x, y, z - 1 ) ] );
						}
					}
				}
			}
		}
	}
	void tUberBreakableClump::fAssignMeshesToPieces( const tGrowableArray<tBreakMeshRefPtr>& meshes )
	{
		for( u32 i = 0; i < mPieces.fCount( ); ++i )
		{
			tUberBreakablePiece& piece = *mPieces[ i ];
			for( u32 j = 0; j < meshes.fCount( ); ++j )
			{
				tMeshEntity* mesh = meshes[ j ]->mMesh.fGetRawPtr( );
				if( mesh->fStateIndex( ) == 0 && Math::tIntersectionAabbObb<f32>( mesh->fWorldSpaceBox( ), piece.fBox( ) ).fIntersects( ) )
				{
					if( tUberBreakableLogic::cAcquireProperties )
						mesh->fAcquirePropertiesFromAncestors( );
					piece.mMeshes.fPushBack( meshes[ j ] );
				}
			}
		}
	}
	void tUberBreakableClump::fRemoveEmptyPieces( )
	{
		for( u32 i = 0; i < mPieces.fCount( ); ++i )
			mPieces[ i ]->fDeleteIfEmpty( ); // just deletes from scene graph, still preserve pointer in array
	}
	void tUberBreakableClump::fLinkClumps( tUberBreakablePiece& piece, b32 up, const tGrowableArray<tUberBreakableClumpPtr>& clumps )
	{
		const Math::tRayf ray = up ? piece.fComputeProbeUp( ) : piece.fComputeProbeDown( );
		tUberBreakablePiecePtr piecePtr = tUberBreakablePiecePtr( &piece );

		for( u32 i = 0; i < clumps.fCount( ); ++i )
		{
			if( clumps[ i ] == this )
				continue;

			tUberBreakableClump& clump = *clumps[ i ];
			for( u32 j = 0; j < clump.mPieces.fCount( ); ++j )
			{
				Math::tRayCastHit hit;
				clump.mPieces[ j ]->fRayCast( ray, hit );
				if( hit.fHit( ) )
				{
					if( up )
					{
						clump.mPieces[ j ]->mBelow.fFindOrAdd( piecePtr );
						piece.mAbove.fFindOrAdd( clump.mPieces[ j ] );
					}
					else
					{
						clump.mPieces[ j ]->mAbove.fFindOrAdd( piecePtr );
						piece.mBelow.fFindOrAdd( clump.mPieces[ j ] );
					}
				}
			}
		}
	}
	b32 tUberBreakableClump::fStepDebugRendering( f32 dt )
	{
#ifdef sig_devmenu
		if( mCurrentColorizedPiece < mPieces.fCount( ) )
		{
			for( u32 i = 0; i < mPieces[ mCurrentColorizedPiece ]->mMeshes.fCount( ); ++i )
				mPieces[ mCurrentColorizedPiece ]->mMeshes[ i ]->mMesh->fSetRgbaTint( Math::tVec4f::cOnesVector );
		}

		++mCurrentColorizedPiece;
		if( mCurrentColorizedPiece >= mPieces.fCount( ) )
		{
			mCurrentColorizedPiece = 0;
			return true;
		}
#endif//sig_devmenu
		return false;
	}
#ifdef sig_devmenu
	namespace
	{
		static void fRenderPiece( tUberBreakablePiece& piece, const Math::tVec4f& color )
		{
			if( piece.fSceneGraph( ) )
				piece.fSceneGraph( )->fDebugGeometry( ).fRenderOnce( piece.fBox( ), color );
		}
	}
#endif//sig_devmenu
	void tUberBreakableClump::fRenderDebug( )
	{
#ifdef sig_devmenu
		if( mCurrentColorizedPiece < mPieces.fCount( ) && mPieces[ mCurrentColorizedPiece ]->fSceneGraph( ) )
		{
			Math::tVec4f color = Math::tVec4f( 0.f, 1.f, 0.f, 0.25f );
			if( mPieces[ mCurrentColorizedPiece ]->mPositionFlags & tUberBreakablePiece::cPosBottom )
				color = Math::tVec4f( 0.f, 0.f, 0.f, 0.75f );
			else if( mPieces[ mCurrentColorizedPiece ]->mPositionFlags & tUberBreakablePiece::cPosTop )
				color = Math::tVec4f( 1.f, 1.f, 1.f, 0.75f );
			fRenderPiece( *mPieces[ mCurrentColorizedPiece ], color );
			for( u32 i = 0; i < mPieces[ mCurrentColorizedPiece ]->mAdjacent.fCount( ); ++i )
				fRenderPiece( *mPieces[ mCurrentColorizedPiece ]->mAdjacent[ i ], Math::tVec4f( 1.f, 0.f, 1.f, 0.25f ) );
			for( u32 i = 0; i < mPieces[ mCurrentColorizedPiece ]->mAbove.fCount( ); ++i )
				fRenderPiece( *mPieces[ mCurrentColorizedPiece ]->mAbove[ i ], Math::tVec4f( 1.f, 0.f, 0.f, 0.25f ) );
			for( u32 i = 0; i < mPieces[ mCurrentColorizedPiece ]->mBelow.fCount( ); ++i )
				fRenderPiece( *mPieces[ mCurrentColorizedPiece ]->mBelow[ i ], Math::tVec4f( 0.f, 0.f, 1.f, 0.25f ) );
			if( mPieces[ mCurrentColorizedPiece ]->fSceneGraph( ) )
			{
				for( u32 i = 0; i < mPieces[ mCurrentColorizedPiece ]->mMeshes.fCount( ); ++i )
					mPieces[ mCurrentColorizedPiece ]->mMeshes[ i ]->mMesh->fSetRgbaTint( Math::tVec4f( color.fXYZ( ), 1.0f ) );
			}
		}
#endif//sig_devmenu
	}
}
