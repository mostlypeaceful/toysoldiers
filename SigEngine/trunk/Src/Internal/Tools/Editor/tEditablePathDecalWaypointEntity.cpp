#include "ToolsPch.hpp"
#include "tEditablePathDecalWaypointEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tPathEntity.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSigmlConverter.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Math/tIntersectionRayPlane.hpp"
#include "Gfx/tDecalMaterial.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tApplication.hpp"
#include "Gfx/tFollowPathCamera.hpp"
#include "tPathDecalEntityDef.hpp"
#include "tBase64.hpp"
#include "FileSystem.hpp"
#include "Editor/tEditableTerrainEntity.hpp"
#include "tSceneGraphCollectTris.hpp"

#define DRAW_BOUNDING_OBBS 1
#define DRAW_CONNECTION_LINES 0
#define DRAW_WIREFRAME_TRIS 0

namespace Sig { namespace
{
	static const f32			gSmallRadius = 0.1f;
	static const f32			gBigRadius = 1.f;
	static const Math::tVec4f	gInnerBallTint = Math::tVec4f( 0.1f, 0.3f, 0.2f, 0.6f );
	static const Math::tVec4f	gShellSphereTint = Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.5f );

	namespace
	{
		static u32 fMakeVertexColor( f32 pathLength, f32 fadeDist, f32 vScale, f32 vValue, tPlatformId pid = cCurrentPlatform )
		{
			const f32 distFromEnd = fMin( vValue * vScale, pathLength - vValue * vScale );
			const f32 alpha = fadeDist ? fSaturate( distFromEnd / fadeDist ) : 1.f;
			return Gfx::tVertexColor( 1.f, 1.f, 1.f, alpha * alpha ).fForGpu( pid );
		}
	}
}}


static const char* cSplineType = "SplineDecal.SplineType"; 
static const char* cUScale = "SplineDecal.UScale"; 
static const char* cVScale = "SplineDecal.VScale"; 
static const char* cFadeDist = "SplineDecal.FadeDist";
static const char* cAcceptsLights = "SplineDecal.AcceptsLights";
static const char* cDepthBias = "SplineDecal.DepthBias";
static const char* cSlopeScaleBias = "SplineDecal.SlopeScaleBias";
static const char* cCameraDepthOffset = "SplineDecal.CamDepthOffset";
static const char* cDrawBoundingBoxes = "Display.BoundingBoxes";
static const char* cApplyToAllMeshes = "SplineDecal.ApplyToMeshes";
static const char* cApplyToTag = "SplineDecal.ApplyToTag";

enum tSplineTypes
{
	cCatmullRom,
	cBezier,
	cNoCurve,

	cNumSplineTypes
};

const std::string cCatmullRomStr("Catmull-Rom");
const std::string cBezierStr("Bezier");
const std::string cNoCurveStr("No Curve");



namespace Sig { namespace Sigml
{

	class tools_export tPathDecalWaypointObject : public tWaypointObjectBase
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tPathDecalWaypointObject, 0xF989D493 );
	public:
		tPathDecalWaypointObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPathDecalWaypointObject& o )
	{
	}

	register_rtti_factory( tPathDecalWaypointObject, false );

	tPathDecalWaypointObject::tPathDecalWaypointObject( )
	{
	}
	void tPathDecalWaypointObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tPathDecalWaypointObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	tEditableObject* tPathDecalWaypointObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditablePathDecalWaypoint( container, this );
	}
}}

namespace Sig { namespace Sigml
{
	class tools_export tPathDecalObject : public tObject
	{
	public:
		implement_rtti_serializable_base_class( tPathDecalObject, 0xF989D49E );
		tDynamicArray<u32>			mWaypointGuids;
		tDynamicArray<Math::tVec3u>	mTris;
		tDynamicArray<Math::tVec3f>	mVerts;
		tDynamicArray<Math::tVec2f> mUVs;
		Math::tAabbf				mBounds;
		tFilePathPtr				mDiffuseTextureFilePath;
		tFilePathPtr				mNormalMapFilePath;
		b32							mAcceptsLights;
		s32							mDepthBias;
		s32							mSlopeScaleDepthBias;
		f32							mCameraDepthOffset;
		f32							mPathLength;
	public:
		tPathDecalObject( );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		void fGetDependencyFiles( tFilePathPtrList& resourcePathsOut );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer, class tDataType>
	void fDecodeChunk( const char* inDataTag, const char* inNumDataTag, tSerializer& s, tDynamicArray< tDataType >& array )
	{
		std::string inEncoded;
		s( inDataTag, inEncoded );
		u32 numData = 0;
		s( inNumDataTag, numData );

		tGrowableArray<Sig::byte> decoded;
		tBase64::fDecode( inEncoded.c_str( ), inEncoded.length( ), decoded );

		array.fNewArray( numData );
		sigassert( decoded.fCount( ) == numData * sizeof( array[0] ) );

		fMemCpy( array.fBegin( ), decoded.fBegin( ), decoded.fCount( ) );
	}

	template<class tSerializer, class tDataType>
	void fEncodeChunk( const char* inDataTag, const char* inNumDataTag, tSerializer& s, tDynamicArray< tDataType >& array )
	{
		std::string outEncoded;
		tBase64::fEncode( (const Sig::byte*)array.fBegin( ), array.fCount( ) * sizeof( array[0] ), outEncoded );
		s( inDataTag, outEncoded );

		u32 numData = array.fCount( );
		s( inNumDataTag, numData );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPathDecalObject& o )
	{
		s( "Waypoints", o.mWaypointGuids );

		if( s.fIn( ) )
		{
			fDecodeChunk( "Verts", "NumVerts", s, o.mVerts );
			fDecodeChunk( "Tris", "NumTris", s, o.mTris );
			fDecodeChunk( "UVs", "NumUVs", s, o.mUVs );
		}
		else
		{
			fEncodeChunk( "Verts", "NumVerts", s, o.mVerts );
			fEncodeChunk( "Tris", "NumTris", s, o.mTris );
			fEncodeChunk( "UVs", "NumUVs", s, o.mUVs );
		}

		s( "Bounds", o.mBounds );
		s( "DiffuseTexture", o.mDiffuseTextureFilePath );
		s( "NormalMap", o.mNormalMapFilePath );
		s( "AcceptsLights", o.mAcceptsLights );
		s( "DepthBias", o.mDepthBias );
		s( "SlopeScaleDepthBias", o.mSlopeScaleDepthBias );
		s( "CameraDepthOffset", o.mCameraDepthOffset );
		s( "PathLength", o.mPathLength );
	}

	register_rtti_factory( tPathDecalObject, false );

	tPathDecalObject::tPathDecalObject( )
		: mAcceptsLights( true )
		, mDepthBias( 0 )
		, mSlopeScaleDepthBias( 0 )
		, mCameraDepthOffset( 0.f )
		, mPathLength( 0.f )
	{
	}
	void tPathDecalObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tPathDecalObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	tEntityDef* tPathDecalObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tPathDecalEntityDef* entityDef = new tPathDecalEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );
		entityDef->mBounds = mBounds;
		entityDef->mAcceptsLights = mAcceptsLights;
		entityDef->mDepthBias = mDepthBias;
		entityDef->mSlopeScaleDepthBias = mSlopeScaleDepthBias;
		entityDef->mCameraDepthOffset = mCameraDepthOffset;

		if( mDiffuseTextureFilePath.fLength( ) > 0 && FileSystem::fFileExists( ToolsPaths::fMakeResAbsolute( mDiffuseTextureFilePath ) ) )
			entityDef->mDiffuseTexture = sigmlConverter.fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tTextureFile>( mDiffuseTextureFilePath ) );
		else
			log_warning( "No diffuse texture specified or texture doesn't exist: " << mDiffuseTextureFilePath );

		if( mNormalMapFilePath.fLength( ) > 0 )
		{
			if( FileSystem::fFileExists( ToolsPaths::fMakeResAbsolute( mNormalMapFilePath ) ) && mAcceptsLights )
				entityDef->mNormalMap = sigmlConverter.fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tTextureFile>( mNormalMapFilePath ) );
			else
				log_warning( "Normal map specified but texture doesn't exist: " << mNormalMapFilePath );
		}

		// Copy index sets.
		entityDef->mTriIndices.fResize( mTris.fCount( ) * 3);
		for( u32 i = 0; i < mTris.fCount( ); ++i )
		{
			for( u32 j = 0; j < 3; ++j )
			{
				const u32 index = mTris[ i ].fAxis( j );
				sigassert( index < std::numeric_limits< u16 >::max( ) );
				entityDef->mTriIndices[ i * 3 + j ] = index;
			}
		}

		// Copy verts and UVs.
		sigassert( mVerts.fCount( ) == mUVs.fCount( ) );
		entityDef->mVerts.fResize( mVerts.fCount( ) );

		tGrowableArray< tVertexData > convertedVerts;
		tEditablePathDecalEntity::fConvertIndexedListToVertexData( mTris, mVerts, mUVs, convertedVerts );

		const f32 pathLength = mPathLength;
		const f32 fadeDist = mEditableProperties.fGetValue( cFadeDist, 0.f );
		const f32 vScale = mEditableProperties.fGetValue( cVScale, 1.f );
		const tPlatformId pid = cCurrentPlatform;

		for( u32 i = 0; i < convertedVerts.fCount( ); ++i )
		{
			entityDef->mVerts[ i ].mP		= convertedVerts[ i ].mPos;
			entityDef->mVerts[ i ].mN		= convertedVerts[ i ].mN;
			entityDef->mVerts[ i ].mTan		= convertedVerts[ i ].mTan;
			entityDef->mVerts[ i ].mUv		= convertedVerts[ i ].mUV;
			entityDef->mVerts[ i ].mColor	= fMakeVertexColor( pathLength, fadeDist, vScale, convertedVerts[ i ].mUV.y, pid );
		}

		return entityDef;
	}
	void tPathDecalObject::fGetDependencyFiles( tFilePathPtrList& resourcePathsOut )
	{
		tObject::fGetDependencyFiles( resourcePathsOut );

		const tFilePathPtr diffuseTexPath( mEditableProperties.fGetValue( fEditablePropDiffuseTextureFilePath( ), std::string("") ) );
		const tFilePathPtr normalTexPath( mEditableProperties.fGetValue( fEditablePropNormalMapFilePath( ), std::string("") ) );

		resourcePathsOut.fPushBack( diffuseTexPath );
		resourcePathsOut.fPushBack( normalTexPath );
	}
	tEditableObject* tPathDecalObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		if( this->mWaypointGuids.fCount( ) == 0 )
			return NULL;

		// Refresh after everything is in place.
		tEditablePathDecalEntity* newDecal = new tEditablePathDecalEntity( container, this );
		newDecal->fRefreshTexture( );

		return newDecal;
	}

}}



