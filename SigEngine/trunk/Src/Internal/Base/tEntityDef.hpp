#ifndef __tEntityDef__
#define __tEntityDef__
#include "tDelegate.hpp"

#include "Gfx/tPotentialVisibilitySet.hpp"

namespace Sig
{
	class tEntity;
	class tEntityDefProperties;
	class tSceneGraphFile;

	namespace Gfx 
	{ 
		// for LOD
		class tRenderableEntity; 
		class tGeometryFile; 
		class tMaterial;
	}

	namespace Anim
	{
		class tAnimatedSkeleton;
	}

	class base_export tEntityCreationVisibilitySets
	{
		declare_reflector( );
	public:
		tEntityCreationVisibilitySets( ) ;
		tEntityCreationVisibilitySets( tNoOpTag );
		~tEntityCreationVisibilitySets( );

		tDynamicArray< tLoadInPlacePtrWrapper< tLoadInPlaceStringPtr > > mSerializedVisibilitySets;	

		tEntityCreationVisibilitySets operator|( const tEntityCreationVisibilitySets& other ) const;

		b32 fHasVisibilitySet( ) const { return mVisibilitySet.fTreatAsObject().fGetRawPtr() != 0; }
		b32 fHasSerializedData( ) const { return mSerializedVisibilitySets.fCount() > 0; }

		void fSetVisibilitySet( Gfx::tVisibilitySetRef* set );
		Gfx::tVisibilitySetRef& fVisibilitySet( ) const;

	private:
		void fGetAllSets( tGrowableArray< tStringPtr >& output ) const;	
		tLoadInPlaceRuntimeObject< tRefCounterPtr< Gfx::tVisibilitySetRef > > mVisibilitySet;
	};

	class tEntityCreationFlags
	{
		declare_reflector( );
	public:
		enum tCreateFlagBits
		{
			cFlagImmediateScript				= ( 1 << 0 ),
			cFlagPhysicsCreateStatic			= ( 1 << 1 ),
			cFlagPhysicsDisableChildCollision	= ( 1 << 2 ),

			// If set then then the attachment uses it's parent relative transform
			// to arrive at an offset from the bone position. If not set then the
			// attachment behaves as though it is skinned to that bone.
			cFlagBoneRelativeAttachment			= ( 1 << 3 ),  

			cDisinheritMask = ~( cFlagImmediateScript ), //add flags here that you dont want to inherit
		};

		u16 mRenderFlags;
		u16 mCreateFlags;

		tEntityCreationVisibilitySets mVisibilitySet;

		inline tEntityCreationFlags( ) 
			: mRenderFlags( 0 )
			, mCreateFlags( 0 ) 
		{ }

		inline tEntityCreationFlags( tNoOpTag )
			: mVisibilitySet( cNoOpTag )
		{ }

		inline tEntityCreationFlags( u16 rf, u16 df ) 
			: mRenderFlags( rf )
			, mCreateFlags( df ) 
		{ }

		inline tEntityCreationFlags operator|( const tEntityCreationFlags& other ) const 
		{ 
			tEntityCreationFlags flags( mRenderFlags|other.mRenderFlags, (mCreateFlags|other.mCreateFlags) & cDisinheritMask ); 
			flags.mVisibilitySet = mVisibilitySet | other.mVisibilitySet;
			return flags;
		}

		b32 fCreateCollision( ) const			{ return (mCreateFlags & cFlagPhysicsCreateStatic) && !(mCreateFlags & cFlagPhysicsDisableChildCollision); }
		b32 fBoneRelativeAttachment( ) const	{ return mCreateFlags & cFlagBoneRelativeAttachment; }


	private:
	};

	struct tCollectEntitiesParams
	{
		tEntity&					mParent;
		const tEntityCreationFlags& mCreationFlags;
		tResourcePtr				mOwnerResource;

		tCollectEntitiesParams( tEntity& parent, const tEntityCreationFlags& creationFlags, tResource* ownerResource = NULL )
			: mParent( parent )
			, mCreationFlags( creationFlags )
			, mOwnerResource( ownerResource )
		{ }
	};

