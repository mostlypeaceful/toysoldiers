#include "BasePch.hpp"
#include "tSceneRefEntity.hpp"
#include "tSceneGraphFile.hpp"
#include "Logic/tAnimatable.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "tApplication.hpp"

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

	void tSceneRefEntityDef::fCollectEntities( const tCollectEntitiesParams& params ) const
	{
		const tResourcePtr& resource = mReferenceFile->fGetResourcePtr( );
		const tSceneGraphFile* sgFile = resource->fCast<tSceneGraphFile>( );
		if( !sgFile )
			return;

		tEntity* realParent = &params.mParent;
		tSceneRefEntity* entity = NEW_TYPED( tSceneRefEntity )( resource );

		if( mBoneAttachment )
			realParent = &fInsertReferenceFrame( params.mParent, mObjectToLocal );
		else
			entity->fMoveTo( mObjectToLocal );

		entity->fSpawn( *realParent );
		entity->fCollectEntities( params.mCreationFlags | fCreationFlags( ), this );
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
		, mEntityDef( 0 )
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
	void tSceneRefEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
		sigassert( sgFile );
		fPropagateSkeletonInternal( skeleton, sgFile );
	}
	void tSceneRefEntity::fOnSpawn( )
	{
		fCreateAndPropagateSkeleton( );

		/*	
			Second, initialize children's skeletons and propagate,
			 cleaning up any invalid skins caused by the main propagate above.
			This is redundant propagation work, should probably be avoided, but ultimately works.
		*/
		tEntity::fOnSpawn( );
	}
	void tSceneRefEntity::fOnEmptyNest( )
	{
		if( !mClearingChildren )
		{
			if( fHasLogic( ) )
			{
				Sqrat::Object logic = fScriptLogicObject( );
				if( !logic.IsNull( ) )
				{
					Sqrat::Function f( logic, _SC("OnEmptyNest") );
					sigcheckfail( !f.IsNull( ), return );
					f.Execute( );
				}
				else
					fLogic( )->fOnEmptyNest( );
				return; // the logic should decide whether or not to delete the parent
			}
			sigassert( !tApplication::fInstance( ).fSceneGraph( )->fInSpawnList( this ) );
			fDelete( );
		}
	}
	void tSceneRefEntity::fCollectEntities( const tEntityCreationFlags& createFlags, const tEntityDefProperties* entityDefOverride )
	{
		const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
		if( !sgFile )
		{
			log_warning( "Invalid call to tSceneRefEntity::fCollectEntities, file wasn't loaded: " << mSgResource->fGetPath( ) );
			return;
		}

		sigassert( sgFile );
		if( entityDefOverride )
		{
			sgFile->fApplyProperties( *this, *entityDefOverride, createFlags );
			sgFile->fEntityOnCreate( *this, *entityDefOverride );
			sgFile->fCollectEntities( tCollectEntitiesParams( *this, createFlags, mSgResource.fGetRawPtr( ) ) );
			
			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fAfterSiblingsHaveBeenCreated( );

			sgFile->fEntityOnChildrenCreate( *this, *entityDefOverride );
		}
		else
		{
			sgFile->fApplyProperties( *this, createFlags );
			sgFile->fEntityOnCreate( *this );
			sgFile->fCollectEntities( tCollectEntitiesParams( *this, createFlags, mSgResource.fGetRawPtr( ) ) );

			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fAfterSiblingsHaveBeenCreated( );

			sgFile->fEntityOnChildrenCreate( *this );
		}
	}
	void tSceneRefEntity::fCollectEntities( const tEntityCreationFlags& createFlags, const tSceneRefEntityDef* entityDefOverride )
	{
		mEntityDef = entityDefOverride;

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
			Gfx::tRenderableEntity::fSetFadeSettings( 
				*this, 
				( Gfx::tRenderableEntity::tFadeSetting )lodSettings->mFadeSetting, 
				Gfx::tRenderableEntity::cFadeNever, // Scene files do not cary near fade settings
				lodSettings->mFadeOverride );

			if( lodSettings->mLODMediumDistanceOverride || lodSettings->mLODFarDistanceOverride )
				Gfx::tRenderableEntity::fSetLODDists( *this, lodSettings->mLODMediumDistanceOverride, lodSettings->mLODFarDistanceOverride );
		}
	}
	void tSceneRefEntity::fCreateAndPropagateSkeleton( )
	{
		const tSceneGraphFile* sgFile = mSgResource->fCast< tSceneGraphFile >( );
		log_assert( sgFile, "Scene graph resource " << mSgResource->fGetPath( ) << " is either not loaded or is invalid" );

		if( sgFile->mSkeletonFile )
		{
			tLogic* logic = fLogic( );
			Logic::tAnimatable* animatable = logic ? logic->fQueryAnimatable( ) : 0;
			if( animatable )
			{
				if( animatable->fCreateAnimatedSkeleton( sgFile->mSkeletonFile->fGetResourcePtr( ), sgFile->mSkeletonBinding, sgFile->mSkeletonBindingInv, *this ) )
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
