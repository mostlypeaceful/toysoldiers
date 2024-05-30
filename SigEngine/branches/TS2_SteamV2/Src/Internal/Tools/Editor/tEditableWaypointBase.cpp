#include "ToolsPch.hpp"
#include "tEditableWaypointBase.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig { namespace
{
	const f32 gSmallRadius = 0.1f;
	const f32 gBigRadius = 1.f;
}}

namespace Sig { namespace Sigml
{
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tWaypointObjectBase& o )
	{
		s( "Connections", o.mConnectionGuids );
		s( "BackConnections", o.mBackConnectionGuids );
	}

	register_rtti_factory( tWaypointObjectBase, false );

	tWaypointObjectBase::tWaypointObjectBase( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
	}
	void tWaypointObjectBase::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tWaypointObjectBase::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	tEntityDef* tWaypointObjectBase::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tPathEntityDef* entityDef = new tPathEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );
		entityDef->mBounds = Math::tAabbf( Math::tVec3f( -gSmallRadius ), Math::tVec3f( +gSmallRadius ) );
		entityDef->mGuid = mGuid;
		entityDef->mNextPoints.fNewArray( mConnectionGuids.fCount( ) );
		for( u32 i = 0; i < mConnectionGuids.fCount( ); ++i )
			entityDef->mNextPoints[ i ] = mConnectionGuids[ i ];

		return entityDef;
	}
	tEditableObject* tWaypointObjectBase::fCreateEditableObject( tEditableObjectContainer& container )
	{
		sigassert( 0 );
		return NULL;
	}

}}




namespace Sig
{
	namespace
	{
		static const Math::tVec4f gInnerBallTint = Math::tVec4f( 0.1f, 0.3f, 1.0f, 0.75f );
		static const Math::tVec4f gShellSphereTint = Math::tVec4f( 1.0f, 0.25f, 1.0f, 0.5f );
	}

	tEditableWaypointBase::tEditableWaypointBase( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fCommonCtor( );
	}

	tEditableWaypointBase::tEditableWaypointBase( tEditableObjectContainer& container, const Sigml::tWaypointObjectBase* ao )
		: tEditableObject( container )
	{
		fDeserializeBaseObject( ao );
		fCommonCtor( );

		mSavedConnectionGuids = ao->mConnectionGuids;
	}

	void tEditableWaypointBase::fCommonCtor( )
	{
		mShellRenderState = Gfx::tRenderState::cDefaultColorTransparent;

		// set the dummy object's bounds as our own
		Math::tAabbf localSpaceBox = mContainer.fGetDummySphereTemplate( ).fGetBounds( );
		localSpaceBox.mMin *= gBigRadius;
		localSpaceBox.mMax *= gBigRadius;
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		// setup the smaller center
		mDummySphere->fSetRgbaTint( gInnerBallTint );
		const Math::tMat3f mSmall( gSmallRadius );
		mDummySphere->fSetParentRelativeXform( mSmall );
		mDummySphere->fSetInvisible( false );

		// create the outer shell (larger radius, wireframe)
		mShellSphere.fReset( new tDummyObjectEntity( 
			mContainer.fGetDummySphereTemplate( ).fGetModifiedRenderBatch( &mShellRenderState ), 
			mContainer.fGetDummySphereTemplate( ).fGetBounds( ), true ) );
		mShellSphere->fSetRgbaTint( gShellSphereTint );
		const Math::tMat3f mBig( gBigRadius );
		mShellSphere->fSetParentRelativeXform( mBig );
		mShellSphere->fSpawnImmediate( *this );


		// create lines
		mConnectionLines.fReset( new Gfx::tWorldSpaceLines( ) );
		mConnectionLines->fResetDeviceObjects( 
			Gfx::tDevice::fGetDefaultDevice( ), 
			mContainer.fGetSolidColorMaterial( ), 
			mContainer.fGetSolidColorGeometryAllocator( ), 
			mContainer.fGetSolidColorIndexAllocator( ) );
		mConnectionLines->fSetLockedToParent( false );
		mConnectionLines->fSpawnImmediate( *this );

		tEditableObject::fUpdateStateTint( );
	}

