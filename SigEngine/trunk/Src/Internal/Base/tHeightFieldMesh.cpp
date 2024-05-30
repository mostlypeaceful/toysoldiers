#include "BasePch.hpp"
#include "tHeightFieldMesh.hpp"

namespace Sig
{
	///
	/// N.B.! triangle vertex/index ordering of the vertices in a quad depends on whether the
	/// the quad is "flipped" or "normal". This diagram applies to all cases where we must construct
	/// triangles for a given quad, both logical and graphical.
	/*
			"Normal" quad triangle winding (CCW):
			tl ------------ tr
			  |          / |
			  |   0    /   |
			  |      /     |
			  |    /       |
			  |  /     1   |
			  |/           |
			bl ------------ br
			triangle 0: <tl, bl, tr>
			triangle 1: <tr, bl, br>


			"Flipped" quad triangle winding (still CCW):
			tl ------------ tr
			  |\           |
			  |  \     1   |
			  |    \       |
			  |      \     |
			  |  0     \   |
			  |          \ |
			bl ------------ br
			triangle 0: <tl, bl, br>
			triangle 1: <br, tr, tl>
	*/



	tHeightFieldRez::tHeightFieldRez( f32 wslx, f32 wslz, u32 vrx, u32 vrz, u32 cvrx, u32 cvrz )
		: mWorldSpaceLengthX( wslx )
		, mWorldSpaceLengthZ( wslz )
		, mVertexResX( fAlignHigh( vrx, cvrx + 1 ) )
		, mVertexResZ( fAlignHigh( vrz, cvrz + 1 ) )
		, mLogicalVertResX( fAlignHigh( vrx, cvrx ) + 1 )
		, mLogicalVertResZ( fAlignHigh( vrz, cvrz ) + 1 )
		, mChunkQuadResX( cvrx )
		, mChunkQuadResZ( cvrz )
	{
		mNumChunksX = mVertexResX / mChunkQuadResX;
		mNumChunksZ = mVertexResZ / mChunkQuadResZ;
	}

	tHeightFieldRez::tHeightFieldRez( tNoOpTag )
	{
	}


	void tHeightFieldMesh::tLogicalVertex::fClearRenderVertexIds( )
	{
		mNumRenderVertexIds = 0;
	}

	void tHeightFieldMesh::tLogicalVertex::fAddRenderVertexId( u32 id )
	{
		if( mNumRenderVertexIds == 0 )
		{
			mBaseRenderVertexId = id;
		}
		else if( id < mBaseRenderVertexId )
		{
			for( u32 i = 0; i < mNumRenderVertexIds; ++i )
				mRenderVertexIds[ mNumRenderVertexIds ] = ( mBaseRenderVertexId + mRenderVertexIds[ mNumRenderVertexIds ] ) - id;
			mBaseRenderVertexId = id;
		}

		sigassert( mNumRenderVertexIds < mRenderVertexIds.fCount( ) );
		mRenderVertexIds[ mNumRenderVertexIds++ ] = id - mBaseRenderVertexId;
	}



	const f32 tHeightFieldMesh::cHeightHole = -Math::cInfinity;


	tHeightFieldMesh::tHeightFieldMesh( const Math::tVec2f& worldSpaceLengths, const Math::tVec2i& vertexRes )
		: mRez( worldSpaceLengths.x, worldSpaceLengths.y, vertexRes.x, vertexRes.y, 64, 64 )
	{
		sigassert( fRez( ).fNumChunks( ) > 0 );
		mLogicalVerts.fNewArray( fRez( ).fLogicalVertCount( ) );
		mQuadTree.fConstruct( fComputeBounds( ), *this, 16, 16 );
	}

	tHeightFieldMesh::tHeightFieldMesh( tNoOpTag )
		: tEntityDef( cNoOpTag )
		, mRez( cNoOpTag )
		, mLogicalVerts( cNoOpTag )
		, mQuadTree( cNoOpTag )
	{
	}

