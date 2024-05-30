#include "ToolsPch.hpp"
#include "MeshSimplify.hpp"
#include "MeshPrimitives.hpp"
#include "tPriorityQueue.hpp"

#define TEST_PRIORITY_QUEUE

namespace Sig { namespace MeshSimplify
{
	struct tSortPairs
	{
		inline b32 operator()( const tContractPair* a, const tContractPair* b ) const
		{
			return a->fGetCost( ) < b->fGetCost( );
		}
	};

	class tConnectiveMesh
	{
		tGrowableArray< tVert > mVerts;
		tGrowableArray< tFace > mFaces;

#ifdef TEST_PRIORITY_QUEUE
		tPriorityQueue< tContractPair*, tSortPairs > mPairs;
#else
		tGrowableArray< tContractPair*> mPairs;
#endif

		u32 mValidVerts;
		u32 mValidEdges;
		u32 mValidFaces;

		b32 mDisregardEdges;

	public:
		tConnectiveMesh( const tGrowableArray< Math::tVec3f >& verts, tGrowableArray< Math::tVec3u >& triangleIndices, f32 optimizeTarget, b32 disregardEdges )
			: mDisregardEdges( disregardEdges )
		{
			log_line( 0, "Simplifying mesh. Optimize target: " << optimizeTarget );

			optimizeTarget *= 5.f; // Increase how aggressively the mesh will be simplified.

			// Generate connective tissue.
			mVerts.fSetCount( verts.fCount( ) );
			for( tVertID i = 0; i < verts.fCount( ); ++i )
			{
				// Create vert.
				mVerts[i] = tVert( verts[i] );
			}

			mFaces.fSetCount( triangleIndices.fCount( ) );
			for( u32 i = 0; i < triangleIndices.fCount( ); ++i )
			{
				// Create face.
				mFaces[i] = tFace( triangleIndices[i][0], triangleIndices[i][1], triangleIndices[i][2] );

				// Add plane vertex to all associated verts.
				tQuadric planeQ = fGeneratePlaneConstraint( mFaces[i] );
				planeQ /= planeQ.fGetArea( );

				tVert& v0 = fGetVert( mFaces[i][ 0 ] );
				tVert& v1 = fGetVert( mFaces[i][ 1 ] );
				tVert& v2 = fGetVert( mFaces[i][ 2 ] );

				// Add the plane's Q to all the verts.
				v0.mQ += planeQ;
				v1.mQ += planeQ;
				v2.mQ += planeQ;

				// Record this face as a neighbor for all verts.
				v0.fAddNeighbor( i );
				v1.fAddNeighbor( i );
				v2.fAddNeighbor( i );
			}

			// Make boundary Q planes.
			fGenerateBoundaries( );

			mValidVerts = mVerts.fCount( );
			mValidFaces = mFaces.fCount( );

			// Generate Pairs.
			for( tVertID i = 0; i < mVerts.fCount( ); ++i )
			{
				// Skip any locked verts.
				if( mVerts[i].fLocked( ) )
					continue;

				// Get all neighbor verts.
				tVertList verts;
				fCollectNeighborVertsFromVert( i, verts );

				// Create all valid neighbor pairs.
				for( u32 j = 0; j < verts.fCount( ); j++ )
				{
					const tVertID jthVert = verts[j];
					if( mVerts[jthVert].fLocked( ) )
						continue;

					// Only create pairs forward so we don't get doubles.
					if( i < jthVert )
						fCreatePair( i, jthVert );
				}
			}

			// Optimize until the target is reached.
			u32 numContractions = 0;
			while( mPairs.fCount( ) > 0 )
			{
#ifndef TEST_PRIORITY_QUEUE
				std::sort( mPairs.fBegin( ), mPairs.fEnd( ), tSortPairs( ) );
#endif

#ifdef TEST_PRIORITY_QUEUE
				tContractPair* pair = mPairs.fGet( );
#else
				tContractPair* pair = mPairs.fFront( );
				mPairs.fErase( 0 );
#endif
				if( pair->fGetCost( ) >= optimizeTarget )
				{
					delete pair;
					break;
				}

				//log_line( 0, "Simplifying Mesh. Faces: " << mValidFaces << ". Target Error: " << optimizeTarget << ". Error: " << pair->fGetCost( ) );

				tDecimateEvent thisContraction;
				fFillDecimationData( pair, thisContraction );

				fPerformDecimation( thisContraction );

				++numContractions;
			}

#ifdef TEST_PRIORITY_QUEUE
			while( mPairs.fCount( ) > 0 )
				delete mPairs.fGet( );
#else
			for( u32 i = 0; i < mPairs.fCount( ); ++i )
				delete mPairs[i];

			mPairs.fDeleteArray( );
#endif
		}

