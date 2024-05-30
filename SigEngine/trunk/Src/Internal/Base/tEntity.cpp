#include "BasePch.hpp"
#include "tEntity.hpp"
#include "tEntityDef.hpp"
#include "tApplication.hpp"
#include "tSceneRefEntity.hpp"
#include "Logic/tAnimatable.hpp"
#include "tFxFileRefEntity.hpp"
#include "FX/tFxFile.hpp"
#include "tProfiler.hpp"
#include "Threads/tThread.hpp"
#include "Gfx/tLightEntity.hpp"

namespace Sig
{
	namespace
	{
		static const tFilePathPtr cScriptNullPath( "(null : 0x00000000)" );
		static const tStringPtr cNullStringPtr;
		static s32 gGlobalEntityCount = 0;
		static s32 gGlobalHighWaterEntityCount = 0;
	}

	s32 tEntity::fGlobalEntityCount( )
	{
		return gGlobalEntityCount;
	}

	s32 tEntity::fGlobalHighWaterEntityCount( )
	{
		return gGlobalHighWaterEntityCount;
	}

	void tComponentList::fAdd( const tLogicPtr& component ) 
	{ 
		sigassert( mOwner );
		mComponents.fPushBack( tLogicPtr( ) );
		mOwner->fAcquireLogicInternal( mComponents.fBack( ), component );
	}

	void tComponentList::fOnSpawn( )
	{
		for( u32 i = 0; i < mComponents.fCount( ); ++i )
			mComponents[ i ].fOnSpawn( );
	}

	void tComponentList::fOnDelete( )
	{
		for( u32 i = 0; i < mComponents.fCount( ); ++i )
			mComponents[ i ].fOnDelete( );
	}

	void tComponentList::fOnPause( b32 paused )
	{
		for( u32 i = 0; i < mComponents.fCount( ); ++i )
		{
			tLogic* logic = mComponents[ i ].fCodeObject( );
			if( logic )
				logic->fOnPause( paused );
		}
	}

	void tComponentList::fRemoveFromRunLists( )
	{
		for( u32 i = 0; i < mComponents.fCount( ); ++i )
		{
			tLogic* logic = mComponents[ i ].fCodeObject( );
			if( logic )
				logic->fRemoveFromRunLists( );
		}
	}

	void tComponentList::fSetSceneGraph( tSceneGraph* sg )
	{
		for( u32 i = 0; i < mComponents.fCount( ); ++i )
		{
			tLogic* logic = mComponents[ i ].fCodeObject( );
			if( logic )
				logic->fSetSceneGraph( sg );
		}
	}


	tEntity::tEntity( )
		: mParent( 0 )
		, mController( 0 )
		, mGameTags( 0 )
		, mObjectToWorld( Math::tMat3f::cIdentity )
		, mWorldToObject( Math::tMat3f::cIdentity )
		, mParentRelative( Math::tMat3f::cIdentity )
		, mWorldToObjectDirty( 0 )
		, mLockedToParent( true )
		, mInSpawnList( false )
		, mDeleteAfterSpawn( false )
		, pad1( false )
	{
		//sigassert_is_main_thread( );

		gGlobalHighWaterEntityCount = fMax( ++gGlobalEntityCount, gGlobalHighWaterEntityCount );
		mComponents.fSetOwner( this );
	}
	tEntity::~tEntity( )
	{
		sigassert_is_main_thread( );

#ifdef target_game
		sigassert( !tApplication::fInstance( ).fSceneGraph( ) || !tApplication::fInstance( ).fSceneGraph( )->fInMTRunList( ) );
#endif//target_game

		fClearChildren( );
		fRemoveFromSceneGraph( );
		mController = tLogicPtr( );

		sigassert( gGlobalEntityCount > 0 );
		--gGlobalEntityCount;
	}
	b32 tEntity::fIsAncestorOfMine( const tEntity& potentialAncestor ) const
	{
		for( tEntity* i = mParent; i; i = i->fParent( ) )
			if( &potentialAncestor == i )
				return true;
		return false;
	}
	b32 tEntity::fIsDescendentOfMine( const tEntity& potentialDescendent ) const
	{
		for( u32 i = 0; i < fChildCount( ); ++i )
		{
			if( fChild( i ) == &potentialDescendent )
				return true;
			if( fChild( i )->fIsDescendentOfMine( potentialDescendent ) )
				return true;
		}
		return false;
	}
	tEntity* tEntity::fFirstAncestorWithLogic( ) const
	{
		for( tEntity* i = ( tEntity* )this; i; i = i->fParent( ) )
			if( i->fHasLogic( ) )
				return i;

		return 0;
	}
	tEntity* tEntity::fFirstDescendentWithName( const tStringPtr& name, b32 recursive ) const
	{
		for( u32 i = 0; i < fChildCount( ); ++i )
			if( fChild( i )->fName( ) == name )
				return fChild( i ).fGetRawPtr( );
		if( recursive )
		{
			for( u32 i = 0; i < fChildCount( ); ++i )
			{
				tEntity* find = fChild( i )->fFirstDescendentWithName( name, true );
				if( find )
					return find;
			}
		}
		return 0;
	}
	void tEntity::fAllDescendentsWithName( const tStringPtr& name, tGrowableArray<tEntity*>& output, b32 recursive ) const
	{
		for( u32 i = 0; i < fChildCount( ); ++i )
			if( fChild( i )->fName( ) == name )
				output.fPushBack( fChild( i ).fGetRawPtr( ) );

		if( recursive )
		{
			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fAllDescendentsWithName( name, output, true );
		}
	}
	void tEntity::fAllDescendentsWithAllTags( tEntityTagMask tags, tGrowableArray<tEntity*>& output, b32 recursive ) const
	{
		for( u32 i = 0; i < fChildCount( ); ++i )
			if( fChild( i )->fHasGameTagsAll( tags ) )
				output.fPushBack( fChild( i ).fGetRawPtr( ) );

		if( recursive )
		{
			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fAllDescendentsWithAllTags( tags, output, true );
		}
	}
	void tEntity::fAllDescendentsWithAnyTags( tEntityTagMask tags, tGrowableArray<tEntity*>& output, b32 recursive ) const
	{
		for( u32 i = 0; i < fChildCount( ); ++i )
			if( fChild( i )->fHasGameTagsAny( tags ) )
				output.fPushBack( fChild( i ).fGetRawPtr( ) );

		if( recursive )
		{
			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fAllDescendentsWithAnyTags( tags, output, true );
		}
	}
	tEntity* tEntity::fSpawnChild( const tFilePathPtr& sigmlPath )
	{
		return fSpawnChildFromProxy( sigmlPath, 0 );
	}
	tEntity* tEntity::fSpawnChildFromProxy( const tFilePathPtr& sigmlPath, tEntity* proxy )
	{
		log_sigcheckfail( sigmlPath.fExists( ), "Attempting to spawn child without a valid sigmlPath", return 0 );
		log_sigcheckfail( sigmlPath != cScriptNullPath, "A null script variable was implicitly converted to a filepath to be spawned", return 0 );

		const tResourcePtr resource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tSceneGraphFile>( sigmlPath ) );
		log_sigcheckfail( resource && resource->fLoaded( ), "Attempting to spawn child without a loaded scene graph file [" << sigmlPath << "]", return 0 );

