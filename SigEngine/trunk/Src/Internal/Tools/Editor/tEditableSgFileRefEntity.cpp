#include "ToolsPch.hpp"
#include "tEditableSgFileRefEntity.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditablePropertyTypes.hpp"
#include "tSceneGraphFile.hpp"
#include "tMeshEntity.hpp"
#include "tAssetPluginDll.hpp"
#include "iSigEdPlugin.hpp"
#include "tMesh.hpp"

namespace Sig
{

	static const Math::tVec4f gMediumSphereTint = Math::tVec4f( 0.5f, 0.5f, 1.0f, 1.0f );
	static const Math::tVec4f gFarSphereTint = Math::tVec4f( 0.0f, 0.0f, 0.8f, 1.0f );

	namespace
	{
		static const char* fEditablePropStateToDisplay( ) { return "Display.StateIndex"; }

		//Deprecated properties, will get removed from resources when they are resaved.
		static const char* fEditablePropDisableContextAnims( ) { return "Disable.ContextAnims"; }
	}

	tEditableSgFileRefEntity::tEditableSgFileRefEntity( tEditableObjectContainer& container, const tResourceId& resourceId, const tResourcePtr& sgResource )
		: tEditableObject( container )
		, mLastDisplayIndex( -1 )
	{
		fAddEditableProperties( );

		fCommonCtor( resourceId, sgResource );
	}

	tEditableSgFileRefEntity::tEditableSgFileRefEntity( tEditableObjectContainer& container, const Sigml::tSigmlReferenceObject& sigmlObject )
		: tEditableObject( container )
		, mLastDisplayIndex( -1 )
	{
		fAddEditableProperties( );

		fDeserializeBaseObject( &sigmlObject );
		mEditableProperties.fRemove( "Render.AutoFadeOut" ); // deprecated

		tResourcePtr sgResource = tEditableObject::fGetContainer( ).fGetResourceDepot( )->fQuery( sigmlObject.fConstructResourceId( mSgId ) );

		fCommonCtor( mSgId, sgResource );
	}

