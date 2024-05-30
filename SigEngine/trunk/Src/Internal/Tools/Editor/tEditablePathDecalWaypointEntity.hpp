#ifndef __tEditablePathDecalWaypointEntity__
#define __tEditablePathDecalWaypointEntity__
#include "Editor/tEditableWaypointBase.hpp"
#include "Gfx/tStaticDecalTris.hpp"
#include "Sigml.hpp"

namespace Sig { namespace Sigml { class tPathDecalObject; class tPathDecalWaypointObject; } }

namespace
{
	static const char* fEditablePropDiffuseTextureFilePath( ) { return "SplineDecal.TextureDiffuse"; }
	static const char* fEditablePropNormalMapFilePath( ) { return "SplineDecal.TextureNormal"; }
}

namespace Sig
{
	class tEditablePathDecalWaypoint;
	define_smart_ptr( tools_export, tRefCounterPtr, tEditablePathDecalWaypoint );

	class tEditablePathDecalEntity;
	define_smart_ptr( tools_export, tRefCounterPtr, tEditablePathDecalEntity );


	/// 
	/// \brief A single waypoint to use in a path decal line.
	class tools_export tEditablePathDecalWaypoint : public tEditableObject
	{
		define_dynamic_cast( tEditablePathDecalWaypoint, tEditableObject );

		Gfx::tRenderableEntityPtr mShellSphere;
		Gfx::tRenderState mShellRenderState;

		s32 mInsertIdx;
		b32 mCloneDirty;
		b32 mDisableRefresh;

		tEditablePathDecalEntityPtr mParent;

		friend tEditablePathDecalEntity;

	public:
		tEditablePathDecalWaypoint( tEditableObjectContainer& container );
		tEditablePathDecalWaypoint( tEditableObjectContainer& container, const Sigml::tPathDecalWaypointObject* ao );
		virtual ~tEditablePathDecalWaypoint( );

		virtual std::string fGetToolTip( ) const;
		void fAddToWorld( );
		void fRemoveFromWorld( );

		virtual tEntityPtr fClone( );

		void fDisableGeometryUpdates( b32 disable );

		void fSetParent( tEditablePathDecalEntity* parent ) { mParent = tEditablePathDecalEntityPtr( parent ); } 
		tEditablePathDecalEntity* fGetParent( ) { return mParent.fGetRawPtr( ); }

		virtual tEditablePropertyTable&			fGetEditableProperties( );
		virtual const tEditablePropertyTable&	fGetEditableProperties( ) const;

		virtual b32 fSupportsRotation( ) { return false; }

		void fNotifyParentNeedsUpdate( );

		void fAcquireEntireDecal( tGrowableArray<tEditablePathDecalWaypoint*>& wayPoints );

		void fAfterAllObjectsCloned( const tEditorSelectionList& siblingObjects );
		void fPreStateChange( );
		void fPostStateChange( );