namespace Sig
{
	tEditablePathDecalWaypoint::tEditablePathDecalWaypoint( tEditableObjectContainer& container )
		: tEditableObject( container )
		, mInsertIdx( -1 )
		, mCloneDirty( false )
		, mDisableRefresh( false )
		, mParent( NULL )
	{
		fCommonCtor( );
	}

	tEditablePathDecalWaypoint::tEditablePathDecalWaypoint( tEditableObjectContainer& container, const Sigml::tPathDecalWaypointObject* ao )
		: tEditableObject( container )
		, mInsertIdx( -1 )
		, mCloneDirty( false )
		, mDisableRefresh( false )
		, mParent( NULL )
	{
		fDeserializeBaseObject( ao );
		fCommonCtor( );
	}

	tEditablePathDecalWaypoint::~tEditablePathDecalWaypoint( )
	{
	}
	
	void tEditablePathDecalWaypoint::fCommonCtor( )
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

		tEditableObject::fUpdateStateTint( );
	}

	std::string tEditablePathDecalWaypoint::fGetToolTip( ) const
	{
		//std::stringstream ret;
		//ret << "id: ";
		//ret << this;
		//ret << " parent: ";
		//ret << mParent.fGetRawPtr( );
		//return ret.str();

		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Path Decal Waypoint - " + name;
		return "Path Decal Waypoint";
	}

	void tEditablePathDecalWaypoint::fAddToWorld( )
	{
		if( mInsertIdx != -1 )
		{
			// If the parent's been deleted, add it to the world.
			if( !mParent->fSceneGraph( ) )
				mParent->fAddToWorld( );

			mParent->fInsertNode( this, mInsertIdx );
			mInsertIdx = -1;
		}

		tEditableObject::fAddToWorld( );
	}

	void tEditablePathDecalWaypoint::fRemoveFromWorld( )
	{
		mInsertIdx = mParent ? mParent->fEliminateNode( this ) : -1;
		
		tEditableObject::fRemoveFromWorld( );
	}

	tEntityPtr tEditablePathDecalWaypoint::fClone( )
	{
		tEditablePathDecalWaypoint* newClone = tEditableObject::fClone( )->fDynamicCast< tEditablePathDecalWaypoint >( );

		if( mParent )
		{
			newClone->mCloneDirty = true;
			newClone->mParent = mParent;
			newClone->mInsertIdx = mParent->fFindIndex( this );
		}

		return tEntityPtr( newClone );
	}

	void tEditablePathDecalWaypoint::fDisableGeometryUpdates( b32 disable )
	{ 
		mDisableRefresh = disable; 
		if( mParent )
			mParent->fDisableGeometryUpdates( disable );
	}

	namespace
	{
		static tEditablePropertyTable cNullProps;
	}

	tEditablePropertyTable& tEditablePathDecalWaypoint::fGetEditableProperties( )
	{
		return mParent ? mParent->fGetEditableProperties( ) : cNullProps;
	}

	const tEditablePropertyTable& tEditablePathDecalWaypoint::fGetEditableProperties( ) const
	{
		return mParent ? mParent->fGetEditableProperties( ) : cNullProps;
	}

	void tEditablePathDecalWaypoint::fNotifyParentNeedsUpdate( )
	{
		if( mDisableRefresh )
			return;

		if( mParent )
			mParent->fNotifyNeedsUpdate( );
	}

	void tEditablePathDecalWaypoint::fAcquireEntireDecal( tGrowableArray<tEditablePathDecalWaypoint*>& wayPoints )
	{
		if( mParent )
			mParent->fAcquireEntireDecal( wayPoints );
	}

	void tEditablePathDecalWaypoint::fAfterAllObjectsCloned( const tEditorSelectionList& siblingObjects )
	{
		if( !mCloneDirty )
			return;

		mCloneDirty = false;

		tEditablePathDecalEntityPtr idParent = mParent;

		tEditablePathDecalEntity* clonedParent = mParent->fClone( )->fDynamicCast< tEditablePathDecalEntity >( );
		clonedParent->fAddBack( this );

		// Try to gather bro waypoints.
		for( u32  i = 0; i < siblingObjects.fCount( ); ++i )
		{
			tEditablePathDecalWaypoint* siblingWaypoint = siblingObjects[ i ]->fDynamicCast< tEditablePathDecalWaypoint >( );
			if( !siblingWaypoint )
				continue;

			if( siblingWaypoint->mCloneDirty && idParent == siblingWaypoint->mParent )
			{
				clonedParent->fSortedInsert( siblingWaypoint );
				siblingWaypoint->mParent.fReset( clonedParent );
				siblingWaypoint->mCloneDirty = false;
			}
		}

		clonedParent->fRefreshGeometry( );
	}

	void tEditablePathDecalWaypoint::fPreStateChange( )
	{
		fDisableGeometryUpdates( true );
	}

	void tEditablePathDecalWaypoint::fPostStateChange( )
	{
		fDisableGeometryUpdates( false );
	}

	Sigml::tObjectPtr tEditablePathDecalWaypoint::fSerialize( b32 clone ) const
	{
		Sigml::tPathDecalWaypointObject* ao = new Sigml::tPathDecalWaypointObject( );
		fSerializeBaseObject( ao, clone );

		return Sigml::tObjectPtr( ao );
	}

	void tEditablePathDecalWaypoint::fOnMoved( b32 recomputeParentRelative )
	{
		fNotifyParentNeedsUpdate( );

		tEditableObject::fOnMoved( recomputeParentRelative );
	}

	void tEditablePathDecalWaypoint::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
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



	tEditablePathDecalEntity::tEditablePathDecalEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
		, mDisableRefresh( false )
		, mPathLength( 0.f )
	{
		fCommonCtor( );
	}

	tEditablePathDecalEntity::tEditablePathDecalEntity( tEditableObjectContainer& container, const Sigml::tPathDecalObject* ao )
		: tEditableObject( container )
		, mDisableRefresh( false )
		, mPathLength( ao->mPathLength )
	{
		fCommonCtor( );

		fDeserializeBaseObject( ao );

		mCachedWaypointGuids = ao->mWaypointGuids;

		Gfx::tRenderState newBiasState = mTexturedTris->fRenderState( );
		newBiasState.fSetDepthBias( mEditableProperties.fGetValue( cDepthBias, 0 ) );
		newBiasState.fSetSlopeScaleBias( mEditableProperties.fGetValue( cSlopeScaleBias, 0 ) );
		mTexturedTris->fSetRenderState( newBiasState );

		mTexturedTris->fSetCameraDepthOffset( mEditableProperties.fGetValue( cCameraDepthOffset, 0.f ) );

		mSaveVerts.fSetCount( ao->mVerts.fCount( ) );
		for( u32 i = 0; i < ao->mVerts.fCount( ); ++i )
			mSaveVerts[ i ] = ao->mVerts[ i ];

		mSaveTris.fSetCount( ao->mTris.fCount( ) );
		for( u32 i = 0; i < ao->mTris.fCount( ); ++i )
			mSaveTris[ i ] = ao->mTris[ i ];

		mSaveUVs.fSetCount( ao->mUVs.fCount( ) );
		for( u32 i = 0; i < ao->mUVs.fCount( ); ++i )
			mSaveUVs[ i ] = ao->mUVs[ i ];

		sigassert( mSaveUVs.fCount( ) == mSaveVerts.fCount( ) );

		mBounds = ao->mBounds;
		mTexturedTris->fSetObjectSpaceBox( mBounds );
	}

	tEditablePathDecalEntity::~tEditablePathDecalEntity( )
	{
	}

	void tEditablePathDecalEntity::fCommonCtor( )
	{
		tDynamicArray<std::string> splineNames;
		splineNames.fPushBack( cCatmullRomStr );
		splineNames.fPushBack( cBezierStr );
		splineNames.fPushBack( cNoCurveStr );
		// TODO: Other types.

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( cSplineType, splineNames ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cFadeDist, 0.f, 0.0f, 1024.f, 0.1f, 1 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cUScale, 1.f, 0.5f, 1024.f, 1.f, 2 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cVScale, 1.f, 0.5f, 1024.f, 1.f, 2 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( fEditablePropDiffuseTextureFilePath( ), "" ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFileNameString( fEditablePropNormalMapFilePath( ), "" ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cDepthBias, -1, -127, 127, 1, 0 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cSlopeScaleBias, 0, -127, 127, 1, 0 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( cCameraDepthOffset, 0.f, -1000.f, +1000.f, 0.1f, 1 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cAcceptsLights, true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cDrawBoundingBoxes, true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( cApplyToAllMeshes, false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyString( cApplyToTag, "Ground" ) ) );

		mEditableProperties.fSetDataNoNotify( Sigml::tObject::fEditablePropGroundRelative( ), true );

		// create lines
		mConnectionLines.fReset( new Gfx::tWorldSpaceLines( ) );
		mConnectionLines->fResetDeviceObjects( 
			Gfx::tDevice::fGetDefaultDevice( ), 
			mContainer.fGetSolidColorMaterial( ), 
			mContainer.fGetSolidColorGeometryAllocator( ), 
			mContainer.fGetSolidColorIndexAllocator( ) );
		mConnectionLines->fSetLockedToParent( false );
		mConnectionLines->fSpawnImmediate( *this );


		mWireframeTris.fReset( new Gfx::tWorldSpaceLines( ) );
		mWireframeTris->fResetDeviceObjects(
			Gfx::tDevice::fGetDefaultDevice( ), 
			mContainer.fGetSolidColorMaterial( ), 
			mContainer.fGetSolidColorGeometryAllocator( ), 
			mContainer.fGetSolidColorIndexAllocator( ) );
		mWireframeTris->fSetLockedToParent( false );
		mWireframeTris->fSpawnImmediate( *this );

		fRefreshTexHandles( );

		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );

		mTexturedTris.fReset( new Gfx::tStaticDecalGeometry( ) );
		mTexturedTris->fResetDeviceObjectsMaterial(
			Gfx::tDevice::fGetDefaultDevice( ),
			0, 0,
			0, 0,
			fConstructMaterial( ),
			allocator.mDecalGeomAllocator,
			allocator.mIndexAllocator );
		mTexturedTris->fSetLockedToParent( false );
		mTexturedTris->fSpawnImmediate( *this );

		tEditableObject::fUpdateStateTint( );
	}

	void tEditablePathDecalEntity::fAddBack( tEditablePathDecalWaypoint* add )
	{
		mControlWaypoints.fPushBack( tEditablePathDecalWaypointPtr( add ) );
		add->fSetParent( this );

		fNotifyNeedsUpdate( );
	}

	void tEditablePathDecalEntity::fAddFront( tEditablePathDecalWaypoint* add )
	{
		mControlWaypoints.fPushFront( tEditablePathDecalWaypointPtr( add ) );
		add->fSetParent( this );

		fNotifyNeedsUpdate( );
	}

	void tEditablePathDecalEntity::fRemoveBack( tEditablePathDecalWaypoint* remove )
	{
		mControlWaypoints.fFindAndEraseOrdered( remove );
		remove->fSetParent( fClone( )->fDynamicCast< tEditablePathDecalEntity >( ) );

		fNotifyNeedsUpdate( );

		// If there are no nodes left, remove the line too.
		if( mControlWaypoints.fCount( ) == 0 )
			fRemoveFromWorld( );
	}

	void tEditablePathDecalEntity::fTickUpdate( )
	{
		fRefreshGeometry( );
	}

	void tEditablePathDecalEntity::fNotifyNeedsUpdate( )
	{
		if( !mDisableRefresh )
			mContainer.fAddEntityToUpdate( tEntityPtr( this ) );
	}

	void tEditablePathDecalEntity::fJoin( tEditablePathDecalWaypoint* anchorWaypoint, tEditablePathDecalWaypoint* addedWaypoint )
	{
		sigassert( this == anchorWaypoint->fGetParent( ) );

		tEditablePathDecalEntity* sourceLine = addedWaypoint->fGetParent( );
		sigassert( sourceLine );

		const b32 addToFront = mControlWaypoints.fFront( ).fGetRawPtr( ) == anchorWaypoint;
		sigassert( addToFront || mControlWaypoints.fBack( ).fGetRawPtr( ) == anchorWaypoint );

		fDisableGeometryUpdates( true );
		if( addedWaypoint == sourceLine->mControlWaypoints.fFront( ).fGetRawPtr( ) )
		{
			for( u32 i = 0; i < sourceLine->mControlWaypoints.fCount( ); ++i )
			{
				if( addToFront )
					fAddFront( sourceLine->mControlWaypoints[ i ].fGetRawPtr( ) );
				else
					fAddBack( sourceLine->mControlWaypoints[ i ].fGetRawPtr( ) );
			}
		}
		else if( addedWaypoint == sourceLine->mControlWaypoints.fBack( ).fGetRawPtr( ) )
		{
			for( s32 i = sourceLine->mControlWaypoints.fCount( )-1; i >= 0; --i )
			{
				if( addToFront )
					fAddFront( sourceLine->mControlWaypoints[ i ].fGetRawPtr( ) );
				else
					fAddBack( sourceLine->mControlWaypoints[ i ].fGetRawPtr( ) );
			}
		}
		else
			sigassert( 0 );
		fDisableGeometryUpdates( false );

		sourceLine->mControlWaypoints.fSetCount( 0 );
		sourceLine->fRemoveFromWorld( );

		fNotifyNeedsUpdate( );
	}

	b32 tEditablePathDecalEntity::fCanJoin( tEditablePathDecalWaypoint* anchorWaypoint, tEditablePathDecalWaypoint* addedWaypoint )
	{
		sigassert( mControlWaypoints.fCount( ) );

		// Don't allow loops.
		if( anchorWaypoint->fGetParent( ) == addedWaypoint->fGetParent( ) )
			return false;

		// Only add to the head or tail of a line.
		if( !(mControlWaypoints.fFront( ).fGetRawPtr( ) == anchorWaypoint
			|| mControlWaypoints.fBack( ).fGetRawPtr( ) == anchorWaypoint ) )
			return false; 

		tEditablePathDecalEntity* sourceLine = addedWaypoint->fGetParent( );

		// If we're adding from the front or back of the other line, everything is ok.
		return addedWaypoint == sourceLine->mControlWaypoints.fFront( ).fGetRawPtr( ) || addedWaypoint == sourceLine->mControlWaypoints.fBack( ).fGetRawPtr( );
	}

	tEditablePathDecalEntity* tEditablePathDecalEntity::fSplit( tEditablePathDecalWaypoint* splitPoint )
	{
		u32 foundIdx = -1;
		for( u32 i = 0; i < mControlWaypoints.fCount( ); ++i )
		{
			if( mControlWaypoints[ i ].fGetRawPtr( ) == splitPoint )
			{
				foundIdx = i;
				break;
			}
		}

		sigassert( foundIdx != -1 );

		tEditablePathDecalEntity* retDecal = fClone( )->fDynamicCast< tEditablePathDecalEntity >( );
		retDecal->mControlWaypoints.fSetCount( 0 );

		for( u32 i = foundIdx+1; i < mControlWaypoints.fCount( ); ++i )
		{
			retDecal->fAddBack( mControlWaypoints[ i ].fGetRawPtr( ) );
			mControlWaypoints.fEraseOrdered( i-- );
		}

		fNotifyNeedsUpdate( );

		return retDecal;
	}

	b32 tEditablePathDecalEntity::fCanSplit( tEditablePathDecalWaypoint* splitPoint )
	{
		return mControlWaypoints.fBack( ).fGetRawPtr( ) != splitPoint;
	}

	void tEditablePathDecalEntity::fAddWaypoints( const tGrowableArray< tEditablePathDecalWaypointPtr >& newWaypoints )
	{
		for( u32 i = 0; i < newWaypoints.fCount( ); ++i )
		{
			tEditablePathDecalWaypoint* thisWp = newWaypoints[ i ].fGetRawPtr( );
			fDisableGeometryUpdates( true );
			fAddBack( thisWp );
			fDisableGeometryUpdates( false );
		}

		fNotifyNeedsUpdate( );
	}

	void tEditablePathDecalEntity::fRemoveWaypoints( const tGrowableArray< tEditablePathDecalWaypointPtr >& removedWaypoints )
	{
		for( u32 i = 0; i < removedWaypoints.fCount( ); ++i )
		{
			const b32 found = mControlWaypoints.fFindAndEraseOrdered( removedWaypoints[ i ] );
			sigassert( found );
		}

		fNotifyNeedsUpdate( );

		// If there are no nodes left, remove the line too.
		if( mControlWaypoints.fCount( ) == 0 )
			fRemoveFromWorld( );
	}

	s32 tEditablePathDecalEntity::fEliminateNode( tEditablePathDecalWaypoint* findNode )
	{
		for( u32 i = 0; i < mControlWaypoints.fCount( ); ++i )
		{
			if( mControlWaypoints[ i ].fGetRawPtr( ) == findNode )
			{
				mControlWaypoints.fEraseOrdered( i );

				// If there are no nodes left, remove the line too.
				if( mControlWaypoints.fCount( ) == 0 )
					fRemoveFromWorld( );
				else
					fNotifyNeedsUpdate( );

				return i;
			}
		}

		// Deletion waypoint not found in this line.
		return -1;
	}

	void tEditablePathDecalEntity::fInsertNode( tEditablePathDecalWaypoint* insertNode, u32 idx )
	{
		mControlWaypoints.fInsert( idx, tEditablePathDecalWaypointPtr( insertNode ) );
		fNotifyNeedsUpdate( );
	}

	s32 tEditablePathDecalEntity::fFindIndex( tEditablePathDecalWaypoint* findNode ) const
	{
		return mControlWaypoints.fIndexOf( findNode );
	}

	void tEditablePathDecalEntity::fSortedInsert( tEditablePathDecalWaypoint* insertNode )
	{
		const s32 insertIdx = fBinarySearch( insertNode->mInsertIdx );

		// Mid is insertion point
		mControlWaypoints.fInsert( insertIdx, tEditablePathDecalWaypointPtr( insertNode ) );
	}

	s32 tEditablePathDecalEntity::fBinarySearch( s32 search )
	{
		s32 left = 0;
		s32 right = mControlWaypoints.fCount( );

		while( left < right )
		{
			s32 mid = left + ((right - left) / 2 );

			if( search > mControlWaypoints[mid]->mInsertIdx )
			{
				left = mid + 1;
			}
			else if( search < mControlWaypoints[mid]->mInsertIdx )
			{
				right = mid;
			}
			else
				return mid;
		}

		return left;
	}

	void tEditablePathDecalEntity::fFixUpGuidRefs( const tHashTable< u32, u32 >& conversionTable )
	{
		for( u32 i = 0; i < mCachedWaypointGuids.fCount( ); ++i )
		{
			u32* foundIdx = conversionTable.fFind( mCachedWaypointGuids[ i ] );
			sigassert( foundIdx );
			mCachedWaypointGuids[ i ] = *foundIdx;
		}
	}

	void tEditablePathDecalEntity::fAfterAllObjectsDeserialized( )
	{
		fDisableGeometryUpdates( true );
		for( u32 i = 0; i < mCachedWaypointGuids.fCount( ); ++i )
		{
			tEditableObject* eo = mContainer.fFindObjectByGuid( mCachedWaypointGuids[ i ] );
			if( !eo ) continue;
			tEditablePathDecalWaypoint* waypoint = eo->fDynamicCast< tEditablePathDecalWaypoint >( );
			if( !waypoint ) continue;
			fAddBack( waypoint );
		}
		fDisableGeometryUpdates( false );

		fRefreshGeometry( );

		mCachedWaypointGuids.fDeleteArray( );
	}

	void tEditablePathDecalEntity::fConvertIndexedListToVertexData( 
		const tDynamicArray< Math::tVec3u >& idxs, 
		const tDynamicArray< Math::tVec3f >& verts, 
		const tDynamicArray< Math::tVec2f >& uvs, 
		tGrowableArray< tVertexData >& convertedVerts )
	{
		convertedVerts.fSetCount( verts.fCount( ) );
		for( u32 i = 0; i < idxs.fCount( ); ++i )
		{
			const u32 v0Idx = idxs[i][0];
			const u32 v1Idx = idxs[i][1];
			const u32 v2Idx = idxs[i][2];

			tVertexData& v0 = convertedVerts[ v0Idx ];
			tVertexData& v1 = convertedVerts[ v1Idx ];
			tVertexData& v2 = convertedVerts[ v2Idx ];

			v0.mPos = verts[ v0Idx ];
			v1.mPos = verts[ v1Idx ];
			v2.mPos = verts[ v2Idx ];

			v0.mUV = uvs[ v0Idx ];
			v1.mUV = uvs[ v1Idx ];
			v2.mUV = uvs[ v2Idx ];

			const Math::tVec3f faceNorm = Math::tTrianglef( v0.mPos, v1.mPos, v2.mPos ).fComputeUnitNormal( );

			v0.mN += faceNorm;
			++v0.mNumNorms;
			v1.mN += faceNorm;
			++v1.mNumNorms;
			v2.mN += faceNorm;
			++v2.mNumNorms;

			fCalculateTangents( v0.mPos, v1.mPos, v2.mPos, v0.mUV, v1.mUV, v2.mUV, v0, v1, v2 );
		}

		for( u32 i = 0; i < convertedVerts.fCount( ); ++i )
			fFinalizeTangets( convertedVerts[i] );
	}

	Sigml::tObjectPtr tEditablePathDecalEntity::fSerialize( b32 clone ) const
	{
		Sigml::tPathDecalObject* pathObj = new Sigml::tPathDecalObject( );
		fSerializeBaseObject( pathObj, clone );

		// Save waypoints.
		for( u32 i = 0; i < mControlWaypoints.fCount( ); ++i )
			if( mControlWaypoints[ i ]->fInContainer( ) )
				pathObj->mWaypointGuids.fPushBack( mControlWaypoints[ i ]->fGuid( ) );

		// Save tris.
		pathObj->mTris.fResize( mSaveTris.fCount( ) );
		for( u32 i = 0; i < mSaveTris.fCount( ); ++i )
			pathObj->mTris[ i ] = mSaveTris[ i ];

		// Save verts
		pathObj->mVerts.fResize( mSaveVerts.fCount( ) );
		for( u32 i = 0; i < mSaveVerts.fCount( ); ++i )
			pathObj->mVerts[ i ] = mSaveVerts[ i ];

		// Save UVs.
		pathObj->mUVs.fResize( mSaveUVs.fCount( ) );
		for( u32 i = 0; i < mSaveUVs.fCount( ); ++i )
			pathObj->mUVs[ i ] = mSaveUVs[ i ];

		// Save bounds.
		pathObj->mBounds = mBounds;

		// Save texture file name.
		pathObj->mDiffuseTextureFilePath = tFilePathPtr( mEditableProperties.fGetValue( fEditablePropDiffuseTextureFilePath( ), std::string("") ) );
		pathObj->mNormalMapFilePath = tFilePathPtr( mEditableProperties.fGetValue( fEditablePropNormalMapFilePath( ), std::string("") ) );

		// Save depth bias.
		pathObj->mDepthBias = mEditableProperties.fGetValue( cDepthBias, 0 );
		pathObj->mSlopeScaleDepthBias = mEditableProperties.fGetValue( cSlopeScaleBias, 0 );

		// Save camera depth offset.
		pathObj->mCameraDepthOffset = mEditableProperties.fGetValue( cCameraDepthOffset, 0.f );

		// Save light thing.
		pathObj->mAcceptsLights = mEditableProperties.fGetValue( cAcceptsLights, true );

		// Save path length
		pathObj->mPathLength = mPathLength;

		return Sigml::tObjectPtr( pathObj );
	}

	Math::tVec3f tEditablePathDecalEntity::fComputeWallVector( const Math::tVec3f& current, const Math::tVec3f& next )
	{
		Math::tVec3f toNextUnit = (next - current);
		toNextUnit.fNormalizeSafe( );
		return Math::tVec3f( 0.f, 1.f, 0.f ).fCross( toNextUnit );
	}

	void tEditablePathDecalEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		for( u32 i = 0; i < mControlWaypoints.fCount( ); ++i )
			mControlWaypoints[i]->fNotifyPropertyChanged( property );

		const std::string propName = property.fGetName( );
		if( propName == fEditablePropNormalMapFilePath( )
			|| propName == cAcceptsLights)
		{
			fRefreshTexture( );
		}
		else if( 
			   propName == cFadeDist
			|| propName == cUScale
			|| propName == cVScale
			|| propName == cSplineType
			|| propName == cDrawBoundingBoxes
			|| propName == cApplyToAllMeshes
			|| propName == cApplyToTag )
		{
			fRefreshGeometry( );
		}
		else if( propName == cDepthBias )
		{
			Gfx::tRenderState newBiasState = mTexturedTris->fRenderState( );

			s32 depthBias = 0;
			depthBias = property.fGetData( depthBias );
			newBiasState.fSetDepthBias( depthBias );

			mTexturedTris->fSetRenderState( newBiasState );
		}
		else if( propName == cSlopeScaleBias )
		{
			Gfx::tRenderState newBiasState = mTexturedTris->fRenderState( );

			s32 depthBias = 0;
			depthBias = property.fGetData( depthBias );
			newBiasState.fSetSlopeScaleBias( depthBias );

			mTexturedTris->fSetRenderState( newBiasState );
		}
		else if( propName == cCameraDepthOffset )
		{
			f32 depthOffset = 0.f;
			mTexturedTris->fSetCameraDepthOffset( property.fGetData( depthOffset ) );
		}
		else if( propName == fEditablePropDiffuseTextureFilePath( ) )
		{
			const b32 wasNull = mDiffuseTextureRes.fNull( ) || !mDiffuseTextureRes->fLoaded( );
			fRefreshTexture( );
			if( wasNull )
				fRefreshGeometry( );
		}
	}

	namespace
	{
		/// 
		/// \brief This class also assigns UVs while it is gathering the triangles.
		template<class tVolume>
		struct tCollectTrisIntersectCallback : public tEntityBVH::tIntersectVolumeCallback<tVolume>
		{
			mutable tGrowableArray< tTexturedTri >&		mTris;
			mutable tGrowableArray< Math::tPlanef >		mBoundingPlanes;

			mutable tHashTable< Math::tTrianglef, u32 >	mDuplicateFinder;

			mutable tGrowableArray<Gfx::tSolidColorRenderVertex>& mDEBUGLINES;

			f32 mMaxV;

			Math::tVec2f mUVCornerLeft, mUVCornerRight;
			Math::tVec2f mUVNextLeft, mUVNextRight;

			Math::tTrianglef mZeroTri, mOneTri;

			b32 mApplyToAllMeshes;
			std::string mApplyToTag;

			explicit tCollectTrisIntersectCallback( 
				tGrowableArray< tTexturedTri >& tris,
				const tGrowableArray< Math::tPlanef >& bounds,
				b32 applyToAll,
				const std::string& applyToTag,
				f32 startV,
				f32 uScale,
				f32 vScale,
				const Math::tVec3f& vSpan,
				const Math::tVec3f& cornerLeft, 
				const Math::tVec3f& cornerRight,
				const Math::tVec3f& nextLeft,
				const Math::tVec3f& nextRight,
				tGrowableArray<Gfx::tSolidColorRenderVertex>& debugGeom ) 
				: mTris( tris )
				, mBoundingPlanes( bounds )
				, mApplyToAllMeshes( applyToAll )
				, mDEBUGLINES( debugGeom )
				, mZeroTri( cornerLeft, nextLeft, cornerRight )
				, mOneTri( nextLeft, nextRight, cornerRight )
			{
				mMaxV = vSpan.fLength( ) / vScale;

				mUVCornerLeft	= Math::tVec2f( 0.f, startV );
				mUVCornerRight	= Math::tVec2f( uScale, startV );
				mUVNextLeft		= Math::tVec2f( 0.f, startV + + mMaxV  );
				mUVNextRight	= Math::tVec2f( uScale, startV + mMaxV );

				const tProjectFile& projectFile = tProjectFile::fInstance( );
				for( u32 i = 0; i < projectFile.mGameTags.fCount( ); ++i )
				{
					if( StringUtil::fStricmp( projectFile.mGameTags[ i ].mName.c_str(), applyToTag.c_str() ) == 0 )
					{
						std::stringstream ss;
						ss << "GameTag." << i; // actual property name is by index e.g. "Ground" -> "GameTag.0"
						mApplyToTag = ss.str();
					}
				}

				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mZeroTri.mA, Gfx::tVertexColor( 1.f, 1.f, 0.f, 1.f ).fForGpu( ) ) );
				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mZeroTri.mB, Gfx::tVertexColor( 1.f, 1.f, 0.f, 1.f ).fForGpu( ) ) );

				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mZeroTri.mB, Gfx::tVertexColor( 1.f, 1.f, 0.f, 1.f ).fForGpu( ) ) );
				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mZeroTri.mC, Gfx::tVertexColor( 1.f, 1.f, 0.f, 1.f ).fForGpu( ) ) );

				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mZeroTri.mC, Gfx::tVertexColor( 1.f, 1.f, 0.f, 1.f ).fForGpu( ) ) );
				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mZeroTri.mA, Gfx::tVertexColor( 1.f, 1.f, 0.f, 1.f ).fForGpu( ) ) );


				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mOneTri.mA, Gfx::tVertexColor( 0.f, 1.f, 1.f, 1.f ).fForGpu( ) ) );
				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mOneTri.mB, Gfx::tVertexColor( 0.f, 1.f, 1.f, 1.f ).fForGpu( ) ) );

				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mOneTri.mB, Gfx::tVertexColor( 0.f, 1.f, 1.f, 1.f ).fForGpu( ) ) );
				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mOneTri.mC, Gfx::tVertexColor( 0.f, 1.f, 1.f, 1.f ).fForGpu( ) ) );

				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mOneTri.mC, Gfx::tVertexColor( 0.f, 1.f, 1.f, 1.f ).fForGpu( ) ) );
				//mDEBUGLINES.fPushBack( Gfx::tSolidColorRenderVertex( mOneTri.mA, Gfx::tVertexColor( 0.f, 1.f, 1.f, 1.f ).fForGpu( ) ) );
			}

			void operator()( const tVolume& v, tEntityBVH::tObjectPtr octreeObject, b32 aabbWhollyContained ) const
			{
				tSpatialEntity* collectFrom = static_cast< tSpatialEntity* >( octreeObject->fOwner( ) );

				b32 applicable =
					mApplyToAllMeshes ||
					collectFrom->fFirstAncestorOfType< tEditableTerrainEntity >( );

				if( !applicable && mApplyToTag.size() > 0 )
				{
					tEditableObject* eo = collectFrom->fFirstAncestorOfType<tEditableObject>( );
					if ( eo )
					{
						tGrowableArray< tEditablePropertyPtr > tags;
						eo->fGetEditableProperties( ).fGetGroup( Sigml::tObject::fEditablePropGameTagName( ), tags );

						for ( u32 i=0 ; !applicable && i<tags.fCount( ) ; ++i )
						{
							std::string tagName = tags[i]->fGetName( );
							if ( StringUtil::fStricmp( tagName.c_str(), mApplyToTag.c_str() ) == 0 )
								applicable = true;
						}
					}
				}

				if( !applicable )
					return;

				if( !fQuickAabbTest( v, octreeObject, aabbWhollyContained ) )
					return;

				tGrowableArray<Math::tTrianglef> tris;
				collectFrom->fCollectTris( v, tris );

				for( u32 i = 0; i < tris.fCount( ); ++i )
				{
					Math::tTrianglef& thisTri = tris[ i ];

					// Skip any triangles that have already been encountered.
					// If a triangle has known indices for all verts, it's been encountered.
					if( mDuplicateFinder.fFind( thisTri ) )
						continue;

					// Record that this triangle has been processed.
					mDuplicateFinder.fInsert( thisTri, 0 );

					tGrowableArray< Math::tVec3f > points( 3 );
					points[ 0 ] = thisTri.mA + Math::tVec3f( 0.f, 0.01f, 0.f );
					points[ 1 ] = thisTri.mB + Math::tVec3f( 0.f, 0.01f, 0.f );
					points[ 2 ] = thisTri.mC + Math::tVec3f( 0.f, 0.01f, 0.f );

					tGrowableArray< Math::tVec3f > outGeom;

					fClipTri( points, mBoundingPlanes, outGeom );

					// This triangle was altered into a more complex polygon,
					// it needs to be broken back into tris.
					if( outGeom.fCount( ) >= 3 )
					{
						// Make an ugly fan if necessary
						for( u32 j = 1; j < outGeom.fCount( )-1; ++j )
						{
							tTexturedTri newTri( outGeom[0], outGeom[j], outGeom[j+1] );
							if( newTri.fComputeNormal( ).fIsZero( ) )
								continue;

							fGenerateAndAddUVs( newTri );
							mTris.fPushBack( newTri );
						}
					}

					// Any zero size shapes will drop out.
				}
			}

		private:
			static void fClipTri( const tGrowableArray< Math::tVec3f >& tri, const tGrowableArray< Math::tPlanef >& boundingPlanes, tGrowableArray< Math::tVec3f >& outGeom )
			{
				outGeom = tri;
				for( u32 i = 0; i < boundingPlanes.fCount( ); ++i )
				{
					// Polygon was clipped out entirely.
					if( outGeom.fCount( ) == 0 )
						return;

					// Feed the previous product geometry back in as input.
					tGrowableArray< Math::tVec3f > inputGeom = outGeom;
					outGeom.fDeleteArray( );
					fClipAgainstPlane( inputGeom, boundingPlanes[ i ], outGeom );
				}
			}

			static void fClipAgainstPlane( const tGrowableArray< Math::tVec3f >& geom, const Math::tPlanef& plane, tGrowableArray< Math::tVec3f >& outGeom )
			{
				Math::tVec3f previous = geom.fBack( );
				for( u32 i = 0; i < geom.fCount( ); ++i )
				{
					Math::tVec3f current = geom[ i ];

					// If the point is inside this plane
					if( plane.fSignedDistance( current ) > 0 )
					{
						// And the previous point is outside
						if( plane.fSignedDistance( previous ) <= 0 )
						{
							// Then clip the previous point.
							Math::tRayf ray( previous, current-previous );
							Math::tIntersectionRayPlane<f32> intersection( ray, plane );
							if( intersection.fIntersects( ) )
								outGeom.fPushBack( previous + ray.mExtent * intersection.fT( ) );
						}
						outGeom.fPushBack( current );
					}
					// If this point is outside and the previous is inside:
					else if( plane.fSignedDistance( previous ) > 0 )
					{
						// Then clip.
						Math::tRayf ray( previous, current-previous );
						Math::tIntersectionRayPlane<f32> intersection( ray, plane );
						if( intersection.fIntersects( ) )
							outGeom.fPushBack( previous + ray.mExtent * intersection.fT( ) );
					}

					// The current point becomes the previous.
					previous = current;
				}
			}

			void fGenerateAndAddUVs( tTexturedTri& tri ) const
			{
				// Generate em!
				fGenerateUVForPoint( tri.mA, tri.mUVA );
				fGenerateUVForPoint( tri.mB, tri.mUVB );
				fGenerateUVForPoint( tri.mC, tri.mUVC );
			}

			b32 fComputeBarycentric( 
				const Math::tVec3f& p, 
				const Math::tVec3f& triA, 
				const Math::tVec3f& triB, 
				const Math::tVec3f& triC, 
				f32& outA, 
				f32& outB, 
				f32& outC ) const
			{
				Math::tVec3f P( p.x, 0.f, p.z );
				Math::tVec3f A( triA.x, 0.f, triA.z );
				Math::tVec3f B( triB.x, 0.f, triB.z );
				Math::tVec3f C( triC.x, 0.f, triC.z );

				Math::tVec3f N = (B-A).fCross(C-A).fNormalizeSafe( );

				f32 areaABC = N.fDot( (B-A).fCross(C-A) );

				f32 areaPBC = N.fDot( (B-P).fCross(C-P) );
				outA = areaPBC / areaABC;

				f32 areaPCA = N.fDot( (C-P).fCross(A-P) );
				outB = areaPCA / areaABC;

				outC = 1.0f - outA - outB;

				// Floating point error.
				if( outA >= -0.01f && outB >= -0.01f && outC >= -0.01f )
				{
					//outA = fMax( outA, 0.f );
					//outB = fMax( outB, 0.f );
					//outC = fMax( outC, 0.f );
					return true;
				}

				return false;
			}

			void fGenerateUVForPoint( const Math::tVec3f& p, Math::tVec2f& uv ) const
			{
				f32 a = 0.f;
				f32 b = 0.f;
				f32 c = 0.f;

				if( fComputeBarycentric( p, mZeroTri.mA, mZeroTri.mB, mZeroTri.mC, a, b, c ) )
				{
					uv = mUVCornerLeft * a + mUVNextLeft * b + mUVCornerRight * c;
				}
				else if( fComputeBarycentric( p, mOneTri.mA, mOneTri.mB, mOneTri.mC, a, b, c ) )
				{
					uv = mUVNextLeft * a + mUVNextRight * b + mUVCornerRight * c;
				}
				else
				{
					// Problem case.
					uv = Math::tVec2f( 1.f, 0.f );
				}
			}
		};
	}

	void tEditablePathDecalEntity::fRefreshGeometry( )
	{
		if( !fSceneGraph( ) || !mConnectionLines ) 
			return;

		tGrowableArray<Gfx::tSolidColorRenderVertex> connectionLines;
		tGrowableArray<Gfx::tSolidColorRenderVertex> wireframeTris;

		if( !mInContainer || mControlWaypoints.fCount( ) <= 1 )
		{
			mTexturedTris->fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ), 0, 0, 0, 0 );
			mWireframeTris->fSetGeometry( wireframeTris, false );
			mConnectionLines->fSetGeometry( connectionLines, false );
			return;
		}

		// Update the object's render state.
		Gfx::tRenderState newBiasState = mTexturedTris->fRenderState( );
		newBiasState.fSetDepthBias( mEditableProperties.fGetValue( cDepthBias, 0 ) );
		newBiasState.fSetSlopeScaleBias( mEditableProperties.fGetValue( cSlopeScaleBias, 0 ) );
		mTexturedTris->fSetRenderState( newBiasState );

		mTexturedTris->fSetCameraDepthOffset( mEditableProperties.fGetValue( cCameraDepthOffset, 0.f ) );

		mFullWithSubs.fSetCount( 0 );

		if( mControlWaypoints.fCount( ) == 2 )
		{
			mFullWithSubs.fPushBack( tSubPoint( mControlWaypoints.fFront( )->fObjectToWorld( ), mControlWaypoints.fFront( )->fObjectSpaceBox( ) ) );
			mFullWithSubs.fBack( ).mRealControlIdx = 0;
			mFullWithSubs.fPushBack( tSubPoint( mControlWaypoints.fBack( )->fObjectToWorld( ), mControlWaypoints.fBack( )->fObjectSpaceBox( ) ) );
			mFullWithSubs.fBack( ).mRealControlIdx = 1;
		}
		else
		{
			fGenerateSubPoints( );
		}

		// Sanity.
		sigassert( mFullWithSubs.fCount( ) > 1 );

		const u32 connectorColor = Gfx::tVertexColor( 0.f, 0.f, 1.f, 1.f ).fForGpu( );
		const u32 boxColor = Gfx::tVertexColor( 0.f, 0.f, 0.f, 1.f ).fForGpu( );

		f32 lastV = 0.f;
		const f32 uScale = mEditableProperties.fGetValue( cUScale, 1.f );
		const f32 vScale = mEditableProperties.fGetValue( cVScale, 1.f );

		tGrowableArray< tTexturedTri > totalTris;

		for( u32 i = 0;  i < mFullWithSubs.fCount( )-1; ++i )
		{
			const Math::tVec3f currentPos = mFullWithSubs[ i ].mXForm.fGetTranslation( );
			const Math::tVec3f currentScale = mFullWithSubs[ i ].mXForm.fGetScale( );

			const Math::tVec3f nextPos = mFullWithSubs[ i+1 ].mXForm.fGetTranslation( );
			const Math::tVec3f nextScale = mFullWithSubs[ i+1 ].mXForm.fGetScale( );

			const Math::tVec3f toNext = nextPos - currentPos;
			Math::tVec3f toNextUnit = toNext;
			f32 toNextLen;
			toNextUnit.fNormalizeSafe( toNextLen );
			if( toNextLen == 0.f )
			{
				log_line( 0, "Warning: two decal nodes share the same position." );

				mTexturedTris->fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ), 0, 0, 0, 0 );
				mWireframeTris->fSetGeometry( wireframeTris, false );
				mConnectionLines->fSetGeometry( connectionLines, false );

				return;
			}

			const f32 largestY = fMax( currentScale.y, nextScale.y );
			const f32 largestX = fMax( currentScale.x, nextScale.x );

			Math::tVec3f forwardAxis	= toNext * 0.5f + toNextUnit * currentScale.z * 0.5f + toNextUnit * nextScale.z * 0.5f;
			Math::tVec3f sideAxis		= forwardAxis.fCross( Math::tVec3f( 0.f, 1.f, 0.f ) ).fNormalizeSafe( );
			Math::tVec3f upAxis			= sideAxis.fCross( forwardAxis ).fNormalizeSafe( );
			Math::tVec3f center			= currentPos + forwardAxis - toNextUnit * currentScale.z;

			Math::tObbf box( center, sideAxis * largestX, upAxis * largestY, forwardAxis );
			tGrowableArray< Math::tPlanef > planes;

			// Clear this out so the border planes are oriented vertically at the ends.
			// Keeps UVs from behaving poorly at the ends.
			forwardAxis.y = 0.f;
			forwardAxis.fNormalizeSafe( );

			if( i == 0 )
			{
				mFullWithSubs[ i ].mHingePlane = Math::tPlanef( forwardAxis, currentPos );
				planes.fPushBack( mFullWithSubs[ i ].mHingePlane ); // Front = 0
				mFullWithSubs[ i ].mLeft = currentPos + sideAxis * currentScale.x;
				mFullWithSubs[ i ].mRight = currentPos - sideAxis * currentScale.x;
			}
			else
			{
				planes.fPushBack( Math::tPlanef( mFullWithSubs[ i ].mHingePlane.fGetNormal( ) * -1.f, currentPos ) );
			}

			if( i == mFullWithSubs.fCount( )-2 )
			{
				mFullWithSubs[ i+1 ].mHingePlane = Math::tPlanef( -forwardAxis, nextPos );
				planes.fPushBack( mFullWithSubs[ i+1 ].mHingePlane ); // Back = 1
				mFullWithSubs[ i+1 ].mLeft = nextPos + sideAxis * nextScale.x;
				mFullWithSubs[ i+1 ].mRight = nextPos - sideAxis * nextScale.x;
			}
			else
			{
				const Math::tVec3f nextNextPos = mFullWithSubs[ i+2 ].mXForm.fGetTranslation( );
				Math::tVec3f toNextNextUnit = nextNextPos - nextPos;
				toNextNextUnit.y = 0.f;
				toNextNextUnit.fNormalizeSafe( );

				Math::tVec3f toNextFlat( -toNextUnit.x, 0.f, -toNextUnit.z );
				toNextFlat.fNormalizeSafe( );

				Math::tVec3f halfWay = (toNextNextUnit + toNextFlat) * 0.5f;
				if( halfWay.fIsZero( ) )
					halfWay = toNextNextUnit.fCross( Math::tVec3f( 0.f, 1.f, 0.f ) );

				halfWay.fNormalizeSafe( );
				Math::tVec3f frontNorm = halfWay.fCross( Math::tVec3f( 0.f, 1.f, 0.f ) );

				if( frontNorm.fDot( -toNextUnit ) < 0 )
				{
					frontNorm *= -1.f;
					mFullWithSubs[ i+1 ].mLeft = nextPos - halfWay * nextScale.x;
					mFullWithSubs[ i+1 ].mRight = nextPos + halfWay * nextScale.x;
				}
				else
				{
					mFullWithSubs[ i+1 ].mLeft = nextPos + halfWay * nextScale.x;
					mFullWithSubs[ i+1 ].mRight = nextPos - halfWay * nextScale.x;
				}

				Math::tPlanef hinge( frontNorm.fNormalizeSafe( ), nextPos );
				planes.fPushBack( hinge );

				mFullWithSubs[ i+1 ].mHingePlane = hinge;
			}

			const Math::tVec3f leftCurrent		= mFullWithSubs[ i ].mLeft;
			const Math::tVec3f leftNext			= mFullWithSubs[ i+1 ].mLeft;
			const Math::tVec3f rightCurrent		= mFullWithSubs[ i ].mRight;
			const Math::tVec3f rightNext		= mFullWithSubs[ i+1 ].mRight;

			Math::tVec3f wallVector				= fComputeWallVector( leftCurrent, leftNext );
			Math::tVec3f rwallVector			= -fComputeWallVector( rightCurrent, rightNext );

			planes.fPushBack( Math::tPlanef( wallVector.fNormalizeSafe( ), leftCurrent ) ); // Left
			planes.fPushBack( Math::tPlanef( rwallVector.fNormalizeSafe( ), rightCurrent ) ); // Right

			planes.fPushBack( Math::tPlanef( -upAxis.fNormalizeSafe( ), box.fCorner( 4 ) ) ); // Top
			planes.fPushBack( Math::tPlanef( upAxis.fNormalizeSafe( ), box.fCorner( 3 ) ) ); // Bottom

			tGrowableArray< tTexturedTri > tris;
			tCollectTrisIntersectCallback<Math::tObbf> collector( 
				tris, 
				planes, 
				mEditableProperties.fGetValue( cApplyToAllMeshes, false ), 
				mEditableProperties.fGetValue( cApplyToTag, std::string("Ground") ),
				lastV, 
				uScale,
				vScale, 
				toNext, 
				leftCurrent, 
				rightCurrent, 
				leftNext, 
				rightNext, 
				wireframeTris );
			fSceneGraph( )->fCollectTris( box, collector );

			// These tris will be used later.
			totalTris.fJoin( tris );

			// Progress the V texture direction.
			lastV += collector.mMaxV;

			// Orient the real control point to face z along the facing and x for width.
			if( mFullWithSubs[ i ].mRealControlIdx >= 0 )
			{
				tEditablePathDecalWaypoint* thisWaypoint = mControlWaypoints[ mFullWithSubs[ i ].mRealControlIdx ].fGetRawPtr( );

				Math::tMat3f pos = thisWaypoint->fObjectToWorld( );
				Math::tVec3f scale = pos.fGetScale( );

				// Switch to keep the nodes facing z forward down the chain.
				if( i == 0 )
					pos.fOrientZAxis( mFullWithSubs[ i ].mHingePlane.fGetNormalUnit( ) );
				else
					pos.fOrientZAxis( -mFullWithSubs[ i ].mHingePlane.fGetNormalUnit( ) );

				pos.fScaleLocal( scale );

				thisWaypoint->fDisableGeometryUpdates( true );
				thisWaypoint->fMoveTo( pos );
				thisWaypoint->fDisableGeometryUpdates( false );
			}


