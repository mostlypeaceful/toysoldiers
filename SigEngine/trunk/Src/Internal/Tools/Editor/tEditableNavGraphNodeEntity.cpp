//------------------------------------------------------------------------------
// \file tEditableNavGraphNodeEntity.cpp - 06 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tEditableNavGraphNodeEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditableWaypointBase.hpp"
#include "tNavGraphEntity.hpp"

namespace Sig { namespace
{
	const f32 gSmallRadius = 0.1f;
	const f32 gBigRadius = 1.f;

	static const Math::tVec4f gInnerBallTint = Math::tVec4f( 0.1f, 0.3f, 1.0f, 0.75f );
	static const Math::tVec4f gShellSphereTint = Math::tVec4f( 0.85f, 0.85f, 0.85f, 0.35f );
	static const Math::tVec4f gConnectionTint = Math::tVec4f( 0.45f, 0.45f, 1.0f, 0.5f );
}}

namespace Sig { namespace Sigml
{
	//------------------------------------------------------------------------------
	// tNavGraphNodeObject
	//------------------------------------------------------------------------------
	class tNavGraphNodeObject : public tWaypointObjectBase
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tNavGraphNodeObject, 0x200B310 );

	public:
		// There is one of these quads for each outgoing connection. Back connections
		// do not track a quad. Also each quad has 4 verts just like a real quad!
		tDynamicArray< tDynamicArray<Math::tVec3f> > mQuads;

		tNavGraphNodeObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter ) { return NULL; } // Re-disable this type.
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlNodeObjectBase( tSerializer& s, tNavGraphNodeObject& o )
	{
		s( "Connections", o.mConnectionGuids );
		s( "BackConnections", o.mBackConnectionGuids );
		s( "Quads", o.mQuads );
	}

	register_rtti_factory( tNavGraphNodeObject, false );

	tNavGraphNodeObject::tNavGraphNodeObject( )
	{
	}
	void tNavGraphNodeObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlNodeObjectBase( s, *this );
	}
	void tNavGraphNodeObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlNodeObjectBase( s, *this );
	}
	tEditableObject* tNavGraphNodeObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableNavGraphNodeEntity( container, this );
	}

	//------------------------------------------------------------------------------
	// tNavGraphRootObject
	//------------------------------------------------------------------------------
	class tNavGraphRootObject : public tNavGraphNodeObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tNavGraphRootObject, 0xC85165BA );

		tHashTable<u32, u32> mGuidToIndex;
		tGrowableArray<AI::tBuiltPathNode> mNodes;
		tGrowableArray<AI::tBuiltPathEdge> mEdges;

	public:
		tNavGraphRootObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );

	private:
		u32 fGetOrAddNode( tNavGraphNodeObject* navObj );
		void fAddEdge( const tDynamicArray<Math::tVec3f>& quad, u32 orgNode, u32 endNode );
	};

	register_rtti_factory( tNavGraphRootObject, false );

	tNavGraphRootObject::tNavGraphRootObject( ) { }
	void tNavGraphRootObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlNodeObjectBase( s, *this );
	}
	void tNavGraphRootObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlNodeObjectBase( s, *this );
	}
	tEntityDef* tNavGraphRootObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tNavGraphEntityDef* def = new tNavGraphEntityDef;
		fConvertEntityDefBase( def, sigmlConverter );

		tGrowableArray< u32 > processedNodes;
		tGrowableArray< u32 > nodesToExamine;
		if( mConnectionGuids.fCount( ) > 0 )
			nodesToExamine.fPushBack( mConnectionGuids.fFront( ) );
		else if( mBackConnectionGuids.fCount( ) > 0 )
			nodesToExamine.fPushBack( mBackConnectionGuids.fFront( ) );
		else
		{
			log_warning( "Detected nav graph root disconnected from a graph." );
			return NULL;
		}

		while( nodesToExamine.fCount( ) > 0 )
		{
			const u32 thisGuid = nodesToExamine.fPopBack( );
			if( thisGuid == mGuid )
				continue; // Don't add the root node. It's just there to generate the graph and hold properties.

			tNavGraphNodeObject* navObj = (tNavGraphNodeObject*)sigmlConverter.fSigmlFile( ).fFindObjectByGuid( thisGuid ).fGetRawPtr( );

			// Create def data.
			const u32 nodeIdx = fGetOrAddNode( navObj );

			// Mark it inspected.
			processedNodes.fPushBack( thisGuid );

			// Add all potential forward connections.
			for( u32 i = 0; i < navObj->mConnectionGuids.fCount( ); ++i )
			{
				const u32 neighborGuid = navObj->mConnectionGuids[ i ];
				tNavGraphNodeObject* neighborObj = (tNavGraphNodeObject*)sigmlConverter.fSigmlFile( ).fFindObjectByGuid( neighborGuid ).fGetRawPtr( );
				const u32 neighborIdx = fGetOrAddNode( neighborObj );

				const u32* edgeIndex = mNodes[ nodeIdx ].mEdges.fFind( neighborIdx );
				if( edgeIndex )
					continue; // The edge connecting these already exists.

				fAddEdge( navObj->mQuads[i], nodeIdx, neighborIdx );

				// If the other node hasn't been processed, queue it.
				if( !processedNodes.fFind( neighborGuid ) )
					nodesToExamine.fFindOrAdd( neighborGuid );
			}

			// And potential backs.
			for( u32 i = 0; i < navObj->mBackConnectionGuids.fCount( ); ++i )
			{
				const u32 neighborGuid = navObj->mBackConnectionGuids[ i ];

				// If the other node hasn't been processed, queue it.
				if( !processedNodes.fFind( neighborGuid ) )
					nodesToExamine.fFindOrAdd( neighborGuid );
			}
		}

		tDynamicArray<AI::tBuiltPathNode> finalNodes( mNodes.fCount( ) );
		for( u32 i = 0; i < mNodes.fCount( ); ++i )
			finalNodes[i] = mNodes[i];
		tDynamicArray<AI::tBuiltPathEdge> finalEdges( mEdges.fCount( ) );
		for( u32 i = 0; i < mEdges.fCount( ); ++i )
			finalEdges[i] = mEdges[i];

		def->mNavGraph = new AI::tBuiltNavGraph( finalNodes, finalEdges );

		return def;
	}
	tEditableObject* tNavGraphRootObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableNavGraphRootEntity( container, this );
	}

	u32 tNavGraphRootObject::fGetOrAddNode( tNavGraphNodeObject* navObj )
	{
		// Get a node.
		const u32* index = mGuidToIndex.fFind( navObj->mGuid );
		if( index )
			return *index;

		// Or add a node.
		AI::tBuiltPathNode n;
		n.mObjToWorld = navObj->mXform;
		n.mWorldSpaceObb = Math::tObbf( Math::tAabbf( Math::tVec3f( -1.f, -1.f, -1.f ), Math::tVec3f( 1.f, 1.f, 1.f ) ), navObj->mXform );
		mNodes.fPushBack( n );
		const u32 nodeIndex = mNodes.fCount( ) - 1;
		mGuidToIndex.fInsert( navObj->mGuid, nodeIndex );
		return nodeIndex;
	}

	void tNavGraphRootObject::fAddEdge( const tDynamicArray<Math::tVec3f>& quad, u32 orgNode, u32 endNode )
	{
		sigassert( quad.fCount( ) == 4 );

		// Construct the edge.
		AI::tBuiltPathEdge edge;
		edge.mNodeA = orgNode;
		edge.mNodeB = endNode;

		const Math::tVec3f orgVert = mNodes[orgNode].mObjToWorld.fGetTranslation( );
		const Math::tVec3f endVert = mNodes[endNode].mObjToWorld.fGetTranslation( );
		Math::tVec3f to = endVert - orgVert;
		const Math::tVec3f normInSomeDirection = to.fNormalizeSafe( ).fCross( Math::tVec3f::cYAxis );
		
		const Math::tVec3f quadNorm = (quad[1] - quad[0]).fCross( quad[2] - quad[0] ).fNormalizeSafe( );

		tGrowableArray<Math::tVec3f> left, right; // Not guaranteed to be left or right.
		for( u32 i = 0; i < quad.fCount( ); ++i )
		{
			const Math::tVec3f& someVert = quad[i];
			const Math::tVec3f& someNextVert = quad[ (i+1)%4 ];
			const Math::tVec3f toSomeVert = someVert - orgVert;
			if( normInSomeDirection.fDot( toSomeVert ) > 0.f )
				left.fPushBack( someVert );
			else
				right.fPushBack( someVert );

			const Math::tVec3f thisPlaneNorm = quadNorm.fCross( (someNextVert - someVert).fNormalizeSafe( ) );
			edge.mHalfSpaces[i] = Math::tPlanef( thisPlaneNorm, someVert );
		}

		sigassert( left.fCount( ) == 2 && right.fCount( ) == 2 );
		edge.mHighEdge[0] = left[0];
		edge.mHighEdge[1] = left[1];
		edge.mLowEdge[0] = right[0];
		edge.mLowEdge[1] = right[1];

		// Record the edge.
		mEdges.fPushBack( edge );

		// Register the edge with both nodes.
		mNodes[ orgNode ].mEdges.fPushBack( endNode );
		mNodes[ endNode ].mEdges.fPushBack( orgNode );
	}
}}

