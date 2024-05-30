#include "ToolsPch.hpp"
#include "tEditableShapeEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tShapeEntity.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSigmlConverter.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tWxSlapOnListBox.hpp"
#include "tWxSlapOnButton.hpp"
#include "tEditableSgFileRefEntity.hpp"
#include "tMeshEntity.hpp"
#include "tMesh.hpp"

namespace Sig { namespace
{
	const f32 gDefaultRadius = 1.0f; // can never change this without breaking all existing dummy objects
	static const char* fEditablePropStateMask( ) { return "States.StateMask"; }
	static const char* fEditablePropShapeType( ) { return "Shape.VolumeType"; }
	static const char* fEditablePropHullProps( ) { return "Shape.ZHullOptions"; } //the z sorts it at the end.
	static const u32   fStateMaskInitialValue( ) { return ~0; }
	static const char* fEditablePropCreateStaticPhysics( ) { return "Physics.CreateStatic"; }

	static const char* fEditablePropVisiblitySet( ) { return "Potential Visibility.Set Name"; }
	static const char* fEditablePropVisiblitySetInvert( ) { return "Potential Visibility.Invert"; }

	// These will be removed when loaded. It will silently integrate itself into the file if the user chooses to save.
	static const char* gDeprecatedProperties[ ] = { "States.StateIndex" };

	void fFillDefaultHull( Math::tConvexHull& hull )
	{
		tGrowableArray<Math::tVec3f> points;
		points.fPushBack( Math::tVec3f( 0,0,1 ) );
		points.fPushBack( Math::tVec3f( -1,0,-1 ) );
		points.fPushBack( Math::tVec3f( 1,0,-1 ) );
		points.fPushBack( Math::tVec3f( 0,1,0 ) );
		hull.fConstruct( points, Math::tMat3f::cIdentity );
	}

	void fHullToGeometry( const Math::tConvexHull& hull, Gfx::tDynamicGeometry& geo, Gfx::tMaterial& mat )
	{
		u32 tris = hull.fFaces( ).fCount( );
		u32 indices = tris * 3;
		u32 verts = hull.fVerts( ).fCount( );
		const u32 color = 0xffffffff;

		tDynamicArray<u16> sysMemIndices( indices );
		tDynamicArray<Gfx::tSolidColorRenderVertex> sysMemVerts( verts );

		for( u32 i = 0; i < verts; ++i )
			sysMemVerts[ i ] = Gfx::tSolidColorRenderVertex( hull.fVerts( )[ i ], color );

		for( u32 i = 0; i < tris; ++i )
		{
			const Math::tConvexHull::tFace& face = hull.fFaces( )[ i ];
			sysMemIndices[ i*3 + 0 ] = face.mA;
			sysMemIndices[ i*3 + 1 ] = face.mB;
			sysMemIndices[ i*3 + 2 ] = face.mC;
		}

		geo.fAllocateGeometry( mat, verts, indices, tris );
		geo.fCopyVertsToGpu( sysMemVerts.fBegin( ), sysMemVerts.fCount( ) );
		geo.fCopyIndicesToGpu( sysMemIndices.fBegin( ), sysMemIndices.fCount( ) );
	}

	class tEditablePropertyConvexHull : public tEditablePropertyString
	{
		implement_rtti_serializable_base_class( tEditablePropertyConvexHull, 0xA0A834E7 );
	public:
		tEditablePropertyConvexHull( )
			: tEditablePropertyString( fEditablePropHullProps( ) )
			, mGenerate( true )
		{ }

		tEditablePropertyConvexHull( const std::string& rawData )
			: tEditablePropertyString( fEditablePropHullProps( ), rawData )
			, mGenerate( true )
		{ }

		virtual tEditableProperty* fClone( ) const
		{
			tEditablePropertyString* o = new tEditablePropertyConvexHull( mRawData );
			return o;
		}

