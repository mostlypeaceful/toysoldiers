#include "ToolsPch.hpp"
#include "tEditableCameraEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tWorldSpaceLines.hpp"
#include "tSigmlConverter.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tWxSlapOnListBox.hpp"
#include "tWxSlapOnButton.hpp"

namespace Sig { namespace
{
	static const char* fEditablePropFOV( ) { return "Lens.FOV"; }
	static const char* fEditablePropFarPlane( ) { return "Lens.Far"; }
	static const char* fEditablePropLookAt( ) { return "Debug.LookAt"; }

	void fFrustumToGeometry( const Gfx::tLens& lens, Gfx::tWorldSpaceLines& geo )
	{
		tFixedArray< Math::tVec3f, Gfx::tLens::cCornersCount > corners;
		lens.fGetCorners( corners, Math::tMat3f::cIdentity );

		// near far loops, composed of 4 lines, + 4 more lines connecting them, 2 verts per line.
		u32 verts = (2*4 + 4) * 2;
		tGrowableArray<Gfx::tSolidColorRenderVertex> sysMemVerts;
		sysMemVerts.fSetCount( verts );

		// maps corner order to a logical loop order
		const u32 cCornerMaps[ 8 ] = { Gfx::tLens::cFarTopLeft, Gfx::tLens::cFarTopRight, Gfx::tLens::cFarBottomRight, Gfx::tLens::cFarBottomLeft, Gfx::tLens::cNearTopLeft, Gfx::tLens::cNearTopRight, Gfx::tLens::cNearBottomRight, Gfx::tLens::cNearBottomLeft };

		u32 index = 0;
		for( u32 i = 0; i < 4; ++i )
		{
			// far loop
			sysMemVerts[ index++ ] = Gfx::tSolidColorRenderVertex( corners[ cCornerMaps[ i ] ] );
			sysMemVerts[ index++ ] = Gfx::tSolidColorRenderVertex( corners[ cCornerMaps[ Math::fModulus( i + 1, 4u ) ] ] );

			// near loop
			sysMemVerts[ index++ ] = Gfx::tSolidColorRenderVertex( corners[ cCornerMaps[ 4 + i ] ] );
			sysMemVerts[ index++ ] = Gfx::tSolidColorRenderVertex( corners[ cCornerMaps[ 4 + Math::fModulus( i + 1, 4u ) ] ] );

			// connections
			sysMemVerts[ index++ ] = Gfx::tSolidColorRenderVertex( corners[ i ] );
			sysMemVerts[ index++ ] = Gfx::tSolidColorRenderVertex( corners[ 4 + i ] );
		}

		geo.fSetGeometry( sysMemVerts, false );
	}
}}

namespace Sig { namespace Sigml
{
	class tools_export tCameraObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tCameraObject, 0xE992D4CB );
	public:
		tCameraObject( );
		tCameraObject( const tEditableCameraEntity* ao );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tCameraObject& o )
	{
	}

	register_rtti_factory( tCameraObject, false );

	tCameraObject::tCameraObject( )
	{
	}

	tCameraObject::tCameraObject( const tEditableCameraEntity* ao )
	{
	}

	void tCameraObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tCameraObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );
	}

	tEntityDef* tCameraObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tCameraEntityDef* entityDef = new tCameraEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );
		
		//The FOV is in degrees in SigEd. GameApp wants FOV in radians though, so we convert to radians here.
		//NOTE: 1 is just a fallback value in case we fail to get the FOV value; it does not represent a default FOV!
		entityDef->mLensProperties.mFOV = Math::fToRadians( mEditableProperties.fGetValue<f32>( fEditablePropFOV( ), 1 ) );

		return entityDef;
	}

	tEditableObject* tCameraObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableCameraEntity( container, *this );
	}
}}