#if DRAW_BOUNDING_OBBS
			if( mEditableProperties.fGetValue( cDrawBoundingBoxes, true ) )
				fDrawBox( wireframeTris, box, boxColor );
#endif // DRAW_BOUNDING_OBBS

#if DRAW_WIREFRAME_TRIS
			for( u32 i = 0; i < tris.fCount( ); ++i )
			{
				//wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[ i ].mA, Gfx::tVertexColor( tris[ i ].mUVA.x, tris[ i ].mUVA.y, 0.f, 1.f ).fForGpu( ) ) );
				//wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[ i ].mB, Gfx::tVertexColor( tris[ i ].mUVB.x, tris[ i ].mUVB.y, 0.f, 1.f ).fForGpu( ) ) );

				//wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[ i ].mB, Gfx::tVertexColor( tris[ i ].mUVB.x, tris[ i ].mUVB.y, 0.f, 1.f ).fForGpu( ) ) );
				//wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[ i ].mC, Gfx::tVertexColor( tris[ i ].mUVC.x, tris[ i ].mUVC.y, 0.f, 1.f ).fForGpu( ) ) );

				//wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[ i ].mC, Gfx::tVertexColor( tris[ i ].mUVC.x, tris[ i ].mUVC.y, 0.f, 1.f ).fForGpu( ) ) );
				//wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[ i ].mA, Gfx::tVertexColor( tris[ i ].mUVA.x, tris[ i ].mUVA.y, 0.f, 1.f ).fForGpu( ) ) );

				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[i].mA, connectorColor ) );
				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[i].mB, connectorColor ) );

				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[i].mB, connectorColor ) );
				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[i].mC, connectorColor ) );

				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[i].mC, connectorColor ) );
				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( tris[i].mA, connectorColor ) );
			}
