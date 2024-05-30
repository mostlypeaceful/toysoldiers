//------------------------------------------------------------------------------
// \file tProgressiveMesh.cpp - 18 Jul 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tProgressiveMesh.hpp"

namespace Sig
{
	//------------------------------------------------------------------------------
	void tProgressiveMesh::fExpandByRatio( 
		f32 ratio, Gfx::tGeometryBufferSysRam & vb, Gfx::tIndexBufferSysRam & ib )
	{
		fExpandByCount( 
			(u32)( mExpansions.fCount( ) * fClamp( ratio, 0.f, 1.f ) ), vb, ib );
	}

	//------------------------------------------------------------------------------
	void tProgressiveMesh::fExpandByCount( 
		u32 expandCount, Gfx::tGeometryBufferSysRam & vb, Gfx::tIndexBufferSysRam & ib )
	{
		// Special case
		if( expandCount == 0 )
		{
			vb = mM0.mVerts;
			ib = mM0.mIndices;
			return;
		}

		// Clamp to total expansions
		if( expandCount > mExpansions.fCount( ) )
			expandCount = mExpansions.fCount( );

		fExpandVertices( expandCount, vb );
		fExpandIndices( expandCount, ib );
	}

	//------------------------------------------------------------------------------
	void tProgressiveMesh::fBuildProgressiveWindows( 
			Gfx::tGeometryBufferSysRam & vb, 
			tGrowableArray< tProgressiveIndexBuffer > & ibs ) const
	{
		// Build the full vertex buffer
		fExpandVertices( mExpansions.fCount( ), vb );

		// Detect new format due to new vertex count
		Gfx::tIndexFormat ibFormat = Gfx::tIndexFormat::fCreateAppropriateFormat( 
			mM0.mIndices.fIndexFormat( ).mPrimitiveType, vb.fVertexCount( ) );

		tGrowableArray<u32> faces;
		tGrowableArray<u32> newFaces;
		tGrowableArray<u32> deadFaces;
		tGrowableArray<u32> removeFromFaces;

		// One progressive ib for every marker plus 1 for the range 
		// from the last marker to the end
		const u32 markerCount = mCascadeMarkers.fCount( ) + 1;
		ibs.fSetCount( markerCount );

		u32 startExpansion = 0;
		for( u32 m = 0; m < markerCount; ++m )
		{
			// Reset our containers
			faces.fSetCount( 0 );
			newFaces.fSetCount( 0 );
			deadFaces.fSetCount( 0 );
			removeFromFaces.fSetCount( 0 );

			const u32 endExpansion = m < mCascadeMarkers.fCount( ) ? mCascadeMarkers[ m ] : mExpansions.fCount( );

			// Index buffers are structured such that they are ordered as 
			// [ [ changed / new faces ] [ unchanged faces ] [ changing faces ] ]
			tProgressiveIndexBuffer & ib = ibs[ m ];
			ib.mWindows.fSetCount( endExpansion - startExpansion + 1 );

			// Build the initial index buffer before any expansions are applied
			{
				Gfx::tIndexBufferSysRam tempIb;
				fExpandIndices( startExpansion, tempIb );
				faces.fSetCount( tempIb.fIndexCount( ) );
				tempIb.fGetIndices( 0, faces.fBegin( ), tempIb.fIndexCount( ) );
			}


			// Build the new faces list, the dead faces list, and the unchanged faces list
			for( u32 e = startExpansion, newVId = mM0.mVerts.fVertexCount( ) + e; e < endExpansion;  ++e, ++newVId )
			{
				const tExpansion & ex = mExpansions[ e ];

				// Grab the count
				const u32 indicesToChangeCount = ex.mIndicesToChange.fCount( );

				// Find all the faces that have changed indices
				for( s32 i = indicesToChangeCount - 1; i >= 0; --i )
				{
					const u32 startIndex = 3 * ( ex.mIndicesToChange[ i ] / 3 );
					const u32 iTriRel = ex.mIndicesToChange[ i ] - startIndex;

					// Capture the face for later removal
					removeFromFaces.fPushBack( startIndex );

					// Put the face on the beginning of the dead list
					deadFaces.fInsert( 0, &faces[ startIndex ], 3 );

					// Put it on the beginning of the new list and update
					newFaces.fInsert( 0, &faces[ startIndex ], 3 );
					newFaces[ iTriRel ] = newVId;
				}

				// Add all the new faces
				newFaces.fInsert( 0, ex.mNewFaces.fBegin( ), ex.mNewFaces.fCount( ) );
			}

			// Update the face list by removing the changed indices
			std::sort( removeFromFaces.fBegin( ), removeFromFaces.fEnd( ) );
			const u32 toRemoveCount = removeFromFaces.fCount( );
			for( s32 i = toRemoveCount - 1; i >= 0; --i )
				faces.fEraseOrdered( removeFromFaces[ i ], 3 );

			// Build the first window
			tProgressiveIndexBuffer::tWindow window;
			window.mFirstIndex = newFaces.fCount( );
			window.mNumFaces = ( faces.fCount( ) + deadFaces.fCount( ) ) / 3;
			window.mNumVerts = mM0.mVerts.fVertexCount( ) + startExpansion;

			// Build the render windows for the progressive index buffer
			for( u32 e = startExpansion, w = 0; e < endExpansion; ++e, ++w )
			{
				ib.mWindows[ w ] = window; // Apply the window

				// Update the window for this expansion
				const tExpansion & ex = mExpansions[ e ];
				const u32 newIndexCount = ex.mNewFaces.fCount( );
				const u32 changedIndexCount = ex.mIndicesToChange.fCount( ) * 3;
				
				// Update the window for this expansion
				window.mNumVerts += 1;
				window.mNumFaces += ( newIndexCount / 3 );
				window.mFirstIndex -= ( newIndexCount + changedIndexCount );
			}

			// Capture the final window
			ib.mWindows.fBack( ) = window;

			// Build the actual index buffer
			ib.mIndexBuffer.fAllocate( ibFormat, newFaces.fCount( ) + faces.fCount( ) + deadFaces.fCount( ) );
			ib.mIndexBuffer.fSetIndices( 0, newFaces.fBegin( ), newFaces.fCount( ) );
			ib.mIndexBuffer.fSetIndices( newFaces.fCount( ), faces.fBegin( ), faces.fCount( ) );
			ib.mIndexBuffer.fSetIndices( newFaces.fCount( ) + faces.fCount( ), deadFaces.fBegin( ), deadFaces.fCount( ) );

			// Update where the next window starts
			startExpansion = endExpansion;
		}
	}