		void fGetResults( tGrowableArray< Math::tVec3f >& verts, tGrowableArray< Math::tVec3u >& triangleIndices )
		{
			verts.fSetCount( 0 );
			triangleIndices.fSetCount( 0 );

			// Convert back to raw lists.
			tHashTable< Math::tVec3d, u32, tHashTableExpandOnlyResizePolicy > validVerts( 32 );
			for( u32 i = 0; i < mFaces.fCount( ); ++i )
			{
				// Skip any faces that were destroyed.
				if( !mFaces[i].fIsValid( ) )
					continue;

				const tFace& thisFace = mFaces[i];
				Math::tVec3u thisIdxSet;

				for( u32 j = 0; j < 3; ++j )
				{
					// Grab the vertex and see if it already exists in the list of verts we've encountered.
					const Math::tVec3d thisVertPos = fGetVert( thisFace[ j ] ).fGetPos( );
					const u32* foundIdx = validVerts.fFind( thisVertPos );

					if( foundIdx )
					{
						// The vertex already exists, use its found index.
						thisIdxSet[j] = *foundIdx;
					}
					else
					{
						// Vertex not found. Push it into the hash map which will store its index.
						thisIdxSet[j] = verts.fCount( );
						validVerts.fInsert( thisVertPos, verts.fCount( ) );
						verts.fPushBack( Math::tVec3f( thisVertPos[0], thisVertPos[1], thisVertPos[2] ) );
					}
				}

				triangleIndices.fPushBack( thisIdxSet );
			}
		}

		void fGetResultsNoReIndex( tGrowableArray< Math::tVec3f >& verts, tGrowableArray< Math::tVec3u >& triangleIndices )
		{
			sigassert( verts.fCount( ) == mVerts.fCount( ) );
			for( u32 i = 0; i < mVerts.fCount( ); ++i )
			{
				// Only copy back verts that were modified. Prevents a threaded
				// collision when writing the whole mesh back.
				if( !mVerts[i].fDirty( ) )
					continue;

				Math::tVec3d pos = mVerts[i].fGetPos( );
				verts[i] = Math::tVec3f( pos[0], pos[1], pos[2] );
			}

			triangleIndices.fSetCount( 0 );
			for( u32 i = 0; i < mFaces.fCount( ); ++i )
			{
				// Skip any faces that were destroyed.
				if( !mFaces[i].fIsValid( ) )
					continue;

				const tFace& thisFace = mFaces[i];
				Math::tVec3u thisIdxSet( thisFace[0], thisFace[1], thisFace[2] );

				triangleIndices.fPushBack( thisIdxSet );
			}
		}

	private:
		tVert& fGetVert( u32 vIdx )
		{
			sigassert( mVerts.fCount( ) > vIdx );
			return mVerts[vIdx];
		}

		Math::tVec3d fGetVertPos( u32 vIdx )
		{
			sigassert( mVerts.fCount( ) > vIdx );
			return mVerts[vIdx].fGetPos( );
		}

		tFace& fGetFace( u32 fIdx )
		{
			sigassert( mFaces.fCount( ) > fIdx );
			return mFaces[fIdx];
		}

		void fMarkFace( tFaceID idx, u32 mark )
		{
			mFaces[idx].fMark( mark );
		}

		void fAddToMarkFace( tFaceID idx, u32 add )
		{
			mFaces[idx].fAddMark( add );
		}

		u32 fGetFaceMark( tFaceID idx )
		{
			return mFaces[idx].fGetMark( );
		}

		void fMarkVert( tVertID idx, u32 mark )
		{
			mVerts[idx].fMark( mark );
		}

		u32 fGetVertMark( tVertID idx )
		{
			return mVerts[idx].fGetMark( );
		}

		/// 
		/// \brief
		/// Marks every face that borders the indicated vertex.
		void fMarkNeighborhood( tVertID idx, u32 mark )
		{
			const tFaceList& neighborFaces = fGetVert( idx ).fGetNeighbors( );

			for( u32 i = 0; i < neighborFaces.fCount( ); i++ )
			{
				tFaceID f = neighborFaces[i];
				fMarkFace( f, mark );
			}
		}
		
