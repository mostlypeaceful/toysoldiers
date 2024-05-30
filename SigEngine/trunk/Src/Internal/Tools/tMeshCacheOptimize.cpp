#include "ToolsPch.hpp"
#include "tMeshCacheOptimize.hpp"
#include "Gfx/tGeometryBufferSysRam.hpp"

using namespace Sig::Math;

namespace Sig
{

	//direct implementation of: http://home.comcast.net/~tom_forsyth/papers/fast_vert_cache_opt.html

	namespace
	{
		const u32 cSimulatedCacheSize = 32;
		const u32 cNotInCache = ~0;
		const f32 cCacheDecayPower = 1.5f;
		const f32 cLastTriScore = 0.75f;
		const f32 cValenceBoostScale = 2.0f;
		const f32 cValenceBoostPower = 0.5f;
	}

	tMeshCacheOptimize::tMeshCacheOptimize( Gfx::tGeometryBufferSysRam& verts, tDynamicArray< u32 >& triangleIndices, b32 reorderVerts, tGrowableArray< tDynamicArray< u32 > >& additionalIndexBuffersToReorder )
	{
		fBuildData( verts, triangleIndices );
		fIterateData( );
		fDumpData( verts, triangleIndices, reorderVerts, additionalIndexBuffersToReorder );
	}

	void tMeshCacheOptimize::fBuildData( const Gfx::tGeometryBufferSysRam& verts, const tDynamicArray< u32 >& triangleIndices )
	{
		mVertData.fSetCount( verts.fVertexCount( ) );
		mTriData.fSetCount( triangleIndices.fCount( ) / 3 );
		mCacheSim.fSetCapacity( cSimulatedCacheSize + 3 );
		mOrderedTriangleOutput.fSetCount( 0 );
		mOrderedTriangleOutput.fSetCapacity( mTriData.fCount( ) );

		sigassert( triangleIndices.fCount( ) % 3 == 0 );

		for( u32 i = 0; i < triangleIndices.fCount( ); ++i )
		{
			u32 triangle = i / 3;
			u32 index = i % 3;

			mTriData[ triangle ].mIndices[ index ] = triangleIndices[ i ];
			mVertData[ triangleIndices[ i ] ].fIncTriCount( );
		}

		for( u32 i = 0; i < mVertData.fCount( ); ++i )
			mVertData[ i ].fComputeScore( );

		f32 bestScore = -1;
		for( u32 i = 0; i < mTriData.fCount( ); ++i )
		{
			for( u32 a = 0; a < 3; ++ a )
				mVertData[ mTriData[ i ].mIndices[ a ] ].fAddTriangle( i );

			mTriData[ i ].fComputeScore( mVertData );

			if( mTriData[ i ].mScore > bestScore )
			{
				mNextBestTriangle = i;
				bestScore = mTriData[ i ].mScore;
			}
		}
	}

	void tMeshCacheOptimize::fUseVertex( u32 index, u32 forTri )
	{
		tVertexData& data = mVertData[ index ];
		sigassert( data.mNumToUse );

		// if it's already in, pull it out
		if( data.mCachePosition != cNotInCache )
			mCacheSim.fEraseOrdered( data.mCachePosition );

		// add it to front
		mCacheSim.fPushFront( index );

		// remove triangle from to use list.
		--data.mNumToUse;
		u32 triListIndex = data.mTriangleList.fIndexOf( forTri );
		sigassert( triListIndex != ~0 );
		fSwap( data.mTriangleList[ triListIndex ], data.mTriangleList[ data.mNumToUse ] );
	}

	void tMeshCacheOptimize::fCleanupCache( )
	{
		// update everyones score
		for( u32 i = 0; i < mCacheSim.fCount( ); ++i )
		{
			tVertexData& vData = mVertData[ mCacheSim[ i ] ];
			vData.mCachePosition = (i < cSimulatedCacheSize) ? i : cNotInCache;
			vData.fComputeScore( );
		}

		f32 bestScore = -1;
		mNextBestTriangle = ~0;
		for( u32 i = 0; i < mCacheSim.fCount( ); ++i )
		{
			const tVertexData& vData = mVertData[ mCacheSim[ i ] ];
			for( u32 x = 0; x < vData.mNumToUse; ++x )
			{
				tTriangleData& triData = mTriData[ vData.mTriangleList[ x ] ];
				triData.fComputeScore( mVertData );

				if( triData.mScore > bestScore )
				{
					sigassert( !triData.mUsed );
					mNextBestTriangle = vData.mTriangleList[ x ];
					bestScore = triData.mScore;
				}
			}
		}

		// shrink unused verts
		if( mCacheSim.fCount( ) > cSimulatedCacheSize )
			mCacheSim.fSetCount( cSimulatedCacheSize );

		// if we didnt find a best triangle, search for it.
		if( mNextBestTriangle == ~0 )
		{
			for( u32 i = 0; i < mTriData.fCount( ); ++i )
			{
				tTriangleData& triData = mTriData[ i ];
				if( triData.mScore > bestScore )
				{
					sigassert( !triData.mUsed );
					mNextBestTriangle = i;
					bestScore = triData.mScore;
				}				
			}
		}
	}