	//------------------------------------------------------------------------------
	void tProgressiveMesh::fExpandVertices( u32 expandCount, Gfx::tGeometryBufferSysRam & vb ) const
	{
		sigassert( expandCount <= mExpansions.fCount( ) && "Sanity!" );

		// Initial element count
		u32 vertexCount = mM0.mVerts.fVertexCount( );

		// Adjust vertex count for expansions
		vertexCount += expandCount;

		// Allocate vb
		vb.fAllocate( mM0.mVerts.fVertexFormat( ), vertexCount );

		// Copy M0 into the new buffer
		fMemCpy( vb.fBegin( ), mM0.mVerts.fBegin( ), mM0.mVerts.fSizeInBytes( ) );

		const u32 vertexSize = mM0.mVerts.fVertexSize( );

		// Run over the expansions and apply
		Sig::byte * newVertDest = vb.fBegin( ) + mM0.mVerts.fSizeInBytes( );
		for( u32 e = 0; e < expandCount; ++e)
		{
			const tExpansion & ex = mExpansions[ e ];

			// Add the new vertex
			fMemCpy( newVertDest, &mExpansionVertData[ ex.mNewVertDataOffset ], vertexSize );
			newVertDest += vertexSize;

			// Update the old vertex if there's new data
			if( ex.mUpdateVertDataOffset != ~0 )
			{
				fMemCpy( 
					vb.fBegin( ) + ( vertexSize * ex.mUpdateVertIndex ), 
					&mExpansionVertData[ ex.mUpdateVertDataOffset ], vertexSize );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tProgressiveMesh::fExpandIndices( u32 expandCount, Gfx::tIndexBufferSysRam & ib ) const
	{
		sigassert( expandCount <= mExpansions.fCount( ) && "Sanity!" );

		// Initial element count
		u32 indexCount = mM0.mIndices.fIndexCount( );

		// Adjust index count for expansions
		for( u32 e = 0; e < expandCount; ++e )
			indexCount += mExpansions[ e ].mNewFaces.fCount( );

		// Allocate ib, with a potentially different format due to new vertex count
		Gfx::tIndexFormat ibFormat = Gfx::tIndexFormat::fCreateAppropriateFormat( 
			mM0.mIndices.fIndexFormat( ).mPrimitiveType, mM0.mVerts.fVertexCount( ) + expandCount );
		ib.fAllocate( ibFormat, indexCount );

		// Copy M0 into the new buffer
		ib.fSetIndices( 0, 0, mM0.mIndices, mM0.mIndices.fIndexCount( ) );

		const u32 vertexSize = mM0.mVerts.fVertexSize( );

		// Run over the expansions and apply
		u32 newFaceDest = mM0.mIndices.fIndexCount( );
		u32 newVertexIndex = mM0.mVerts.fVertexCount( );
		for( u32 e = 0; e < expandCount; ++e, ++newVertexIndex )
		{
			const tExpansion & ex = mExpansions[ e ];

			// Add the new faces
			ib.fSetIndices( newFaceDest, ex.mNewFaces.fBegin( ), ex.mNewFaces.fCount( ) );
			newFaceDest += ex.mNewFaces.fCount( );

			// Update the old faces
			const u32 indexChangeCount = ex.mIndicesToChange.fCount( );
			for( u32 f = 0; f < indexChangeCount; ++f )
				ib.fSetIndex( ex.mIndicesToChange[ f ], newVertexIndex );
		}
	}
}