		tEntity* realParent = this;
		tSceneRefEntity* entity = NEW_TYPED( tSceneRefEntity )( resource, proxy );

		const tEntityDef* proxyDef = 0;
		if( proxy )
		{
			proxyDef = proxy->fQueryEntityDef( );
			if( proxyDef && proxyDef->mBoneAttachment )
				realParent = &proxyDef->fInsertReferenceFrame( *this, proxyDef->mObjectToLocal );
			else
				entity->fMoveTo( proxy->fParentRelative( ) );

			proxy->fDelete( );
		}

		entity->fSpawn( *realParent );
		entity->fCollectEntities( tEntityCreationFlags( ), proxyDef );

		return entity;
	}
	tEntity* tEntity::fSpawnChildImmediate( const tFilePathPtr& sigmlPath )
	{
		if( !sigmlPath.fExists( ) )
		{
			log_warning( "fSpawnChildImmediate - Attempting to spawn child without a valid sigmlPath" );
			return 0;
		}

		if( sigmlPath == cScriptNullPath )
		{
			log_warning( "fSpawnChildImmediate - You just tried to spawn an entity with a null variable from Squirrel Script. Good thing squirrel turns that into the string: " << sigmlPath );
			tScriptVm::fDumpCallstack( );
			return 0;
		}

		const tResourcePtr resource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<tSceneGraphFile>( sigmlPath ) );
		if( !resource->fLoaded( ) )
		{
			log_warning( "fSpawnChildImmediate - Attempting to spawn child without a loaded scene graph file [" << sigmlPath << "]" );
			return 0;
		}

		tSceneRefEntity* entity = NEW tSceneRefEntity( resource, NULL );

		//collect entities first so we can spawn and gather our children before calling fOnSpawn
		entity->fCollectEntities( tEntityCreationFlags( ), (tEntityDef*)NULL );

		//call fOnSpawn
		entity->fSpawnImmediate( *this );

		return entity;
	}
	FX::tFxFileRefEntity* tEntity::fSpawnFxChild( const tFilePathPtr& fxmlPath, s32 playcount, b32 local )
	{
		const tResourcePtr resource = tApplication::fInstance( ).fResourceDepot( )->fQuery( tResourceId::fMake<FX::tFxFile>( fxmlPath ) );
		if( !resource->fLoaded( ) )
		{
			log_warning( "Attempting to spawn fx child without a loaded fx file: " << fxmlPath );
			return 0;
		}
		FX::tFxFileRefEntity* entity = NEW FX::tFxFileRefEntity( resource, playcount, local, false, 0.f, NULL, *Gfx::tVisibilitySetRefManager::fInstance( ).fBlankRef( ) );
		entity->fSpawn( *this );
		return entity;
	}

    void tEntity::fAttachChild ( tEntity& child )
    {
        Math::tVec3f pos = fObjectToWorld( ).fGetTranslation( );
        child.fMoveTo( pos );
        child.fReparent( *this );
    }

	namespace
	{
		class tSurfaceFxOrientor
		{
		public:
			tSurfaceFxOrientor( tEntity* root, const Math::tVec3f& position, const Math::tVec3f& surfaceNormal, const Math::tVec3f& inputDir )
			{
				sigassert( root );

				Math::tVec3f normal = -surfaceNormal;
				normal.fNormalizeSafe( Math::tVec3f::cYAxis );
				mNormalMat.fSetTranslation( position );
				mNormalMat.fOrientYAxis( normal );

				Math::tVec3f inputNormal = inputDir;
				inputNormal.fNormalizeSafe( Math::tVec3f::cYAxis );
				mInputMat.fSetTranslation( position );
				mInputMat.fOrientYAxis( inputNormal );
				
				mYUp.fSetTranslation( position );
				mYUp.fOrientYAxis( Math::tVec3f::cYAxis, mInputMat.fXAxis( ) );
				fProcessRecursive( root );
			}

			void fApplyYUp( tEntity* e )
			{
				e->fMoveTo( mYUp );
				e->fSetLockedToParent( false );
			}

			void fApplySurfaceNormal( tEntity* e )
			{
				e->fMoveTo( mNormalMat );
				e->fSetLockedToParent( false );
			}

			void fApplyInputNormal( tEntity* e )
			{
				e->fMoveTo( mInputMat );
				e->fSetLockedToParent( false );
			}

			void fProcessRecursive( tEntity* e )
			{
				FX::tFxFileRefEntity* fx = e->fDynamicCast<FX::tFxFileRefEntity>( );
				if( fx )
				{
					const tEffectRefEntityDef& def = fx->fEntityDef( );
					switch( def.mSurfaceOrientation )
					{
					case tEffectRefEntityDef::cSurfaceOrientationSigml:
						/*no change*/ break;
					case tEffectRefEntityDef::cSurfaceOrientationYUp:
						fApplyYUp( e ); break;
					case tEffectRefEntityDef::cSurfaceOrientationSurfaceNormal:
						fApplySurfaceNormal( e ); break;
					case tEffectRefEntityDef::cSurfaceOrientationInputNormal:
						fApplyInputNormal( e ); break;
					default: 
						break;
					}
				}

				for( u32 i = 0; i < e->fChildCount( ); ++i )
					fProcessRecursive( e->fChild( i ).fGetRawPtr( ) );
			}

			Math::tMat3f mYUp;
			Math::tMat3f mNormalMat;
			Math::tMat3f mInputMat;
		};
	}

	tEntity* tEntity::fSpawnSurfaceFxSystem( const tFilePathPtr& sigmlPath, const Math::tVec3f& position, const Math::tVec3f& surfaceNormal, const Math::tVec3f& inputDir )
	{
		tEntity* ent = fSpawnChild( sigmlPath );
		if( ent )
		{
			tSurfaceFxOrientor thing( ent, position, surfaceNormal, inputDir );		
		}
		return ent;
	}
