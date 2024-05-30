#ifndef __tEntity__
#define __tEntity__
#include "tLogic.hpp"
#include "tLogicPtr.hpp"

#define sig_usequadtree

#ifdef sig_usequadtree
#	include "tDynamicQuadtree.hpp"
#else
#	include "tDynamicOctree.hpp"
#endif


namespace Sig
{
	class tEntity;
	class tEntityDef;
	class tEntityDefProperties;
	class tEntityCreationFlags;
	class tSpatialEntity;
	class tEntityCloud;
	class tAnimatedSkeleton;
	class tLogicPtr;
	namespace FX{ class tFxFileRefEntity; }

#ifdef sig_usequadtree
	typedef tDynamicQuadtree		tEntityBVH;
	typedef tDynamicQuadtreeRoot	tEntityBVHRoot;
#else
	typedef tDynamicOctree			tEntityBVH;
	typedef tDynamicOctreeRoot		tEntityBVHRoot;
#endif

	typedef u32						tEntityTagMask;
	struct tEntityEnumProperty
	{
		u32 mEnumKey, mEnumValue;
		inline tEntityEnumProperty( ) : mEnumKey( ~0 ), mEnumValue( ~0 ) { }
		inline tEntityEnumProperty( u32 k, u32 v ) : mEnumKey( k ), mEnumValue( v ) { }
		inline b32 operator==( const tEntityEnumProperty& other ) const { return mEnumKey == other.mEnumKey && mEnumValue == other.mEnumValue; }
	};
	typedef tGrowableArray<tEntityEnumProperty> tEntityEnumPropertyList;

	define_smart_ptr( base_export, tRefCounterPtr, tEntity );


	///
	/// \brief tEntity is the base type for all objects that live in the scene graph and also "in the world". 
	///
	/// tEntity does not imply volume or shape (see tSpatialEntity), but it does imply position/orientation/scale.
	/// tEntity contains data for scene graph management (parent/child hierarchy).
	///
	/// tEntity extends tLogic. On the surface this would imply that logic should be built into the tEntity type.
	/// However, in general, entities should be managed by the dynamically set member mController instance, leaving tEntity a "dumb" type.
	/// tEntity in fact derives from tLogic only for those cases where certain core logic is so innate to an entity's being that it just simply 
	/// doesn't make sense to make it optional: particle systems are a case in point. However, even such entity types that extend the tLogic update 
	/// methods should still be inter-operable with the member mController object, allowing for scripted/external control of the entity.
	class base_export tEntity : public tLogic
	{
		friend class tSceneGraph;
		define_dynamic_cast( tEntity, tLogic );
	public:
		typedef tGrowableArray<tEntityPtr>	tChildList;
		typedef tChildList::tConstIterator	tChildIterator;
		typedef tEntityBVH::tObject			tSpatialSetObject;

		static u32 fGlobalEntityCount( );
		static u32 fGlobalHighWaterEntityCount( );

#ifdef sig_logging
		static const tGrowableArray<tEntity*>& fGlobalEntityTracker( );
#endif//sig_logging

	private: // basic entity data for scene graph
		tEntity*					mParent;
		tChildList					mChildren;
		tLogicPtr*					mController;
		tStringPtr					mName;
		tEntityTagMask				mGameTags;
		tEntityEnumPropertyList		mEnumProps;

	protected: // moveable reference frame data
		Math::tMat3f				mObjectToWorld;
		mutable Math::tMat3f		mWorldToObject;
		Math::tMat3f				mParentRelative;
		b8							mLockedToParent;
		mutable volatile u32		mWorldToObjectDirty;
		b8							mInSpawnList;
		b8							pad0;

	public:
		tEntity( );
		virtual ~tEntity( );