		///  
		/// \brief
		/// Adds the speicifed amount to each face that borders the vertex.
		void fMarkNeighborhoodIncrement( tVertID idx, u32 add )
		{
			const tFaceList& neighborFaces = fGetVert( idx ).fGetNeighbors( );

			for( u32 i = 0; i < neighborFaces.fCount( ); i++ )
			{
				tFaceID f = neighborFaces[i];
				fAddToMarkFace( f, add );
			}
		}

		/// 
		/// \brief
		/// Separate all faces surrounding a vert into lists >= and < the pivot value. This is
		/// useful for finding out which faces are touched by two verts when used in conjuction with
		/// fMarkNeighboorhood and fMarkNeighborhoodIncrement.
		void fPartitionMarkedNeighbors( tVertID v, u32 pivot, tFaceList& below, tFaceList& above )
		{
			const tFaceList& neighborFaces = fGetVert( v ).fGetNeighbors( );

			for( u32 i = 0; i < neighborFaces.fCount( ); ++i )
			{
				const tFaceID f = neighborFaces[i];
				const u32 faceMark = fGetFaceMark( f );
				if( faceMark )
				{
					if( faceMark < pivot )
						below.fPushBack( f );
					else
						above.fPushBack( f );

					fMarkFace( f, 0 );
				}
			}
		}

		/// 
		/// \brief
		/// Collects all vertexes that share an edge with the indicated vertex.
		void fCollectNeighborVertsFromVert( tVertID v, tVertList& neighborVerts )
		{
			const tFaceList& faces = mVerts[v].fGetNeighbors( );

			// Clear all the verts related to neighbor faces of this vert.
			for( u32 i = 0; i < faces.fCount( ); i++ )
			{
				for( u32 j = 0; j < 3; j++ )
					fMarkVert( mFaces[ faces[i] ][ j ], 0 );
			}

			// Do not include the center vert. We're looking for neighbors.
			fMarkVert( v, 1 );

			// Collect every vert.
			for( u32 i = 0; i < faces.fCount( ); i++ )
			{
				for( u32 j = 0; j < 3; j++ )
				{
					tVertID thisVert = mFaces[ faces[i] ][j];
					if( !fGetVertMark( thisVert ) )
					{
						// Get the vert and mark it so we don't grab it twice.
						neighborVerts.fPushBack( thisVert );
						fMarkVert( thisVert, 1 );
					}
				}
			}
		}

		Math::tVec3d fComputeNormal( const tFace& face )
		{
			const Math::tVec3d v0 = fGetVertPos( face[0] );
			const Math::tVec3d v1 = fGetVertPos( face[1] );
			const Math::tVec3d v2 = fGetVertPos( face[2] );

			return (v1-v0).fCross( v2-v0 );
		}

		tQuadric fGeneratePlaneConstraint( const tFace& face )
		{
			f64 len;
			const Math::tVec3d n = fComputeNormal( face ).fNormalize( len );
			return fGeneratePlaneConstraint( n, fGetVertPos( face[0] ), len * 0.5 );
		}

		tQuadric fGeneratePlaneConstraint( const Math::tVec3d& n, const Math::tVec3d& pos, const f64 area )
		{
			Math::tPlaned facePlane( n, pos );

			return tQuadric( facePlane.a, facePlane.b, facePlane.c, facePlane.d, area );
		}

		void fGenerateBorderConstraint( tVertID i, tVertID j, const tFaceList& faces )
		{
			sigassert( faces.fCount( ) == 1 );

			const Math::tVec3d origin = mVerts[i].fGetPos( );
			const Math::tVec3d e = mVerts[j].fGetPos( ) - origin;

			for( u32 f = 0; f < faces.fCount( ); f++ )
			{
				Math::tVec3d n = fComputeNormal( mFaces[ faces[f] ] );
				n = e.fCross( n ).fNormalize( );

				tQuadric Q = fGeneratePlaneConstraint( n, origin, e.fDot( e ) );

				Q *= 1000.0;
				Q /= Q.fGetArea( );

				mVerts[i].mQ += Q;
				mVerts[j].mQ += Q;
			}
		}

		void fCollectUnmarkedFaces( tVertID vid, tFaceList& faces )
		{
			sigassert( vid < mVerts.fCount( ) );

			const tVert& v = mVerts[vid];
			const tFaceList& neighbors = v.fGetNeighbors( );

			// Loop through neighbor faces and collect every unmarked face.
			for( u32 i = 0; i < neighbors.fCount( ); i++)
			{
				tFaceID fid = neighbors[i];
				if( !fGetFaceMark( fid ) )
				{
					faces.fPushBack( fid );
					fMarkFace( fid, 1 );
				}
			}
		}

