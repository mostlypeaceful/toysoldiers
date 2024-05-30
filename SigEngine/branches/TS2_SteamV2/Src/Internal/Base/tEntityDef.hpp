#ifndef __tEntityDef__
#define __tEntityDef__
#include "tDelegate.hpp"

namespace Sig
{
	class tEntity;
	class tEntityDefProperties;
	class tAnimatedSkeleton;
	class tSceneGraphFile;

	class tEntityCreationFlags
	{
		declare_reflector( );
	public:
		enum tCreateFlagBits
		{
			cFlagDisableContextAnims = ( 1 << 0 ),
			cFlagImmediateScript = ( 1 << 1 ),

			cDisinheritMask = ~( cFlagImmediateScript ),
		};

		u16 mRenderFlags;
		u16 mCreateFlags;
		inline tEntityCreationFlags( ) : mRenderFlags( 0 ), mCreateFlags( 0 ) { }
		inline tEntityCreationFlags( tNoOpTag ) { }
		inline tEntityCreationFlags( u16 rf, u16 df ) : mRenderFlags( rf ), mCreateFlags( df ) { }
		inline tEntityCreationFlags operator|( const tEntityCreationFlags& other ) const { return tEntityCreationFlags( mRenderFlags|other.mRenderFlags, (mCreateFlags|other.mCreateFlags) & cDisinheritMask ); }
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
		void					fApplyPropsAndSpawnWithScript( tEntity& entity, tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
		void					fAddBoneProxy( tEntity& parent, tAnimatedSkeleton& skeleton ) const;
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
		virtual void	fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const = 0;
	};
}

#endif//__tEntityDef__
