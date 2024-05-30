//------------------------------------------------------------------------------
// \file tProgressiveMeshTool.hpp - 18 Jul 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tProgressiveMeshTool__
#define __tProgressiveMeshTool__

#include "tProgressiveMesh.hpp"
#include "tPriorityQueue.hpp"
#include "Gfx\tVertexFormat.hpp"


namespace Sig { namespace LOD 
{
	///
	/// \class tQuadric
	/// \brief Error metric object
	class tools_export tQuadric
	{
	public:

		static u32 fCalculateDimension( const Gfx::tVertexFormat & vf );
		static u32 fCalculateCoeffsNeeded( u32 dimension );
		static u32 fCalculateSize( u32 coeffsNeeded );

		// NOTE: Must not fDelete quadrics from fInPlaceNew
		// ASSUMES: mem hast at least fCalculateSize( fCalculateCoeffsNeeded( dimension ) ) bytes
		// ASSUMES: coeffsNeeded == fCalculateCoeffsNeeded( dimension ), asserts this
		static tQuadric * fInPlaceNew( void * mem, u32 dimension, u32 coeffsNeeded );

		static tQuadric * fNew( u32 dimension );
		static void fDelete( tQuadric * q );

		// Converts the vector v to vRn where vRn has only the vertex elements that 
		// quadrics can operate on
		// ASSUMES: vRn is fCalculateDimension( vf ) size
		static void fConvertToRn( f64 * vRn, const void * v, const Gfx::tVertexFormat & vf );

		// Converts the Rn vector vRn into the vector v with format specified by vf
		// ASSUMES: vRn is fCalculateDimension( vf ) size
		static void fConvertFromRn( void * v, const f64 * vRn, const Gfx::tVertexFormat & vf );

	public:

		inline u32 fDimension( ) const { return mDimension; }
		
		// Builds a fundamental quadric
		void fBuild( 
			const void * v0, 
			const void * v1, 
			const void * v2,
			const Gfx::tVertexFormat & vf );

		// ASSUMES: v is dim length 
		b32 fCalculateOptimal( f64 * v ) const;
		f64 fCalculateCost( const f64 * v ) const;

		// Q must be of same dimension to avoid assert
		tQuadric& operator+=( const tQuadric& Q );
		tQuadric& operator=( const tQuadric& Q );

	private:

		tQuadric( const tQuadric& ); // Disable copy construct

		tQuadric( u32 dimension, u32 coeffCount ); // CTOR


		u32 fTensorCoeffCount( ) const;
		void fTensor( f64 * m ) const; // ASSUMES: m is dim x dim size

	private:
		
		const u8 mDimension;
		const u8 mCoeffCount;
		u16 mUnused;

		// Coeffs are stored for A, b, c in order
		f64 mCoeffs[1];
	};

	///
	/// \class tFace
	/// \brief Tracks face information
	struct tools_export tFace
	{ 
		tFace( ) : mIsDead( false ) { mVertIds.fFill( ~0 ); }

		u32 fOtherVert( u32 notVId0, u32 notVId1 ) const;
		void fOtherVerts( u32 notVId, u32 & vIdOut0, u32 & vIdOut1 ) const;
		u32 fReplaceVert( u32 vOld, u32 vNew ); // returns index of changed vert

		b32 mIsDead;
		tFixedArray<u32, 3> mVertIds; // Verts which make up this face
	};
	typedef tGrowableArray< tFace > tFaceList;
	typedef tPair<u32, u32> tFaceChange; // { FaceIndex, IndexOnFace }

	///
	/// \class tVertex
	/// \brief Tracks vertex information
	struct tools_export tVertex
	{
		enum tLockedReason
		{
			cLockedReasonBoundary = ( 1 << 0 ),
			cLockedReasonCascade = ( 1 << 1 )
		};

		tVertex( ) : mIsDead( false ), mLockedReasons( 0 ), mData( NULL ), mQuadric( NULL ) { }

		b8 mIsDead; // Is this vertex still in play?
		u8 mLockedReasons; // Can this vert be dropped or moved?
		u16 mUnused;

