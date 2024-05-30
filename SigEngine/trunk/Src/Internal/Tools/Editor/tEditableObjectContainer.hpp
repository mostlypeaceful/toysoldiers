#ifndef __tEditableObjectContainer__
#define __tEditableObjectContainer__
#include "tEditableObject.hpp"
#include "tEditorAction.hpp"
#include "tEditorSelectionList.hpp"
#include "tSceneGraph.hpp"
#include "tResourceDepot.hpp"
#include "Gfx/tDevice.hpp"
#include "Gfx/tSolidColorLines.hpp"
#include "Gfx/tSolidColorBox.hpp"
#include "Gfx/tSolidColorSphere.hpp"
#include "Gfx/tSolidColorQuads.hpp"
#include "Gfx/tDynamicTextureVRam.hpp"
#include "Dx9Util.hpp"

namespace Sig
{
	class tEditableObject;

	typedef tHashTable<std::string,Math::tVec4f> tLayerColorTableBase;
	
	class tools_export tLayerColorTable : public tLayerColorTableBase
	{
	public:
		tLayerColorTable( ) : tLayerColorTableBase( 32 ) { }
	};

	///
	/// \brief Container for tEditableObjects, in many respects this class acts
	/// as a wrapper around the editor's scene graph, providing some additional
	/// editor-specific functionality.
	class tools_export tEditableObjectContainer : public Gfx::tDeviceResource
	{
		friend class tEditableObject;
	public:
		typedef tEditableObject::tState tObjectState;
		typedef tDelegate<void ( u32 nObjectsComplete, u32 nObjectsTotal )> tOnObjectSerialized;
		typedef tGrowableArray< tEntityPtr > tEntityMasterList;
		typedef tFixedArray<tEntityMasterList,tEditableObject::cStateCount> tObjectSet;
		typedef tEvent<void ( tEditableObjectContainer &, const tEntityPtr & )> tOnObjectAdded;
		typedef tEvent<void ( tEditableObjectContainer &, const tEntityPtr & )> tOnObjectRemoved;

	private:
		u32								mNextGuid;
		tObjectSet						mObjectSet;
		tLayerColorTable				mLayerColors;
		tEditorSelectionList			mSelectionList;
		tEditorSelectionList			mPrevSelectionList;
		tEditorActionStack				mActionStack;
		tSceneGraphPtr					mSceneGraph;
		tResourceDepotPtr				mResourceDepot;
		Gfx::tDevicePtr					mDevice;
		Gfx::tSolidColorLines			mSelectedBoxTemplate;
		Gfx::tSolidColorLines			mSphereCageSparseTemplate;
		Gfx::tSolidColorLines			mSphereCageDenseTemplate;
		Gfx::tRenderState				mDummyRenderStateOverride;
		Gfx::tSolidColorBox				mDummyBoxTemplate;
		Gfx::tSolidColorSphere			mDummySphereTemplate;
		Gfx::tSolidColorCylinder		mDummyCylinderTemplate;
		Gfx::tSolidColorQuads			mDummyQuadTemplate;
		tResourcePtr					mSharedHeightFieldDiffuseMap;
		tResourcePtr					mSharedHeightFieldNormalMap;
		Dx9Util::tTextureCachePtr		mTextureCache;
		tOnObjectAdded					mOnObjectAdded;
		tOnObjectRemoved				mOnObjectRemoved;
		tGrowableArray< tEntityPtr>		mEntitiesNeedingUpdate;

		template< class ObjType >
		class tEditableObjectOfTypeRayCastCallback : public tSceneGraph::tDefaultRayCastCallback
		{
		public:
			tEditableObjectOfTypeRayCastCallback( ) { }
			tEditableObjectOfTypeRayCastCallback( tEntity* const* ignoreList, u32 numToIgnore ) : tSceneGraph::tDefaultRayCastCallback( ignoreList, numToIgnore ) { }
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

				ObjType* desiredType = spatial->fDynamicCast< ObjType >( );
				if( !desiredType )
					return;

				if( i->fQuickRejectByBox( ray ) )
					return;

				Math::tRayCastHit hit;

				spatial->fRayCast( ray, hit );

				if( hit.fHit( ) && hit.mT < mHit.mT )
				{
					mHit			= hit;
					mFirstEntity	= spatial;
				}
			}
		};

	public:
		explicit tEditableObjectContainer( const tSceneGraphPtr& sg, const tResourceDepotPtr& resDepot );
		~tEditableObjectContainer( );
		virtual void fOnDeviceLost( Gfx::tDevice* device ) { }
		virtual void fOnDeviceReset( Gfx::tDevice* device );

		void fUpdateObjectsTick( );
		void fAddEntityToUpdate( tEntityPtr newEnt );

		u32 fNextGuid( );
		u32 fDeserializeGuid( u32 oldGuid );
		void fResetAllGuids( ); // dangerous, used only to correct a corrupt scene (i.e., where duplicate GUIDs already exist)
		tEditableObject* fFindObjectByGuid( u32 guid );