#ifdef sig_devmenu
	void tEntity::fDumpChildrenToOutput( u32 depth ) const
	{
		std::stringstream tabs, out;
		for( u32 i = 0; i < depth; ++i )
			tabs << "\t";

		out << tabs.str( ) << fNameCStr( ) << "[";
		tSceneRefEntity* refEnt = fDynamicCast<tSceneRefEntity>( );
		if( refEnt )
			out << refEnt->fSgResource( )->fAbsolutePhysicalPath( ).fCStr( );
		out << "]";

		log_line( 0, out.str( ) );
		for( u32 i = 0; i < mChildren.fCount( ); ++i )
		{
			if( mChildren[ i ] )
				mChildren[ i ]->fDumpChildrenToOutput( depth + 1 );
			else
				log_line( 0, tabs.str( ) << "NULL" );
		}
	}
#endif//sig_devmenu
	void tEntity::fAcquirePropertiesFromAncestors( )
	{
		for( tEntity* ancestor = fParent( ); ancestor; ancestor = ancestor->fParent( ) )
			fAcquirePropertiesFromEntity( *ancestor );
	}
	void tEntity::fAcquirePropertiesFromEntity( const tEntity& copyFrom )
	{
		mGameTags |= copyFrom.mGameTags;
		for( u32 i = 0; i < copyFrom.mEnumProps.fCount( ); ++i )
			mEnumProps.fFindOrAdd( copyFrom.mEnumProps[ i ] );
	}
	const char* tEntity::fSubNameCStr( ) const
	{
		const char* i = mName.fCStr( );
		for( ; i && *i; ++i )
			if( *i == ':' )
				return i + 1;
		return i;
	}
	const char* tEntity::fDebugSgResourcePath( ) const
	{
		const tSceneRefEntity* const sgref = this->fFirstAncestorOfType< tSceneRefEntity >( );
		return sgref ? sgref->fResourcePath( ).fCStr( ) : "";
	}
	b32 tEntity::fHasGameTagsAnyInherited( tEntityTagMask tags  ) const
	{
		for( const tEntity* ancestor = this; ancestor; ancestor = ancestor->fParent( ) )
			if( ancestor->fHasGameTagsAny( tags ) )
				return true;
		return false;
	}
	b32 tEntity::fHasGameTagsAllInherited( tEntityTagMask tags ) const
	{
		for( const tEntity* ancestor = this; ancestor; ancestor = ancestor->fParent( ) )
			if( ancestor->fHasGameTagsAll( tags ) )
				return true;
		return false;
	}
	void tEntity::fSetEnumValue( const tEntityEnumProperty& enumProp )
	{
		for( u32 i = 0; i < mEnumProps.fCount( ); ++i )
		{
			if( mEnumProps[ i ].mEnumKey == enumProp.mEnumKey )
			{
				mEnumProps[ i ].mEnumValue = enumProp.mEnumValue;
				return;
			}
		}
		mEnumProps.fPushBack( enumProp );
	}
	void tEntity::fSetEnumValueFromEntity( u32 key, const tEntity* entity )
	{
		const u32 val = entity->fQueryEnumValue( key );
		fSetEnumValue( tEntityEnumProperty( key, val ) );
	}
	void tEntity::fRemoveEnumProperty( u32 enumKey )
	{
		for( u32 i = 0; i < mEnumProps.fCount( ); ++i )
		{
			if( mEnumProps[ i ].mEnumKey == enumKey )
			{
				mEnumProps.fErase( i );
				return;
			}
		}
	}
	u32 tEntity::fQueryEnumValue( u32 enumKey, u32 enumValueDefault ) const
	{
		for( u32 i = 0; i < mEnumProps.fCount( ); ++i )
			if( mEnumProps[ i ].mEnumKey == enumKey )
				return mEnumProps[ i ].mEnumValue;
		return enumValueDefault;
	}
	u32 tEntity::fQueryEnumValueInherited( u32 enumKey, u32 enumValueDefault ) const
	{
		for( const tEntity* ancestor = this; ancestor; ancestor = ancestor->fParent( ) )
		{
			const u32 v = ancestor->fQueryEnumValue( enumKey, ~0 );
			if( v != ~0 )
				return v;
		}
		return enumValueDefault;
	}
	void tEntity::fPreserve( )
	{
		if( fSceneGraph( ) )
			fSceneGraph( )->fPreserve( *this );
	}
	void tEntity::fSpawn( tEntity& parent )
	{
		if( parent.fSceneGraph( ) )
			parent.fSceneGraph( )->fSpawn( *this, parent );
		else
			fSpawnImmediate( parent );
	}
	void tEntity::fDelete( )
	{
		if( mInSpawnList )
		{
			mDeleteAfterSpawn = true;
		}
		else
		{
			if( fSceneGraph( ) )
				fSceneGraph( )->fDelete( *this );
			else
				fDeleteImmediate( );
		}
	}
	b32 tEntity::fInDeletionList( ) const
	{
		return !fSceneGraph( ) || fSceneGraph( )->fInDeletionList( *this );
	}
	void tEntity::fSpawnImmediate( tEntity& parent )
	{
		// hook child into scene graph
		parent.fAddChild( *this );

		// notify child entity that it's been spawned
		if( fSceneGraph( ) )
			fOnSpawn( );

		// make sure we start at the right spot
		if( mLockedToParent )
			fMoveTo( parent.fObjectToWorld( ) * fParentRelative( ) );
	}
	void tEntity::fDeleteImmediate( )
	{
		if( fSceneGraph( ) )
		{
			fOnDelete( );
			fSceneGraph( )->fRemove( *this );
		}
		if( mParent )
			mParent->fRemoveChild( *this );
	}
	void tEntity::fReparent( tEntity& newParent )
	{
		newParent.fAddChild( *this );

		if( mLockedToParent )
			fRecomputeParentRelative( );
	}
	void tEntity::fTransferChildren( tEntity& newParent )
	{
		while( mChildren.fCount( ) > 0 )
			newParent.fAddChild( *mChildren.fFront( ) );
	}
	void tEntity::fClearChildren( )
	{
		while( mChildren.fCount( ) > 0 )
			mChildren.fFront( )->fDeleteImmediate( );
	}
	void tEntity::fPropagateSkeleton( Anim::tAnimatedSkeleton& skeleton )
	{
		const Logic::tAnimatable* animatable = fLogic( ) ? fLogic( )->fQueryAnimatable( ) : 0;
		if( !animatable || animatable->fShouldSkeletonPropagate( skeleton ) )
		{
			// recurse on children
			for( u32 i = 0; i < fChildCount( ); ++i )
				fChild( i )->fPropagateSkeleton( skeleton );
		}
	}
	void tEntity::fPropagateSkeletonInternal( Anim::tAnimatedSkeleton& skeleton, const tEntityDefProperties* entityDef )
	{
		if( entityDef )
			entityDef->fAddBoneProxy( *this, skeleton );
		tEntity::fPropagateSkeleton( skeleton );
	}
	void tEntity::fRemoveFromSceneGraph( )
	{
		if( fSceneGraph( ) )
			fSceneGraph( )->fRemove( *this );
	}
	void tEntity::fRemoveFromRunLists( )
	{
		tLogic::fRemoveFromRunLists( );
		tLogic* logic = fLogic( );
		if( logic ) 
			logic->fRemoveFromRunLists( );
		mComponents.fRemoveFromRunLists( );
	}
	void tEntity::fSetSceneGraph( tSceneGraph* sg )
	{
		tLogic::fSetSceneGraph( sg );
		tLogic* logic = fLogic( );
		if( logic ) 
			logic->fSetSceneGraph( sg );
		mComponents.fSetSceneGraph( sg );
	}
	void tEntity::fOnSpawn( )
	{
		sigassert( mSceneGraph );

		tLogic::fOnSpawn( );
		for( u32 i = 0; i < fChildCount( ); ++i )
			fChild( i )->fOnSpawn( );

		profile( cProfilePerfOnSpawn );
		if( !mController.fIsNull( ) )
		{
			mController.fOnSpawn( );
		}

		mComponents.fOnSpawn( );
	}
	void tEntity::fOnDelete( )
	{
		sigassert( mSceneGraph );

		if( !mController.fIsNull( ) )
			mController.fOnDelete( );

		mComponents.fOnDelete( );
		
		if( mInSpawnList )
			mSceneGraph->mSpawnList.fFindAndErase( this );

		tLogic::fOnDelete( );
		for( u32 i = 0; i < fChildCount( ); ++i )
		{
			tEntity* child = fChild( i ).fGetRawPtr( );
			if( child->fReadyForDeletion( ) )
				child->fOnDelete( );
			else
			{
				const u32 childCount0 = fChildCount( );

				child->fOnParentDelete( );

				const u32 childCount1 = fChildCount( );

				if( childCount0 != childCount1 )
					--i; // only go backward if child was removed from array of children
			}
		}

        mWeakPtrHead.fUnlinkAll( );
	}
	void tEntity::fOnParentDelete( )
	{
		fSceneGraph( )->fRootEntity( ).fAddChild( *this );
	}
	void tEntity::fOnPause( b32 paused )
	{
		tLogic* logic = fLogic( );
		if( logic )
			logic->fOnPause( paused );
		mComponents.fOnPause( paused );

		tLogic::fOnPause( paused );
		for( u32 i = 0; i < fChildCount( ); ++i )
			fChild( i )->fOnPause( paused );
	}
	b32 tEntity::fReadyForDeletion( )
	{
		tLogic* logic = fLogic( );
		return logic ? logic->fReadyForDeletion( ) : tLogic::fReadyForDeletion( );
	}
	void tEntity::fAddChild( tEntity& child )
	{
		sigassert( this != &child );

		tEntityPtr childPtr( &child );

		if( child.mParent )
			child.mParent->mChildren.fFindAndErase( childPtr );

		mChildren.fPushBack( childPtr );

		if( fSceneGraph( ) )
			fSceneGraph( )->fInsert( child );
		else
			child.fRemoveFromSceneGraph( );

		child.mParent = this;
	}
	void tEntity::fRemoveChild( tEntity& child )
	{
		sigassert( child.mParent == this );

		tEntityPtr childPtr( &child );
		child.mParent = 0;
		mChildren.fFindAndErase( childPtr );

		if( mChildren.fCount( ) == 0 )
			fOnEmptyNest( );
	}
	b32 tEntity::fHandleLogicEvent( const Logic::tEvent& e )
	{
		if( fLogic( ) && fLogic( )->fHandleLogicEvent( e ) )
			return true;

		for( u32 i = 0; i < mComponents.fCount( ); ++i )
			if( mComponents[ i ].fCodeObject( )->fHandleLogicEvent( e ) )
				return true;

		return false;
	}
}