#endif // DRAW_WIREFRAME_TRIS
		}

		// store maximal V value
		mPathLength = lastV * vScale;

#if DRAW_CONNECTION_LINES
		fDrawConnections( connectionLines, connectorColor );
#endif // DRAW_CONNECTION_LINES

		// Orient the last node.
		if( mFullWithSubs.fBack( ).mRealControlIdx >= 0 )
		{
			tEditablePathDecalWaypoint* thisWaypoint = mControlWaypoints[ mFullWithSubs.fBack( ).mRealControlIdx ].fGetRawPtr( );

			Math::tMat3f pos = thisWaypoint->fObjectToWorld( );
			Math::tVec3f scale = pos.fGetScale( );

			// Have to switch the normal to keep the nodes pointing z-forward.
			pos.fOrientZAxis( -mFullWithSubs.fBack( ).mHingePlane.fGetNormalUnit( ) );
			pos.fScaleLocal( scale );

			thisWaypoint->fDisableGeometryUpdates( true );
			thisWaypoint->fMoveTo( pos );
			thisWaypoint->fDisableGeometryUpdates( false );
		}

		// Build the decal's geometry.
		tGrowableArray< tVertexData > verts;
		tGrowableArray< Math::tVec3u > idxs;
		fConvertTextureTriGeometry( totalTris, verts, idxs );
		fBuildGeometry( verts, idxs );

		mWireframeTris->fSetGeometry( wireframeTris, false );
		mWireframeTris->fSetObjectSpaceBox( mBounds );
	}

	void tEditablePathDecalEntity::fConvertTextureTriGeometry( 
		const tGrowableArray< tTexturedTri >& tris, 
		tGrowableArray< tVertexData >& outVerts, 
		tGrowableArray< Math::tVec3u >& outIdxs )
	{
		tVertexSorter vertSorter;
		mSaveVerts.fSetCount( 0 );
		mSaveTris.fSetCount( 0 );

		// Build index lists and do some tangent/normal math.
		mSaveTris.fSetCount( tris.fCount( ) );
		for( u32 i = 0; i < tris.fCount( ); ++i )
		{
			fProcessVerts( tris[i], vertSorter, outVerts, outIdxs );

			mSaveTris[i] = outIdxs[i];
		}

		mBounds.fInvalidate( );

		// Finalize some tangent/normal math.
		mSaveVerts.fSetCount( outVerts.fCount( ) );
		mSaveUVs.fSetCount( outVerts.fCount( ) );
		for( u32 i = 0; i < outVerts.fCount( ); ++i )
		{
			tVertexData& thisVert = outVerts[i];

			// For posterity.
			mSaveVerts[i] = outVerts[i].mPos;
			mSaveUVs[i] = outVerts[i].mUV;

			fFinalizeTangets( thisVert );

			mBounds |= thisVert.mPos;
		}
	}

	void tEditablePathDecalEntity::fBuildGeometry( tGrowableArray< tVertexData >& outVerts, tGrowableArray< Math::tVec3u >& outIdxs )
	{
		tFilePathPtr diffTex( mEditableProperties.fGetValue( fEditablePropDiffuseTextureFilePath( ), std::string("") ) );
		if( diffTex.fLength( ) == 0 )
			return;

		const f32 vScale = mEditableProperties.fGetValue( cVScale, 1.f );
		const f32 fadeDist = mEditableProperties.fGetValue( cFadeDist, 1.f );

		const u32 numIdxs = ( outIdxs.fCount( ) * 3 );
		tDynamicArray<u16> renderIds( numIdxs );
		for( u32 i = 0; i < outIdxs.fCount( ); ++i )
			for( u32 j = 0; j < 3; ++j )
				renderIds[ i * 3 + j ] = ( u16 )outIdxs[ i ].fAxis( j );

		tDynamicArray< Gfx::tDecalRenderVertex > renderVerts( outVerts.fCount( ) );
		for( u32 i = 0; i < outVerts.fCount( ); ++i )
		{
			tVertexData& v0Data		= outVerts[ i ];
			renderVerts[ i ].mP		= v0Data.mPos;
			renderVerts[ i ].mUv	= v0Data.mUV;
			renderVerts[ i ].mN		= v0Data.mN;
			renderVerts[ i ].mTan	= v0Data.mTan;
			renderVerts[ i ].mColor = fMakeVertexColor( mPathLength, fadeDist, vScale, v0Data.mUV.y );
		}

		mTexturedTris->fSetObjectSpaceBox( mBounds );
		mTexturedTris->fCreateGeometry( *Gfx::tDevice::fGetDefaultDevice( ), renderIds.fBegin( ), numIdxs, renderVerts.fBegin( ), renderVerts.fCount( ) );
	}

	void tEditablePathDecalEntity::fProcessVerts( 
		const tTexturedTri& verts, 
		tVertexSorter& sorter, 
		tGrowableArray< tVertexData >& outVerts,
		tGrowableArray< Math::tVec3u >& outIdxs )
	{
		Math::tVec3u thisIdxSet;
		const Math::tVec3f faceNorm = verts.fComputeUnitNormal( );

		fProcessVert( 0, verts.mA, faceNorm, verts.mUVA, thisIdxSet, sorter, outVerts );
		fProcessVert( 1, verts.mB, faceNorm, verts.mUVB, thisIdxSet, sorter, outVerts );
		fProcessVert( 2, verts.mC, faceNorm, verts.mUVC, thisIdxSet, sorter, outVerts );

		fCalculateTangents( 
			verts.mA,					verts.mB,					verts.mC, 
			verts.mUVA,					verts.mUVB,					verts.mUVC, 
			outVerts[ thisIdxSet[0] ],	outVerts[ thisIdxSet[1] ],	outVerts[ thisIdxSet[2] ] );

		outIdxs.fPushBack( thisIdxSet );
	}

	void tEditablePathDecalEntity::fCalculateTangents( 
		const Math::tVec3f& v0, 
		const Math::tVec3f& v1, 
		const Math::tVec3f& v2, 
		const Math::tVec2f& uv0, 
		const Math::tVec2f& uv1, 
		const Math::tVec2f& uv2,
		tVertexData& outV0,
		tVertexData& outV1,
		tVertexData& outV2 )
	{
		// This is step one of calculating bitangents.
		f32 x0 = v1.x - v0.x;
		f32 x1 = v2.x - v0.x;
		f32 y0 = v1.y - v0.y;
		f32 y1 = v2.y - v0.y;
		f32 z0 = v1.z - v0.z;
		f32 z1 = v2.z - v0.z;

		f32 s0 = uv1.x - uv0.x;
		f32 s1 = uv2.x - uv0.x;
		f32 t0 = uv1.y - uv0.y;
		f32 t1 = uv2.y - uv0.y;

		f32 rDenom = (s0 * t1 - s1 * t0);
		if( fEqual( rDenom, 0.f ) )
			rDenom = 1.f;
		f32 r = 1.f / rDenom;
		Math::tVec4f sdir((t1 * x0 - t0 * x1) * r, (t1 * y0 - t0 * y1) * r, (t1 * z0 - t0 * z1) * r, 0.f);
		Math::tVec3f tdir((s0 * x1 - s1 * x0) * r, (s0 * y1 - s1 * y0) * r, (s0 * z1 - s1 * z0) * r);

		outV0.mTan += sdir;
		outV1.mTan += sdir;
		outV2.mTan += sdir;

		outV0.mBitan += tdir;
		outV1.mBitan += tdir;
		outV2.mBitan += tdir;

		sigassert( !outV0.mTan.fIsNan( ) );
		sigassert( !outV1.mTan.fIsNan( ) );
		sigassert( !outV2.mTan.fIsNan( ) );

		sigassert( !outV0.mBitan.fIsNan( ) );
		sigassert( !outV1.mBitan.fIsNan( ) );
		sigassert( !outV2.mBitan.fIsNan( ) );
	}

	void tEditablePathDecalEntity::fFinalizeTangets( tVertexData& vert )
	{
		vert.mN.fNormalizeSafe( );
		vert.mNumNorms = 1;

		const Math::tVec3f n = vert.mN;
		const Math::tVec3f t = Math::tVec3f( vert.mTan.x, vert.mTan.y, vert.mTan.z );

		vert.mTan = Math::tVec4f( (t - n * n.fDot(t)).fNormalizeSafe( ), 0.f );
		vert.mTan.w = (n.fCross(t).fDot( vert.mBitan ) < 0.0f) ? -1.0f : 1.0f;
	}

	void tEditablePathDecalEntity::fProcessVert( 
		u32 i, 
		const Math::tVec3f& vert, 
		const Math::tVec3f& normal, 
		const Math::tVec2f& uv, 
		Math::tVec3u& outIdxSet, 
		tVertexSorter& sorter,
		tGrowableArray< tVertexData >& outVerts )
	{
		// Grab the vertex and see if it already exists in the list of verts we've encountered.
		const u32* foundIdx = sorter.fFind( vert );

		if( foundIdx )
		{
			// The vertex already exists, use its found index.
			outIdxSet[i] = *foundIdx;

			tVertexData& foundRec = outVerts[ *foundIdx ];
			foundRec.mN += normal;
			++foundRec.mNumNorms;
		}
		else
		{
			// Vertex not found. Push it into the hash map which will store its index.
			outIdxSet[i] = outVerts.fCount( );

			tVertexData newVert;
			newVert.mPos		= vert;
			newVert.mN			= normal;
			newVert.mUV			= uv;
			newVert.mNumNorms	= 1;

			sorter.fInsert( vert, outIdxSet[i] );
			outVerts.fPushBack( newVert );
		}
	}


	void tEditablePathDecalEntity::fRefreshTexture( )
	{
		fRefreshTexHandles( );
		mTexturedTris->fChangeMaterial( fConstructMaterial( ) );
	}

	void tEditablePathDecalEntity::fAcquireEntireDecal( tGrowableArray<tEditablePathDecalWaypoint*>& wayPoints )
	{
		for( u32 i = 0; i < mControlWaypoints.fCount( ); ++i )
		{
			tEditablePathDecalWaypoint* thisWaypoint = mControlWaypoints[i].fGetRawPtr( );
			if( wayPoints.fFind( thisWaypoint ) )
				continue;

			wayPoints.fPushBack( thisWaypoint );
		}
	}

	void tEditablePathDecalEntity::fGenerateSubPoints( )
	{
		static const u32 numNodesPerSegment = 3;
		const f32 timeInterval = 1.f / (numNodesPerSegment+1);

		const u32 type = mEditableProperties.fGetValue( cSplineType, ( u32 )cNumSplineTypes );

		for( u32 i = 0; i < mControlWaypoints.fCount( )-1; ++i )
		{
			// The control point is the first part of this segment.
			mFullWithSubs.fPushBack( tSubPoint( mControlWaypoints[ i ]->fObjectToWorld( ), mControlWaypoints[ i ]->fObjectSpaceBox( ) ) );
			mFullWithSubs.fBack( ).mRealControlIdx = i;

			// Insert just the control waypoint if there is no curve.
			if( type == cNoCurve )
				continue;

			const Math::tVec3f currentPos = mControlWaypoints[ i ]->fObjectToWorld( ).fGetTranslation( );
			const Math::tVec3f currentScale = mControlWaypoints[ i ]->fObjectToWorld( ).fGetScale( );

			const Math::tVec3f nextPos = mControlWaypoints[ i+1 ]->fObjectToWorld( ).fGetTranslation( );
			const Math::tVec3f nextScale = mControlWaypoints[ i+1 ]->fObjectToWorld( ).fGetScale( );

			const Math::tVec3f toNextScale = nextScale - currentScale;

			const Math::tVec3f pos0 = ( i == 0 ) ? currentPos : mControlWaypoints[ i-1 ]->fObjectToWorld( ).fGetTranslation( );
			const Math::tVec3f pos1 = currentPos;
			const Math::tVec3f pos2 = nextPos;
			const Math::tVec3f pos3 = ( i == mControlWaypoints.fCount( ) - 2 ) ? nextPos : mControlWaypoints[ i+2 ]->fObjectToWorld( ).fGetTranslation( );
			
			for( u32 j = 0; j < numNodesPerSegment; ++j )
			{
				const f32 t = (j+1) * timeInterval;

				Math::tMat3f xForm( Math::MatrixUtil::cIdentityTag );

				// Switch on which type of spline curve to use.
				if( type == cCatmullRom )
				{
					xForm.fSetTranslation( Math::fCatmullRom( pos0, pos1, pos2, pos3, t ) );
				}
				else if( type == cBezier )
				{
					// TODO!!
					log_line( 0, "Bezier not implemented." );
					xForm.fSetTranslation( Math::fCatmullRom( pos0, pos1, pos2, pos3, t ) );
				}
				else
				{
					// Unrecognized spline.
					log_line( 0, "Unrecognized spline.\n" );
				}

				xForm.fSetDiagonal( currentScale + toNextScale * t );

				mFullWithSubs.fPushBack( tSubPoint( xForm, mControlWaypoints[ i ]->fObjectSpaceBox( ) ) );
			}
		}

		// The above loop stops before the last node.
		mFullWithSubs.fPushBack( tSubPoint( mControlWaypoints.fBack( )->fObjectToWorld( ), mControlWaypoints.fBack( )->fObjectSpaceBox( ) ) );
		mFullWithSubs.fBack( ).mRealControlIdx = mControlWaypoints.fCount( ) - 1;
	}

	void tEditablePathDecalEntity::fDrawBox( tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const Math::tObbf& box, const u32 color )
	{
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 0 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 2 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 6 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 4 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 0 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 4 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 2 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 6 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 1 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 3 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 7 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 5 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 1 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 5 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 3 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 7 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 0 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 1 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 2 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 3 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 4 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 5 ), color ) );

		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 6 ), color ) );
		verts.fPushBack( Gfx::tSolidColorRenderVertex( box.fCorner( 7 ), color ) );
	}

	void tEditablePathDecalEntity::fDrawConnections( tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const u32 color )
	{

		const Math::tVec3f origin = mControlWaypoints.fFront( )->fObjectToWorld( ).fGetTranslation( ) + Math::tVec3f( 0.f, 4.f, 0.f );

		for( u32 i = 0; i < mControlWaypoints.fCount( ); ++i )
		{
			if( !mControlWaypoints[ i ]->fSceneGraph( ) )
				continue;
			if(  mControlWaypoints[ i ]->fIsHidden( ) )
				continue;

			const Math::tVec3f currentPos = mControlWaypoints[ i ]->fObjectToWorld( ).fGetTranslation( );
			const Math::tVec3f toConnect = currentPos - origin;
			f32 toConnectLen = 0.f;
			const Math::tVec3f toConnectNorm = Math::tVec3f( toConnect ).fNormalizeSafe( Math::tVec3f::cZAxis, toConnectLen );
			const Math::tVec3f midpoint = origin + 0.5f * toConnect;

			Math::tMat3f basis = Math::tMat3f::cIdentity;
			basis.fOrientZAxis( toConnectNorm );

			const Math::tVec3f extraPoint0 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm + ( 0.02f * toConnectLen ) * basis.fXAxis( );
			const Math::tVec3f extraPoint1 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm - ( 0.02f * toConnectLen ) * basis.fXAxis( );
			const Math::tVec3f extraPoint2 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm + ( 0.02f * toConnectLen ) * basis.fYAxis( );
			const Math::tVec3f extraPoint3 = midpoint - ( 0.05f * toConnectLen ) * toConnectNorm - ( 0.02f * toConnectLen ) * basis.fYAxis( );

			verts.fPushBack( Gfx::tSolidColorRenderVertex( origin, color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( currentPos, color ) );

			verts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint0, color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, color ) );

			verts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint1, color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, color ) );

			verts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint2, color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, color ) );

			verts.fPushBack( Gfx::tSolidColorRenderVertex( extraPoint3, color ) );
			verts.fPushBack( Gfx::tSolidColorRenderVertex( midpoint, color ) );
		}

		mConnectionLines->fSetGeometry( verts, false );
	}

	void tEditablePathDecalEntity::fRefreshTexHandles( )
	{
		tFilePathPtr diffusePath( mEditableProperties.fGetValue( fEditablePropDiffuseTextureFilePath( ), std::string("") ) );
		if( diffusePath.fLength( ) > 0 )
			mDiffuseTextureRes = mContainer.fGetResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( diffusePath ), this );
		else
			mDiffuseTextureRes.fReset( NULL );

		tFilePathPtr normalPath( mEditableProperties.fGetValue( fEditablePropNormalMapFilePath( ), std::string("") ) );
		if( normalPath.fLength( ) > 0 )
			mNormalMapRes = mContainer.fGetResourceDepot( )->fQueryLoadBlock( tResourceId::fMake<Gfx::tTextureFile>( normalPath ), this );
		else
			mNormalMapRes.fReset( NULL );
	}

	Gfx::tDecalMaterial* tEditablePathDecalEntity::fConstructMaterial( )
	{
		Gfx::tDecalMaterial* decalMat = new Gfx::tDecalMaterial( );

		// Keep a reference to the decal material file.
		const Gfx::tDefaultAllocators& defaultAllocs = Gfx::tDefaultAllocators::fInstance( );
		decalMat->fSetMaterialFileResourcePtrOwned( defaultAllocs.mDecalMaterialFile );

		// Set textures and sampling.
		decalMat->mDiffuseMap.fSetDynamic( mDiffuseTextureRes.fGetRawPtr( ) );
		decalMat->mDiffuseMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );
		decalMat->mNormalMap.fSetDynamic( mNormalMapRes.fGetRawPtr( ) );
		decalMat->mNormalMap.fSetSamplingModes( Gfx::tTextureFile::cFilterModeWithMip, Gfx::tTextureFile::cAddressModeWrap );

		decalMat->fSetAcceptsLights( mEditableProperties.fGetValue( cAcceptsLights, true ) );

		return decalMat;
	}
}