		virtual void fCreateGui( tCreateGuiData& data ) 
		{ 
			tWxSlapOnButton* genButton = new tWxSlapOnButton( data.mParent, "Generate Hull", " " );
			genButton->fButton( )->Connect( wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( tEditablePropertyConvexHull::fKickGenerate ), NULL, this );
		}

		void fKickGenerate( wxCommandEvent& )
		{
			mGenerate = true;
			fSetData<std::string>( std::string( fRawData( ) ) );
		}

		b32 fGenerate( ) const { return mGenerate; }
		void fSetGenerate( b32 gen ) { mGenerate = gen; }

	private:
		b32 mGenerate;
	};

	register_rtti_factory( tEditablePropertyConvexHull, false )

}}

namespace Sig { namespace Sigml
{
	class tools_export tShapeObject : public tObject
	{
		declare_null_reflector();
		implement_rtti_serializable_base_class( tShapeObject, 0x412A9E1E );
	public:
		tShapeObject( );
		tShapeObject( const tEditableShapeEntity* ao );
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );

		Math::tConvexHull mHull;
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tShapeObject& o )
	{
		s( "Hull", o.mHull );
	}

	register_rtti_factory( tShapeObject, false );

	tShapeObject::tShapeObject( )
	{
		fFillDefaultHull( mHull );
	}

	tShapeObject::tShapeObject( const tEditableShapeEntity* ao )
	{
		mHull = ao->mHull;
	}

	void tShapeObject::fSerialize( tXmlSerializer& s )
	{
		fSerializeXmlObject( s, *this );
	}
	void tShapeObject::fSerialize( tXmlDeserializer& s )
	{
		fSerializeXmlObject( s, *this );

		for( u32 i = 0; i < array_length( gDeprecatedProperties ); ++i )
			mEditableProperties.fRemove( gDeprecatedProperties[ i ] );
	}

	tEntityDef* tShapeObject::fCreateEntityDef( tSigmlConverter& sigmlConverter )
	{
		tShapeEntityDef* entityDef = new tShapeEntityDef( );
		fConvertEntityDefBase( entityDef, sigmlConverter );

		const u32 stateMask = mEditableProperties.fGetValue( fEditablePropStateMask( ), fStateMaskInitialValue( ) );
		entityDef->mStateMask = stateMask & tStateableEntity::cMaxStateMaskValue;
		entityDef->mBounds = Math::tAabbf( Math::tVec3f( -gDefaultRadius ), Math::tVec3f( +gDefaultRadius ) );
		entityDef->mShapeType = ( tShapeEntityDef::tShapeType )mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
		
		if( entityDef->mShapeType == tShapeEntityDef::cShapeTypeConvexHull )
		{
			entityDef->mHull = NEW Math::tConvexHull( );
			*entityDef->mHull = mHull;
		}

		// create some visibility set information if user provided it.
		std::string visibleSetName = mEditableProperties.fGetValue( fEditablePropVisiblitySet( ), std::string( "" ) );
		if( visibleSetName.length( ) )
		{
			entityDef->mPotentialVisibility.fPushBack( tShapeEntityDef::tPVSInformation( sigmlConverter.fAddLoadInPlaceStringPtr( visibleSetName.c_str( ) ), mEditableProperties.fGetValue( fEditablePropVisiblitySetInvert( ), false ) ) );
		}

		return entityDef;
	}

	tEditableObject* tShapeObject::fCreateEditableObject( tEditableObjectContainer& container )
	{
		return new tEditableShapeEntity( container, *this );
	}

}}