	typedef tDelegate<void ( tEntity& entity, const tEntityDefProperties& entityDef )> tEntityScriptAttachCb;

	class base_export tEntityDefProperties
	{
		declare_reflector( );
	public:
		struct tEnumProperty
		{
			declare_reflector( );
		public:
			u32 mEnumKey, mEnumValue;
		public:
			tEnumProperty( ) : mEnumKey( ~0 ), mEnumValue( ~0 ) { }
			tEnumProperty( u32 key, u32 value ) : mEnumKey( key ), mEnumValue( value ) { }
		};
		typedef tDynamicArray< u32 >			tTagPropertyList;
		typedef tDynamicArray< tEnumProperty >	tEnumPropertyList;
	public:
		tLoadInPlaceStringPtr*		mName;
		tLoadInPlaceResourcePtr*	mScriptFile;
		tLoadInPlaceStringPtr*		mOnEntityCreateOverride;
		tLoadInPlaceResourcePtr*	mSkeletonFile;
		tLoadInPlaceStringPtr*		mBoneAttachment;
		tEntityCreationFlags		mCreationFlags;
		tTagPropertyList			mTagProperties;
		tEnumPropertyList			mEnumProperties;
	public:
		tEntityDefProperties( );
		tEntityDefProperties( tNoOpTag );
		tStringPtr				fEntityName( ) const { return mName ? mName->fGetStringPtr( ) : tStringPtr( ); }
		tEntityCreationFlags	fCreationFlags( ) const { return mCreationFlags; }
		void					fApplyProperties( tEntity& entity, const tEntityCreationFlags& creationFlags ) const;
		void					fApplyProperties( tEntity& entity, const tEntityDefProperties& override, const tEntityCreationFlags& creationFlags ) const;
		b32						fEntityOnCreate( tEntity& entity ) const;
		void					fEntityOnCreate( tEntity& entity, const tEntityDefProperties& override ) const;
		void					fEntityOnChildrenCreate( tEntity& entity ) const;
		void					fEntityOnChildrenCreate( tEntity& entity, const tEntityDefProperties& override ) const;
		void					fEntityOnSiblingsCreate( tEntity& entity ) const;
		void					fEntityOnSiblingsCreate( tEntity& entity, const tEntityDefProperties& override ) const;
		void					fApplyPropsAndSpawnWithScript( tEntity& entity, const tCollectEntitiesParams& params ) const;
		void					fAddBoneProxy( tEntity& parent, Anim::tAnimatedSkeleton& skeleton ) const;
		tEntity&				fInsertReferenceFrame( tEntity& parent, const Math::tMat3f& objectToWorld ) const;
	};

	///
	/// \brief TODO document
	class base_export tEntityDef : public Rtti::tSerializableBaseClass, public tEntityDefProperties
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tEntityDef, 0x3AC67D44 );

	public:
		Math::tAabbf				mBounds;
		Math::tMat3f				mObjectToLocal;
		Math::tMat3f				mLocalToObject;

	public:
		tEntityDef( );
		tEntityDef( tNoOpTag );
		virtual ~tEntityDef( );
		
		virtual b32		fHasRenderableBounds( ) const { return false; }

		///
		/// \brief This method should return true if it actually process sub-resources or is modified in some way due to those sub-resources
		virtual b32		fOnSubResourcesLoaded( const tResource& ownerResource ) { return false; }
		virtual void	fOnFileLoaded( ) { }
		virtual void	fOnFileUnloading( ) { }
		virtual void	fCollectEntities( const tCollectEntitiesParams& params ) const = 0;

		
		virtual void fSelectLOD( Gfx::tRenderableEntity* entity, f32 ratio, b32 shadows, b32 normals ) const { }

		void fInitLOD( Gfx::tRenderableEntity* entity ) const;

		static void fSetLOD( 
			Gfx::tRenderableEntity* entity, 
			Gfx::tGeometryFile* geoFile, 
			u32 chunkIndex,
			u32 userFlags,
			Gfx::tMaterial* material,
			f32 ratio, 
			b32 shadows, 
			b32 normals );

		static b32 fLODDistChanged( f32 oldD, f32 newD );
	};
}

#endif//__tEntityDef__