		void * mData; // Vertex info
		tQuadric * mQuadric; // QEM
		tGrowableArray<u32> mFaceIds; // Faces which share this vert
		tGrowableArray<u32> mEdgeIds; // Edges which share this vert
	};
	typedef tGrowableArray< tVertex > tVertexList;

	///
	/// \class tEdge
	/// \brief Tracks edge information
	struct tools_export tEdge
	{
		tEdge( ) 
			: mIsDead( false )
			, mCancelledReasons( 0 )
			, mContractionCost( 0.f )
			, mTarget( NULL ) { mVertIds.fFill( ~0 ); }
		
		u32 fOtherVert( u32 vId ) const;
		void fReplaceVert( u32 vOld, u32 vNew );

		tFixedArray<u32, 2> mVertIds; // Ids of the verts which make this edge

		b8 mIsDead;
		u8 mCancelledReasons;
		u16 mUnused;

		f32 mContractionCost;
		f64 * mTarget;
	};
	typedef tGrowableArray< tEdge > tEdgeList;

	///
	/// \class tContractionRecord
	/// \brief Used for tracking contractions to undo
	struct tools_export tContractionRecord : public tRefCounter
	{
		struct tVertInfo
		{
			u32 mId;
			tDynamicBuffer mData;
		};

		tVertInfo mVertChanged; 
		tVertInfo mVertDestroyed;

		tGrowableArray< tFaceChange > mFaceChanges;
		tGrowableArray<u32> mFacesDestroyed; // Face indices
	};

	define_smart_ptr( tools_export, tRefCounterPtr, tContractionRecord );
	typedef tGrowableArray< tContractionRecordPtr > tContractionList;

	///
	/// \class tDecimationControls
	/// \brief Tweakable controls of the simplification algorithm
	struct tools_export tDecimationControls
	{
		enum tTargetPolicy
		{
			// Attempts QEM optimal, otherwise minimum of v0, v1, Lerp( v0, v1, 0.5 )
			cTargetPolicyOptimal = 0,

			// Chooses mimimum of v0, v1
			cTargetPolicySubset,

			cTargetPolicyCount
		};

		tDecimationControls( )
			: mTargetPolicy( cTargetPolicyOptimal )
			, mCascadeTolerance( -1 ) { }

		b32 fPolicyIs( tTargetPolicy test ) const { return mTargetPolicy == test; }
		b32 fRestrictsCascades( ) const { return mCascadeTolerance >= 0.f; }

		// How is the target point for an edge collapse chosen
		tTargetPolicy mTargetPolicy;

		// If < 0 then all cascaded collapses are allowed, otherwise
		// only if the cost of the next available non-cascade collapse
		// is within tolerance * average from the best collapse will
		// the the collapse be allowed. Use Math::cInfinity to signal
		// that any non-cascaded collapse is allowed
		f32 mCascadeTolerance;
	};

	///
	/// \class tMeshConnectivity
	/// \brief Represents a connected mesh allowing for QEM based simplification	
	class tools_export tMeshConnectivity
	{

	public:

		tMeshConnectivity(
			const Gfx::tVertexFormat & vf,
			const void * vb, u32 vbCount,
			const u32 * ib, u32 ibCount,
			const tDecimationControls & controls );

		~tMeshConnectivity( );

		const tDecimationControls & fControls( ) const { return mControls; }

		// Access to vert information
		const Gfx::tVertexFormat & fVertexFormat( ) const { return mVertexFormat; }
		u32 fActiveVertCount( ) const { return mActiveVertCount; }

		// Access to face information
		u32 fActiveFaceCount( ) const { return mActiveFaceCount; }
		const tFaceList & fFaces( ) const { return mFaces; }

		// Access to the list of edge contractions that have occurred
		const tContractionList & fContractions( ) const { return mContractions; }

		// Access to the list of indices into the contraction table that mark
		// where cascade information was reset
		const tGrowableArray<u32> & fCascadeResetMarkers( ) const { return mCascadeResetMarkers; }