		void fCollectNeighborFacesFromEdge( tVertID v0, tVertID v1, tFaceList& faces)
		{
			fMarkNeighborhood( v0, 1 );
			fMarkNeighborhood( v1, 0 );
			fCollectUnmarkedFaces( v0, faces );
		}

		/// 
		/// \brief
		/// Generates plane constraints for outside edges of the mesh to preserve
		/// them from being contracted.
		void fGenerateBoundaries( )
		{
			for( tVertID i = 0; i < mVerts.fCount( ); i++ )
			{
				tVertList star;
				fCollectNeighborVertsFromVert( i, star );

				for( u32 j = 0; j < star.fCount( ); j++ )
				{
					if( i >= star[j] )
						continue;

					tFaceList faces;
					fCollectNeighborFacesFromEdge( i, star[j], faces );

					if( faces.fCount( ) == 1 )
					{
						if( mDisregardEdges )
						{
							mVerts[i].fSetLocked( true );
							mVerts[ star[j] ].fSetLocked( true );
							break;
						}
						else
							fGenerateBorderConstraint( i, star[j], faces );
					}
				}
			}
		}

		void fCreatePair( tVertID v0, tVertID v1 )
		{
			tContractPair* pair = new tContractPair( v0, v1 );

			fGetVert( v0 ).fAddPair( pair );
			fGetVert( v1 ).fAddPair( pair );

			fComputePair( pair );

#ifdef TEST_PRIORITY_QUEUE
			mPairs.fPut( pair );
#else
			mPairs.fPushBack( pair );
#endif
		}

		/// 
		/// \brief Checks the list of faces have the correct normal orientation.
		b32 fCheckNeighborFacesPointUp( const tFaceList& neighbors )
		{
			static const Math::tVec3d cUp( 0.0, 1.0, 0.0 );

			for( u32 i = 0; i < neighbors.fCount( ); ++i )
			{
				const tFaceID thisFace = neighbors[i];
				const u32 mark = fGetFaceMark( thisFace );
				if( mark == 2 )
					continue;

				const tFace& f = mFaces[ thisFace ];
				const Math::tVec3d faceNorm = fComputeNormal( f );

				fMarkFace( thisFace, 2 );

				// If this face's normal is pointing down, that's a bad triangle.
				if( faceNorm.y <= 0.0 )
					return true;
			}

			return false;
		}

		b32 fIsDegenerate( tVertID vIdx0, tVertID vIdx1, tVert& v0, tVert& v1, const Math::tVec3d& cand )
		{
			// Set up the faces so that we know mark == 1 is a face that will be changed but not destroyed.
			fMarkNeighborhood( vIdx0, 0 );
			fMarkNeighborhood( vIdx1, 0 );
			fMarkNeighborhood( vIdx0, 1 );
			fMarkNeighborhoodIncrement( vIdx1, 1 );

			const Math::tVec3d originalV0 = v0.fGetPos( );
			const Math::tVec3d originalV1 = v1.fGetPos( );

			// Fake move the verts to post-contraction position.
			v0.fSetPos( cand );
			v1.fSetPos( cand );

			b32 foundBad = fCheckNeighborFacesPointUp( v0.fGetNeighbors( ) );
			if( !foundBad )
				foundBad = fCheckNeighborFacesPointUp( v1.fGetNeighbors( ) );

			// Reset verts to original positions.
			v0.fSetPos( originalV0 );
			v1.fSetPos( originalV1 );

			return foundBad;
		}

		void fComputePair( tContractPair* pair )
		{
			const tVertID vIdx0 = pair->fV0( );
			const tVertID vIdx1 = pair->fV1( );
			tVert& v0 = fGetVert( vIdx0 );
			tVert& v1 = fGetVert( vIdx1 );

			tQuadric Q = v0.mQ;
			Q += v1.mQ;

			// Determine candidate for pair and store cost.
			pair->fComputeCandidateAndCost( Q, v0.fGetPos( ),v1.fGetPos( ) );

			// Add penalty for degeneracy.
			if( fIsDegenerate( vIdx0, vIdx1, v0, v1, pair->fGetCandidate( ) ) )
				pair->fAddPenaltyCost( 1000000.f );
		}
			