	public: // logic
		inline b32					fHasLogic( ) const { return mController != 0; }
		const tLogicPtr*			fLogicPtr( ) const { return mController; }
		tLogic*						fLogic( ) const { return mController ? mController->fCodeObject( ) : 0; }
		template<class t> t*		fLogicDerived( ) const { tLogic* logic = fLogic( ); return logic ? logic->fDynamicCast< t >( ) : 0; }
		template<class t> t*		fLogicDerivedStaticCast( ) const { tLogic* logic = fLogic( ); return logic ? logic->fStaticCast< t >( ) : 0; }

		// N.B.! entity will acquire ownership of this pointer and 'delete' it in the destructor or the next time fSetLogic is called
		void						fAcquireLogic( tLogicPtr* logicPtr );

	public: // hierarchy
		inline tEntity*				fParent( ) const { return mParent; }
		inline u32					fChildCount( ) const { return mChildren.fCount( ); }
		inline const tEntityPtr&	fChild( u32 i ) const { return mChildren[ i ]; }
		b32							fIsAncestorOfMine( const tEntity& entity ) const;
		b32							fIsDescendentOfMine( const tEntity& entity ) const;
		tEntity*					fFirstAncestorWithLogic( ) const; // will return self if fHasLogic is true
		tEntity*					fFirstDescendentWithName( const tStringPtr& name, b32 recursive = true ) const;
		void						fAllDescendentsWithName( const tStringPtr& name, tGrowableArray<tEntity*>& output, b32 recursive = true ) const;
		void						fAllDescendentsWithAllTags( tEntityTagMask tags, tGrowableArray<tEntity*>& output, b32 recursive = true ) const;
		tEntity*					fSpawnChild( const tFilePathPtr& sigmlPath ); // uses global resource depot, assumes resource is loaded
		tEntity*					fSpawnChildFromProxy( const tFilePathPtr& sigmlPath, tEntity* proxy ); // uses global resource depot, assumes resource is loaded
		tEntity*					fSpawnSurfaceFxSystem( const tFilePathPtr& sigmlPath, const Math::tVec3f& position, const Math::tVec3f& surfaceNormal, const Math::tVec3f& inputDir, const Math::tVec3f& xDir ); //inputDir is the direction of the effect, like the projectileVel, sideNormal is a fallback vector used if inputNormal aligns with either the surface or Y axis
		FX::tFxFileRefEntity*		fSpawnFxChild( const tFilePathPtr& fxmlPath, s32 playCount=-1, b32 local = false ); // uses global resource depot, assumes resource is loaded
		if_devmenu( void			fDumpChildrenToOutput( u32 depth = 0 ) const );

	public: // properties
		void						fAcquirePropertiesFromAncestors( );
		void						fAcquirePropertiesFromEntity( const tEntity& copyFrom );
		inline void					fSetName( const tStringPtr& name ) { mName = name; }
		inline const tStringPtr&	fName( ) const { return mName; }
		inline const char*			fNameCStr( ) const { return mName.fCStr( ); }
		inline tStringPtr			fSubName( ) const { return tStringPtr( fSubNameCStr( ) ); }
		const char*					fSubNameCStr( ) const;
		inline void					fAddGameTags( tEntityTagMask tags ) { mGameTags |= tags; }
		inline void					fAddGameTagsRecursive( tEntityTagMask tags ) { fAddGameTags( tags ); for( u32 i = 0; i < fChildCount( ); ++i ) fChild( i )->fAddGameTagsRecursive( tags ); }
		inline void					fRemoveGameTags( tEntityTagMask tags ) { mGameTags &= ~tags; }
		inline void					fRemoveGameTagsRecursive( tEntityTagMask tags ) { fRemoveGameTags( tags ); for( u32 i = 0; i < fChildCount( ); ++i ) fChild( i )->fRemoveGameTagsRecursive( tags ); }
		inline b32					fHasGameTagsAny( tEntityTagMask tags ) const { return ( mGameTags & tags ) != 0; }
		inline b32					fHasGameTagsAll( tEntityTagMask tags ) const { return ( mGameTags & tags ) == tags; }
		b32							fHasGameTagsAnyInherited( tEntityTagMask tags  ) const; // goes up the line of ancestors to find the first one with any of the specified tags
		b32							fHasGameTagsAllInherited( tEntityTagMask tags ) const;  // goes up the line of ancestors to find the first one with all of the specified tags
		void						fSetEnumValue( const tEntityEnumProperty& enumProp );
		void						fSetEnumValueForScript( u32 key, u32 value ) { fSetEnumValue( tEntityEnumProperty( key, value ) ); }
		void						fSetEnumValueFromEntity( u32 key, const tEntity* entity );
		void						fRemoveEnumProperty( u32 enumKey );
		u32							fQueryEnumValue( u32 enumKey, u32 enumValueDefault ) const;
		u32							fQueryEnumValue( u32 enumKey ) const { return fQueryEnumValue( enumKey, ~0 ); }
		u32							fQueryEnumValueInherited( u32 enumKey, u32 enumValueDefault = ~0 ) const; // goes up the line of ancestors to find the enum value