	tHeightFieldMesh::~tHeightFieldMesh( )
	{
	}

	void tHeightFieldMesh::fCollectTris( const Math::tAabbf& aabb, tGrowableArray<Math::tTrianglef>& trisOut ) const
	{
		mQuadTree.fCollectTris( trisOut, aabb, fComputeBounds( ), *this );
	}

	b32 tHeightFieldMesh::fTestRay( const Math::tRayf& rayInObject, Math::tRayCastHit& bestHit ) const
	{
		bestHit = Math::tRayCastHit( );

		tHeightFieldRayCastHit heightFieldRayCastHit;
		mQuadTree.fTestRay( heightFieldRayCastHit, rayInObject, fComputeBounds( ), *this );

		if( heightFieldRayCastHit.fHit( ) )
		{
			bestHit = heightFieldRayCastHit;
			return true;
		}

		return false;
	}

	b32 tHeightFieldMesh::fTestFrustum( const Math::tFrustumf& frustumInObject ) const
	{
		return mQuadTree.fTestFrustum( frustumInObject, fComputeBounds( ), *this );
	}

	void tHeightFieldMesh::fInitializeRenderVerts( tHeightFieldRenderVertex* renderVerts, u32 vertCount )
	{
		for( u32 i = 0; i < mLogicalVerts.fCount( ); ++i )
			mLogicalVerts[ i ].fClearRenderVertexIds( );

		const u32 idsPerChunk = 6 * fRez( ).fNumQuadsPerChunk( );

		u32 ivertex = 0;

		// create a snaking pattern, rather than the basic grid (or rather, a snaking pattern of basic grids),
		// so that we can add a constant offset to get from one "chunk" of verts to the next; each chunk is
		// itself a simple grid of verts in the X-Z plane

		for( u32 ichunkz = 0; ichunkz < fRez( ).fNumChunksZ( ); ++ichunkz )
		{
			const f32 chunkCornerZ = fRez( ).fChunkWorldCornerZ( ichunkz );
			for( u32 ichunkx = 0; ichunkx < fRez( ).fNumChunksX( ); ++ichunkx )
			{
				const f32 chunkCornerX = fRez( ).fChunkWorldCornerX( ichunkx );
				for( u32 iz = 0; iz < fRez( ).fChunkVertexResZ( ); ++iz )
				{
					const f32 z = chunkCornerZ + fRez( ).fChunkLocalZ( iz );
					for( u32 ix = 0; ix < fRez( ).fChunkVertexResX( ); ++ix )
					{
						const f32 x = chunkCornerX + fRez( ).fChunkLocalX( ix );

						renderVerts[ ivertex ] = tHeightFieldRenderVertex( );
						renderVerts[ ivertex ].P = Math::tVec3f( x, 0.f, z );

						const u32 logicalVertexIndex = fRez( ).fLogicalVertIndex( ichunkx, ichunkz, ix, iz );
						mLogicalVerts[ logicalVertexIndex ].fAddRenderVertexId( ivertex );

						if( ix < fRez( ).fChunkQuadResX( ) && iz < fRez( ).fChunkQuadResZ( ) )
						{
							// store quad info
							const u32 baseIdx = 
								idsPerChunk * ( ichunkz * fRez( ).fNumChunksX( ) + ichunkx ) +
								iz * 6 * fRez( ).fChunkQuadResX( ) + ix * 6;
							mLogicalVerts[ logicalVertexIndex ].fSetBaseRenderIndexId( baseIdx );
						}

						++ivertex;
					}
				}
			}
		}

		sigassert( ivertex == vertCount );
	}