		void fFillDecimationData( tContractPair* pair, tDecimateEvent& eventData )
		{
			eventData.v0 = pair->fV0( );
			eventData.v1 = pair->fV1( );
			eventData.mCandidate = pair->fGetCandidate( );

			fMarkNeighborhood( eventData.v0, 0 );
			fMarkNeighborhood( eventData.v1, 0 );
			fMarkNeighborhood( eventData.v0, 1 );
			fMarkNeighborhoodIncrement( eventData.v1, 1 );

			fPartitionMarkedNeighbors( eventData.v0, 2, eventData.mChangedFaces, eventData.mDeadFaces );
			eventData.mChangedFaces.fSetCount( 0 );
			fPartitionMarkedNeighbors( eventData.v1, 2, eventData.mChangedFaces, eventData.mDeadFaces );
		}

		void fPerformDecimation( const tDecimateEvent& eventData )
		{
			// Track valid prims.
			--mValidVerts;
			mValidFaces -= eventData.mDeadFaces.fCount( );

			tVert& v0 = fGetVert( eventData.v0 );
			tVert& v1 = fGetVert( eventData.v1 );

			// Accumulate Q.
			v0.mQ += v1.mQ;

			// Fix up and throw out duplicate pairs.
			ProcessEdges( eventData );

			// Change model geometry.
			fContractPair( eventData );

			// Update the edges with new info.
			for( u32 i = 0; i < v0.fNumPairs( ); ++i )
			{
				tContractPair* v0Pair = v0.fGetPair( i );
				fComputePair( v0Pair );
#ifdef TEST_PRIORITY_QUEUE
				mPairs.fUpdate( v0Pair );
#endif

				const tVertID otherVertIdx = v0Pair->fGetOtherVert( eventData.v0 );
				tVert& otherVert = mVerts[ otherVertIdx ];

				for( u32 j = 0; j < otherVert.fNumPairs( ); ++j )
				{
					// Don't need to compute this pair, it's connected to v0.
					tContractPair* otherPair = otherVert.fGetPair( j );
					if( otherPair->fGetOtherVert( otherVertIdx ) == eventData.v0 )
						continue;

					fComputePair( otherPair );

#ifdef TEST_PRIORITY_QUEUE
					mPairs.fUpdate( otherPair );
#endif
				}
			}
		}

		/// 
		/// \brief Moves all the pairs from the destroyed vertex to the target.
		/// Ensures no duplicate pairs.
		void ProcessEdges( const tDecimateEvent& eventData )
		{
			tVert& v0 = fGetVert( eventData.v0 );
			tVert& v1 = fGetVert( eventData.v1 );

			tVertList pairNeighbors;

			for( u32 i = 0; i < v0.fNumPairs( ); ++i )
				pairNeighbors.fPushBack( v0.fGetPair( i )->fGetOtherVert( eventData.v0 ) );

			for( u32 i = 0; i < v1.fNumPairs( ); ++i )
			{
				tContractPair* pair = v1.fGetPair( i );
				tVertID otherVert = pair->fGetOtherVert( eventData.v1 );
				sigassert( otherVert != eventData.v1 );

				if( otherVert == eventData.v0 || pairNeighbors.fFind( otherVert ) )
				{
					// Destroy extra pair. (Or the pair between v0 and v1)
					const b32 success = fGetVert( otherVert ).fRemovePair( pair );
					sigassert( success );
					fDeletePair( pair );
				}
				else
				{
					// Move this pair to v0.
					pair->fSetVerts( eventData.v0, otherVert );
					v0.fAddPair( pair );
				}
			}

			v1.fClearPairs( );
		}

		void fContractPair( const tDecimateEvent& eventData )
		{
			// Alter the geometry.
			tVert& v0 = fGetVert( eventData.v0 );
			v0.fSetPos( eventData.mCandidate );

			// Clear dead faces.
			for( u32 i = 0; i < eventData.mDeadFaces.fCount( ); ++i )
				fDestroyFace( eventData.mDeadFaces[i] );

			// Update altered faces.
			for( u32 i = 0; i < eventData.mChangedFaces.fCount( ); ++i )
			{
				const tFaceID idx = eventData.mChangedFaces[i];
				fGetFace( idx ).fRemapVert( eventData.v1, eventData.v0 );
				v0.fAddNeighbor( idx );
			}

			// Destroy v1.
			fDestroyVert( eventData.v1 );
		}

		void fDeletePair( tContractPair* pair )
		{
#ifdef TEST_PRIORITY_QUEUE
			mPairs.fErase( pair );
#else
			mPairs.fFindAndErase( pair );
#endif
			delete pair;
		}

		void fDestroyVert( tVertID idx )
		{
			tVert& v = mVerts[idx];

			v.fMarkInvalid( );
			v.fClearNeighbors( );
		}