namespace Sig
{
	namespace
	{
		static const Math::tVec4f cDummyTint = Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.5f );
	}

	tCameraDummyObjectEntity::tCameraDummyObjectEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox )
		: tEditableObject::tDummyObjectEntity( batchPtr, objectSpaceBox, false )
	{
	}

	void tCameraDummyObjectEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		tEditableObject::tDummyObjectEntity::fRayCast( ray, hit );
	}

	b32	tCameraDummyObjectEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		return tEditableObject::tDummyObjectEntity::fIntersects( v );
	}

	tEditableCameraEntity::tEditableCameraEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fCommonCtor( );
	}

	tEditableCameraEntity::tEditableCameraEntity( tEditableObjectContainer& container, const Sigml::tCameraObject& ao )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fDeserializeBaseObject( &ao );
		fCommonCtor( );
	}

	void tEditableCameraEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment() ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropBoneRelativeAttachment(), false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropFOV(), tProjectFile::fInstance().mEngineConfig.mEditorDefaults.mDefaultLensProperties.mFOV, 0.01f, 179.f, 0.1f, 1 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropFarPlane(), 1000.0, 0.01f, 999999.f, 0.1f, 1 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( fEditablePropLookAt() ) ) );
	}

	void tEditableCameraEntity::fCommonCtor( )
	{
		// setup the hull object
		Gfx::tDefaultAllocators& defAllocators = Gfx::tDefaultAllocators::fInstance( );

		mFrustumGeometry.fReset( new Gfx::tWorldSpaceLines( ) );
		mFrustumGeometry->fResetDeviceObjects( Gfx::tDevice::fGetDefaultDevice( ), defAllocators.mSolidColorMaterial, defAllocators.mSolidColorGeomAllocator, defAllocators.mIndexAllocator );

		mDummyFrustum.fReset( new tCameraDummyObjectEntity( Gfx::tRenderBatchPtr( ), Math::tAabbf::cZeroSized ) );
		mDummyFrustum->fSpawnImmediate( *this );
		mDummyFrustum->fSetInvisible( true );
		mDummyFrustum->fSetRgbaTint( cDummyTint );
		mDummyFrustum->fSetInvisible( false );

		fRebuildVisualization( );
		tEditableObject::fUpdateStateTint( );

		mAdjustingNewLook = false;
	}

	Gfx::tLens tEditableCameraEntity::fBuildLens( b32 buildVisualizationLens ) const
	{
		const tProjectFile& file = tProjectFile::fInstance( );

		// If buildVisualizationLens is set, shorten the far plane for the visualization geometry.
		const f32 farPlane = buildVisualizationLens ? 10.f : mEditableProperties.fGetValue<f32>( fEditablePropFarPlane( ), 10.f );
		const f32 nearPlane = 1.f;
		const f32 aspectRatio = (f32)file.mEngineConfig.mEditorDefaults.mScreenResolution.x / file.mEngineConfig.mEditorDefaults.mScreenResolution.y;
		const f32 fov = fFOV( );

		Gfx::tLens lens;
		lens.fSetPerspective( nearPlane, farPlane, aspectRatio, fov );

		return lens;
	}

	void tEditableCameraEntity::fRebuildVisualization( )
	{
		Gfx::tLens lens = fBuildLens( true );

		fFrustumToGeometry( lens, *mFrustumGeometry );
		mDummyFrustum->fSetRenderBatch( mFrustumGeometry->fGeometry( ).fGetRenderBatch( ) );

		// set object bounds.
		Math::tAabbf localSpaceBox = lens.fAAbb( Math::tMat3f::cIdentity );
		mDummyFrustum->fSetObjectSpaceBox( localSpaceBox );

		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );
	}

	void tEditableCameraEntity::fSetFOV( f32 newFOV )
	{
		// UI fov is entire range so double it.
		mEditableProperties.fSetDataNoNotify<f32>( fEditablePropFOV(), Math::fToDegrees( newFOV * 2.f ) );
	}

	tEditableCameraEntity::~tEditableCameraEntity( )
	{
	}

	void tEditableCameraEntity::fOnMoved( b32 recomputeParentRelative )
	{
		// Lock out any recursive on moving.
		if( !mAdjustingNewLook )
		{
			mAdjustingNewLook = true;

			// Whenever we move, keep our eyes on the look target if there is one.
			std::string lookAtName = fGetEditableProperties( ).fGetValue<std::string>( fEditablePropLookAt(), "" );
			if( lookAtName != "" )
			{
				tGrowableArray<tEditableObject*> objs;
				mContainer.fCollectByName( objs, lookAtName );

				if( objs.fCount() >= 1 )
				{
					tEditableObject* lookAtEntity = objs[0];

					Math::tMat3f newPos = fObjectToWorld();
					const Math::tVec3f newLook = ( lookAtEntity->fObjectToWorld().fGetTranslation() - newPos.fGetTranslation() ).fNormalize( );
					newPos.fOrientZAxis( newLook );
					fMoveTo( newPos );
				}
			}
		}

		tEditableObject::fOnMoved( recomputeParentRelative );

		mAdjustingNewLook = false;
	}

	f32 tEditableCameraEntity::fFOV( ) const
	{
		const f32 fov = mEditableProperties.fGetValue<f32>( fEditablePropFOV( ), 1.f ); //this is not the "default" value. and the fall back should never be needed.

		// UI fov is entire range so split it in half.
		return Math::fToRadians( fov * 0.5f );
	}

	std::string tEditableCameraEntity::fDisplayName( ) const
	{
		// default to property name
		std::string name = fGetEditableProperties( ).fGetValue<std::string>( Sigml::tObjectProperties::fEditablePropObjectName( ), "" );
		if( name == "" )
		{
			// if we dont have one, generate a hex identifier.
			std::stringstream ss;
			ss << std::hex << (u32)this;
			name = ss.str( );
		}

		return name;
	}

	void tEditableCameraEntity::fExtractCameraData( Gfx::tCamera& camera ) const
	{
		Gfx::tLens lens = fBuildLens( );
		camera.fSetTripodAndLens( Gfx::tTripod( fObjectToWorld() ), lens );
	}

	void tEditableCameraEntity::fDriveCamera( const Gfx::tCamera& newPositionData )
	{
		// Move this entity to the same spot as the supplied camera data.
		// Does not give a hell about the lens data.
		Math::tMat3f xform;
		newPositionData.fGetTripod( ).fConstructWorldMatrix( xform );
		fMoveTo( xform );
	}

	std::string tEditableCameraEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Camera - " + name;
		return "Camera";
	}

	void tEditableCameraEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );

		if( &entity == mDummyFrustum.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummyFrustum->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummyFrustum->fSetRgbaTint( cDummyTint );
		}
	}

	Sigml::tObjectPtr tEditableCameraEntity::fSerialize( b32 clone ) const
	{
		Sigml::tCameraObject* ao = new Sigml::tCameraObject( this );
		fSerializeBaseObject( ao, clone );
		return Sigml::tObjectPtr( ao );
	}

	void tEditableCameraEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		//if( property.fGetName( ) == fEditablePropFOV( ) )
		{
			fRebuildVisualization( );
		}
	}

}