namespace Sig
{
	

	tEditableNavGraphNodeEntity::tEditableNavGraphNodeEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
		, mConnectorOverride( 
			Gfx::tRenderState::cColorBuffer | Gfx::tRenderState::cDepthBuffer | Gfx::tRenderState::cAlphaBlend | Gfx::tRenderState::cPolyTwoSided, 
			Gfx::tRenderState::cBlendSrcAlpha,
			Gfx::tRenderState::cBlendOneMinusSrcAlpha,
			0 )
	{
		fCommonCtor( );
	}

	tEditableNavGraphNodeEntity::tEditableNavGraphNodeEntity( tEditableObjectContainer& container, const Sigml::tNavGraphNodeObject* ao )
		: tEditableObject( container )
		, mConnectorOverride( 
			Gfx::tRenderState::cColorBuffer | Gfx::tRenderState::cDepthBuffer | Gfx::tRenderState::cAlphaBlend | Gfx::tRenderState::cPolyTwoSided, 
			Gfx::tRenderState::cBlendSrcAlpha,
			Gfx::tRenderState::cBlendOneMinusSrcAlpha,
			0 )
	{
		fDeserializeBaseObject( ao );
		fCommonCtor( );

		mSavedConnectionGuids = ao->mConnectionGuids;
	}

	tEditableNavGraphNodeEntity::~tEditableNavGraphNodeEntity( )
	{
		fClearConnections( );
	}