		void fDestroyFace( tFaceID idx )
		{
			tFace& f = fGetFace( idx );
			f.fMarkInvalid( );

			fGetVert( f[0] ).fRemoveNeighbor( idx );
			fGetVert( f[1] ).fRemoveNeighbor( idx );
			fGetVert( f[2] ).fRemoveNeighbor( idx );
		}

		void fDumpInfo( ) 
		{
#ifndef TEST_PRIORITY_QUEUE
			std::sort( mPairs.fBegin( ), mPairs.fEnd( ), tSortPairs( ) );
#endif

			log_line( 0, "===================================================" );
			for( u32 i = 0; i < mFaces.fCount( ); ++i )
			{
				if( !mFaces[i].fIsValid( ) )
				{
					log_line( 0, "Gone " );
				}
				else
					log_line( 0, "Face " << i << ":   " << mFaces[i][0] << "," << mFaces[i][1] << "," << mFaces[i][2] );
			}
			log_line( 0, "===================================================" );

			log_line( 0, "===================================================" );
			for( u32 i = 0; i < mVerts.fCount( ); ++i )
			{
				if( !mVerts[i].fIsValid( ) )
				{
					log_line( 0, "Gone " );
				}
				else
					log_line( 0, "Vert " << i << ":   " << mVerts[i].fGetPos( ).x << "," << mVerts[i].fGetPos( ).y << "," << mVerts[i].fGetPos( ).z );
			}
			log_line( 0, "===================================================" );

#ifndef TEST_PRIORITY_QUEUE
			log_line( 0, "===================================================" );
			for( u32 i = 0; i < mPairs.fCount( ); ++i )
			{
				log_line( 0, "Pair	" << i << ":		" << mPairs[i]->fV0( ) << "," << mPairs[i]->fV1( ) 
					<< "	Cost: " << mPairs[i]->fGetCost( )
					<< "	Cand: " << mPairs[i]->fGetCandidate( ).x << "," << mPairs[i]->fGetCandidate( ).y << "," << mPairs[i]->fGetCandidate( ).z );
			}
			log_line( 0, "===================================================" );
#endif
		}

		void fProbeContractionIntegrity( tDecimateEvent& thisContraction )
		{
			tVert& v0 = mVerts[ thisContraction.v0 ];
			tVert& v1 = mVerts[ thisContraction.v1 ];

			if( fIsDegenerate( thisContraction.v0, thisContraction.v1, v0, v1, thisContraction.mCandidate ) )
			{
				sigassert( 0 );
			}
		}

		void fProbeAllPairsIntegrity( )
		{
			//for( u32 i = 0; i < mPairs.fCount( ); ++i )
			//{
			//	tContractPair* pair = mPairs[i];

			//	const u32 vIdx0 = pair->fV0( );
			//	const u32 vIdx1 = pair->fV1( );

			//	tVert& v0 = mVerts[ vIdx0 ];
			//	tVert& v1 = mVerts[ vIdx1 ];

			//	const b32 isDegen = fIsDegenerate( vIdx0, vIdx1, v0, v1, pair->fGetCandidate( ) );
			//	const b32 causedDegen = pair->fGetCausesDegeneracy( );

			//	if( isDegen ^ causedDegen )
			//	{
			//		sigassert( 0 );
			//	}
			//}
		}
	};


	void fSimplify( tGrowableArray< Math::tVec3f >& verts, tGrowableArray< Math::tVec3u >& triangleIndices, const f32 optimizeTarget, const b32 disregardEdges, const b32 reIndex )
	{
		const u32 numInputVerts = verts.fCount( );
		const u32 numInputTris = triangleIndices.fCount( );
		const Time::tStamp timeBegin = Time::fGetStamp( );

		tConnectiveMesh mesh( verts, triangleIndices, optimizeTarget, disregardEdges );
		if( reIndex )
			mesh.fGetResults( verts, triangleIndices );
		else
			mesh.fGetResultsNoReIndex( verts, triangleIndices );

		const Time::tStamp timeEnd = Time::fGetStamp( );
		const u32 numOutputVerts = verts.fCount( );
		const u32 numOutputTris = triangleIndices.fCount( );
		log_line( 0, "Mesh simplification results: tris=" << (100.f*numOutputTris)/numInputTris << "%, verts=" << (100.f*numOutputVerts)/numInputVerts << "%, time=" << Time::fGetElapsedS( timeBegin, timeEnd ) << "s" );
	}
}}