	tEditableWaypointBase::~tEditableWaypointBase( )
	{
		fDisconnect( );
	}

	void tEditableWaypointBase::fRefreshLines( )
	{
		if( !fSceneGraph( ) || !mConnectionLines ) return;

		const u32 vtxColor = Gfx::tVertexColor( 1.f, 0.f, 1.f, 1.f ).fForGpu( );

		tGrowableArray<Gfx::tSolidColorRenderVertex> solidColorVerts;

		if( !mInContainer )
		{
			mConnectionLines->fSetGeometry( solidColorVerts, false );
			return;
		}

		for( u32 i = 0; i < mConnections.fCount( ); ++i )
		{
			if( !mConnections[ i ]->fSceneGraph( ) )
				continue;
			if(  mConnections[ i ]->fIsHidden( ) )
				continue;

			const Math::tVec3f origin = fObjectToWorld( ).fGetTranslation( );
			const Math::tVec3f connectPos = mConnections[ i ]->fObjectToWorld( ).fGetTranslation( );
			const Math::tVec3f toConnect = connectPos - origin;
			f32 toConnectLen = 0.f;
			const Math::tVec3f toConnectNorm = Math::tVec3f( toConnect ).fNormalizeSafe( Math::tVec3f::cZAxis, toConnectLen );
			const Math::tVec3f midpoint = origin + 0.5f * toConnect;
			
			Math::tMat3f basis = Math::tMat3f::cIdentity;
			basis.fOrientZAxis( toConnectNorm );

			const Math::tVec3f extraPoint0 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm + ( 0.02f * toConnectLen ) * basis.fXAxis( );
			const Math::tVec3f extraPoint1 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm - ( 0.02f * toConnectLen ) * basis.fXAxis( );
			const Math::tVec3f extraPoint2 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm + ( 0.02f * toConnectLen ) * basis.fYAxis( );
			const Math::tVec3f extraPoint3 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm - ( 0.02f * toConnectLen ) * basis.fYAxis( );

			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( origin, vtxColor ) );
			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( connectPos, vtxColor ) );

			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint0, vtxColor ) );
			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, vtxColor ) );

			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint1, vtxColor ) );
			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, vtxColor ) );

			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint2, vtxColor ) );
			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, vtxColor ) );

			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint3, vtxColor ) );
			solidColorVerts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, vtxColor ) );
		}

		mConnectionLines->fSetGeometry( solidColorVerts, false );
	}

	void tEditableWaypointBase::fConnect( tEditableWaypointBase* to, b32 removeConnection )
	{
		if( to == this )
			return;

		if( removeConnection )
		{
			if( mConnections.fFind( to ) )
			{
				mConnections.fFindAndErase( to );
				to->mBackConnections.fFindAndErase( this );
				fRefreshLines( );
			}
			else if( to->mConnections.fFind( to ) )
			{
				to->mConnections.fFindAndErase( to );
				mBackConnections.fFindAndErase( this );
				to->fRefreshLines( );
			}
		}
		else
		{
			if( to->mConnections.fFind( this ) )
				return; // TODO for now connections are bi-directional, meaning we don't need two connections between the objects
			if( mConnections.fFind( to ) )
				return;// already have this connection

			sigassert( !to->mBackConnections.fFind( this ) );

			mConnections.fPushBack( tEditableWaypointBasePtr( to ) );
			to->mBackConnections.fPushBack( this );
			fRefreshLines( );
		}
	}

	void tEditableWaypointBase::fDisconnect( b32 outwardOnly )
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			mConnections[ i ]->mBackConnections.fFindAndErase( this );
		mConnections.fSetCount( 0 );
		fRefreshLines( );

		if( !outwardOnly )
		{
			for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			{
				mBackConnections[ i ]->mConnections.fFindAndErase( this );
				mBackConnections[ i ]->fRefreshLines( );
			}
			mBackConnections.fSetCount( 0 );
		}
	}

	void tEditableWaypointBase::fAcquireEntirePath( tGrowableArray<tEditableWaypointBase*>& wayPoints )
	{
		if( wayPoints.fFind( this ) )
			return;
		wayPoints.fPushBack( this );
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			mConnections[ i ]->fAcquireEntirePath( wayPoints );
	}

	void tEditableWaypointBase::fFixUpGuidRefs( const tHashTable< u32, u32 >& conversionTable )
	{
		for( u32 i = 0; i < mSavedConnectionGuids.fCount( ); ++i )
		{
			u32* foundIdx = conversionTable.fFind( mSavedConnectionGuids[ i ] );
			sigassert( foundIdx );
			mSavedConnectionGuids[ i ] = *foundIdx;
		}
	}

	void tEditableWaypointBase::fAfterAllObjectsDeserialized( )
	{
		for( u32 i = 0; i < mSavedConnectionGuids.fCount( ); ++i )
		{
			tEditableObject* eo = mContainer.fFindObjectByGuid( mSavedConnectionGuids[ i ] );
			if( !eo ) continue;
			tEditableWaypointBase* waypoint = eo->fDynamicCast< tEditableWaypointBase >( );
			if( !waypoint ) continue;
			fConnect( waypoint );
		}

		mSavedConnectionGuids.fDeleteArray( );
	}

	void tEditableWaypointBase::fAddToWorld( )
	{
		tEditableObject::fAddToWorld( );

		fRefreshLines( );
		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			mBackConnections[ i ]->fRefreshLines( );
	}

	void tEditableWaypointBase::fRemoveFromWorld( )
	{
		tEntityPtr preserveReference( this );

		tEditableObject::fRemoveFromWorld( );

		fRefreshLines( );
		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			mBackConnections[ i ]->fRefreshLines( );
	}

	void tEditableWaypointBase::fOnMoved( b32 recomputeParentRelative )
	{
		tEditableObject::fOnMoved( recomputeParentRelative );
		fRefreshLines( );
		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			mBackConnections[ i ]->fRefreshLines( );
	}

	void tEditableWaypointBase::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );
		if( !fIsFrozen( ) )
		{
			if( &entity == mDummySphere.fGetRawPtr( ) )
				mDummySphere->fSetRgbaTint( gInnerBallTint );
			else if( &entity == mShellSphere.fGetRawPtr( ) )
				mShellSphere->fSetRgbaTint( gShellSphereTint );
		}
	}

	b32 tEditableWaypointBase::fConnectEnsureSingleLinkForward( tEditableWaypointBase* to )
	{
		// If this waypoint is already connected to the to, then we don't need to link it.
		if( (mBackConnections.fCount( ) > 0 && to == mBackConnections.fFront( )) ||
			(mConnections.fCount( ) > 0 && to == mConnections.fFront( )) )
			return false;

		// Don't add to the middle of a line or from the middle.
		if( (to->mBackConnections.fCount( ) > 0 && to->mConnections.fCount( ) > 0) ||
			(mBackConnections.fCount( ) > 0 && mConnections.fCount( ) > 0) )
			return false;

		if( to->mBackConnections.fCount( ) > 0 && to->mConnections.fCount( ) == 0 )
		{
			sigassert( to->mBackConnections.fCount( ) == 1 );

			fReverseConnectionsBackward( to );
		}

		if( mBackConnections.fCount( ) == 0 && mConnections.fCount( ) > 0 )
		{
			sigassert( mConnections.fCount( ) == 1 );

			fReverseConnectionsForward( this );
		}

		// Do standard add.
		sigassert( !to->mBackConnections.fFind( this ) );

		mConnections.fPushBack( tEditableWaypointBasePtr( to ) );
		to->mBackConnections.fPushBack( this );

		return true;
	}

	void tEditableWaypointBase::fRecordConnections( Sigml::tWaypointObjectBase* ao ) const
	{
		for( u32 i = 0; i < mConnections.fCount( ); ++i )
			if( mConnections[ i ]->fInContainer( ) )
				ao->mConnectionGuids.fPushBack( mConnections[ i ]->fGuid( ) );

		for( u32 i = 0; i < mBackConnections.fCount( ); ++i )
			if( mBackConnections[ i ]->fInContainer( ) )
				ao->mBackConnectionGuids.fPushBack( mBackConnections[ i ]->fGuid( ) );
	}

	void tEditableWaypointBase::fReverseConnectionsBackward( tEditableWaypointBase* backWaypoint )
	{
		tGrowableArray< tEditableWaypointBase* > waypoints;

		// Ensure end point.
		while( backWaypoint )
		{
			waypoints.fPushBack( backWaypoint );

			if( backWaypoint->mBackConnections.fCount( ) > 0 )
			{
				sigassert( backWaypoint->mBackConnections.fCount( ) == 1 );
				backWaypoint = backWaypoint->mBackConnections.fFront( );
			}
			else 
				backWaypoint = NULL;
		}

		for( u32 i = 0; i < waypoints.fCount( ); ++i )
		{
			tEditableWaypointBase* thisWaypoint = waypoints[ i ];

			tEditableWaypointBase* switchToBack = NULL;
			tEditableWaypointBase* switchToFront = NULL;

			// Get the ptr to switch to the back.
			if( thisWaypoint->mConnections.fCount( ) > 0 )
			{
				sigassert( thisWaypoint->mConnections.fCount( ) == 1 );
				switchToBack = thisWaypoint->mConnections.fFront( ).fGetRawPtr( );
				thisWaypoint->mConnections.fSetCount( 0 );
			}

			// Get the ptr for the front.
			if( thisWaypoint->mBackConnections.fCount( ) > 0 )
			{
				sigassert( thisWaypoint->mBackConnections.fCount( ) == 1 );
				switchToFront = thisWaypoint->mBackConnections.fFront( );
				thisWaypoint->mBackConnections.fSetCount( 0 );
			}

			// Switch found pointers.
			if( switchToBack )
				thisWaypoint->mBackConnections.fPushBack( switchToBack );
			if( switchToFront )
				thisWaypoint->mConnections.fPushBack( tEditableWaypointBasePtr( switchToFront ) );

			thisWaypoint->fRefreshLines( );
		}
	}

	void tEditableWaypointBase::fReverseConnectionsForward( tEditableWaypointBase* foreWaypoint )
	{
		tGrowableArray< tEditableWaypointBase* > waypoints;

		// Ensure end point.
		while( foreWaypoint )
		{
			waypoints.fPushBack( foreWaypoint );

			if( foreWaypoint->mConnections.fCount( ) > 0 )
			{
				sigassert( foreWaypoint->mConnections.fCount( ) == 1 );
				foreWaypoint = foreWaypoint->mConnections.fFront( ).fGetRawPtr( );
			}
			else 
				foreWaypoint = NULL;
		}

		for( u32 i = 0; i < waypoints.fCount( ); ++i )
		{
			tEditableWaypointBase* thisWaypoint = waypoints[ i ];

			tEditableWaypointBase* switchToBack = NULL;
			tEditableWaypointBase* switchToFront = NULL;

			// Get the ptr to switch to the back.
			if( thisWaypoint->mBackConnections.fCount( ) > 0 )
			{
				sigassert( thisWaypoint->mBackConnections.fCount( ) == 1 );
				switchToBack = thisWaypoint->mBackConnections.fFront( );
				thisWaypoint->mBackConnections.fSetCount( 0 );
			}

			// Get the ptr for the front.
			if( thisWaypoint->mConnections.fCount( ) > 0 )
			{
				sigassert( thisWaypoint->mConnections.fCount( ) == 1 );
				switchToFront = thisWaypoint->mConnections.fFront( ).fGetRawPtr( );
				thisWaypoint->mConnections.fSetCount( 0 );
			}

			// Switch found pointers.
			if( switchToBack )
				thisWaypoint->mConnections.fPushBack( tEditableWaypointBasePtr( switchToBack ) );
			if( switchToFront )
				thisWaypoint->mBackConnections.fPushBack( switchToFront );

			thisWaypoint->fRefreshLines( );
		}
	}

}