	void tEditableNavGraphNodeEntity::fRefresh( )
	{
		fClearConnections( );

		const b32 nodeHid = fIsHidden( );
		if( !nodeHid )
		{
			if( !fSceneGraph( ) || !mInContainer ) 
				return;
		}

		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			const b32 connHid = mConnections[ i ]->fIsHidden( );

			if( !mConnections[ i ]->fSceneGraph( ) && !connHid )
			{
				// It's important the mGeometry and mConnections match.
				mGeometry.fPushBack( Gfx::tSolidColorQuadsPtr( NULL ) );
				continue;
			}

			// Get basic info
			const Math::tVec3f origin = fObjectToWorld( ).fGetTranslation( );
			const Math::tVec3f connectPos = mConnections[ i ]->fObjectToWorld( ).fGetTranslation( );
			f32 dummy = 0.f;
			const Math::tVec3f toConnectNorm = Math::tVec3f( connectPos - origin ).fNormalizeSafe( Math::tVec3f::cZAxis, dummy );

			// Get initial separation basis. (positive x axis to the right)
			Math::tMat3f basis = Math::tMat3f::cIdentity;
			basis.fOrientZAxis( toConnectNorm );

			Math::tVec3f v0, v1, v2, v3;

			// Do initial split and get best vert candidate.
			tGrowableArray<Math::tVec3f> vertsToSplit;
			tGrowableArray<Math::tVec3f> orgVertsLeft, orgVertsRight;
			tGrowableArray<Math::tVec3f> conVertsLeft, conVertsRight;
			fGetFlat( vertsToSplit );
			fSplitCorners( vertsToSplit, basis, origin, v0, v3, &orgVertsLeft, &orgVertsRight );

			mConnections[ i ]->fGetFlat( vertsToSplit );
			fSplitCorners( vertsToSplit, basis, connectPos, v1, v2, &conVertsLeft, & conVertsRight );

			Math::tVec3f dummyVec;

			// Do second split and check previous candidates against real facts.
			const Math::tVec3f rightLineNorm = (v2 - v3).fNormalizeSafe( Math::tVec3f::cZAxis, dummy );
			basis.fOrientZAxis( rightLineNorm );
			fSplitCorners( orgVertsRight, basis, origin, dummyVec, v3 );
			fSplitCorners( conVertsRight, basis, connectPos, dummyVec, v2 );

			const Math::tVec3f leftLineNorm = (v1 - v0).fNormalizeSafe( Math::tVec3f::cZAxis, dummy );
			basis.fOrientZAxis( leftLineNorm );
			fSplitCorners( orgVertsLeft, basis, origin, v0, dummyVec );
			fSplitCorners( conVertsLeft, basis, connectPos, v1, dummyVec );

			Gfx::tSolidColorQuads* newQuadGeom = new Gfx::tSolidColorQuads( );
			newQuadGeom->fResetDeviceObjects(
				Gfx::tDevice::fGetDefaultDevice( ), 
				mContainer.fGetSolidColorMaterial( ), 
				mContainer.fGetSolidColorGeometryAllocator( ), 
				mContainer.fGetSolidColorIndexAllocator( ) );
			newQuadGeom->fSetRenderStateOverride( &mConnectorOverride );
			newQuadGeom->fAddQuad( v0, v1, v2, v3 );
			newQuadGeom->fSubmitGeometry( );
			mGeometry.fPushBack( Gfx::tSolidColorQuadsPtr( newQuadGeom ) );

			// Only show if it's not hidden. The geometry still needs to exist so it can be
			// saved with the file.
			if( !connHid && !nodeHid )
			{
				Gfx::tRenderableEntityPtr newQuad;
				newQuad.fReset( new tDummyObjectEntity( 
					mGeometry.fBack( )->fGetModifiedRenderBatch( &mConnectorOverride ), 
					mGeometry.fBack( )->fGetBounds( ), true ) );
				newQuad->fSetInvisible( false );
				newQuad->fSpawnImmediate( fSceneGraph( )->fRootEntity( ) );

				mConnectionQuads.fPushBack( newQuad );
			}
		}

		sigassert( mGeometry.fCount( ) == mConnections.fCount( ) );

