#include "BasePch.hpp"
#include "tSceneRefEntity.hpp"
#include "tSceneGraphFile.hpp"
#include "Logic/tAnimatable.hpp"
#include "Gfx/tRenderableEntity.hpp"

namespace Sig
{
	///
	/// \section tSceneRefEntityDef
	///

	register_rtti_factory( tSceneRefEntityDef, true )

	tSceneRefEntityDef::tSceneRefEntityDef( )
		: mReferenceFile( 0 )
		, mLODSettings( 0 )
	{
	}

	tSceneRefEntityDef::tSceneRefEntityDef( tNoOpTag )
		: tEntityDef( cNoOpTag )
	{
	}

	tSceneRefEntityDef::~tSceneRefEntityDef( )
	{
		delete mLODSettings;
	}

	b32 tSceneRefEntityDef::fOnSubResourcesLoaded( const tResource& ownerResource )
	{
		const tResourcePtr& resource = mReferenceFile->fGetResourcePtr( );
		const tSceneGraphFile* sgFile = resource->fCast<tSceneGraphFile>( );
		if( !sgFile )
			return false;

		if( sgFile->mBounds.fIsValid( ) )
			mBounds = sgFile->mBounds.fTransform( mObjectToLocal );
		return true;
	}

	void tSceneRefEntityDef::fCollectEntities( tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		const tResourcePtr& resource = mReferenceFile->fGetResourcePtr( );
		const tSceneGraphFile* sgFile = resource->fCast<tSceneGraphFile>( );
		if( !sgFile )
			return;

		tEntity* realParent = &parent;
		tSceneRefEntity* entity = NEW tSceneRefEntity( resource );

		if( mBoneAttachment )
			realParent = &fInsertReferenceFrame( parent, mObjectToLocal );
		else
			entity->fMoveTo( mObjectToLocal );

		entity->fSpawn( *realParent );
		entity->fCollectEntities( creationFlags | fCreationFlags( ), this );
	}
}


namespace Sig
{
	///
	/// \section tSceneRefEntity
	///

	tSceneRefEntity::tSceneRefEntity( const tResourcePtr& sgResource, const tEntity* proxy )
		: mSgResource( sgResource )
		, mClearingChildren( false )
	{
		sigassert( mSgResource->fGetClassId( ) == Rtti::fGetClassId<tSceneGraphFile>( ) && "tSceneGraphFile.hpp needs to be included at the point that the resource id for this tSceneRefEntity was instantiated" );

		if( proxy )
		{
			fAcquirePropertiesFromEntity( *proxy );
			fSetName( proxy->fName( ) );
		}
	}
	tSceneRefEntity::~tSceneRefEntity( )
	{
		mClearingChildren = true;
	}
	void tSceneRefEntity::fPropagateSkeleton( tAnimatedSkeleton& skeleton )
	{
		const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
		sigassert( sgFile );
		fPropagateSkeletonInternal( skeleton, sgFile );
	}
	void tSceneRefEntity::fOnSpawn( )
	{
		// then create/propagate skeleton
		fCreateAndPropagateSkeleton( );

		// base on spawn first (so entities are all initialized)
		tEntity::fOnSpawn( );
	}
	void tSceneRefEntity::fOnEmptyNest( )
	{
		if( !mClearingChildren )
			fDelete( );
	}
	void tSceneRefEntity::fCollectEntities( const tEntityCreationFlags& createFlags, const tEntityDefProperties* entityDefOverride )
	{
		const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
		if( !sgFile )
		{
			log_warning( Log::cFlagResource, "Invalid call to tSceneRefEntity::fCollectEntities, file wasn't loaded: " << mSgResource->fGetPath( ) );
			return;
		}

		sigassert( sgFile );
		if( entityDefOverride )
		{
			sgFile->fApplyProperties( *this, *entityDefOverride, createFlags );
			sgFile->fEntityOnCreate( *this, *entityDefOverride );
			sgFile->fCollectEntities( *this, createFlags );
			
			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fAfterSiblingsHaveBeenCreated( );

			sgFile->fEntityOnChildrenCreate( *this, *entityDefOverride );
		}
		else
		{
			sgFile->fApplyProperties( *this, createFlags );
			sgFile->fEntityOnCreate( *this );
			sgFile->fCollectEntities( *this, createFlags );

			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fAfterSiblingsHaveBeenCreated( );

			sgFile->fEntityOnChildrenCreate( *this );
		}
	}
	void tSceneRefEntity::fCollectEntities( const tEntityCreationFlags& createFlags, const tSceneRefEntityDef* entityDefOverride )
	{
		tSceneRefEntity::fCollectEntities( createFlags, ( const tEntityDefProperties* )entityDefOverride );

		tSceneLODSettings* lodSettings = 0;
		if( entityDefOverride )
			lodSettings = entityDefOverride->mLODSettings;
		if( !lodSettings )
		{
			const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
			sigassert( sgFile );
			lodSettings = sgFile->mLODSettings;
		}
		if( lodSettings )
		{
			sigassert( lodSettings->mFadeOverride || lodSettings->mFadeSetting );
			Gfx::tRenderableEntity::fSetFadeSetting( *this, ( Gfx::tRenderableEntity::tFadeSetting )lodSettings->mFadeSetting, lodSettings->mFadeOverride );
		}
	}
	void tSceneRefEntity::fCreateAndPropagateSkeleton( )
	{
		const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
		sigassert( sgFile );
		if( sgFile->mSkeletonFile )
		{
			tLogic* logic = fLogic( );
			Logic::tAnimatable* animatable = logic ? logic->fQueryAnimatable( ) : 0;
			if( animatable )
			{
				if( animatable->fCreateAnimatedSkeleton( sgFile->mSkeletonFile->fGetResourcePtr( ), sgFile->mSkeletonBinding, sgFile->mSkeletonBindingInv ) )
				{
					// recursively propagate skeleton to children
					for( u32 i = 0; i < fChildCount( ); ++i )
						fChild( i )->fPropagateSkeleton( *animatable->fAnimatedSkeleton( ) );

					// notify logic of skeleton propagation completion
					logic->fOnSkeletonPropagated( );
				}
			}
		}
	}

	namespace
	{
		static tSceneRefEntity* fConvert( const tEntity* obj )
		{
			sigassert( obj );
			return obj->fDynamicCast< tSceneRefEntity >( );
		}
	}

	void tSceneRefEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tSceneRefEntity, tEntity, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("Convert"), &fConvert)
			.Prop(_SC("ResourcePath"),	&tSceneRefEntity::fResourcePath)
			;

		vm.fRootTable( ).Bind(_SC("SceneRefEntity"), classDesc);
	}
}