	void tMeshCacheOptimize::fUseTriangle( u32 index )
	{
		tTriangleData& triData = mTriData[ index ];
		sigassert( !triData.mUsed );

		mNextBestTriangle = ~0;
		mOrderedTriangleOutput.fPushBack( index );

		triData.mUsed = true;
		triData.mScore = -1.f;

		for( u32 i = 0; i < 3; ++i )
			fUseVertex( triData.mIndices[ i ], index );

		fCleanupCache( );
	}

	void tMeshCacheOptimize::fIterateData( )
	{
		while( mNextBestTriangle != ~0 )
			fUseTriangle( mNextBestTriangle );
	}

	// after the vertex buffer has been reordered.
	void tMeshCacheOptimize::fRemapIndexBuffer( tDynamicArray< u32 >& indices ) const
	{
		for( u32 i = 0; i < indices.fCount( ); ++i )
		{
			u32& oldIndex = indices[ i ];
			sigassert( mVertData[ oldIndex ].mNewIndex != ~0 );
			oldIndex = mVertData[ oldIndex ].mNewIndex;
		}
	}

	void tMeshCacheOptimize::fDumpData( Gfx::tGeometryBufferSysRam& verts, tDynamicArray< u32 >& triangleIndices, b32 reorderVerts, tGrowableArray< tDynamicArray< u32 > >& additionalIndexBuffersToReorder )
	{
		//first the tris
		sigassert( mOrderedTriangleOutput.fCount( ) * 3 == triangleIndices.fCount( ) );

		for( u32 i = 0; i < mOrderedTriangleOutput.fCount( ); ++i )
			for( u32 x = 0; x < 3; ++x )
				triangleIndices[ i * 3 + x ] = mTriData[ mOrderedTriangleOutput[ i ] ].mIndices[ x ];

		if( reorderVerts )
		{
			// now reorder the vertices so that they appear in the order they are needed
			Gfx::tGeometryBufferSysRam vertsCopy = verts;
			u32 vertOutPos = 0;

			for( u32 i = 0; i < triangleIndices.fCount( ); ++i )
			{
				u32 vIndex = triangleIndices[ i ];
				if( !mVertData[ vIndex ].fUsed( ) )
				{
					mVertData[ vIndex ].mNewIndex = vertOutPos;
					verts.fCopyVertex( vertsCopy, vIndex, vertOutPos++ );
				}
			}

			// update indexes to moved vertices
			fRemapIndexBuffer( triangleIndices );
			for( u32 i = 0; i < additionalIndexBuffersToReorder.fCount( ); ++i )
				fRemapIndexBuffer( additionalIndexBuffersToReorder[ i ] );
		}
	}





	// BORING STUFF

	tMeshCacheOptimize::tVertexData::tVertexData( )
		: mCachePosition( cNotInCache )
		, mCurrentScore( -1 )
		, mNewIndex( ~0 )
		, mNumToUse( 0 )
	{
	}

	void tMeshCacheOptimize::tVertexData::fIncTriCount( )
	{
		++mNumToUse;
	}

	void tMeshCacheOptimize::tVertexData::fAddTriangle( u32 index )
	{
		if( mTriangleList.fCapacity( ) != mNumToUse )
			mTriangleList.fSetCapacity( mNumToUse );

		mTriangleList.fPushBack( index );
	}

	void tMeshCacheOptimize::tTriangleData::fComputeScore( const tGrowableArray< tVertexData >& data )
	{
		if( mUsed )
			mScore = -1;
		else
		{
			mScore = 0.f;
			for( u32 i = 0; i < 3; ++i )
				mScore += data[ mIndices[ i ] ].mCurrentScore;
		}
	}

	void tMeshCacheOptimize::tVertexData::fComputeScore( )
	{
		if( mNumToUse == 0 )
		{
			// No tri needs this vertex!
			mCurrentScore = -1.0f;
			return;
		}

		mCurrentScore = 0.0f;
		if( mCachePosition == cNotInCache )
		{
			// Vertex is not in FIFO cache - no score.
		}
		else
		{
			if( mCachePosition < 3 )
			{
				// This vertex was used in the last triangle,
				// so it has a fixed score, whichever of the three
				// it's in. Otherwise, you can get very different
				// answers depending on whether you add
				// the triangle 1,2,3 or 3,1,2 - which is silly.
				mCurrentScore = cLastTriScore;
			}
			else
			{
				sigassert( mCachePosition < cSimulatedCacheSize );
				// Points for being high in the cache.
				const float Scaler = 1.0f / ( cSimulatedCacheSize - 3 );
				mCurrentScore = 1.0f - ( mCachePosition - 3 ) * Scaler;
				mCurrentScore = powf( mCurrentScore, cCacheDecayPower );
			}
		}

		// Bonus points for having a low number of tris still to
		// use the vert, so we get rid of lone verts quickly.
		f32 valenceBoost = powf( mNumToUse, -cValenceBoostPower );
		mCurrentScore += cValenceBoostScale * valenceBoost;
	}
	
}