//--------------------------------------------------------------------------------------------------------------
//
//    Moveable Reference Frame Implementation
//
//--------------------------------------------------------------------------------------------------------------

namespace Sig
{
	namespace
	{
		static void fCombinedObjectSpaceBoxRecursive( const tEntity& ent, const Math::tMat3f& toObjectSpace, Math::tAabbf& box )
		{
			tSpatialEntity* spatialEnt = ent.fDynamicCast<tSpatialEntity>( );
			if( spatialEnt )
			{
				sigassert( spatialEnt->fObjectSpaceBox( ).fIsValid( ) );
				box |= spatialEnt->fObjectSpaceBox( ).fTransform( toObjectSpace * ent.fObjectToWorld( ) );
			}
			else if( ent.fIsHelper( ) )
			{
				const Math::tVec3f p = toObjectSpace.fXformPoint( ent.fObjectToWorld( ).fGetTranslation( ) );
				box |= ( p + 0.01f );
				box |= ( p - 0.01f );
			}

			for( u32 i = 0; i < ent.fChildCount( ); ++i )
				fCombinedObjectSpaceBoxRecursive( *ent.fChild( i ), toObjectSpace, box );
		}
	}
	void tEntity::fMoveTo( const Math::tVec3f& newPos )
	{
		mObjectToWorld.fSetTranslation( newPos );
		fOnMoved( true );
	}
	void tEntity::fMoveTo( const Math::tMat3f& newXform )
	{
		mObjectToWorld = newXform;
		fOnMoved( true );
	}
	void tEntity::fSetParentRelativeXform( const Math::tMat3f& newXform )
	{
		mParentRelative = newXform;
		mObjectToWorld = mParent ? ( mParent->fObjectToWorld( ) * newXform ) : newXform;
		fOnMoved( false );
	}
	void tEntity::fTranslate( const Math::tVec3f& delta )
	{
		fMoveTo( fObjectToWorld( ).fGetTranslation( ) + delta );
	}
	void tEntity::fTransform( const Math::tMat3f& xform )
	{
		mObjectToWorld = xform * mObjectToWorld;
		fOnMoved( true );
	}
	const Math::tMat3f& tEntity::fWorldToObject( ) const
	{
		// Attempt access to the lock if it's dirty
		switch( interlocked_cmp_ex( &mWorldToObjectDirty, 2u, 1u ) )
		{
			// The xform was dirty and we got the lock
			// so update the tranform and clear the dirty flag
		case 1:
			{
				mWorldToObject = mObjectToWorld.fInverse( );
				interlocked_ex( &mWorldToObjectDirty, 0u );
			}
			break;

			// The xform was dirty and currently being updated
			// so spin on the volatile variable until it's cleared
		case 2:
			while( mWorldToObjectDirty );
			break;
		}

		// Either the xform was not dirty or it's been updated
		return mWorldToObject;
	}
	Math::tAabbf tEntity::fCombinedObjectSpaceBox( ) const
	{
		Math::tAabbf box;
		box.fInvalidate( );
		fCombinedObjectSpaceBoxRecursive( *this, fWorldToObject( ), box );
		return box;
	}
	Math::tAabbf tEntity::fCombinedWorldSpaceBox( ) const
	{
		return fCombinedObjectSpaceBox( ).fTransform( fObjectToWorld( ) );
	}
	Math::tVec3f tEntity::fWorldSpaceCOM( ) const
	{
		return fObjectToWorld( ).fXformPoint( fObjectSpaceCOM( ) );
	}
	Math::tVec3f tEntity::fObjectSpaceCOM( ) const
	{
		return fCombinedObjectSpaceBox( ).fComputeCenter( );
	}
	void tEntity::fOnMoved( b32 recomputeParentRelative )
	{
		// update world to object
		interlocked_ex( &mWorldToObjectDirty, 1u );

		// recompute parent relative matrix
		if( recomputeParentRelative )
			fRecomputeParentRelative( );

		// tell all my children that i've moved
		for( u32 i = 0; i < fChildCount( ); ++i )
			fChild( i )->fOnParentMoved( mObjectToWorld );
	}
	void tEntity::fOnParentMoved( const Math::tMat3f& parentXform )
	{
		if( mLockedToParent )
		{
			mObjectToWorld = parentXform * fParentRelative( );
			fOnMoved( false );
		}
		else
			fRecomputeParentRelative( );
	}
	void tEntity::fRecomputeParentRelative( )
	{
		mParentRelative = mParent ? ( mParent->fWorldToObject( ) * mObjectToWorld ) : mObjectToWorld;
	}


