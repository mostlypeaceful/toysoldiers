#include "ToolsPch.hpp"
#include "tEditableObjectContainer.hpp"
#include "tEditableTerrainEntity.hpp"
#include "FX/tParticleSystem.hpp"
#include "Math/tIntersectionAabbFrustum.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig
{
	namespace
	{
		struct tools_export tEditableObjectRayCastCallback : public tSceneGraph::tDefaultRayCastCallback
		{
			tEditableObjectRayCastCallback( ) { }
			tEditableObjectRayCastCallback( tEntity* const* ignoreList, u32 numToIgnore ) : tSceneGraph::tDefaultRayCastCallback( ignoreList, numToIgnore ) { }
			void operator()( const Math::tRayf& ray, const tEntityBVH::tObjectPtr& i ) const
			{
				if( i->fQuickRejectByFlags( ) )
					return;

				tSpatialEntity* spatial = fCullAgainstIgnoreList( i );

				if( !spatial || spatial->fIsHelper( ) )
					return;

				Gfx::tRenderableEntity* renderable = spatial->fDynamicCast< Gfx::tRenderableEntity >( );
				if( renderable && ( renderable->fInvisible( ) || renderable->fDisabled( ) ) )
					return;

				Math::tRayCastHit hit;

				FX::tParticleSystem* psys = spatial->fDynamicCast< FX::tParticleSystem >( );
				if( psys )
				{
					// perform ray-cast with unit box
					f32 t = Math::cInfinity;
					if( Math::tAabbf(-1.f,+1.f).fIntersectsWalls( ray.fTransform( spatial->fWorldToObject( ) ), t ) )
						hit = Math::tRayCastHit( t, Math::tVec3f::cYAxis );
				}
				else
				{
					if( i->fQuickRejectByBox( ray ) )
						return;
					spatial->fRayCast( ray, hit );
				}

				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
		};

		struct tools_export tEditableObjectIntersectionCallback : public tEntityBVH::tIntersectVolumeCallback<Math::tFrustumf>
		{
			tEditableObjectContainer::tEntityMasterList& mIntersection;

			tEditableObjectIntersectionCallback( tEditableObjectContainer::tEntityMasterList& intersection ) 
				: mIntersection( intersection ) { }

			inline tEditableObject* fCull( const Math::tFrustumf& v, const tEntityBVH::tObjectPtr& i, b32 aabbWhollyContained ) const
			{
				tSpatialEntity* spatial = static_cast< tSpatialEntity* >( i );

				if( spatial->fIsHelper( ) )							return 0;

				Gfx::tRenderableEntity* renderable = spatial->fDynamicCast< Gfx::tRenderableEntity >( );
				if( !renderable ) return 0;
				if( renderable->fInvisible( ) ) return 0;

				tEditableObject* o = spatial->fFirstAncestorOfType< tEditableObject >( );
				if( !o || o->fIsSelectionBoxEntity( *spatial ) )	return 0;
				if( aabbWhollyContained )							return o;
				if( !fQuickAabbTest( v, i, aabbWhollyContained ) )	return 0;

				FX::tParticleSystem* psys = spatial->fDynamicCast< FX::tParticleSystem >( );
				if( psys )
				{
					if( !Math::tIntersectionAabbFrustum<f32>( Math::tAabbf(-1.f,+1.f).fTransform(spatial->fObjectToWorld( )), v ).fIntersects( ) )
						return 0;
				}
				else
				{
					if( !spatial->fIntersects( v ) )				return 0;
				}

																	return o;
			}

			void operator()( const Math::tFrustumf& v, const tEntityBVH::tObjectPtr& i, b32 aabbWhollyContained ) const
			{
				tEditableObject* test = fCull( v, i, aabbWhollyContained );
				if( test && !mIntersection.fFind( test ) )
					mIntersection.fPushBack( tEntityPtr( test ) );
			}
		};
	}

	tEditableObjectContainer::tEditableObjectContainer( const tSceneGraphPtr& sg, const tResourceDepotPtr& resDepot )
		: mNextGuid( 1 )
		, mSceneGraph( sg )
		, mResourceDepot( resDepot )
		, mDummyRenderStateOverride( Gfx::tRenderState::cDefaultColorTransparent )
		, mOnObjectAdded( false )
		, mOnObjectRemoved( false )
	{
	}

	tEditableObjectContainer::~tEditableObjectContainer( )
	{
		for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
			for( u32 j = 0; j < mObjectSet[ i ].fCount( ); ++j )
				mObjectSet[ i ][ j ]->fDynamicCast< tEditableObject >( )->mInContainer = false;
	}

	void tEditableObjectContainer::fOnDeviceReset( Gfx::tDevice* device )
	{
		fGenerateSelectedBoxTemplate( );
		fGenerateSphereCageSparseTemplate( );
		fGenerateSphereCageDenseTemplate( );
		fGenerateDummyTemplates( );
	}

	void tEditableObjectContainer::fUpdateObjectsTick( )
	{
		for( u32 i = 0; i < mEntitiesNeedingUpdate.fCount( ); ++i )
		{
			if( mEntitiesNeedingUpdate[ i ].fNull( ) )
			{
				log_line( 0, "Warning: null entity." );
				continue;
			}

			tEditableObject* eo = mEntitiesNeedingUpdate[ i ]->fDynamicCast< tEditableObject >( );
			if( !eo )
			{
				log_line( 0, "Warning: non-editable object in update loop." );
				continue;
			}

			eo->fTickUpdate( );
		}

		mEntitiesNeedingUpdate.fSetCount( 0 );
	}

	void tEditableObjectContainer::fAddEntityToUpdate( tEntityPtr newEnt )
	{
		mEntitiesNeedingUpdate.fFindOrAdd( newEnt );
	}

	u32 tEditableObjectContainer::fNextGuid( )
	{
		return mNextGuid++;
	}

	u32 tEditableObjectContainer::fDeserializeGuid( u32 oldGuid )
	{
		if( oldGuid == 0 )
			return 0;
		mNextGuid = fMax( mNextGuid, oldGuid + 1 );
		return oldGuid;
	}

	void tEditableObjectContainer::fResetAllGuids( )
	{
		mNextGuid = 1;

		for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
		{
			const tEntityMasterList& list = mObjectSet[ i ];
			for( u32 j = 0; j < list.fCount( ); ++j )
			{
				tEditableObject* eo = list[ j ]->fDynamicCast< tEditableObject >( );
				eo->mGuid = fNextGuid( );
			}
		}
	}

	tEditableObject* tEditableObjectContainer::fFindObjectByGuid( u32 guid )
	{
		for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
		{
			tEntityMasterList& list = mObjectSet[ i ];
			for( u32 j = 0; j < list.fCount( ); ++j )
			{
				tEditableObject* eo = list[ j ]->fDynamicCast< tEditableObject >( );
				if( eo->fGuid( ) == guid )
					return eo;
			}
		}

		return 0;
	}

	tEntityPtr tEditableObjectContainer::fPick( const Math::tRayf& ray, f32* bestTout, tEntity* const* ignoreList, u32 numToIgnore ) const
	{
		tEditableObjectRayCastCallback rayCastCb( ignoreList, numToIgnore );

		return fPickInternal( rayCastCb, ray, bestTout, ignoreList, numToIgnore );
	}

	void tEditableObjectContainer::fIntersect( const Math::tFrustumf& frustum, tEntityMasterList& intersection ) const
	{
		mSceneGraph->fIntersect( frustum, tEditableObjectIntersectionCallback( intersection ) );
	}

	void tEditableObjectContainer::fRemoveAllFromWorld( )
	{
		for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
		{
			tEntityMasterList& list = mObjectSet[ i ];
			while( list.fCount( ) > 0 )
			{
				const u32 count = list.fCount( );
				list.fFront( )->fDynamicCast< tEditableObject >( )->fRemoveFromWorld( );
				sigassert( list.fCount( ) < count && "Object was not removed from world" );
			}
		}

		mNextGuid = 1;
	}

	Math::tAabbf tEditableObjectContainer::fComputeBounding( ) const
	{
		Math::tAabbf o;
		o.fInvalidate( );
		const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateShown ];
		for( u32 i = 0; i < shown.fCount( ); ++i )
		{
			tEditableObject* eo = shown[ i ]->fDynamicCast< tEditableObject >( );
			const Math::tAabbf worldBox = eo->fWorldSpaceBox( );
			if( !worldBox.fIsValid( ) )
				log_warning( 0, "Invalid fWorldSpaceBox found!" );
			else if( worldBox.mMin.fIsNan( ) || worldBox.mMax.fIsNan( ) )
				log_warning( 0, "NaN fWorldSpaceBox found!" );
			else
				o |= eo->fWorldSpaceBox( );
		}

		//Math::tAabbf o( Math::tVec3f( -100.f ), Math::tVec3f(100.f) );
		return o;
	}

	void tEditableObjectContainer::fReset( 
		const Gfx::tDevicePtr& device, 
		const Gfx::tMaterialPtr& solidColorMaterial, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& solidColorGeometry,
		const Gfx::tIndexBufferVRamAllocatorPtr& solidColorIndices )
	{
		mDevice = device;

		mSceneGraph->fDebugGeometry( ).fResetDeviceObjects( *fGetResourceDepot( ), mDevice );

		fRegisterWithDevice( device.fGetRawPtr( ) );

		// create geometry template objects
		mSelectedBoxTemplate.fResetDeviceObjects( device, solidColorMaterial, solidColorGeometry, solidColorIndices );
		mSphereCageSparseTemplate.fResetDeviceObjects( device, solidColorMaterial, solidColorGeometry, solidColorIndices );
		mSphereCageDenseTemplate.fResetDeviceObjects( device, solidColorMaterial, solidColorGeometry, solidColorIndices );
		mDummyBoxTemplate.fResetDeviceObjects( device, solidColorMaterial, solidColorGeometry, solidColorIndices );
		mDummySphereTemplate.fResetDeviceObjects( device, solidColorMaterial, solidColorGeometry, solidColorIndices );
		mDummyQuadTemplate.fResetDeviceObjects( device, solidColorMaterial, solidColorGeometry, solidColorIndices );
		fOnDeviceReset( device.fGetRawPtr( ) );
	}

	void tEditableObjectContainer::fOnSharedHeightFieldMapsModified( )
	{
		for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
		{
			tEntityMasterList& list = mObjectSet[ i ];
			for( u32 j = 0; j < list.fCount( ); ++j )
			{
				tEditableTerrainGeometry* etg = list[ j ]->fDynamicCast< tEditableTerrainGeometry >( );
				if( etg ) etg->fUpdateDynamicTextureReferences( );
			}
		}
	}

	void tEditableObjectContainer::fHideSelected( tEditorSelectionList& selection )
	{
		for( u32 i = 0; i < selection.fCount( ); ++i )
		{
			tEntityPtr master = selection[ i ];
			master->fDynamicCast< tEditableObject >( )->fHide( true );
		}
		selection.fClear( false );
	}

	void tEditableObjectContainer::fHideUnselected( tEditorSelectionList& selection )
	{
		const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateShown ];
		for( u32 i = 0; i < shown.fCount( ); ++i )
		{
			if( !selection.fContains( shown[ i ] ) )
			{
				tEntityPtr master = shown[ i ];
				master->fDynamicCast< tEditableObject >( )->fHide( true );
				--i;
			}
		}
	}

	void tEditableObjectContainer::fUnhideAll( )
	{
		const tEntityMasterList& hidden = mObjectSet[ tEditableObject::cStateHidden ];
		while( hidden.fCount( ) > 0 )
		{
			tEntityPtr master = hidden.fFront( );
			master->fDynamicCast< tEditableObject >( )->fHide( false );
		}
	}

	void tEditableObjectContainer::fFreezeSelected( tEditorSelectionList& selection )
	{
		for( u32 i = 0; i < selection.fCount( ); ++i )
		{
			tEntityPtr master = selection[ i ];
			master->fDynamicCast< tEditableObject >( )->fFreeze( true );
		}
		selection.fClear( false );
	}

	void tEditableObjectContainer::fFreezeUnselected( tEditorSelectionList& selection )
	{
		const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateShown ];
		for( u32 i = 0; i < shown.fCount( ); ++i )
		{
			if( !selection.fContains( shown[ i ] ) )
			{
				tEntityPtr master = shown[ i ];
				master->fDynamicCast< tEditableObject >( )->fFreeze( true );
				--i;
			}
		}
	}

	void tEditableObjectContainer::fUnfreezeAll( )
	{
		const tEntityMasterList& frozen = mObjectSet[ tEditableObject::cStateFrozen ];
		while( frozen.fCount( ) > 0 )
		{
			tEntityPtr master = frozen.fFront( );
			master->fDynamicCast< tEditableObject >( )->fFreeze( false );
		}
	}

	void tEditableObjectContainer::fGetShown( tEntityMasterList& shownEnts ) const
	{
		shownEnts = mObjectSet[ tEditableObject::cStateShown ];
	}

	void tEditableObjectContainer::fGetHidden( tEntityMasterList& hiddenEnts ) const
	{
		hiddenEnts = mObjectSet[ tEditableObject::cStateHidden ];
	}

	void tEditableObjectContainer::fGetFrozen( tEntityMasterList& hiddenEnts ) const
	{
		hiddenEnts = mObjectSet[ tEditableObject::cStateFrozen ];
	}

	void tEditableObjectContainer::fGetLayer( tGrowableArray< tEditableObject* >& output, const std::string& layer ) const
	{
		for( u32 istate = 0; istate < tEditableObject::cStateCount; ++istate )
		{
			const tEntityMasterList& objs = mObjectSet[ istate ];
			for( u32 i = 0; i < objs.fCount( ); ++i )
			{
				tEditableObject* o = objs[ i ]->fDynamicCast< tEditableObject >( );
				if( o && o->fLayer( ) == layer )
					output.fPushBack( o );
			}
		}
	}

	namespace
	{
		struct tSortObjectsByGuid
		{
			inline b32 operator()( const Sigml::tObjectPtr& a, const Sigml::tObjectPtr& b ) const
			{
				return a->mGuid < b->mGuid;
			}
		};
	}	

	namespace
	{
		void fSerializeImp( tEditableObject* eo, Sigml::tObjectPtrArray& objects, tEditableObjectContainer::tOnObjectSerialized* perObjectCb, tGrowableArray<u32>& existingGuids, tGrowableArray<u32>& dupGuids, u32& numObjectsSerialized, u32 totalObjectCount )
		{
			if( eo->fGuid( ) == 0 || existingGuids.fFind( eo->fGuid( ) ) )
				dupGuids.fFindOrAdd( eo->fGuid( ) );
			else
				existingGuids.fPushBack( eo->fGuid( ) );

			Sigml::tObjectPtr sigmlObject = eo->fSerialize( false );
			if( !sigmlObject.fNull( ) )
				objects.fPushBack( sigmlObject );
			if( perObjectCb )
				(*perObjectCb)( numObjectsSerialized++, totalObjectCount );
		}
	}

	void tEditableObjectContainer::fSerialize( Sigml::tObjectPtrArray& objects, tOnObjectSerialized* perObjectCb, b32 onlySelected )
	{
		tGrowableArray<u32> existingGuids;
		tGrowableArray<u32> dupGuids;

		const u32 startObjectCount = objects.fCount( );
		u32 numObjectsSerialized = 0;
		const u32 totalObjectCount = fGetObjectCount( );

		if( onlySelected )
		{
			const tEditorSelectionList& list = mSelectionList;
			for( u32 j = 0; j < list.fCount( ); ++j )
			{
				tEditableObject* eo = list[ j ]->fDynamicCast< tEditableObject >( );
				fSerializeImp( eo, objects, perObjectCb, existingGuids, dupGuids, numObjectsSerialized, totalObjectCount );
			}
		}
		else
		{
			for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
			{
				const tEntityMasterList& list = mObjectSet[ i ];
				for( u32 j = 0; j < list.fCount( ); ++j )
				{
					tEditableObject* eo = list[ j ]->fDynamicCast< tEditableObject >( );
					fSerializeImp( eo, objects, perObjectCb, existingGuids, dupGuids, numObjectsSerialized, totalObjectCount );
				}
			}
		}

		if( dupGuids.fCount( ) > 0 )
		{
			log_warning( 0, "Duplicate and/or invalid GUIDs were detected. Correcting, but could introduce problems for existing paths." );

			fResetAllGuids( );
			objects.fSetCount( startObjectCount );
			fSerialize( objects, 0, onlySelected );
		}
		else
		{
			// sort serialized objects by GUID
			std::sort( objects.fBegin( ), objects.fEnd( ), tSortObjectsByGuid( ) );
		}
	}

	b32 tEditableObjectContainer::fDeserialize( const Sigml::tObjectPtrArray& objects, tOnObjectSerialized* perObjectCb )
	{
		mNextGuid = 1;

		for( u32 i = 0; i < objects.fCount( ); ++i )
		{
			tEditableObject* eo = 0;
			if( objects[ i ] )
				eo = objects[ i ]->fCreateEditableObject( *this );
			if( eo )
				eo->fAddToWorld( );
			if( perObjectCb )
				(*perObjectCb)( i, objects.fCount( ) );
		}

		for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
		{
			const tEntityMasterList& list = mObjectSet[ i ];
			for( u32 j = 0; j < list.fCount( ); ++j )
			{
				tEditableObject* eo = list[ j ]->fDynamicCast< tEditableObject >( );
				eo->fAfterAllObjectsDeserialized( );
			}
		}


		return true;
	}

	void tEditableObjectContainer::fImportObjects( const Sigml::tObjectPtrArray& objects, tOnObjectSerialized* perObjectCb )
	{
		tGrowableArray< tEditableObject* > newObjs;

		// When importing, only do the base deserialization. GUID fix up steps are necessary
		// to keep things consistent.
		tHashTable<u32,u32> guidMap( objects.fCount( ) );
		for( u32 i = 0; i < objects.fCount( ); ++i )
		{
			tEditableObject* eo = 0;
			if( objects[ i ] )
				eo = objects[ i ]->fCreateEditableObject( *this );
			if( eo )
			{
				eo->fAddToWorld( );
				newObjs.fPushBack( eo );

				// Record old GUID and place new GUID.
				const u32 newGuid = fNextGuid( );
				guidMap[ eo->fGuid( ) ] = newGuid;
				eo->fSetGuid( newGuid );
			}
			if( perObjectCb )
				(*perObjectCb)( i, objects.fCount( ) );
		}

		// Fix up GUIDs.
		for( u32 i = 0; i < newObjs.fCount( ); ++i )
			newObjs[ i ]->fFixUpGuidRefs( guidMap );

		// After all deserialized.
		mSelectionList.fClear( );
		for( u32 i = 0; i < newObjs.fCount( ); ++i )
		{
			tEditableObject* eo = newObjs[ i ]->fDynamicCast< tEditableObject >( );
			eo->fAfterAllObjectsDeserialized( );
			mSelectionList.fAdd( tEntityPtr( newObjs[ i ] ) );
		}

	}

	void tEditableObjectContainer::fSetLayers( const Sigml::tLayerList& layers )
	{
		mLayerColors = tLayerColorTable( );
		for( u32 i = 0; i < layers.fCount( ); ++i )
		{
			if( i == 0 )
				mLayerColors[""] = layers[ i ].fColorRgba( );
			else
				mLayerColors[layers[ i ].fName( )] = layers[ i ].fColorRgba( );
		}
	}

	void tEditableObjectContainer::fMergeLayerColorEntries( const Sigml::tLayerList& layers )
	{
		for( u32 i = 0; i < layers.fCount( ); ++i )
		{
			Math::tVec4f* foundLayer = mLayerColors.fFind( layers[ i ].fName( ) );
			if( !foundLayer )
				mLayerColors[layers[ i ].fName( )] = layers[ i ].fColorRgba( );
		}
	}

	u32 tEditableObjectContainer::fGetObjectCount( ) const
	{
		u32 total = 0;
		for( u32 i = 0; i < mObjectSet.fCount( ); ++i )
			total += mObjectSet[ i ].fCount( );
		return total;
	}

	u32 tEditableObjectContainer::fGetShownCount( ) const
	{
		return mObjectSet[ tEditableObject::cStateShown ].fCount( );
	}
	
	u32 tEditableObjectContainer::fGetHiddenCount( ) const
	{
		return mObjectSet[ tEditableObject::cStateHidden ].fCount( );
	}

	u32 tEditableObjectContainer::fGetFrozenCount( ) const
	{
		return mObjectSet[ tEditableObject::cStateFrozen ].fCount( );
	}

	void tEditableObjectContainer::fGenerateSelectedBoxTemplate( )
	{
		const u32 numCorners = 8;
		const u32 numLines = 3 * numCorners;
		tDynamicArray< Gfx::tSolidColorRenderVertex > lineVerts( 2 * numLines );

		const f32 lineLen = 0.25f;
		const u32 color = Gfx::tVertexColor( 0xff, 0xff, 0xff, 0xff ).fForGpu( );

		// get the color assigning out of the way
		for( u32 i = 0; i < lineVerts.fCount( ); ++i )
			lineVerts[ i ].mColor = color;

		// now create the line geometry

		for( u32 i = 0; i < numCorners; ++i )
		{
			const u32 vtxBase = i * 3 * 2;
			const Math::tVec3f corner = Math::tVec3f( (i&1) ? 1.f : -1.f, (i&2) ? 1.f : -1.f, (i&4) ? 1.f : -1.f );
			lineVerts[ vtxBase + 0 ].mP = corner;
			lineVerts[ vtxBase + 1 ].mP = corner + Math::tVec3f( -lineLen * corner.x, 0.f, 0.f );
			lineVerts[ vtxBase + 2 ].mP = corner;
			lineVerts[ vtxBase + 3 ].mP = corner + Math::tVec3f( 0.f, -lineLen * corner.y, 0.f );
			lineVerts[ vtxBase + 4 ].mP = corner;
			lineVerts[ vtxBase + 5 ].mP = corner + Math::tVec3f( 0.f, 0.f, -lineLen * corner.z );
		}

		mSelectedBoxTemplate.fBake( ( Sig::byte* )lineVerts.fBegin( ), lineVerts.fCount( ), false );
	}

	void tEditableObjectContainer::fGenerateSphereCageSparseTemplate( )
	{
		const u32 numCircles = 3;
		const u32 numSegsPerCircle = 32;
		const u32 numLines = 2;
		tDynamicArray< Gfx::tSolidColorRenderVertex > lineVerts( 2 * ( numCircles * numSegsPerCircle + numLines ) );

		const u32 color = Gfx::tVertexColor( 0xff, 0xff, 0xff, 0xff ).fForGpu( );

		// get the color assigning out of the way
		for( u32 i = 0; i < lineVerts.fCount( ); ++i )
			lineVerts[ i ].mColor = color;

		// now create the geometry
		u32 iVtx = 0;

		lineVerts[ iVtx++ ].mP = Math::tVec3f( +1.f, 0.f, 0.f );
		lineVerts[ iVtx++ ].mP = Math::tVec3f( -1.f, 0.f, 0.f );
		lineVerts[ iVtx++ ].mP = Math::tVec3f( 0.f, 0.f, +1.f );
		lineVerts[ iVtx++ ].mP = Math::tVec3f( 0.f, 0.f, -1.f );

		for( u32 i = 0; i < numCircles; ++i )
		{
			Math::tVec3f u0 = Math::tVec3f::cZeroVector;
			Math::tVec3f u1 = Math::tVec3f::cZeroVector;

			u0.fAxis( i ) = 1.f;
			u1.fAxis( ( i + 1 ) % numCircles ) = 1.f;

			for( u32 j = 0; j < numSegsPerCircle; ++j )
			{
				const f32 theta0 = Math::c2Pi * ( ( j + 0.f ) / numSegsPerCircle );
				const f32 theta1 = Math::c2Pi * ( ( j + 1.f ) / numSegsPerCircle );

				const Math::tVec3f p0 = std::cosf( theta0 ) * u0 + std::sinf( theta0 ) * u1;
				const Math::tVec3f p1 = std::cosf( theta1 ) * u0 + std::sinf( theta1 ) * u1;

				lineVerts[ iVtx++ ].mP = p0;
				lineVerts[ iVtx++ ].mP = p1;
			}
		}

		sigassert( iVtx == lineVerts.fCount( ) );

		mSphereCageSparseTemplate.fBake( ( Sig::byte* )lineVerts.fBegin( ), lineVerts.fCount( ), false );
	}

	void tEditableObjectContainer::fGenerateSphereCageDenseTemplate( )
	{
		const u32 numCorners = 8;
		const u32 numLines = 3 * numCorners;
		tDynamicArray< Gfx::tSolidColorRenderVertex > lineVerts( 2 * numLines );

		const f32 lineLen = 0.25f;
		const u32 color = Gfx::tVertexColor( 0xff, 0xff, 0xff, 0xff ).fForGpu( );

		// get the color assigning out of the way
		for( u32 i = 0; i < lineVerts.fCount( ); ++i )
			lineVerts[ i ].mColor = color;

		// now create the line geometry

		for( u32 i = 0; i < numCorners; ++i )
		{
			const u32 vtxBase = i * 3 * 2;
			const Math::tVec3f corner = Math::tVec3f( (i&1) ? 1.f : -1.f, (i&2) ? 1.f : -1.f, (i&4) ? 1.f : -1.f );
			lineVerts[ vtxBase + 0 ].mP = corner;
			lineVerts[ vtxBase + 1 ].mP = corner + Math::tVec3f( -lineLen * corner.x, 0.f, 0.f );
			lineVerts[ vtxBase + 2 ].mP = corner;
			lineVerts[ vtxBase + 3 ].mP = corner + Math::tVec3f( 0.f, -lineLen * corner.y, 0.f );
			lineVerts[ vtxBase + 4 ].mP = corner;
			lineVerts[ vtxBase + 5 ].mP = corner + Math::tVec3f( 0.f, 0.f, -lineLen * corner.z );
		}

		mSphereCageDenseTemplate.fBake( ( Sig::byte* )lineVerts.fBegin( ), lineVerts.fCount( ), false );
	}

	void tEditableObjectContainer::fGenerateDummyTemplates( )
	{
		mDummyBoxTemplate.fSetRenderStateOverride( &mDummyRenderStateOverride );
		mDummySphereTemplate.fSetRenderStateOverride( &mDummyRenderStateOverride );
		mDummyBoxTemplate.fGenerate( 1.f );
		mDummySphereTemplate.fGenerate( 1.f );
		mDummyQuadTemplate.fGenerate( 1.f );
	}

	void tEditableObjectContainer::fInsert( const tEntityPtr& object )
	{
		tEditableObject* eo = object->fDynamicCast< tEditableObject >( );
		sigassert( eo );

		mObjectSet[ eo->fState( ) ].fPushBack( object );

		if( !eo->fIsHidden( ) )
			object->fSpawnImmediate( mSceneGraph->fRootEntity( ) );

		mOnObjectAdded.fFire( *this, object );
	}

	void tEditableObjectContainer::fRemove( tEditableObject* object )
	{
		object->fDeleteImmediate( );

		tEntityMasterList& list = mObjectSet[ object->fState( ) ];
		for( u32 i = 0; i < list.fCount( ); ++i )
		{
			if( list[ i ].fGetRawPtr( ) == object )
			{
				tEntityPtr tempPtr( object );
				list.fErase( i );
				mOnObjectRemoved.fFire( *this, tempPtr );
				break;
			}
		}
	}
}