	void tHeightFieldMesh::fInitializeRenderIndices( u16* renderIds, u32 idsCount )
	{
		const u32 idsPerChunk = 6 * fRez( ).fNumQuadsPerChunk( );

		sigassert( idsCount == idsPerChunk * fRez( ).fNumChunks( ) );

		// first we create the indices for a single chunk
		u32 iindex = 0;
		for( u32 iz = 0; iz < fRez( ).fChunkQuadResZ( ); ++iz )
		{
			for( u32 ix = 0; ix < fRez( ).fChunkQuadResX( ); ++ix )
			{
				// quad defaults to "normal", not "flipped"
				fComputeQuadRenderIndices( cQuadNormal, ix, iz, renderIds + iindex );
				iindex += 6;
			}
		}

		sigassert( iindex == idsPerChunk );

		// now we replicate the indices we just created across the rest of the chunks
		for( u32 i = 1; i < fRez( ).fNumChunks( ); ++i )
		{
			// copy triangle ids into gpu land; the ids for each chunk begin the same,
			// but they can be modified later (flipped/removed triangles)
			fMemCpy( renderIds + i * idsPerChunk, renderIds, idsPerChunk * sizeof( renderIds[ 0 ] ) );
		}
	}

	void tHeightFieldMesh::fRefreshLogicalBlock( u32 minx, u32 minz, u32 maxx, u32 maxz )
	{
		for( u32 z = minz; z <= maxz; ++z )
		{
			for( u32 x = minx; x <= maxx; ++x )
			{
				// update quad trees bounds
				mQuadTree.fUpdateBounds( x, z, *this );

				// re-compute quad triangulation if not removed
				tLogicalVertex& lv = mLogicalVerts[ fRez( ).fLogicalVertIndex( x, z ) ];
				if( x < maxx && z < maxz && lv.fGetQuadState( ) != cQuadRemoved )
					lv.fSetQuadState( fComputeQuadTriangulation( x, z ) );
			}
		}
	}

	void tHeightFieldMesh::fRefreshGraphicsBlock( u32 xmin, u32 zmin, u32 xmax, u32 zmax, u32 minRenderVtxIdx, u32 maxRenderVtxIdx )
	{
		sigassert( maxRenderVtxIdx >= minRenderVtxIdx );
		const u32 numRenderVerts = maxRenderVtxIdx - minRenderVtxIdx + 1;
		tHeightFieldRenderVertex* renderVerts = fLockRenderVerts( minRenderVtxIdx, numRenderVerts );
		sigassert( renderVerts );

		u16* renderIds = fLockRenderIds( );
		sigassert( renderIds );

		// update heights and vertex normals on gpu
		for( u32 z = zmin; z <= zmax; ++z )
		{
			for( u32 x = xmin; x <= xmax; ++x )
			{
				tLogicalVertex& lv = mLogicalVerts[ fRez( ).fLogicalVertIndex( x, z ) ];

				// compute the proper triangulation for this quad
				if( x < xmax && z < zmax )
				{
					fComputeQuadRenderIndices( 
						lv.fGetQuadState( ), 
						x % fRez( ).fChunkQuadResX( ),
						z % fRez( ).fChunkQuadResZ( ),
						renderIds + lv.fGetMinRenderIndexId( ) );
				}

				// compute vertex normal
				const Math::tVec3f n = fComputeLogicalVertexNormal( x, z );

				// copy height and normal to gpu buffer
				for( u32 i = 0; i < lv.fGetNumRenderVertexIds( ); ++i )
				{
					const s32 gpuVtxIdx = lv.fGetRenderVertexId( i ) - minRenderVtxIdx;
					sigassert( gpuVtxIdx >= 0 && gpuVtxIdx < ( s32 )numRenderVerts );
					renderVerts[ gpuVtxIdx ].P.y = lv.fGetHeight( );
					renderVerts[ gpuVtxIdx ].N = n;
				}
			}
		}

		fUnlockRenderVerts( renderVerts );
		fUnlockRenderIds( renderIds );
	}