	void tEditableSgFileRefEntity::fAddEditableProperties( )
	{
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCustomString( Sigml::tObject::fEditablePropBoneAttachment( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropBoneRelativeAttachment( ), false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropInvisible( ) ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropCastShadow( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropReceiveShadow( ), true ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyFloat( fEditablePropStateToDisplay( ), 0, -1, 254, 1, 0 ) ) );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropPhysicsIgnoreChildCollision( ), false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropPhysicsCreateStatic( ), false ) ) );
		
		Sigml::tObjectProperties::fAddFadeSettingsEditableProperties( mEditableProperties );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCDCFloat( Sigml::tObjectProperties::fEditablePropLODMediumDistanceName( ), 0.f, 0.f, 10000.f, 1.f, 0 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyCDCFloat( Sigml::tObjectProperties::fEditablePropLODFarDistanceName( ), 0.f, 0.f, 10000.f, 1.f, 0 ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObjectProperties::fEditablePropShowLODFromCameraName( ), false ) ) );

		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropShowRaycastMeshName(), false ) ) );
		mEditableProperties.fInsert( tEditablePropertyPtr( new tEditablePropertyBool( Sigml::tObject::fEditablePropFreezeRaycastMeshName(), false ) ) );
	}

	void tEditableSgFileRefEntity::fCommonCtor( const tResourceId& resourceId, const tResourcePtr& sgResource )
	{
		if( mSgFileRefEntity )
		{
			mSgFileRefEntity->fDeleteImmediate( );
			mSgFileRefEntity.fRelease( );
		}

		// initialize sigb renderables
		mLastDisplayIndex = -1;

		// until the object is loaded, set the dummy object's bounds as our own
		const Math::tAabbf localSpaceBox = mContainer.fGetDummyBoxTemplate( ).fGetBounds( );
		fSetLocalSpaceMinMax( localSpaceBox.mMin, localSpaceBox.mMax );

		mSgResource = sgResource;

		// These entities can now hold a null resource handle and they will appear as red
		// boxes if they have a null resource.
		if( mSgResource )
		{
			mToolTip = Sigml::fSigbPathToSigml( mSgResource->fGetPath() ).fCStr( );

			// Set the dummy visible and green first so the OnLoaded will set it invisible if it is.
			mDummyBox->fSetRgbaTint( Math::tVec4f( 0.1f, 1.0f, 0.2f, 0.5f ) );
			mDummyBox->fSetInvisible( false );

			mOnResourceLoaded.fFromMethod< tEditableSgFileRefEntity, &tEditableSgFileRefEntity::fOnResourceLoaded >( this );
			mSgFileRefEntity.fReset( new tSgFileRefEntity( sgResource ) );
			mSgResource->fCallWhenLoaded( mOnResourceLoaded );
			mSgId = mSgResource->fGetResourceId( );

			if( resourceId != mSgResource->fGetResourceId() )
				log_warning( "Programmer warning: you've passed in a different resource ID than the resource you think you are using." );
		}
		else
		{
			// We were handed a null resource. Red box it and carry on with life.
			mToolTip = "unbuilt resource - " + std::string( mSgId.fGetPath().fCStr() );
			mSgId = resourceId;

			mDummyBox->fSetInvisible( false );
			mDummyBox->fSetRgbaTint( Math::tVec4f( 1.0f, 0.0f, 0.0f, 0.75f ) );
		}

		//remove deprecated properties
		mEditableProperties.fRemove( fEditablePropDisableContextAnims( ) );

		// we didnt know if this was a mesh or sigml until now
		if( fIsSigml( ) )
			mEditableProperties.fRemove( Sigml::tObject::fEditablePropPhysicsCreateStatic( ) );
		else
			mEditableProperties.fRemove( Sigml::tObject::fEditablePropPhysicsIgnoreChildCollision( ) );


		// Set up Shells
		const f32 medium = fMediumDistance();
		const f32 farDist = fFarDistance( );

		// MEDIUM SHELL
		mMediumDistSphere.fReset( new Gfx::tRenderableEntity( 
			mContainer.fGetSphereCageSparseTemplate( ).fGetRenderBatch( ), 
			mContainer.fGetDummyBoxTemplate( ).fGetBounds( ) ) );
		mMediumDistSphere->fSetRgbaTint( gMediumSphereTint );
		Math::tMat3f mediumMat( Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, Math::cPiOver4 ) ) );
		mediumMat.fScaleLocal( medium );

		mMediumDistSphere->fSpawnImmediate( *this );
		mMediumDistSphere->fSetParentRelativeXform( mediumMat );

		// FAR SHELL
		mFarDistSphere.fReset( new Gfx::tRenderableEntity( 
			mContainer.fGetSphereCageSparseTemplate( ).fGetRenderBatch( ), 
			mContainer.fGetDummyBoxTemplate( ).fGetBounds( ) ) );
		mFarDistSphere->fSetRgbaTint( gFarSphereTint );

		mFarDistSphere->fSpawnImmediate( *this );
		mFarDistSphere->fSetParentRelativeXform( Math::tMat3f( medium ) );

		// People have to turn on the LOD shells if they want to look at them.
		mEditableProperties.fSetDataNoNotify( Sigml::tObjectProperties::fEditablePropShowLODFromCameraName(), false );
		mMediumDistSphere->fSetDisabled( true );
		mFarDistSphere->fSetDisabled( true );
	}

	namespace
	{
		static u32 fRenderableEntityCount( tEntity& root )
		{
			u32 o = 0;
			if( root.fDynamicCast< Gfx::tRenderableEntity >( ) )
				++o;
			for( u32 i = 0; i < root.fChildCount( ); ++i )
				o += fRenderableEntityCount( *root.fChild( i ) );
			return o;
		}
	}

	void tEditableSgFileRefEntity::fOnResourceLoaded( tResource& theResource, b32 success )
	{
		if( !mSgFileRefEntity )
		{
			return;
		}

		const tSceneGraphFile* sgFile = mSgFileRefEntity->fSgResource( )->fCast< tSceneGraphFile >( );
		if( success && sgFile )
		{
			mSgFileRefEntity->fSpawnImmediate( *this );

			if( fRenderableEntityCount( *mSgFileRefEntity ) > 0 )
			{
				fSetLocalSpaceMinMax( sgFile->mBounds.mMin, sgFile->mBounds.mMax );
				mDummyBox->fSetInvisible( true );
			}
			else
			{
				const Math::tAabbf objSpaceBox = mSgFileRefEntity->fCombinedObjectSpaceBox( );
				fSetLocalSpaceMinMax( objSpaceBox.mMin, objSpaceBox.mMax );
				mDummyBox->fSetParentRelativeXform( objSpaceBox.fAdjustMatrix( Math::tMat3f::cIdentity, 0.0f ) );
			}

			const b32 showRaycast = mEditableProperties.fGetValue<b32>( Sigml::tObjectProperties::fEditablePropShowRaycastMeshName(), false );
			if( showRaycast )
				fBuildRaycastVisualization();

			fUpdateStateTint( );
		}
		else
		{
			mSgFileRefEntity.fRelease( );
			mDummyBox->fSetInvisible( false );
			mDummyBox->fSetRgbaTint( Math::tVec4f( 1.0f, 0.0f, 0.0f, 0.75f ) );
		}

		const s32 displayIndex = fGetDisplayIndex( );
		fUpdateDisplayedStates( displayIndex );
		mLastDisplayIndex = displayIndex;

		fUpdateShadowCasting( );
		fUpdateFadeSettings( );
	}

	tEditableSgFileRefEntity::~tEditableSgFileRefEntity( )
	{
	}

	tFilePathPtr tEditableSgFileRefEntity::fResourcePath( ) const
	{
		if( mSgResource.fNull( ) )
			return tFilePathPtr( );
		return mSgResource->fGetPath( );
	}

	//------------------------------------------------------------------------------
	const tResourcePtr & tEditableSgFileRefEntity::fResource( ) const
	{
		return mSgResource;
	}

	b32 tEditableSgFileRefEntity::fIsSigml( ) const
	{
		const tFilePathPtr resPath = fResourcePath( );
		return StringUtil::fCheckExtension( resPath.fCStr( ), ".sigb" );
	}

	b32 tEditableSgFileRefEntity::fIsEditable( ) const
	{
		return fIsSigml( );
	}

	b32 tEditableSgFileRefEntity::fIsRedBox( ) const
	{
		return !mSgFileRefEntity;
	}

	void tEditableSgFileRefEntity::fResetReference( const tFilePathPtr& newRefPath )
	{
		tResourceId rid = tResourceId::fMake<tSceneGraphFile>( newRefPath );
		tResourcePtr sgResource = tEditableObject::fGetContainer( ).fGetResourceDepot( )->fQuery( rid );

		fCommonCtor( rid, sgResource );
	}

	void tEditableSgFileRefEntity::fRefreshDependents( const tResourcePtr& reloadedResource, b32 unloadOnly )
	{
		tResourcePtr sgResource = mSgResource;

		if( unloadOnly )
		{
			// Have to ensure the held resource is non null.
			if( mSgFileRefEntity && sgResource && reloadedResource )
			{
				tResource::tVisitedList visitedList0,visitedList1;
				if( sgResource == reloadedResource || 
					sgResource->fIsSubResource( *reloadedResource, visitedList0 ) || 
					reloadedResource->fIsSubResource( *sgResource, visitedList1 ) )
				{
					mSgFileRefEntity->fDeleteImmediate( );
					mSgFileRefEntity.fRelease( );
				}
			}
		}
		else if( !mSgFileRefEntity )
			fCommonCtor( mSgId, sgResource );
	}

	std::string tEditableSgFileRefEntity::fGetToolTip( ) const
	{
		if( fIsFrozen( ) )
			return std::string( );
		const std::string name = tEditableObject::fGetToolTip( );
		if( name.length( ) > 0 )
			return mToolTip + " - " + name;
		return mToolTip;
	}

	std::string tEditableSgFileRefEntity::fGetAssetPath( ) const
	{
		return fResourcePath().fCStr();
	}

	void tEditableSgFileRefEntity::fOnDeviceLost( Gfx::tDevice* device )
	{
		tEditableObject::fOnDeviceLost( device );
	}

	void tEditableSgFileRefEntity::fOnDeviceReset( Gfx::tDevice* device )
	{
		tEditableObject::fOnDeviceReset( device );

		mMediumDistSphere->fSetRenderBatch( mContainer.fGetSphereCageSparseTemplate().fGetRenderBatch() );
		mFarDistSphere->fSetRenderBatch( mContainer.fGetSphereCageSparseTemplate().fGetRenderBatch() );
	}

	Sigml::tObjectPtr tEditableSgFileRefEntity::fSerialize( b32 clone ) const
	{
		Sigml::tSigmlReferenceObject* o = new Sigml::tSigmlReferenceObject( );
		fSerializeBaseObject( o, clone );
		if( mSgResource )
			o->mReferencePath = Sigml::fSigbPathToSigml( mSgResource->fGetPath() );
		else
			o->mReferencePath = Sigml::fSigbPathToSigml( mSgId.fGetPath() );
		return Sigml::tObjectPtr( o );
	}

	void tEditableSgFileRefEntity::fNotifyPropertyChanged( tEditableProperty& property )
	{
		tEditableObject::fNotifyPropertyChanged( property );

		if( property.fGetName( ) == fEditablePropStateToDisplay( ) )
		{
			const u32 displayIndex = fGetDisplayIndex( );
			fUpdateDisplayedStates( displayIndex );
			mLastDisplayIndex = displayIndex;
		}
		// Check if any of the relevant properties have been messed with.
		else if( property.fGetName() == Sigml::tObjectProperties::fEditablePropLODMediumDistanceName()
			|| property.fGetName() == Sigml::tObjectProperties::fEditablePropLODFarDistanceName()
			|| property.fGetName() == Sigml::tObjectProperties::fEditablePropShowLODFromCameraName() )
		{
			const b32 timeToDie = mEditableProperties.fGetValue<b32>( Sigml::tObjectProperties::fEditablePropShowLODFromCameraName(), false );

			// Check if the camera-view is enabled.
			if( timeToDie )
			{
				const f32 medium = mEditableProperties.fGetValue<f32>( Sigml::tObjectProperties::fEditablePropLODMediumDistanceName(), 0.f );
				const f32 farDistance = mEditableProperties.fGetValue<f32>( Sigml::tObjectProperties::fEditablePropLODFarDistanceName(), 0.f );

				// Medium sphere
				Math::tMat3f mediumMat( Math::tQuatf( Math::tAxisAnglef( Math::tVec3f::cYAxis, Math::cPiOver4 ) ) );
				mediumMat.fScaleLocal( medium );
				mMediumDistSphere->fSetParentRelativeXform( mediumMat );

				// Far sphere
				mFarDistSphere->fSetParentRelativeXform( Math::tMat3f( farDistance ) );

				Gfx::tRenderableEntity::fSetLODDists( *mSgFileRefEntity, medium, farDistance );
				mMediumDistSphere->fSetDisabled( false );
				mFarDistSphere->fSetDisabled( false );

				mMediumDistSphere->fSetRgbaTint( gMediumSphereTint );
				mFarDistSphere->fSetRgbaTint( gFarSphereTint );
			}
			// Otherwise reset them to zero/regular behavior.
			else
			{
				Gfx::tRenderableEntity::fSetLODDists( *mSgFileRefEntity, 0.f, 0.f );
				mMediumDistSphere->fSetDisabled( true );
				mFarDistSphere->fSetDisabled( true );
			}
		}
		else if( property.fGetName() == Sigml::tObjectProperties::fEditablePropShowRaycastMeshName() )
		{
			const b32 timeToLive = mEditableProperties.fGetValue<b32>( Sigml::tObjectProperties::fEditablePropShowRaycastMeshName(), false );

			if( timeToLive )
			{
				if( mRaycastSubmeshes.fCount() > 0 )
					fSetRaycastVisOn( true );
				else
					fBuildRaycastVisualization();
			}
			else
			{
				// Wake up. Time to die.
				fSetRaycastVisOn( false );
			}
		}
		else if( property.fGetName() == Sigml::tObjectProperties::fEditablePropFreezeRaycastMeshName() )
		{
			const b32 timeToLive = mEditableProperties.fGetValue<b32>( Sigml::tObjectProperties::fEditablePropShowRaycastMeshName(), false );
			if( !timeToLive )
				return;

			const b32 frozen = mEditableProperties.fGetValue<b32>( Sigml::tObjectProperties::fEditablePropFreezeRaycastMeshName(), false );
			fSetFreezeRaycastVis( frozen );
		}
	}

	s32 tEditableSgFileRefEntity::fGetDisplayIndex( ) const
	{
		return mEditableProperties.fGetValue<s32>( fEditablePropStateToDisplay( ), 0 );
	}

	void tEditableSgFileRefEntity::fUpdateDisplayedStates( u32 displayIndex )
	{
		tMeshEntity::tChangeMeshState change( mLastDisplayIndex, displayIndex );
		change.fChangeState( *this );
	}

	std::string tEditableSgFileRefEntity::fGetSoftName( ) const
	{
		std::string name = tEditableObject::fGetName( );
		if( name.length( ) )
			return name;
		
		name = StringUtil::fNameFromPath( fResourcePath( ).fCStr( ) );
		if( !name.length( ) )
			name = "Sigml or Mshml";

		return name;
	}

	f32 tEditableSgFileRefEntity::fMediumDistance( ) const
	{
		return mEditableProperties.fGetValue( Sigml::tObjectProperties::fEditablePropLODMediumDistanceName(), 0.f );
	}

	f32 tEditableSgFileRefEntity::fFarDistance( ) const
	{
		return mEditableProperties.fGetValue( Sigml::tObjectProperties::fEditablePropLODFarDistanceName(), 0.f );
	}

	b32 tEditableSgFileRefEntity::fShowLODInEditor( ) const
	{
		return mEditableProperties.fGetValue( Sigml::tObjectProperties::fEditablePropShowLODFromCameraName(), false );
	}

	void tEditableSgFileRefEntity::fBuildRaycastVisualization( )
	{
		// DRAW A THING
		tGrowableArray< tMeshEntity* > submeshes;
		mSgFileRefEntity->fAllDescendentsOfType( submeshes );

		mRaycastSubmeshes.fSetCount( submeshes.fCount() );

		const b32 frozen = mEditableProperties.fGetValue<b32>( Sigml::tObjectProperties::fEditablePropFreezeRaycastMeshName(), false );

		for( u32 i = 0; i < mRaycastSubmeshes.fCount(); ++i )
		{
			mRaycastSubmeshes[i].fReset( new Gfx::tWorldSpaceLines( ) );
			mRaycastSubmeshes[i]->fResetDeviceObjects(
				Gfx::tDevice::fGetDefaultDevice( ), 
				mContainer.fGetSolidColorMaterial( ), 
				mContainer.fGetSolidColorGeometryAllocator( ), 
				mContainer.fGetSolidColorIndexAllocator( ) );
			mRaycastSubmeshes[i]->fSetLockedToParent( !frozen );
			mRaycastSubmeshes[i]->fSpawnImmediate( *this );

			tGrowableArray<Gfx::tSolidColorRenderVertex> wireframeTris;

			const tPolySoupTriangleList& tris = submeshes[i]->fSubMesh()->mTriangles;
			const tPolySoupVertexList& verts = submeshes[i]->fSubMesh()->mVertices;

			const u32 outgroup = Gfx::tVertexColor( 1.f, 0.f, 0.f, 1.f ).fForGpu( );

			for( u32 tri = 0; tri < tris.fCount(); ++tri )
			{
				Math::tVec3u thisTri = tris[tri];
				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( verts[ thisTri.x ], outgroup ) );
				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( verts[ thisTri.y ], outgroup ) );

				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( verts[ thisTri.y ], outgroup ) );
				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( verts[ thisTri.z ], outgroup ) );

				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( verts[ thisTri.z ], outgroup ) );
				wireframeTris.fPushBack( Gfx::tSolidColorRenderVertex( verts[ thisTri.x ], outgroup ) );
			}

			mRaycastSubmeshes[i]->fSetGeometry( wireframeTris, false );
			mRaycastSubmeshes[i]->fSetObjectSpaceBox( mRaycastSubmeshes[i]->fObjectSpaceBox() );
		}
	}

	void tEditableSgFileRefEntity::fSetRaycastVisOn( b32 visOn )
	{
		for( u32 i = 0; i < mRaycastSubmeshes.fCount(); ++i )
		{
			if( mRaycastSubmeshes[i].fNull() )
				continue;

			mRaycastSubmeshes[i]->fSetDisabled( !visOn );
		}
	}

	void tEditableSgFileRefEntity::fSetFreezeRaycastVis( b32 freeze )
	{
		for( u32 i = 0; i < mRaycastSubmeshes.fCount(); ++i )
		{
			if( mRaycastSubmeshes[i].fNull() )
				continue;

			mRaycastSubmeshes[i]->fSetLockedToParent( !freeze );

			if( !freeze )
				mRaycastSubmeshes[i]->fMoveTo( this->fObjectToWorld() );
		}
	}
}