	private:
		void fCommonCtor( );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		void fOnMoved( b32 recomputeParentRelative );
		void fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint );
	};


	struct tTexturedTri : public Math::tTrianglef
	{
		tTexturedTri( ) { }

		tTexturedTri( 
			const Math::tVec3f& a,
			const Math::tVec3f& b,
			const Math::tVec3f& c )
			: Math::tTrianglef( a, b, c )
		{
		}

		Math::tVec3f fVert( u32 i ) const
		{
			if( i == 0 )
				return mA;
			else if( i == 1 )
				return mB;

			return mC;
		}

		Math::tVec2f fUV( u32 i ) const
		{
			if( i == 0 )
				return mUVA;
			else if( i == 1 )
				return mUVB;

			return mUVC;
		}

		Math::tVec2f mUVA;
		Math::tVec2f mUVB;
		Math::tVec2f mUVC;
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tTexturedTri& o )
	{
		s( "A", o.mA );
		s( "B", o.mB );
		s( "C", o.mC );

		s( "UVA", o.mUVA );
		s( "UVB", o.mUVB );
		s( "UVC", o.mUVC );
	}



	struct tVertexData
	{
		tVertexData( )
			: mN( Math::tVec3f::cZeroVector )
			, mTan( Math::tVec4f::cZeroVector )
			, mBitan( Math::tVec3f::cZeroVector )
			, mNumNorms( 0 )
		{ }

		Math::tVec3f mPos;
		Math::tVec2f mUV;
		Math::tVec3f mN;
		Math::tVec4f mTan;
		Math::tVec3f mBitan;
		u32 mNumNorms;
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tVertexData& o )
	{
		s( "norm", o.mN );
		s( "pos", o.mPos );
		s( "uv", o.mUV );
		s( "tan", o.mTan );
		s( "numnorms", o.mNumNorms );
	}



	class tools_export tEditablePathDecalEntity : public tEditableObject
	{
		define_dynamic_cast( tEditablePathDecalWaypoint, tEditableObject );

		Gfx::tWorldSpaceLinesPtr		mConnectionLines;
		Gfx::tWorldSpaceLinesPtr		mWireframeTris; //!< Also the OBBs
		Gfx::tStaticDecalGeometryPtr	mTexturedTris;

		struct tSubPoint
		{
			tSubPoint( ) : mRealControlIdx( -1 ) { }

			tSubPoint( const Math::tMat3f& xform, const Math::tAabbf& box )
				: mXForm( xform )
				, mBoundingBox( box )
				, mRealControlIdx( -1 )
			{ }

			tSubPoint( 
				const Math::tMat3f& xform, 
				const Math::tVec3f& left, 
				const Math::tVec3f& right, 
				const Math::tAabbf& box,
				const Math::tPlanef& hinge )
				: mXForm( xform )
				, mLeft( left )
				, mRight( right )
				, mHingePlane( hinge )
				, mBoundingBox( box )
				, mRealControlIdx( -1 )
			{ }

			Math::tMat3f mXForm;
			Math::tVec3f mLeft, mRight;
			Math::tPlanef mHingePlane;
			Math::tAabbf mBoundingBox;

			s32 mRealControlIdx;
		};

		tGrowableArray< tEditablePathDecalWaypointPtr >	mControlWaypoints;
		tGrowableArray< tSubPoint >						mFullWithSubs;
		tGrowableArray< Math::tVec3u >					mSaveTris;
		tGrowableArray< Math::tVec3f >					mSaveVerts;
		tGrowableArray< Math::tVec2f >					mSaveUVs;
		Math::tAabbf									mBounds;

		tDynamicArray<u32> mCachedWaypointGuids;

		tResourcePtr mDiffuseTextureRes;
		tResourcePtr mNormalMapRes;

		b32 mDisableRefresh;

		f32 mPathLength;

	public:
		tEditablePathDecalEntity( tEditableObjectContainer& container );
		tEditablePathDecalEntity( tEditableObjectContainer& container, const Sigml::tPathDecalObject* ao );
		virtual ~tEditablePathDecalEntity( );

		void fAddBack( tEditablePathDecalWaypoint* add );
		void fAddFront( tEditablePathDecalWaypoint* add );
		void fRemoveBack( tEditablePathDecalWaypoint* remove );

		void fTickUpdate( );

		void fDisableGeometryUpdates( b32 disable ) { mDisableRefresh = disable; }
		void fNotifyNeedsUpdate( );

		/// 
		/// \brief Will combine two path decal lines. The line containing the added waypoint will be discarded.
		void fJoin( tEditablePathDecalWaypoint* anchorWaypoint, tEditablePathDecalWaypoint* addedWaypoint );
		b32 fCanJoin( tEditablePathDecalWaypoint* anchorWaypoint, tEditablePathDecalWaypoint* addedWaypoint );

		/// 
		/// \brief Creates a new path decal that contains all the points after the split point.
		tEditablePathDecalEntity* fSplit( tEditablePathDecalWaypoint* splitPoint );
		b32 fCanSplit( tEditablePathDecalWaypoint* splitPoint );

		const tGrowableArray< tEditablePathDecalWaypointPtr >& fGetWaypoints( ) const { return mControlWaypoints; }
		void fAddWaypoints( const tGrowableArray< tEditablePathDecalWaypointPtr >& newWaypoints );
		void fRemoveWaypoints( const tGrowableArray< tEditablePathDecalWaypointPtr >& removedWaypoints );

		/// 
		/// \brief Returns the index of the eliminated one so that it can be tracked and re-inserted.
		s32 fEliminateNode( tEditablePathDecalWaypoint* findNode );
		void fInsertNode( tEditablePathDecalWaypoint* insertNode, u32 idx );
		
		s32 fFindIndex( tEditablePathDecalWaypoint* findNode ) const;
		
		/// 
		/// \brief Does an insert based on the nodes insert index. Requires all nodes in the array
		/// to have accurate insert indices.
		void fSortedInsert( tEditablePathDecalWaypoint* insertNode );
		s32 fBinarySearch( s32 search );

		void fRefreshGeometry( );
		void fRefreshTexture( );

		void fAcquireEntireDecal( tGrowableArray<tEditablePathDecalWaypoint*>& wayPoints );

		void fFixUpGuidRefs( const tHashTable< u32, u32 >& conversionTable );

		static void fConvertIndexedListToVertexData( 
			const tDynamicArray< Math::tVec3u >& idxs, 
			const tDynamicArray< Math::tVec3f >& verts, 
			const tDynamicArray< Math::tVec2f >& uvs, 
			tGrowableArray< tVertexData >& convertedVerts );

	private:
		void fCommonCtor( );

		void fAfterAllObjectsDeserialized( );
		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		static Math::tVec3f fComputeWallVector( const Math::tVec3f& current, const Math::tVec3f& next );

		virtual void fNotifyPropertyChanged( tEditableProperty& property );

		void fGenerateSubPoints( );

		void fDrawBox( tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const Math::tObbf& box, const u32 color );
		void fDrawConnections( tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const u32 color );

		void fRefreshTexHandles( );
		Gfx::tDecalMaterial* fConstructMaterial( );

		void fConvertTextureTriGeometry( 
			const tGrowableArray< tTexturedTri >& tris, 
			tGrowableArray< tVertexData >& outVerts, 
			tGrowableArray< Math::tVec3u >& outTris );
		void fBuildGeometry( 
			tGrowableArray< tVertexData >& outVerts, 
			tGrowableArray< Math::tVec3u >& outIdxs );
		typedef tHashTable< Math::tVec3f, u32 > tVertexSorter;
		void fProcessVerts( 
			const tTexturedTri& verts, 
			tVertexSorter& sorter, 
			tGrowableArray< tVertexData >& outCollectedVerts,
			tGrowableArray< Math::tVec3u >& outIdxs );
		static void fCalculateTangents( 
			const Math::tVec3f& v0, 
			const Math::tVec3f& v1, 
			const Math::tVec3f& v2, 
			const Math::tVec2f& uv0, 
			const Math::tVec2f& uv1, 
			const Math::tVec2f& uv2,
			tVertexData& outV0,
			tVertexData& outV1,
			tVertexData& outV2 );
		static void fFinalizeTangets( tVertexData& vert );
		void fProcessVert( 
			u32 i, 
			const Math::tVec3f& vert, 
			const Math::tVec3f& normal, 
			const Math::tVec2f& uv, 
			Math::tVec3u& outIdxSet, 
			tVertexSorter& vertexSorter,
			tGrowableArray< tVertexData >& outCollectedVerts );
	};

}

#endif//__tEditablePathDecalWaypointEntity__