	void tHeightFieldMesh::fComputeLogicalQuadAabb( u32 logicalX, u32 logicalZ, Math::tAabbf& aabb ) const
	{
		sigassert( logicalX < mRez.fNumLogicalVertsX( )-1 );
		sigassert( logicalZ < mRez.fNumLogicalVertsZ( )-1 );

		const u32 iright	= logicalX + 1;
		const u32 ibottom	= logicalZ + 1;

		aabb.fInvalidate( );

		aabb |= fComputeLogicalVertexPosition( logicalX, logicalZ );
		aabb |= fComputeLogicalVertexPosition( logicalX, ibottom );
		aabb |= fComputeLogicalVertexPosition( iright, logicalZ );
		aabb |= fComputeLogicalVertexPosition( iright, ibottom );
	}

	b32 tHeightFieldMesh::fComputeLogicalTris( u32 logicalX, u32 logicalZ, Math::tTrianglef& tri0, Math::tTrianglef& tri1 ) const
	{
		sigassert( logicalX < mRez.fNumLogicalVertsX( )-1 );
		sigassert( logicalZ < mRez.fNumLogicalVertsZ( )-1 );

		const tQuadState quadState	= mLogicalVerts[ mRez.fLogicalVertIndex( logicalX, logicalZ ) ].fGetQuadState( );

#ifdef target_game // TODO REFACTOR should maybe be controllable from app? or to the function?
		if( quadState == cQuadRemoved )
			return false;
#endif//target_game

		const u32 iright			= logicalX + 1;
		const u32 ibottom			= logicalZ + 1;

		const Math::tVec3f tl 		= fComputeLogicalVertexPosition( logicalX, logicalZ );
		const Math::tVec3f tr 		= fComputeLogicalVertexPosition( iright, logicalZ );
		const Math::tVec3f bl 		= fComputeLogicalVertexPosition( logicalX, ibottom );
		const Math::tVec3f br 		= fComputeLogicalVertexPosition( iright, ibottom );

		switch( quadState )
		{
		case cQuadRemoved:
		case cQuadNormal:
			tri0.mA = tl; tri0.mB = bl; tri0.mC = tr;
			tri1.mA = tr; tri1.mB = bl; tri1.mC = br;
			break;
		case cQuadFlipped:
			tri0.mA = tl; tri0.mB = bl; tri0.mC = br;
			tri1.mA = br; tri1.mB = tr; tri1.mC = tl;
			break;
		//case cQuadRemoved:
		//	tri0.mA = tri0.mB = tri0.mC = tl;
		//	tri1.mA = tri1.mB = tri1.mC = tl;
		//	break;
		default:
			sigassert( !"invalid case" );
			break;
		}

		return true;
	}

	Math::tVec3f tHeightFieldMesh::fComputeLogicalVertexPosition( u32 logicalX, u32 logicalZ ) const
	{
		return Math::tVec3f(
			mRez.fLogicalVertexWorldX( logicalX ),
			mLogicalVerts[ mRez.fLogicalVertIndex( logicalX, logicalZ ) ].fGetHeight( ),
			mRez.fLogicalVertexWorldZ( logicalZ ) );
	}