	tEntity* tEntity::fClosestEntityNamed( const tGrowableArray<tEntityPtr>& array, const Math::tVec3f& toPoint, const tStringPtr& name, f32 maxDistSqr )
	{
		f32 minDist = maxDistSqr;
		tEntity* result = NULL;

		for( u32 i = 0; i < array.fCount( ); ++i )
		{
			if( array[ i ]->fName( ) == name )
			{
				f32 dist = (array[ i ]->fObjectToWorld( ).fGetTranslation( ) - toPoint).fLengthSquared( );
				if( dist < minDist )
				{
					minDist = dist;
					result = array[ i ].fGetRawPtr( );
				}
			}
		}

		return result;
	}

	tEntity* tEntity::fClosestEntity( const tGrowableArray<tEntityPtr>& array, const Math::tVec3f& toPoint, f32 maxDistSqr )
	{
		f32 minDist = maxDistSqr;
		tEntity* result = NULL;

		for( u32 i = 0; i < array.fCount( ); ++i )
		{
			f32 dist = (array[ i ]->fObjectToWorld( ).fGetTranslation( ) - toPoint).fLengthSquared( );
			if( dist < minDist )
			{
				minDist = dist;
				result = array[ i ].fGetRawPtr( );
			}
		}

		return result;
	}

}