		// commonly used utilities
		static tEntity*				fClosestEntityNamed( const tGrowableArray<tEntityPtr>& array, const Math::tVec3f& toPoint, const tStringPtr& name, f32 maxDistSqr = Math::cInfinity );
		static tEntity*				fClosestEntity( const tGrowableArray<tEntityPtr>& array, const Math::tVec3f& toPoint, f32 maxDistSqr = Math::cInfinity );

	public: // lifetime, children, spawn/delete
		void						fPreserve( );
		void						fSpawn( tEntity& parent );
		void						fDelete( );
		b32							fInDeletionList( ) const;

		// don't use the 'Immediate' versions unless you have to - and you probably don't have to.
		void						fSpawnImmediate( tEntity& parent );
		void						fDeleteImmediate( );

		void						fReparent( tEntity& newParent );
		void						fReparentImmediate( tEntity& newParent );
		void						fTransferChildren( tEntity& newParent );
		void						fClearChildren( );

		virtual void				fAfterSiblingsHaveBeenCreated( ) { }

	public: // overrideable
		virtual b32					fIsHelper( ) const { return false; }
		virtual const tEntityDef*	fQueryEntityDef( ) const { return 0; } // some types store their entity def - don't really on this being non-null
		virtual void				fPropagateSkeleton( tAnimatedSkeleton& skeleton );
		virtual void				fApplyCreationFlags( const tEntityCreationFlags& creationFlags ) { }

	protected: // methods that are only intended to be called by derived types, but whose behavior might differ by type
		void						fPropagateSkeletonInternal( tAnimatedSkeleton& skeleton, const tEntityDefProperties* entityDef ); // helper, eventually recursively calls virtual versions
		void						fRemoveFromSceneGraph( );
		void						fRemoveFromRunLists( );
	private:
		void						fSetSceneGraph( tSceneGraph* sg );

	protected: // methods that should only be called by tEntity, tSceneGraph or derived types invoking base functionality
		virtual void				fOnSpawn( );
		virtual void				fOnDelete( );
		virtual void				fOnParentDelete( );
		virtual void				fOnPause( b32 paused );
		virtual b32					fReadyForDeletion( );
		virtual void				fOnEmptyNest( ) { } // called to notify entity that there's no more children - called in response to fClearChildren or fRemoveChild
		void						fAddChild( tEntity& child );
		void						fRemoveChild( tEntity& child );