	Math::tVec3f tHeightFieldMesh::fComputeLogicalVertexNormal( u32 logicalX, u32 logicalZ ) const
	{
		const u32 ileft = logicalX == 0 ? logicalX : logicalX - 1;
		const u32 iright = logicalX == mRez.fNumLogicalVertsX( )-1 ? logicalX : logicalX + 1;
		const u32 itop = logicalZ == 0 ? logicalZ : logicalZ - 1;
		const u32 ibottom = logicalZ == mRez.fNumLogicalVertsZ( )-1 ? logicalZ : logicalZ + 1;

		const f32 xleft = mRez.fLogicalVertexWorldX( ileft );
		const f32 xright = mRez.fLogicalVertexWorldX( iright );
		const f32 xcenter = mRez.fLogicalVertexWorldX( logicalX );
		const f32 ztop = mRez.fLogicalVertexWorldZ( itop );
		const f32 zbottom = mRez.fLogicalVertexWorldZ( ibottom );
		const f32 zcenter = mRez.fLogicalVertexWorldZ( logicalZ );
		const f32 hleft = mLogicalVerts[ mRez.fLogicalVertIndex( ileft, logicalZ ) ].fGetHeight( );
		const f32 hright = mLogicalVerts[ mRez.fLogicalVertIndex( iright, logicalZ ) ].fGetHeight( );
		const f32 hcenter = mLogicalVerts[ mRez.fLogicalVertIndex( logicalX, logicalZ ) ].fGetHeight( );
		const f32 htop = mLogicalVerts[ mRez.fLogicalVertIndex( logicalX, itop ) ].fGetHeight( );
		const f32 hbottom = mLogicalVerts[ mRez.fLogicalVertIndex( logicalX, ibottom ) ].fGetHeight( );

		const Math::tVec3f rc = Math::tVec3f( xright - xcenter, hright - hcenter, 0.f );
		const Math::tVec3f tc = Math::tVec3f( 0.f, htop - hcenter, ztop - zcenter );
		const Math::tVec3f lc = Math::tVec3f( xleft - xcenter, hleft - hcenter, 0.f );
		const Math::tVec3f bc = Math::tVec3f( 0.f, hbottom - hcenter, zbottom - zcenter );

		Math::tVec3f n = Math::tVec3f::cZeroVector;

		n += rc.fCross( tc ).fNormalizeSafe( Math::tVec3f::cZeroVector );
		n += tc.fCross( lc ).fNormalizeSafe( Math::tVec3f::cZeroVector );
		n += lc.fCross( bc ).fNormalizeSafe( Math::tVec3f::cZeroVector );
		n += bc.fCross( rc ).fNormalizeSafe( Math::tVec3f::cZeroVector );

		return n.fNormalizeSafe( Math::tVec3f::cZAxis );
	}

	void tHeightFieldMesh::fGetTerrainQuadIndices( f32 localX, f32 localZ, u32& ileft, u32& iright, u32& itop, u32& ibottom, f32& dx, f32& dz ) const
	{
		localX = fClamp( localX, mRez.fMinX( ), mRez.fMaxX( ) );
		localZ = fClamp( localZ, mRez.fMinZ( ), mRez.fMaxZ( ) );

		localX -= mRez.fMinX( );
		localZ -= mRez.fMinZ( );

		dx = std::fmod( localX, mRez.fVertexDeltaX( ) ) / mRez.fVertexDeltaX( );
		dz = std::fmod( localZ, mRez.fVertexDeltaZ( ) ) / mRez.fVertexDeltaZ( );

		sigassert( fInBounds( dx, 0.f, 1.f ) && fInBounds( dz, 0.f, 1.f ) );

		ileft = ( u32 )( ( localX / mRez.fWorldLengthX( ) ) * ( mRez.fNumLogicalVertsX( ) - 1 ) );
		itop  = ( u32 )( ( localZ / mRez.fWorldLengthZ( ) ) * ( mRez.fNumLogicalVertsZ( ) - 1 ) );

		if( ileft >= mRez.fNumLogicalVertsX( ) - 1 )
		{
			ileft = mRez.fNumLogicalVertsX( ) - 2;
			dx = 1.f;
		}
		if( itop >= mRez.fNumLogicalVertsZ( ) - 1 )
		{
			itop = mRez.fNumLogicalVertsZ( ) - 2;
			dz = 1.f;
		}

		iright			= ileft + 1;
		ibottom			= itop + 1;
	}