//--------------------------------------------------------------------------------------------------------------
//
//    Script-Specific Implementation
//
//--------------------------------------------------------------------------------------------------------------
#include "tShapeEntity.hpp"

namespace Sig
{
	namespace
	{
		static const char* fToString( const tEntity* obj )
		{
			return obj->fName( ).fExists( ) ? obj->fNameCStr( ) : obj->fDebugTypeName( );
		}
		static b32 fIsEntityValid( tEntity* entity )
		{
			return entity != 0;
		}
		static Math::tVec3f fGetPosition( const tEntity* obj )
		{
			return obj->fObjectToWorld( ).fGetTranslation( );
		}
		static void fSetPosition( tEntity* obj, const Math::tVec3f& pos )
		{
			obj->fMoveTo( pos );
		}
		static Math::tMat3f fGetMatrix( const tEntity* obj )
		{
			return obj->fObjectToWorld( );
		}
		static f32 fDistance( const tEntity* a, const tEntity* b )
		{
			return a->fObjectToWorld( ).fGetTranslation( ).fDistance( b->fObjectToWorld( ).fGetTranslation( ) );
		}
		static void fTranslateParentRelative( tEntity* ent, f32 x, f32 y, f32 z )
		{
			Math::tMat3f mat = ent->fParentRelative( );
			mat.fTranslateGlobal( Math::tVec3f( x, y, z ) );
			ent->fSetParentRelativeXform( mat );
		}
		static tShapeEntity* fAsShapeEntity( const tEntity* obj )
		{
			return obj->fDynamicCast< tShapeEntity >( );
		}
		static tEntity* fFirstChildWithName( tEntity* entity, const tStringPtr& name )
		{
			tEntity* child = entity->fFirstDescendentWithName( name );
			//if( !child )
			//	log_warning( "No child found in fFirstChildWithName with name: " << name );
			return child;
		}
		static void fForEachChild( tEntity* entity, Sqrat::Function func )
		{
			for( u32 i = 0; i < entity->fChildCount( ); ++i )
				func.Execute( entity->fChild( i ).fGetRawPtr( ) );
		}
		static void fForEachChildWithName( tEntity* entity, Sqrat::Function func, const tStringPtr& name )
		{
			for( u32 i = 0; i < entity->fChildCount( ); ++i )
				if( entity->fChild( i )->fName( ) == name )
					func.Execute( entity->fChild( i ).fGetRawPtr( ) );
		}
		static void fForEachChildWithNameRecursive( tEntity* entity, Sqrat::Function func, const tStringPtr& name )
		{
			for( u32 i = 0; i < entity->fChildCount( ); ++i )
			{
				if( entity->fChild( i )->fName( ) == name )
					func.Execute( entity->fChild( i ).fGetRawPtr( ) );

				fForEachChildWithNameRecursive( entity->fChild( i ).fGetRawPtr( ), func, name );
			}
		}
		static void fReparentForScript( tEntity* entity, tEntity* newParent )
		{
			sigassert( entity );
			sigassert( newParent );
			entity->fReparent( *newParent );
		}
		static void fOffsetLikeParentChild( tEntity* parent, const tStringPtr& parentOffsetName, tEntity* child, const tStringPtr& childOffsetName )
		{
			sigassert( parent && child );
			tEntity* parentOffset = fFirstChildWithName( parent, parentOffsetName );
			tEntity* childOffset = fFirstChildWithName( child, childOffsetName );
			sigassert( parentOffset && childOffset );

			//MATH:
			// A * Ac = B * Bc
			// i.e.  we want the child of A to equal the child of B
			//  Ac = parent relative of child entity of A
			// now solve by multiplying both sides by Bc^-1 (the inverse of the parent relative matrix)
			// A * Ac * Bc^-1 = B

			// note: A * Ac = A child's object to world

			child->fMoveTo( parentOffset->fObjectToWorld( ) * childOffset->fParentRelative( ).fInverse( ) );
		}
		Gfx::tLightEntity* fSpawnLight( tEntity* parent )
		{
			sigassert( parent );
			const char* debugName = "$ScriptLight";
			Gfx::tLightEntityPtr light( NEW Gfx::tLightEntity( Math::tMat3f::cIdentity, Gfx::tLight( ), debugName ) );
			light->fSpawn( *parent );
			return light.fGetRawPtr( );
		}
		void fHide( tEntity* e )
		{
			Gfx::tRenderableEntity::fSetInvisible( *e, true );
		}
		void fShow( tEntity* e )
		{
			Gfx::tRenderableEntity::fSetInvisible( *e, false );
		}