namespace Sig
{
	namespace
	{
		static const Math::tVec4f cDummyTint = Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.5f );
	}


	tShapeDummyObjectEntity::tShapeDummyObjectEntity( const Gfx::tRenderBatchPtr& batchPtr, const Math::tAabbf& objectSpaceBox )
		: tEditableObject::tDummyObjectEntity( batchPtr, objectSpaceBox, true )
		, mConvexMode( false )
	{
	}

	void tShapeDummyObjectEntity::fRayCast( const Math::tRayf& ray, Math::tRayCastHit& hit ) const
	{
		if( mConvexMode )
		{
			tEditableObject* eo = fFirstAncestorOfType< tEditableObject >( );
			if( !eo ) return;
			const Math::tRayf rayInLocal = ray.fTransform( fWorldToObject( ) );

			const Math::tAabbf& localBox = fObjectSpaceBox( );
			localBox.fIntersectsWalls( rayInLocal, hit.mT );
		}
		else
			tEditableObject::tDummyObjectEntity::fRayCast( ray, hit );
	}

	b32	tShapeDummyObjectEntity::fIntersects( const Math::tFrustumf& v ) const
	{
		if( mConvexMode )
		{
			tEditableObject* eo = fFirstAncestorOfType< tEditableObject >( );
			if( !eo ) return false;

			return tRenderableEntity::fIntersects( v );
		}
		else
			return tEditableObject::tDummyObjectEntity::fIntersects( v );
	}

	tEditableShapeEntity::tEditableShapeEntity( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fCommonCtor( );
	}

	tEditableShapeEntity::tEditableShapeEntity( tEditableObjectContainer& container, const Sigml::tShapeObject& ao )
		: tEditableObject( container )
	{
		fAddEditableProperties( );
		fDeserializeBaseObject( &ao );
		mHull = ao.mHull;
		fCommonCtor( );
	}

	void tEditableShapeEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropBoneRelativeAttachment( ), false ) ) );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyMask( fEditablePropStateMask( ), fStateMaskInitialValue( ) ) ) );

		static_assert( tShapeEntityDef::cShapeTypeCount == 5 );
		tDynamicArray<std::string> shapeTypes( tShapeEntityDef::cShapeTypeCount );
		shapeTypes[ tShapeEntityDef::cShapeTypeBox		] = "Box";
		shapeTypes[ tShapeEntityDef::cShapeTypeSphere	] = "Sphere";
		shapeTypes[ tShapeEntityDef::cShapeTypeConvexHull	] = "Convex Hull";
		shapeTypes[ tShapeEntityDef::cShapeTypeCylinder	] = "Cylinder";
		shapeTypes[ tShapeEntityDef::cShapeTypeCapsule	] = "Capsule";
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyEnum( fEditablePropShapeType( ), shapeTypes, tShapeEntityDef::cShapeTypeBox ) ) );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyConvexHull( ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropCreateStaticPhysics( ), false ) ) );


		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyString( fEditablePropVisiblitySet( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( fEditablePropVisiblitySetInvert( ), false ) ) );		
	}

	void tEditableShapeEntity::fCommonCtor( )
	{
		mRenderState = Gfx::tRenderState::cDefaultColorTransparent;
		mRenderState.fEnableDisable( Gfx::tRenderState::cPolyTwoSided, true );

		const Math::tMat3f xform( gDefaultRadius );
		const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );

		// setup the box object
		mDummyBox->fSetRgbaTint( cDummyTint );
		mDummyBox->fSetParentRelativeXform( xform );
		if( shapeType == tShapeEntityDef::cShapeTypeBox )
			mDummyBox->fSetInvisible( false );

		// setup the sphere object
		mDummySphere->fSetRgbaTint( cDummyTint );
		mDummySphere->fSetParentRelativeXform( xform );
		if( shapeType == tShapeEntityDef::cShapeTypeSphere )
			mDummySphere->fSetInvisible( false );

		// setup the hull object
		Gfx::tDefaultAllocators& defAllocators = Gfx::tDefaultAllocators::fInstance( );
		const Math::tAabbf localSpaceSphereBox = mContainer.fGetDummySphereTemplate( ).fGetBounds( );
		mHullGeometry.fResetDeviceObjects( defAllocators.mSolidColorGeomAllocator, defAllocators.mIndexAllocator );
		mDummyHull.fReset( new tShapeDummyObjectEntity( mHullGeometry.fGetRenderBatch( ), localSpaceSphereBox ) );
		mDummyHull->fSpawnImmediate( *this );
		mDummyHull->fSetInvisible( true );
		mDummyHull->fSetRgbaTint( cDummyTint );
		if( shapeType == tShapeEntityDef::cShapeTypeConvexHull )
			mDummyHull->fSetInvisible( false );

		const Gfx::tDefaultAllocators& allocator = Gfx::tDefaultAllocators::fInstance( );
		Gfx::tSolidColorMaterial* mat = NEW Gfx::tSolidColorMaterial( );
		const Gfx::tDefaultAllocators& defaultAllocs = Gfx::tDefaultAllocators::fInstance( );
		mat->fSetMaterialFileResourcePtrOwned( allocator.mSolidColorMaterialFile );
		mHullMaterial.fReset( mat );

		fHullToGeometry( mHull, mHullGeometry, *mHullMaterial );
		mDummyHull->fSetRenderBatch( mHullGeometry.fGetRenderBatch( ) );

		// Other shapes
		mDummyCylinder.fReset( new tDummyObjectEntity( mContainer.fGetDummyCylinderTemplate( ).fGetRenderBatch( ), localSpaceSphereBox, false ) );
		mDummyCylinder->fSpawnImmediate( *this );
		mDummyCylinder->fSetInvisible( true );
		mDummyCylinder->fSetRgbaTint( cDummyTint );
		if( shapeType == tShapeEntityDef::cShapeTypeCylinder )
			mDummyCylinder->fSetInvisible( false );

		mCapsuleGeometry.fResetDeviceObjects( Gfx::tDevicePtr( mContainer.fDevice( ) ), defAllocators.mSolidColorMaterial, defAllocators.mSolidColorGeomAllocator, defAllocators.mIndexAllocator );
		mCapsuleGeometry.fGenerate( 1.f, 1.f );
		mDummyCapsule.fReset( new tShapeDummyObjectEntity( mCapsuleGeometry.fGetRenderBatch( ), localSpaceSphereBox ) );
		mDummyCapsule->fSpawnImmediate( *this );
		mDummyCapsule->fSetInvisible( true );
		mDummyCapsule->fSetRgbaTint( cDummyTint );
		if( shapeType == tShapeEntityDef::cShapeTypeCapsule )
			mDummyCapsule->fSetInvisible( false );

		mDummyCapsule->fSetRenderBatch( mCapsuleGeometry.fGetRenderBatch( ) );

		fSetObjectBounds( );
		tEditableObject::fUpdateStateTint( );
	}

	tEditableShapeEntity::~tEditableShapeEntity( )
	{
	}

	void tEditableShapeEntity::fSetObjectBounds( )
	{
		const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
		Math::tAabbf localSpaceBox;

		if( shapeType == tShapeEntityDef::cShapeTypeConvexHull )
		{
			localSpaceBox = mHull.fToAABB( );
			mDummyHull->mConvexMode = true;
		}
		else
		{
			// set the dummy object's bounds as our own
			localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
			localSpaceBox.mMin *= gDefaultRadius;
			localSpaceBox.mMax *= gDefaultRadius;
			mDummyHull->mConvexMode = false;
		}

		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );
		mDummyHull->fSetObjectSpaceBox( localSpaceBox );
	}

	void tEditableShapeEntity::fOnMoved( b32 recomputeParentRelative )
	{
		tEditableObject::fOnMoved( recomputeParentRelative );

		fGenerateCapsule( );
	}

	void tEditableShapeEntity::fOnDeviceLost( Gfx::tDevice* device )
	{
		tEditableObject::fOnDeviceLost( device );
	}

	void tEditableShapeEntity::fOnDeviceReset( Gfx::tDevice* device )
	{
		tEditableObject::fOnDeviceReset( device );

		mDummyCylinder->fSetRenderBatch( mContainer.fGetDummyCylinderTemplate().fGetRenderBatch() );
		fGenerateCapsule( );
	}

	std::string tEditableShapeEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) ) return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return "Shape - " + name;
		return "Shape";
	}

	b32 tEditableShapeEntity::fUniformScaleOnly( )
	{
		const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
		return( shapeType == tShapeEntityDef::cShapeTypeSphere || shapeType == tShapeEntityDef::cShapeTypeCylinder || shapeType == tShapeEntityDef::cShapeTypeCapsule );
	}

	b32 tEditableShapeEntity::fUniformScaleInXZ( )
	{
		const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
		return( shapeType == tShapeEntityDef::cShapeTypeCylinder || shapeType == tShapeEntityDef::cShapeTypeCapsule );
	}
	
	b32 tEditableShapeEntity::fSupportsScale( )
	{ 
		const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
		return( shapeType != tShapeEntityDef::cShapeTypeConvexHull );
	}


	void tEditableShapeEntity::fUpdateStateTint( tEntity& entity, const Math::tVec4f& rgbaTint )
	{
		tEditableObject::fUpdateStateTint( entity, rgbaTint );
		if( &entity == mDummyBox.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummyBox->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummyBox->fSetRgbaTint( cDummyTint );
		}
		if( &entity == mDummySphere.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummySphere->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummySphere->fSetRgbaTint( cDummyTint );
		}
		if( &entity == mDummyHull.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummyHull->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummyHull->fSetRgbaTint( cDummyTint );
		}
		if( &entity == mDummyCylinder.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummyCylinder->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummyCylinder->fSetRgbaTint( cDummyTint );
		}
		if( &entity == mDummyCapsule.fGetRawPtr( ) )
		{
			if( fIsFrozen( ) )
				mDummyCapsule->fSetRgbaTint( Math::tVec4f( rgbaTint.x, rgbaTint.y, rgbaTint.z, 0.25f ) );
			else
				mDummyCapsule->fSetRgbaTint( cDummyTint );
		}
	}

	Sigml::tObjectPtr tEditableShapeEntity::fSerialize( b32 clone ) const
	{
		Sigml::tShapeObject* ao = new Sigml::tShapeObject( this );
		fSerializeBaseObject( ao, clone );
		return Sigml::tObjectPtr( ao );
	}

	struct tVertCollector
	{
		mutable tGrowableArray<Math::tVec3f> mVerts;

		b32 operator ( ) ( const tEntity& e ) const
		{
			tMeshEntity* me = e.fDynamicCast< tMeshEntity >( );
			if( me && me->fEntityDef( ).fStateIndexSet( 0 ) && me->fEntityDef( ).mStateType != tMeshEntityDef::cStateTypeTransition )
			{
				u32 vCount = me->fSubMesh( )->mVertices.fCount( );
				mVerts.fSetCapacity( mVerts.fCount( ) + vCount );
				for( u32 i = 0; i < vCount; ++i )
					mVerts.fPushBack( e.fObjectToWorld( ).fXformPoint( me->fSubMesh( )->mVertices[ i ] ) );
			}

			return false;
		}
	};


	void tEditableShapeEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		if( property.fGetName( ) == fEditablePropShapeType( ) )
		{
			const u32 shapeType = mEditableProperties.fGetValue<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeBox );
			if( shapeType == tShapeEntityDef::cShapeTypeBox )
			{
				mDummyBox->fSetInvisible( false );
				mDummySphere->fSetInvisible( true );
				mDummyHull->fSetInvisible( true );
				mDummyCylinder->fSetInvisible( true );
				mDummyCapsule->fSetInvisible( true );
			}
			else if( shapeType == tShapeEntityDef::cShapeTypeSphere )
			{
				mDummyBox->fSetInvisible( true );
				mDummySphere->fSetInvisible( false );
				mDummyHull->fSetInvisible( true );
				mDummyCylinder->fSetInvisible( true );
				mDummyCapsule->fSetInvisible( true );
			}
			else if( shapeType == tShapeEntityDef::cShapeTypeConvexHull )
			{
				mDummyBox->fSetInvisible( true );
				mDummySphere->fSetInvisible( true );
				mDummyHull->fSetInvisible( false );
				mDummyCylinder->fSetInvisible( true );
				mDummyCapsule->fSetInvisible( true );
			}
			else if( shapeType == tShapeEntityDef::cShapeTypeCylinder )
			{
				mDummyBox->fSetInvisible( true );
				mDummySphere->fSetInvisible( true );
				mDummyHull->fSetInvisible( true );
				mDummyCylinder->fSetInvisible( false );
				mDummyCapsule->fSetInvisible( true );
			}
			else if( shapeType == tShapeEntityDef::cShapeTypeCapsule )
			{
				mDummyBox->fSetInvisible( true );
				mDummySphere->fSetInvisible( true );
				mDummyHull->fSetInvisible( true );
				mDummyCylinder->fSetInvisible( true );
				mDummyCapsule->fSetInvisible( false );
			}

			fSetObjectBounds( );
		}
		else if( property.fGetName( ) == fEditablePropHullProps( ) )
		{
			tEditablePropertyConvexHull& hullProps = static_cast<tEditablePropertyConvexHull&>( property );
			if( hullProps.fGenerate( ) )
			{
				if( mContainer.fGetPrevSelectionList( ).fCount( ) )
				{
					tVertCollector collector;

					const tEditorSelectionList& prevSelectedObjects = mContainer.fGetPrevSelectionList( );
					for( u32 i = 0; i < prevSelectedObjects.fCount( ); ++i )
					{
						tEditableSgFileRefEntity* mesh = dynamic_cast<tEditableSgFileRefEntity*>( prevSelectedObjects[ i ].fGetRawPtr( ) );
						if( mesh )
							mesh->fEntity( )->fForEachDescendent( collector );
					}

					if( collector.mVerts.fCount( ) )
					{
						// Find the average pt of the point cloud
						Math::tVec3f average = Math::tVec3f::cZeroVector;
						for( u32 i = 0; i < collector.mVerts.fCount( ); ++i )
							average += collector.mVerts[ i ];
						average /= (f32)collector.mVerts.fCount( );

						// move the shape there, may want to preserve orientation
						Math::tMat3f newShapePos = Math::tMat3f::cIdentity;
						newShapePos.fSetTranslation( average );
						fMoveTo( newShapePos );

						// generate hull relative to new location
						mHull.fConstruct( collector.mVerts, fWorldToObject( ) );
						fHullToGeometry( mHull, mHullGeometry, *mHullMaterial );
						mDummyHull->fSetRenderBatch( mHullGeometry.fGetRenderBatch( ) );

						// set the shape to convex if its not already
						mEditableProperties.fSetData<u32>( fEditablePropShapeType( ), tShapeEntityDef::cShapeTypeConvexHull );			

						log_line( 0, "Hull Faces: " << mHull.fFaces( ).fCount( ) );
					}
				}

				//hullProps.fSetGenerate( false );	
				fSetObjectBounds( );
			}
		}
	}

	void tEditableShapeEntity::fGenerateCapsule( )
	{
		if( mDummyCapsule )
		{
			Math::tVec3f scale = fObjectToWorld( ).fGetScale( );
			Math::tVec2f capsuleDims = Math::tCapsule::fDimsFromScale( scale );

			mCapsuleGeometry.fGenerate( capsuleDims.x, capsuleDims.y );
			mDummyCapsule->fSetRenderBatch( mCapsuleGeometry.fGetRenderBatch( ) );
			mDummyCapsule->fSetParentRelativeXform( Math::tMat3f( 1.f / scale ) );
		}
	}

	std::string tEditableShapeEntity::fGetSoftName( ) const
	{
		// If its named this take presidence
		std::string name = tEditableObject::fGetName( );
		if( name.length( ) )
			return name;

		// otherwise if we have a potentialy visibility set, show this.
		std::string visibilityName = mEditableProperties.fGetValue( fEditablePropVisiblitySet( ), std::string( "" ) );
		if( visibilityName.length( ) )
			return std::string( "PVS - " ) + visibilityName;

		// else just default to base behavior.
		return tEditableObject::fGetSoftName( );
	}

}