	f32 tHeightFieldMesh::fSampleHeight( f32 localX, f32 localZ, const tQuadState* overrideQuadState ) const
	{
		u32 ileft, iright, itop, ibottom; f32 dx, dz;
		fGetTerrainQuadIndices( localX, localZ, ileft, iright, itop, ibottom, dx, dz );

		const f32 tl 				= fLogicalHeight( ileft, itop );
		const f32 tr 				= fLogicalHeight( iright, itop );
		const f32 bl 				= fLogicalHeight( ileft, ibottom );
		const f32 br 				= fLogicalHeight( iright, ibottom );

		// interpolate heights across triangles

		const tQuadState quadState	= 
			overrideQuadState ? *overrideQuadState : mLogicalVerts[ mRez.fLogicalVertIndex( ileft, itop ) ].fGetQuadState( );

		f32 h;
		switch( quadState )
		{
		case cQuadRemoved:
		case cQuadNormal:
			if( dx + dz <= 1.0f ) // triangle 0
				h = tl + ( tr - tl ) * dx + ( bl - tl ) * dz;
			else // triangle 1
				h = br + ( bl - br ) * ( 1.f - dx ) + ( tr - br ) * ( 1.f - dz );
			break;
		case cQuadFlipped:
			if( dx + ( 1.f - dz ) <= 1.0f ) // triangle 0
				h = bl + ( br - bl ) * dx + ( tl - bl ) * ( 1.f - dz );
			else // triangle 1
				h = tr + ( tl - tr ) * ( 1.f - dx ) + ( br - tr ) * dz;
			break;
		//case cQuadRemoved:
		//	h = cHeightHole;
		//	break;
		default:
			sigassert_and_analyze_assume( !"invalid case" );
			break;
		}

		return h;
	}

	tHeightFieldMesh::tQuadState tHeightFieldMesh::fComputeQuadTriangulation( u32 logicalX, u32 logicalZ ) const
	{
		sigassert( logicalX < mRez.fNumLogicalVertsX( ) - 1 );
		sigassert( logicalZ < mRez.fNumLogicalVertsZ( ) - 1 );

		const tQuadState normalState  = cQuadNormal;
		const tQuadState flippedState = cQuadFlipped;

		const f32 xMiddle = fRez( ).fLogicalVertexWorldX( logicalX ) + 0.5f * fRez( ).fVertexDeltaX( );
		const f32 zMiddle = fRez( ).fLogicalVertexWorldZ( logicalZ ) + 0.5f * fRez( ).fVertexDeltaZ( );

		const f32 hNormal  = fSampleHeight( xMiddle, zMiddle, &normalState );
		const f32 hFlipped = fSampleHeight( xMiddle, zMiddle, &flippedState );

		const u32 ix1 = logicalX;
		const u32 ix0 = ix1 == 0 ? ix1 : ix1 - 1;
		const u32 ix2 = ix1 == mRez.fNumLogicalVertsX( ) - 2 ? ix1 : ix1 + 1;
		const u32 ix3 = ix2 == mRez.fNumLogicalVertsX( ) - 2 ? ix2 : ix2 + 1;

		const u32 iz1 = logicalZ;
		const u32 iz0 = iz1 == 0 ? iz1 : iz1 - 1;
		const u32 iz2 = iz1 == mRez.fNumLogicalVertsZ( ) - 2 ? iz1 : iz1 + 1;
		const u32 iz3 = iz2 == mRez.fNumLogicalVertsZ( ) - 2 ? iz2 : iz2 + 1;

		const f32 h00 = fLogicalHeight( ix0, iz0 );
		const f32 h10 = fLogicalHeight( ix1, iz0 );
		const f32 h20 = fLogicalHeight( ix2, iz0 );
		const f32 h30 = fLogicalHeight( ix3, iz0 );

		const f32 h01 = fLogicalHeight( ix0, iz1 );
		const f32 h11 = fLogicalHeight( ix1, iz1 );
		const f32 h21 = fLogicalHeight( ix2, iz1 );
		const f32 h31 = fLogicalHeight( ix3, iz1 );

		const f32 h02 = fLogicalHeight( ix0, iz2 );
		const f32 h12 = fLogicalHeight( ix1, iz2 );
		const f32 h22 = fLogicalHeight( ix2, iz2 );
		const f32 h32 = fLogicalHeight( ix3, iz2 );

		const f32 h03 = fLogicalHeight( ix0, iz3 );
		const f32 h13 = fLogicalHeight( ix1, iz3 );
		const f32 h23 = fLogicalHeight( ix2, iz3 );
		const f32 h33 = fLogicalHeight( ix3, iz3 );

		const f32 wu2 = 36.f;
		const f32 wu1 = 24.f;
		const f32 wu0 = 4.f;
		const f32 wut = 4 * wu2 + 8 * wu1 + 4 * wu0;
		const f32 w2  = wu2 / wut;
		const f32 w1  = wu1 / wut;
		const f32 w0  = wu0 / wut;

		const f32 hSmooth =
			w2 * h00 + w1 * h10 + w1 * h20 + w2 * h30 +
			w1 * h01 + w0 * h11 + w0 * h21 + w1 * h31 +
			w1 * h02 + w0 * h12 + w0 * h22 + w1 * h32 +
			w2 * h03 + w1 * h13 + w1 * h23 + w2 * h33;

		// note! this seems backwards, but it also seems to produce the right results... why???
		if( fAbs( hSmooth - hNormal ) > fAbs( hSmooth - hFlipped ) )
			return cQuadNormal;
		else
			return cQuadFlipped;
	}

