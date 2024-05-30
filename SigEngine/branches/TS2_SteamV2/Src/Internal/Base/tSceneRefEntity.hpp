#ifndef __tSceneRefEntity__
#define __tSceneRefEntity__
#include "tSceneGraphFile.hpp"
#include "tResource.hpp"

namespace Sig
{

	///
	/// \brief Entity def for references to other sigml/mshml files - this is a fundamental building block
	/// allowing for multiply-nested scene referencing.
	class base_export tSceneRefEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tSceneRefEntityDef, 0xE3AD6419 );
	public:
		tLoadInPlaceResourcePtr*	mReferenceFile;
		tSceneLODSettings*			mLODSettings;
	public:
		tSceneRefEntityDef( );
		tSceneRefEntityDef( tNoOpTag );
		virtual ~tSceneRefEntityDef( );
		virtual b32 fHasRenderableBounds( ) const { return true; }
		virtual b32  fOnSubResourcesLoaded( const tResource& ownerResource );
		virtual void fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const;
	};


	///
	/// \brief Represents a scene/entity/level - i.e., the dynamic counterpart to tSceneGraphFile and tSceneRefEntityDef. Does not
	/// add ownership to the resource in terms of load refs - i.e., generally assumes the resource is loaded.
	/// \note Exists solely to have children - when children are gone, tSgFileRefEntity will delete itself.
	class base_export tSceneRefEntity : public tEntity
	{
		define_dynamic_cast( tSceneRefEntity, tEntity );
	protected:
		tResourcePtr	mSgResource;
		b32				mClearingChildren;
	public:
		explicit tSceneRefEntity( const tResourcePtr& sgResource, const tEntity* proxy = 0 );
		virtual ~tSceneRefEntity( );
		virtual void fPropagateSkeleton( tAnimatedSkeleton& skeleton );
		virtual void fOnSpawn( );
		virtual void fOnEmptyNest( );
		const tResourcePtr& fSgResource( ) const { return mSgResource; }
		void fCollectEntities( const tEntityCreationFlags& createFlags, const tEntityDefProperties* entityDefOverride = 0 );
		void fCollectEntities( const tEntityCreationFlags& createFlags, const tSceneRefEntityDef* entityDefOverride );

		tFilePathPtr fResourcePath( ) const { return mSgResource ? mSgResource->fGetPath( ) : tFilePathPtr::cNullPtr; }

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		void fCreateAndPropagateSkeleton( );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tSceneRefEntity );
}


#endif//__tSceneRefEntity__