		// Does the target selection policy create new points?
		b32 fTargetPolicyRequiresData( );

		// The cost of the contraction that will be performed if fDecimate is called
		// and the edge passes any control restrictions, e.g. cascade tolerance
		f32 fNextDecimationCost( ) const;

		// Performs one edge contraction on the mesh
		b32 fDecimate( );

		// Runs through the connected mesh and unlocks any verts and edges that were locked
		// due to cascade restrictions, essentially restarting the cascade tolerance. Returns
		// the number of edges that were unlocked
		u32 fResetCascades( );

		// Counts the number of faces after the conversion for the index buffer starting at
		// oldTriOffset and going for olNumTris faces
		u32 fCalculateNewFaceCount( u32 oldTriOffset, u32 oldNumTris ) const;

		// Capture the geometric state of the mesh connectivity and optionally the index/face remapping
		void fCapture( 
			Gfx::tGeometryBufferSysRam & vb, 
			Gfx::tIndexBufferSysRam &ib, 
			tDynamicArray<u32> * indexRemapping = NULL,
			tDynamicArray<u32> * faceRemapping = NULL) const;

	private:

		struct tCostCompare
		{
			inline b32 operator()( const tEdge * e0, const tEdge * e1 )
			{
				return e0->mContractionCost < e1->mContractionCost;
			}
		};

	private:

		void fBuild( const void * vb, const u32 * ib );
		void fValidateBuild( );

		void fUpdateCandidate( tEdge * edge );
		b32 fSelectTarget( tEdge & edge );
		void fSelectOptimalTarget( tEdge & edge );
		void fSelectSubsetTarget( tEdge & edge );
		void fApplyConsistencyChecks( tEdge & edge );
		f32 fTestNeighborhoodConsistency( 
			const f64 * vRn, const u32 vId, 
			const u32 * faceIdsToIgnore, u32 ignoreCount );

	private:

		tDecimationControls mControls;
		Gfx::tVertexFormat mVertexFormat;
		
		u32 mQuadricDimension;
		u32 mActiveVertCount;
		u32 mActiveFaceCount;
		u32 mCancelledEdgeCount;

		f32 mHighestCost;
		f32 mAverageCost;
		f32 mLowestCascadeCancelledCost;

		tQuadric * mHelperQuadric;

		tDynamicBuffer mVertDataBuffer; // mData, mQuadric
		tDynamicArray<f64> mEdgeTargetBuffer; // mTarget


		tVertexList mVerts;
		tFaceList mFaces;
		tEdgeList mEdges;

		tPriorityQueue<tEdge*, tCostCompare> mCandidates;
		tContractionList mContractions;
		tGrowableArray<u32> mCascadeResetMarkers;
	};

	///
	/// \class tProgressiveMeshTool
	/// \brief Supports the creation of a progressive mesh object by
	///		   simplifying an underlying mesh and converting contraction records
	class tools_export tProgressiveMeshTool : public tProgressiveMesh
	{
	public:


		// ASSUMES: triCount == ibCount / 3, i.e. ib is a triangle list
		tProgressiveMeshTool( 
			const Gfx::tVertexFormat & vf,
			const void * vb, u32 vbCount,
			const u32 * ib, u32 ibCount,
			const tDecimationControls & controls );

		// Access to the underlying mesh connectivity information
		const tMeshConnectivity & fMeshConnectivity( ) const { return mMesh; }

		// Reduce the mesh, returns number of decimations performed
		// NOTE: these calls act from the current state
		u32 fReduceToRatio( f32 percentage );
		u32 fReduceToCount( u32 faceCount );
		u32 fReduceToCost( f32 cost );

		// Converts the current state of the decimated connectivity mesh into
		// the underlying progressive mesh
		void fCapture( ); 

		// Captures the mesh state
		void fCaptureM0( );

	private:

		b32 fDoDecimate( );

	private:

		tMeshConnectivity mMesh;
		tGrowableArray<u32> mCascadeResets;

	};

}} // namespace ::Sig::LOD

#endif//__tProgressiveMeshTool__