		tEditableObject::fUpdateStateTint( );
	}

	void tEditableNavGraphNodeEntity::fConnect( tEditableNavGraphNodeEntity* to, b32 removeConnection )
	{
		if( to == this )
			return;

		// If the to is a root node, it needs to do the root checks.
		if( !removeConnection )
		{
			tEditableNavGraphRootEntity* isRoot = to->fDynamicCast< tEditableNavGraphRootEntity >( );
			if( isRoot )
			{
				isRoot->fConnect( this, removeConnection );
				return;
			}
		}

		if( removeConnection )
		{
			if( mConnections.fFind( to ) )
			{
				mConnections.fFindAndErase( to );
				to->mBackConnections.fFindAndErase( this );
			}
			else if( to->mConnections.fFind( to ) )
			{
				to->mConnections.fFindAndErase( to );
				mBackConnections.fFindAndErase( this );
			}
		}
		else if( !to->mConnections.fFind( this ) && !mBackConnections.fFind( to ) )
		{
			mConnections.fFindOrAdd( tEditableNavGraphNodeEntityPtr( to ) );
			to->mBackConnections.fFindOrAdd( tEditableNavGraphNodeEntityPtr( this ) );
		}

		to->fRefresh( );
		fRefresh( );
	}

	void tEditableNavGraphNodeEntity::fDisconnect( b32 outwardOnly )
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			mConnections[ i ]->mBackConnections.fFindAndErase( tEditableNavGraphNodeEntityPtr( this ) );
		mConnections.fSetCount( 0 );
		fRefresh( );

		if( !outwardOnly )
		{
			for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			{
				mBackConnections[ i ]->mConnections.fFindAndErase( tEditableNavGraphNodeEntityPtr( this ) );
				mBackConnections[ i ]->fRefresh( );
			}
			mBackConnections.fSetCount( 0 );
		}
	}

	std::string tEditableNavGraphNodeEntity::fGetToolTip( ) const
	{
		return "Nav Graph Node";
	}

	void tEditableNavGraphNodeEntity::fAcquireEntireGraph( tGrowableArray<tEditableNavGraphNodeEntity*>& nodes )
	{
		tGrowableArray< tEditableNavGraphNodeEntity* > inspectedNodes;
		tGrowableArray< tEditableNavGraphNodeEntity* > nodesToExamine;
		nodesToExamine.fPushBack( this );

		while( nodesToExamine.fCount( ) > 0 )
		{
			tEditableNavGraphNodeEntity* thisNode = nodesToExamine.fPopBack( );

			nodes.fPushBack( thisNode );

			// Mark it inspected.
			inspectedNodes.fPushBack( thisNode );

			// Add all potential forward connections.
			for( u32 i = 0; i < thisNode->mConnections.fCount( ); ++i )
				if( !inspectedNodes.fFind( thisNode->mConnections[ i ] ) )
					nodesToExamine.fFindOrAdd( thisNode->mConnections[ i ].fGetRawPtr( ) );

			// And potential backs.
			for( u32 i = 0; i < thisNode->mBackConnections.fCount( ); ++i )
				if( !inspectedNodes.fFind( thisNode->mBackConnections[ i ] ) )
					nodesToExamine.fFindOrAdd( thisNode->mBackConnections[ i ].fGetRawPtr( ) );
		}
	}

	tEditableNavGraphRootEntity* tEditableNavGraphNodeEntity::fRoot( )
	{
		tGrowableArray< tEditableNavGraphNodeEntity* > inspectedNodes;
		tGrowableArray< tEditableNavGraphNodeEntity* > nodesToExamine;
		nodesToExamine.fPushBack( this );
		
		while( nodesToExamine.fCount( ) > 0 )
		{
			tEditableNavGraphNodeEntity* thisNode = nodesToExamine.fPopBack( );
			if( !thisNode->fSceneGraph( ) )
				continue;

			// Check if this node is the root.
			tEditableNavGraphRootEntity* root = thisNode->fDynamicCast< tEditableNavGraphRootEntity >( );
			if( root )
				return root;

			// Mark it inspected.
			inspectedNodes.fPushBack( thisNode );

			// Add all potential forward connections.
			for( u32 i = 0; i < thisNode->mConnections.fCount( ); ++i )
				if( !inspectedNodes.fFind( thisNode->mConnections[ i ].fGetRawPtr( ) ) )
					nodesToExamine.fFindOrAdd( thisNode->mConnections[ i ].fGetRawPtr( ) );

			// And potential backs.
			for( u32 i = 0; i < thisNode->mBackConnections.fCount( ); ++i )
				if( !inspectedNodes.fFind( thisNode->mBackConnections[ i ].fGetRawPtr( ) ) )
					nodesToExamine.fFindOrAdd( thisNode->mBackConnections[ i ].fGetRawPtr( ) );
		}

		return NULL;
	}

	void tEditableNavGraphNodeEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );
		if( !fIsFrozen( ) )
		{
			if( &entity == mDummySphere.fGetRawPtr( ) )
				mDummyBox->fSetRgbaTint( gInnerBallTint );
			else if( &entity == mShellBox.fGetRawPtr( ) )
				mShellBox->fSetRgbaTint( gShellSphereTint );

			for( u32 i = 0; i < mConnectionQuads.fCount( ); ++i )
				mConnectionQuads[i]->fSetRgbaTint( gConnectionTint );
		}
	}

	void tEditableNavGraphNodeEntity::fFixUpGuidRefs( const tHashTable< u32, u32 >& conversionTable )
	{
		for( u32 i = 0; i < mSavedConnectionGuids.fCount( ); ++i )
		{
			u32* foundIdx = conversionTable.fFind( mSavedConnectionGuids[ i ] );
			sigassert( foundIdx );
			mSavedConnectionGuids[ i ] = *foundIdx;
		}
	}

	void tEditableNavGraphNodeEntity::fCommonCtor( )
	{
		mShellRenderState = Gfx::tRenderState::cDefaultColorTransparent;

		// set the dummy object's bounds as our own
		Math::tAabbf localSpaceBox = mContainer.fGetDummySphereTemplate( ).fGetBounds( );
		localSpaceBox.mMin *= gBigRadius;
		localSpaceBox.mMax *= gBigRadius;
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		// setup the smaller center
		mDummyBox->fSetRgbaTint( gInnerBallTint );
		const Math::tMat3f mSmall( gSmallRadius );
		mDummyBox->fSetParentRelativeXform( mSmall );
		mDummyBox->fSetInvisible( false );

		// create the outer shell (larger radius, wireframe)
		mShellBox.fReset( new tDummyObjectEntity( 
			mContainer.fGetDummyBoxTemplate( ).fGetModifiedRenderBatch( &mShellRenderState ), 
			mContainer.fGetDummyBoxTemplate( ).fGetBounds( ), true ) );
		mShellBox->fSetRgbaTint( gShellSphereTint );
		const Math::tMat3f mBig( gBigRadius );
		mShellBox->fSetParentRelativeXform( mBig );
		mShellBox->fSpawnImmediate( *this );

		tEditableObject::fUpdateStateTint( );
	}

	void tEditableNavGraphNodeEntity::fAfterAllObjectsDeserialized( )
	{
		for( u32 i = 0; i < mSavedConnectionGuids.fCount( ); ++i )
		{
			tEditableObject* eo = mContainer.fFindObjectByGuid( mSavedConnectionGuids[ i ] );
			if( !eo ) continue;
			tEditableNavGraphNodeEntity* node = eo->fDynamicCast< tEditableNavGraphNodeEntity >( );
			if( !node ) continue;
			fConnect( node );
		}

		mSavedConnectionGuids.fDeleteArray( );
	}

	void tEditableNavGraphNodeEntity::fRecordConnections( Sigml::tNavGraphNodeObject* ao ) const
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			if( !mConnections[ i ]->fInContainer( ) && !mConnections[ i ]->fIsHidden( ) )
				continue;
			
			ao->mConnectionGuids.fPushBack( mConnections[ i ]->fGuid( ) );

			Gfx::tSolidColorRenderVertex* quadPtr = mGeometry[ i ]->fQuad( 0 );

			tDynamicArray<Math::tVec3f> thisQuad( 4 );
			for( u32 j = 0; j < 4; ++j )
				thisQuad[j] = quadPtr[j].mP;

			ao->mQuads.fPushBack( thisQuad );
		}

		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			if( mBackConnections[ i ]->fInContainer( ) )
				ao->mBackConnectionGuids.fPushBack( mBackConnections[ i ]->fGuid( ) );
	}

	Sigml::tObjectPtr tEditableNavGraphNodeEntity::fSerialize( b32 clone ) const
	{
		Sigml::tNavGraphNodeObject* ao = new Sigml::tNavGraphNodeObject( );
		fSerializeBaseObject( ao, clone );

		if( !clone )
			fRecordConnections( ao );

		return Sigml::tObjectPtr( ao );
	}

	void tEditableNavGraphNodeEntity::fClearConnections( )
	{
		for( u32 i = 0; i < mConnectionQuads.fCount( ); ++i )
		{
			if( mConnectionQuads[i]->fSceneGraph( ) )
				mConnectionQuads[i]->fDeleteImmediate( );
			mConnectionQuads[i].fRelease( );
		}
		mConnectionQuads.fDeleteArray( );
		mGeometry.fDeleteArray( );
	}

	void tEditableNavGraphNodeEntity::fGetFlat( tGrowableArray<Math::tVec3f>& outVerts )
	{
		Math::tVec3f trans = mShellBox->fObjectToWorld( ).fGetTranslation( );
		const Math::tObbf obb( mShellBox->fObjectSpaceBox( ), mShellBox->fObjectToWorld( ) );
		outVerts.fSetCount( 4 );

		// ^
		// z-
		//  x+ >
		//
		// 3-0
		// | |
		// 2-1

		outVerts[0] = obb.mExtents.x * obb.mAxes[0] + -obb.mExtents.z * obb.mAxes[2] + trans;
		outVerts[1] = obb.mExtents.x * obb.mAxes[0] + obb.mExtents.z * obb.mAxes[2] + trans;
		outVerts[2] = -obb.mExtents.x * obb.mAxes[0] + -obb.mExtents.z * obb.mAxes[2] + trans;
		outVerts[3] = -obb.mExtents.x * obb.mAxes[0] + obb.mExtents.z * obb.mAxes[2] + trans;
	}

	void tEditableNavGraphNodeEntity::fSplitCorners( 
		const tGrowableArray<Math::tVec3f>& verts,
		const Math::tMat3f& basis,
		const Math::tVec3f& org, 
		Math::tVec3f& lVert, 
		Math::tVec3f& rVert,
		tGrowableArray<Math::tVec3f>* allLeftVerts,
		tGrowableArray<Math::tVec3f>* allRightVerts )
	{
		f32 lDist = 0.f;
		f32 rDist = 0.f;

		for( u32 i = 0; i < verts.fCount( ); ++i )
		{
			const Math::tVec3f& thisVert = verts[i] - org;
			f32 dot = basis.fXAxis( ).fDot( thisVert );
			if( dot >= 0.f )
			{
				// Right side
				if( dot >= rDist )
				{
					rDist = dot;
					rVert = verts[i];
				}

				if( allRightVerts )
					allRightVerts->fPushBack( verts[i] );
			}
			else
			{
				// Left side
				dot *= -1.f;
				if( dot >= lDist )
				{
					lDist = dot;
					lVert = verts[i];
				}

				if( allLeftVerts )
					allLeftVerts->fPushBack( verts[i] );
			}
		}
	}
	
	void tEditableNavGraphNodeEntity::fAddToWorld( )
	{
		tEditableObject::fAddToWorld( );

		fRefresh( );
		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			mBackConnections[ i ]->fRefresh( );
	}

	void tEditableNavGraphNodeEntity::fRemoveFromWorld( )
	{
		tEntityPtr preserveReference( this );

		tEditableObject::fRemoveFromWorld( );

		fRefresh( );
		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			mBackConnections[ i ]->fRefresh( );
	}

	void tEditableNavGraphNodeEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tEditableObject::fOnMoved( recomputeParentRelative );
		fRefresh( );
		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			mBackConnections[ i ]->fRefresh( );
	}

	//------------------------------------------------------------------------------
	// tEditableNavGraphRootEntity
	//------------------------------------------------------------------------------
	tEditableNavGraphRootEntity::tEditableNavGraphRootEntity( tEditableObjectContainer& container )
		: tEditableNavGraphNodeEntity( container )
	{
		fCommonCtor( );
	}

	tEditableNavGraphRootEntity::tEditableNavGraphRootEntity( tEditableObjectContainer& container, const Sigml::tNavGraphRootObject* ao )
		: tEditableNavGraphNodeEntity( container, ao )
	{
		fDeserializeBaseObject( ao );
		fCommonCtor( );

		mSavedConnectionGuids = ao->mConnectionGuids;
	}

	tEditableNavGraphRootEntity::~tEditableNavGraphRootEntity( )
	{
	}

	void tEditableNavGraphRootEntity::fConnect( tEditableNavGraphNodeEntity* to, b32 removeConnection )
	{
		if( to == this )
			return;

		if( !removeConnection )
		{
			// Root can only connect to one graph.
			if( mConnections.fCount( ) > 0 || mBackConnections.fCount( ) > 0 )
			{
				tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
					NULL,
					"Root nodes can only connect to one other node.",
					"Root Already Connected" ) );

				warningDialog->ShowModal( );

				return;
			}

			// Only one root can be on a graph.
			if( to->fRoot( ) )
			{
				tStrongPtr<wxMessageDialog> warningDialog( new wxMessageDialog(
					NULL,
					"This nav node graph already has a root node connected to it.",
					"Existing Root Found" ) );

				warningDialog->ShowModal( );

				return;
			}
		}

		tEditableNavGraphNodeEntity::fConnect( to, removeConnection );
	}

	std::string tEditableNavGraphRootEntity::fGetToolTip( ) const
	{
		return "Nav Graph Root";
	}

	Sigml::tObjectPtr tEditableNavGraphRootEntity::fSerialize( b32 clone ) const
	{
		Sigml::tNavGraphRootObject* ao = new Sigml::tNavGraphRootObject( );
		fSerializeBaseObject( ao, clone );

		if( !clone )
			fRecordConnections( ao );

		return Sigml::tObjectPtr( ao );
	}
}