	namespace
	{
		template<class tIds>
		void fComputeQuadRenderIndicesInternal( u32 numVertsPerRow, tHeightFieldMesh::tQuadState quadState, u32 ix, u32 iz, tIds idsOut[6] )
		{
			const u32 xx = 0;
			const u32 tl = ( iz + 0 ) * numVertsPerRow + ( ix + 0 );
			const u32 tr = ( iz + 0 ) * numVertsPerRow + ( ix + 1 );
			const u32 bl = ( iz + 1 ) * numVertsPerRow + ( ix + 0 );
			const u32 br = ( iz + 1 ) * numVertsPerRow + ( ix + 1 );

			switch( quadState )
			{
			case tHeightFieldMesh::cQuadNormal:
				idsOut[ 0 ] = tl; idsOut[ 1 ] = bl; idsOut[ 2 ] = tr;
				idsOut[ 3 ] = tr; idsOut[ 4 ] = bl; idsOut[ 5 ] = br;
				break;
			case tHeightFieldMesh::cQuadFlipped:
				idsOut[ 0 ] = tl; idsOut[ 1 ] = bl; idsOut[ 2 ] = br;
				idsOut[ 3 ] = br; idsOut[ 4 ] = tr; idsOut[ 5 ] = tl;
				break;
			case tHeightFieldMesh::cQuadRemoved:
				idsOut[ 0 ] = xx; idsOut[ 1 ] = xx; idsOut[ 2 ] = xx;
				idsOut[ 3 ] = xx; idsOut[ 4 ] = xx; idsOut[ 5 ] = xx;
				break;
			default:
				sigassert( !"invalid state" );
				break;
			}
		}
	}

	void tHeightFieldMesh::fComputeQuadRenderIndices( tQuadState quadState, u32 ix, u32 iz, u16 idsOut[6] )
	{
		// NOTE!! ix and iz, while appearing to be global indices into the larger logical heightfield, are
		// actually assumed to be relative to the top-left chunk; put another way, they are actually local
		// to the chunk grid, not the overall grid
		fComputeQuadRenderIndicesInternal( fRez( ).fChunkVertexResX( ), quadState, ix, iz, idsOut );
	}
	void tHeightFieldMesh::fComputeQuadRenderIndicesGeneralized( u32 numVertsPerRow, tQuadState quadState, u32 ix, u32 iz, u32 idsOut[6] )
	{
		fComputeQuadRenderIndicesInternal( numVertsPerRow, quadState, ix, iz, idsOut );
	}