		tEntityPtr fPick( const Math::tRayf& ray, f32* bestTout = 0, tEntity* const* ignoreList = 0, u32 numToIgnore = 0 ) const;
		template< class ObjType >
		tEntityPtr fPickByType( const Math::tRayf& ray, f32* bestTout = 0, tEntity* const* ignoreList = 0, u32 numToIgnore = 0 ) const
		{
			tEditableObjectOfTypeRayCastCallback<ObjType> rayCastCb( ignoreList, numToIgnore );

			return fPickInternal( rayCastCb, ray, bestTout, ignoreList, numToIgnore );
		}
		void fIntersect( const Math::tFrustumf& frustum, tEntityMasterList& intersection ) const;

		void fRemoveAllFromWorld( );
		Math::tAabbf fComputeBounding( ) const;
		
		void fSerialize( Sigml::tObjectPtrArray& objects, tOnObjectSerialized* perObjectCb = 0, b32 onlySelected = false );
		b32 fDeserialize( const Sigml::tObjectPtrArray& objects, tOnObjectSerialized* perObjectCb = 0 );
		void fImportObjects( const Sigml::tObjectPtrArray& objects, tOnObjectSerialized* perObjectCb = 0 );
		void fSetLayers( const Sigml::tLayerList& layers );
		void fMergeLayerColorEntries( const Sigml::tLayerList& layers );

		u32 										fGetObjectCount( ) const;
		u32 										fGetShownCount( ) const;
		u32 										fGetHiddenCount( ) const;
		u32 										fGetFrozenCount( ) const;
		tLayerColorTable&							fLayerColors( ) { return mLayerColors; }
		const tLayerColorTable&						fLayerColors( ) const { return mLayerColors; }
		tEditorSelectionList&						fGetSelectionList( ) { return mSelectionList; }
		tEditorSelectionList&						fGetPrevSelectionList( ) { return mPrevSelectionList; }
		tEditorActionStack&							fGetActionStack( ) { return mActionStack; }
		const tEditorActionStack&					fGetActionStack( ) const { return mActionStack; }
		const tResourceDepotPtr&					fGetResourceDepot( ) const { return mResourceDepot; }
		const tSceneGraphPtr&						fGetSceneGraph( ) const { return mSceneGraph; }
		const Gfx::tSolidColorLines&				fGetSelectedBoxTemplate( ) const { return mSelectedBoxTemplate; }
		const Gfx::tSolidColorLines&				fGetSphereCageSparseTemplate( ) const { return mSphereCageSparseTemplate; }
		const Gfx::tSolidColorLines&				fGetSphereCageDenseTemplate( ) const { return mSphereCageDenseTemplate; }
		const Gfx::tSolidColorBox&					fGetDummyBoxTemplate( ) const { return mDummyBoxTemplate; }
		const Gfx::tSolidColorSphere&				fGetDummySphereTemplate( ) const { return mDummySphereTemplate; }
		const Gfx::tSolidColorCylinder&				fGetDummyCylinderTemplate( ) const { return mDummyCylinderTemplate; }
		const Gfx::tSolidColorQuads&				fGetDummyQuadTemplate( ) const { return mDummyQuadTemplate; }
		const Gfx::tDevicePtr&						fGetDevice( ) const { return mDevice; }
		const Gfx::tMaterialPtr&					fGetSolidColorMaterial( ) const { return mDummyBoxTemplate.fMaterial( ); }
		const Gfx::tGeometryBufferVRamAllocatorPtr&	fGetSolidColorGeometryAllocator( ) const { return mDummyBoxTemplate.fGetGeometryAllocator( ); }
		const Gfx::tIndexBufferVRamAllocatorPtr&	fGetSolidColorIndexAllocator( ) const { return mDummyBoxTemplate.fGetIndexAllocator( ); }
		const tResourcePtr&							fSharedHeightFieldDiffuseMap( ) const { return mSharedHeightFieldDiffuseMap; }
		const tResourcePtr&							fSharedHeightFieldNormalMap( ) const { return mSharedHeightFieldNormalMap; }
		tResourcePtr&								fSharedHeightFieldDiffuseMap( ) { return mSharedHeightFieldDiffuseMap; }
		tResourcePtr&								fSharedHeightFieldNormalMap( ) { return mSharedHeightFieldNormalMap; }
		const Dx9Util::tTextureCachePtr&			fTextureCache( ) const { return mTextureCache; }
		tOnObjectAdded &							fGetObjectAddedEvent( ) { return mOnObjectAdded; }
		tOnObjectRemoved &							fGetObjectRemovedEvent( ) { return mOnObjectRemoved; }
		void fSetTextureCache( const Dx9Util::tTextureCachePtr& tc ) { mTextureCache = tc; }

		void fReset( 
			const Gfx::tDevicePtr& device, 
			const Gfx::tMaterialPtr& solidColorMaterial, 
			const Gfx::tGeometryBufferVRamAllocatorPtr& solidColorGeometry,
			const Gfx::tIndexBufferVRamAllocatorPtr& solidColorIndices );