		class tScriptEntityWeakPtr
		{
			tEntityWeakPtr mEntity;
		public:
			tEntity* fGetEntity( )
			{
				sigassert( mEntity.fObject( ) );
				return mEntity.fObject( );
			}
			void fSetEntity( tEntity* e )
			{
				mEntity.fReset( e );
			}
			bool fIsNull( ) const
			{
				return mEntity.fObject( ) == NULL;
			}
		public:
			static void fExportScriptInterface( tScriptVm& vm )
			{
				Sqrat::Class<tScriptEntityWeakPtr> classDesc( vm.fSq( ) );
				classDesc
					.Prop(_SC("Entity"),&fGetEntity,&fSetEntity)
					.Func(_SC("IsNull"),&fIsNull)
					;
				vm.fRootTable( ).Bind(_SC("EntityWeakPtr"), classDesc);
			}
		};
	}

	void tEntity::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tEntity, tLogic, Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("_tostring"),	&fToString)
			.StaticFunc(_SC("IsValid"),		&fIsEntityValid)
			.GlobalFunc(_SC("GetPosition"),	&fGetPosition)
			.GlobalFunc(_SC("SetPosition"),	&fSetPosition)
			.GlobalFunc(_SC("GetMatrix"),	&fGetMatrix)
			.GlobalFunc(_SC("Distance"),	&fDistance)
			.GlobalFunc(_SC("AsShapeEntity"),		&fAsShapeEntity)
			.GlobalFunc(_SC("TranslateParentRelative"), &fTranslateParentRelative)
			.GlobalFunc(_SC("OffsetLikeParentChild"), &fOffsetLikeParentChild)
			.Prop(_SC("Logic"),				&tEntity::fScriptLogicObject, &tEntity::fSetScriptLogicObject)
			.Func(_SC("SetName"),			&tEntity::fSetName)
			.Func(_SC("GetName"),			&tEntity::fNameCStr)
			.Func(_SC("GetNameStr"),		&tEntity::fName)
			.Func(_SC("GetSubName"),		&tEntity::fSubNameCStr)
			.Func(_SC("AddGameTags"),		&tEntity::fAddGameTags)
			.Func(_SC("AddGameTagsRecursive"),		&tEntity::fAddGameTagsRecursive)
			.Func(_SC("HasGameTag"),		&tEntity::fHasGameTagsAny)
			.Func(_SC("HasAnyGameTag"),		&tEntity::fHasGameTagsAny)
			.Func(_SC("HasAllGameTags"),	&tEntity::fHasGameTagsAll)
            .Func(_SC("RemoveGameTagsRecursive"), &tEntity::fRemoveGameTagsRecursive)
			.Overload<u32 (tEntity::*)(u32) const>(_SC("GetEnumValue"),		&tEntity::fQueryEnumValue)
			.Overload<u32 (tEntity::*)(u32,u32) const>(_SC("GetEnumValue"), &tEntity::fQueryEnumValue)
			.Func(_SC("SetEnumValue"),		&tEntity::fSetEnumValueForScript)
			.GlobalFunc(_SC("FirstChildWithName"),		&fFirstChildWithName)
			.GlobalFunc(_SC("ForEachChild"),			&fForEachChild)
			.GlobalFunc(_SC("ForEachChildWithName"),	&fForEachChildWithName)
			.GlobalFunc(_SC("ForEachChildWithNameRecursive"), &fForEachChildWithNameRecursive)
			.GlobalFunc(_SC("SpawnLight"), &fSpawnLight)
			.Func(_SC("SpawnChild"),		&tEntity::fSpawnChild)
			.Func(_SC("SpawnChildFromProxy"),&tEntity::fSpawnChildFromProxy)
			.Func(_SC("SpawnFxChild"),		&tEntity::fSpawnFxChild)
			.Func(_SC("Delete"),			&tEntity::fDelete)
			.Func(_SC("DeleteImmediate"),	&tEntity::fDeleteImmediate)
			.Func(_SC("ClearChildren"),		&tEntity::fClearChildren)
			.GlobalFunc(_SC("Reparent"),	&fReparentForScript)
			.Prop(_SC("ChildCount"),		&tEntity::fChildCount)
			.Prop(_SC("Parent"),			&tEntity::fParent)
			.Prop(_SC("LockedToParent"), &tEntity::fLockedToParent, &tEntity::fSetLockedToParent )
			.Prop(_SC("CombinedWorldSpaceBox"), &tEntity::fCombinedWorldSpaceBox )
			.Prop(_SC("CombinedObjectSpaceBox"), &tEntity::fCombinedObjectSpaceBox )
            .Func(_SC("AttachChild"), &tEntity::fAttachChild )
			.Func<void (tEntity::*)(const Math::tVec3f&)>(_SC("MoveTo"), &tEntity::fMoveTo)
			.Func<void (tEntity::*)(const Math::tMat3f&)>(_SC("MoveToXForm"), &tEntity::fMoveTo)
			.GlobalFunc(_SC("Hide"), &fHide)
			.GlobalFunc(_SC("Show"), &fShow)
			;

		vm.fRootTable( ).Bind( _SC("Entity"), classDesc );

		tScriptEntityWeakPtr::fExportScriptInterface( vm );
	}

	Sqrat::Object tEntity::fScriptLogicObject( ) const
	{
		return !mController.fIsNull( ) ? mController.fScriptObject( ) : Sqrat::Object( );
	}

	void tEntity::fSetScriptLogicObject( const Sqrat::Object& obj )
	{
		fAcquireLogic( tLogicPtr( obj ) );
	}

	void tEntity::fAcquireLogic( const tLogicPtr& logicPtr )
	{
		fAcquireLogicInternal( mController, logicPtr );
	}

	void tEntity::fAcquireLogicInternal( tLogicPtr& dst, const tLogicPtr& src )
	{
		// delete existing controller
		if( !dst.fIsNull( ) )
		{
			dst.fOnDelete( );

			// clear myself on the previously bound logic
			tLogic* logic = dst.fCodeObject( );
			if( logic )
				logic->fSetOwnerEntity( NULL );
		}

		// create new controller
		dst = src;

		// set myself on the newly bound controller
		if( !dst.fIsNull( ) )
		{
			if( tLogic* logic = dst.fCodeObject( ) )
			{
				logic->fSetOwnerEntity( this );
				logic->fSetSceneGraph(  fSceneGraph( ) );
			}

			if( fSceneGraph( ) )
				dst.fOnSpawn( );
		}
	}

	void tEntity::fComputeDebugText( std::string& textOut ) const
	{
#ifdef sig_devmenu
		std::stringstream ss;

		tLogic* logic = fLogic( );
		if( fLogic( ) )
			ss << fLogic( )->fDebugTypeName( ) << std::endl;
		else
			ss << "NoLogic" << std::endl;

		if( fName( ).fLength( ) )
			ss << "Name: " << fNameCStr( ) << std::endl;

		tSceneRefEntity* sceneRef = fDynamicCast< tSceneRefEntity >( );
		if( sceneRef )
			ss << "SceneRef: " << sceneRef->fSgResource( )->fGetPath( ).fCStr( ) << std::endl;

		if( logic )
		{
			Logic::tAnimatable* animatable = logic->fQueryAnimatable( );
			if( animatable )
				animatable->fAddWorldDebugText( ss );

			for( u32 i = 0; i < mComponents.fCount( ); ++i )
			{
				sigassert( mComponents[ i ].fCodeObject( ) );
				mComponents[ i ].fCodeObject( )->fAddWorldDebugText( ss );
			}

			logic->fAddWorldDebugText( ss );
		}

		textOut = ss.str( );
#endif//sig_devmenu
	}

}