	Math::tAabbf tHeightFieldMesh::fComputeChunkBounds( u32 ichunkx, u32 ichunkz ) const
	{
		// compute x-z bounds of chunk
		Math::tAabbf bounds = fRez( ).fChunkBounds( ichunkx, ichunkz );

		// convert spatial coords to logical quad indices
		Math::tVec2u chunkMinMaxLogicalX, chunkMinMaxLogicalZ;
		chunkMinMaxLogicalX.x = fRez( ).fWorldXLogicalQuad( bounds.mMin.x, fRoundDown<u32> );
		chunkMinMaxLogicalX.y = fRez( ).fWorldXLogicalQuad( bounds.mMax.x, fRoundUp<u32> );
		chunkMinMaxLogicalZ.x = fRez( ).fWorldZLogicalQuad( bounds.mMin.z, fRoundDown<u32> );
		chunkMinMaxLogicalZ.y = fRez( ).fWorldZLogicalQuad( bounds.mMax.z, fRoundUp<u32> );

		// determine min/max height
		const Math::tVec2f minMaxHeight = mQuadTree.fComputeMinMaxHeightEstimate( chunkMinMaxLogicalX, chunkMinMaxLogicalZ );
		bounds.mMin.y = minMaxHeight.x;
		bounds.mMax.y = minMaxHeight.y;

		// return bounds in object space
		return bounds;
	}

	void tHeightFieldMesh::fSaveHeightField( tHeightField& heightField ) const
	{
		heightField.mHeights.fSetCount( mLogicalVerts.fCount( ) );
		heightField.mQuadsDisabled.fSetCount( mLogicalVerts.fCount( ) );

		for( u32 i = 0; i < mLogicalVerts.fCount( ); ++i )
		{
			heightField.mHeights[ i ] = mLogicalVerts[ i ].fGetHeight( );
			heightField.mQuadsDisabled[ i ] = ( mLogicalVerts[ i ].fGetQuadState( ) == tHeightFieldMesh::cQuadRemoved );
		}
	}

	void tHeightFieldMesh::fRestoreHeightField( const tHeightField& heightField, b32 refreshGfx )
	{
		fRestoreHeightField( heightField, 0, 0, fRez( ).fNumLogicalVertsX( ) - 1, fRez( ).fNumLogicalVertsZ( ) - 1, refreshGfx );
	}

	void tHeightFieldMesh::fRestoreHeightField( const tHeightField& heightField, u32 minx, u32 minz, u32 maxx, u32 maxz, b32 refreshGfx )
	{
		sigassert( mLogicalVerts.fCount( ) == heightField.mHeights.fCount( ) );

		u32 minRenderVtxIdx = ~0;
		u32 maxRenderVtxIdx =  0;

		// first go through all verts and set height and quad state
		for( u32 z = minz; z <= maxz; ++z )
		{
			for( u32 x = minx; x <= maxx; ++x )
			{
				const u32 logicalIndex = fRez( ).fLogicalVertIndex( x, z );
				tLogicalVertex& lv = mLogicalVerts[ logicalIndex ];
				lv.fSetHeight( heightField.mHeights[ logicalIndex ] );

				if( heightField.fHasQuadData( ) )
					lv.fSetQuadState( heightField.mQuadsDisabled[ logicalIndex ] ? tHeightFieldMesh::cQuadRemoved : tHeightFieldMesh::cQuadNormal );

				// track min and max render vertices
				for( u32 i = 0; i < lv.fGetNumRenderVertexIds( ); ++i )
				{
					const u32 rvtxId = lv.fGetRenderVertexId( i );
					minRenderVtxIdx = fMin( minRenderVtxIdx, rvtxId );
					maxRenderVtxIdx = fMax( maxRenderVtxIdx, rvtxId );
				}
			}
		}

		fRefreshLogicalBlock( minx, minz, maxx, maxz );

		if( refreshGfx )
			fRefreshGraphicsBlock( minx, minz, maxx, maxz, minRenderVtxIdx, maxRenderVtxIdx );
	}
}