		void fChangeHeightFieldMaterial( u32 hfMatType );
		u32 fGetHeightFieldMaterialType( ) const;
		void fOnSharedHeightFieldMapsModified( );

		void fHideSelected( tEditorSelectionList& selection );
		void fHideUnselected( tEditorSelectionList& selection );
		void fUnhideAll( );

		void fFreezeSelected( tEditorSelectionList& selection );
		void fFreezeUnselected( tEditorSelectionList& selection );
		void fUnfreezeAll( );

		void fCopyObjectSet( tObjectSet& objectSet ) const { objectSet = mObjectSet; }
		void fGetShown( tEntityMasterList& shownEnts ) const;
		void fGetHidden( tEntityMasterList& hiddenEnts ) const;
		void fGetFrozen( tEntityMasterList& frozenEnts ) const;
		void fGetLayer( tGrowableArray< tEditableObject* >& output, const std::string& layer ) const;
		void fGetVisibilitySet( tGrowableArray< tEditableObject* >& output, const std::string& set ) const;

		template<class t>
		void fCollectByName( tGrowableArray< t* >& output, const std::string& name ) const
		{
			const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateShown ];
			for( u32 i = 0; i < shown.fCount( ); ++i )
			{
				t* c = shown[ i ]->fDynamicCast< t >( );
				if( !c )
					continue;
				
				if( c->fGetName() != name )
					continue;

				output.fPushBack( c );
			}
		}
		template<class t>
		void fCollectByType( tGrowableArray< t* >& output ) const
		{
			const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateShown ];
			for( u32 i = 0; i < shown.fCount( ); ++i )
			{
				t* c = shown[ i ]->fDynamicCast< t >( );
				if( c ) output.fPushBack( c );
			}
		}
		template<class t>
		void fCollectHiddenByType( tGrowableArray< t* >& output ) const
		{
			const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateHidden ];
			for( u32 i = 0; i < shown.fCount( ); ++i )
			{
				t* c = shown[ i ]->fDynamicCast< t >( );
				if( c ) output.fPushBack( c );
			}
		}
		template<class t>
		void fCollectAllByType( tGrowableArray< t* >& output ) const
		{
			const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateShown ];
			for( u32 i = 0; i < shown.fCount( ); ++i )
			{
				t* c = shown[ i ]->fDynamicCast< t >( );
				if( c ) output.fPushBack( c );
			}
			const tEntityMasterList& frozen = mObjectSet[ tEditableObject::cStateFrozen ];
			for( u32 i = 0; i < frozen.fCount( ); ++i )
			{
				t* c = frozen[ i ]->fDynamicCast< t >( );
				if( c ) output.fPushBack( c );
			}
			const tEntityMasterList& hidden = mObjectSet[ tEditableObject::cStateHidden ];
			for( u32 i = 0; i < hidden.fCount( ); ++i )
			{
				t* c = hidden[ i ]->fDynamicCast< t >( );
				if( c ) output.fPushBack( c );
			}
		}

		template< class t >
		void fCollectSelectedOrOnly( tGrowableArray< t*>& output, b32 ( *filterFunc )( t * ) = 0 ) const
		{
			if( mSelectionList.fCount( ) )
			{
				mSelectionList.fCullByType( output );
				return;
			}

			t* entry = 0;

			const tEntityMasterList& shown = mObjectSet[ tEditableObject::cStateShown ];
			for( u32 i = 0; i < shown.fCount( ); ++i )
			{
				t* c = shown[ i ]->fDynamicCast< t >( );
				if( !c )
					continue;

				if( filterFunc && (*filterFunc)( c ) )
					continue;

				if( entry )
					return;
				
				entry = c;
			}

			if( entry )
				output.fPushBack( entry );
		}

	private:
		void fGenerateSelectedBoxTemplate( );
		void fGenerateSphereCageSparseTemplate( );
		void fGenerateSphereCageDenseTemplate( );
		void fGenerateDummyTemplates( );
		void fInsert( const tEntityPtr& object );
		void fRemove( tEditableObject* object );
		template<class tRayCastOperator> 
		tEntityPtr fPickInternal( const tRayCastOperator& rayCastCb, const Math::tRayf& ray, f32* bestTout = 0, tEntity* const* ignoreList = 0, u32 numToIgnore = 0 ) const
		{
			mSceneGraph->fRayCast( ray, rayCastCb );

			f32 bestT = 1.1f;
			tEntityPtr bestO;

			if( rayCastCb.mHit.fHit( ) && rayCastCb.mHit.mT < bestT )
			{
				sigassert( rayCastCb.mFirstEntity );

				tEditableObject* eo = rayCastCb.mFirstEntity->fFirstAncestorOfType< tEditableObject >( );
				if( eo )
				{
					eo = eo->fGetGroupParent( );

					bestT = rayCastCb.mHit.mT;
					bestO.fReset( eo );
				}
			}

			if( bestTout )
				*bestTout = bestT;

			return bestO;
		}
	};

}

#endif//__tEditableObjectContainer__