	public: // Moveable Reference Frame Interface
		void						fMoveTo( const Math::tVec3f& newPos );
		void						fMoveTo( const Math::tMat3f& newXform );
	public:
		void						fSetParentRelativeXform( const Math::tMat3f& newXform );
		inline void					fSetParentRelativeXformIsolated( const Math::tMat3f& newXform ) { mParentRelative = newXform; }
		void						fTranslate( const Math::tVec3f& delta );
		void						fTransform( const Math::tMat3f& xform );
		b32							fLockedToParent( ) const { return mLockedToParent; }
		void						fSetLockedToParent( b32 locked ) { mLockedToParent = locked; }
		inline const Math::tMat3f&	fObjectToWorld( ) const				{ return mObjectToWorld; }
		const Math::tMat3f&			fWorldToObject( ) const;
		inline const Math::tMat3f&	fParentRelative( ) const			{ return mParentRelative; }
		Math::tAabbf				fCombinedObjectSpaceBox( ) const;
		Math::tAabbf				fCombinedWorldSpaceBox( ) const;
		Math::tVec3f				fWorldSpaceCOM( ) const;
		Math::tVec3f				fObjectSpaceCOM( ) const;
	protected: // moveable reference frame notifications/interface for derived types
		virtual void				fOnMoved( b32 recomputeParentRelative );
		void						fOnParentMoved( const Math::tMat3f& parentXform );
		void						fRecomputeParentRelative( );

	public: // script-specific
		static void					fExportScriptInterface( tScriptVm& vm );
		b32							fIsEqual( tEntity* other );

	public:// script-specific private member functions (exposed only to script, not code)
		Sqrat::Object				fScriptLogicObject( ) const;
		void						fSetScriptLogicObject( const Sqrat::Object& obj );

	public: // big template recursive functions that need to be defined in the header but clutter up the interface
		template<class t>
		t* fFirstAncestorOfType( ) const
		{
			for( const tEntity* i = this; i; i = i->fParent( ) )
			{
				t* test = i->fDynamicCast< t >( );
				if( test ) return test;
			}
			return 0;
		}
		template<class t>
		t* fFirstAncestorWithLogicOfType( ) const
		{
			for( const tEntity* i = this; i; i = i->fParent( ) )
			{
				t* test = i->fLogicDerived< t >( );
				if( test ) return test;
			}
			return 0;
		}
		template<class t>
		t* fFirstDescendentOfType( b32 immediateChildrenOnly = false ) const
		{
			// descend breadth first (i.e., test all my children before testing children's children)
			for( u32 i = 0; i < fChildCount( ); ++i )
			{
				t* test = fChild( i )->fDynamicCast< t >( );
				if( test ) return test;
			}

			// now go deep
			if( !immediateChildrenOnly )
			{
				for( u32 i = 0; i < fChildCount( ); ++i )
				{
					t* test = fChild( i )->fFirstDescendentOfType< t >( false );
					if( test ) return test;
				}
			}

			return 0;
		}
		template<class tForEach>
		b32 fForEachDescendent( const tForEach& forEach, b32 immediateChildrenOnly = false  )
		{
			// descend breadth first (i.e., test all my children before testing children's children)
			for( u32 i = 0; i < fChildCount( ); ++i )
				if( forEach( *fChild( i ) ) )
					return true;
			// now go deep
			if( !immediateChildrenOnly )
			{
				for( u32 i = 0; i < fChildCount( ); ++i )
					if( fChild( i )->fForEachDescendent( forEach, immediateChildrenOnly ) )
						return true;
			}
			return false;
		}
		template<class tForEach>
		b32 fForEachDescendent( const tForEach& forEach, b32 immediateChildrenOnly = false  ) const
		{
			// descend breadth first (i.e., test all my children before testing children's children)
			for( u32 i = 0; i < fChildCount( ); ++i )
				if( forEach( *fChild( i ) ) )
					return true;
			// now go deep
			if( !immediateChildrenOnly )
			{
				for( u32 i = 0; i < fChildCount( ); ++i )
					if( fChild( i )->fForEachDescendent( forEach, immediateChildrenOnly ) )
						return true;
			}
			return false;
		}
	
	
	};
}

namespace Sqrat
{
	sqrat_define_refcounterptr_var(Sig::tEntityPtr)
}

#endif//__tEntity__
