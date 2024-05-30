#include "BasePch.hpp"
#include "tEntityDef.hpp"
#include "tEntity.hpp"
#include "tAnimatedSkeleton.hpp"
#include "tReferenceFrameEntity.hpp"
#include "Scripts/tScriptFile.hpp"

namespace Sig
{

	///
	/// \section tEntityDefProperties
	///

	tEntityDefProperties::tEntityDefProperties( )
		: mName( 0 )
		, mScriptFile( 0 )
		, mOnEntityCreateOverride( 0 )
		, mSkeletonFile( 0 )
		, mBoneAttachment( 0 )
	{
	}

	tEntityDefProperties::tEntityDefProperties( tNoOpTag )
		: mCreationFlags( cNoOpTag )
		, mTagProperties( cNoOpTag )
		, mEnumProperties( cNoOpTag )
	{
	}
	void tEntityDefProperties::fApplyProperties( tEntity& entity, const tEntityCreationFlags& creationFlags ) const
	{
		entity.fApplyCreationFlags( creationFlags );

		// the name property overrides...
		if( entity.fName( ).fLength( ) == 0 )
		{
			if( mName )
				entity.fSetName( mName->fGetStringPtr( ) );
		}

		// TODO REFACTOR 
		// while keeping each tag as an individual u32 bit index in an array is admirable from an expansion point of view, 
		// it seems a little pointless from a realistic perspective (read: performance, just use a bit mask (u64))

		for( u32 i = 0; i < mTagProperties.fCount( ); ++i )
			entity.fAddGameTags( 1u << mTagProperties[ i ] );
		for( u32 i = 0; i < mEnumProperties.fCount( ); ++i )
			entity.fSetEnumValue( tEntityEnumProperty( mEnumProperties[ i ].mEnumKey, mEnumProperties[ i ].mEnumValue ) );
	}
	void tEntityDefProperties::fApplyProperties( tEntity& entity, const tEntityDefProperties& override, const tEntityCreationFlags& creationFlags ) const
	{
		entity.fApplyCreationFlags( creationFlags );

		// the name property overrides...
		if( entity.fName( ).fLength( ) == 0 )
		{
			if( override.mName )
				entity.fSetName( override.mName->fGetStringPtr( ) );
			else if( mName )
				entity.fSetName( mName->fGetStringPtr( ) );
		}

		// TODO REFACTOR 
		// while keeping each tag as an individual u32 bit index in an array is admirable from an expansion point of view, 
		// it seems a little pointless from a realistic perspective (read: performance, just use a bit mask (u64))

		// .. whereas tags and enum properties combine (however an enum from 'override' will still override if a conflict)
		for( u32 i = 0; i < mTagProperties.fCount( ); ++i )
			entity.fAddGameTags( 1u << mTagProperties[ i ] );
		for( u32 i = 0; i < mEnumProperties.fCount( ); ++i )
			entity.fSetEnumValue( tEntityEnumProperty( mEnumProperties[ i ].mEnumKey, mEnumProperties[ i ].mEnumValue ) );
		for( u32 i = 0; i < override.mTagProperties.fCount( ); ++i )
			entity.fAddGameTags( 1u << override.mTagProperties[ i ] );
		for( u32 i = 0; i < override.mEnumProperties.fCount( ); ++i )
			entity.fSetEnumValue( tEntityEnumProperty( override.mEnumProperties[ i ].mEnumKey, override.mEnumProperties[ i ].mEnumValue ) );
	}
	namespace { inline void fInvokeEntityCreateFunction( tEntity& entity, Sqrat::Function scriptCb ) { if( !scriptCb.IsNull( ) ) scriptCb.Execute( &entity ); } }
	b32 tEntityDefProperties::fEntityOnCreate( tEntity& entity ) const
	{
#if defined( target_game )
		if( mOnEntityCreateOverride )
		{
			tScriptVm& vm = tScriptVm::fInstance( );
			const tStringPtr& script = mOnEntityCreateOverride->fGetStringPtr( );

			if( mCreationFlags.mCreateFlags & tEntityCreationFlags::cFlagImmediateScript )
			{
				vm.fRootTable( ).SetInstance(_SC("self"), &entity);
				vm.fCompileStringAndRun( script.fCStr( ), "" );
			}
			else
				fInvokeEntityCreateFunction( entity, vm.fRootTable( ).GetFunction( script.fCStr( ) ) );

			return true; // true bcz fEntityOnCreate was handled
		}
		else if( mScriptFile )
		{
			fInvokeEntityCreateFunction( entity, mScriptFile->fGetResourcePtr( )->fCast< tScriptFile >( )->fIndexStandardExportedFunction( tScriptFile::cEntityOnCreated ) );
			return true; // true bcz fEntityOnCreate was handled
		}
#endif//defined( target_game )
		return false; // false bcz fEntityOnCreate was NOT handled
	}
	void tEntityDefProperties::fEntityOnCreate( tEntity& entity, const tEntityDefProperties& override ) const
	{
		// first give priority to the override, but if it doesn't handle it then fall back to me
		if( !override.fEntityOnCreate( entity ) )
			fEntityOnCreate( entity );
	}
	void tEntityDefProperties::fEntityOnChildrenCreate( tEntity& entity ) const
	{
		if( mScriptFile )
			fInvokeEntityCreateFunction( entity, mScriptFile->fGetResourcePtr( )->fCast< tScriptFile >( )->fIndexStandardExportedFunction( tScriptFile::cEntityOnChildrenCreated ) );
	}
	void tEntityDefProperties::fEntityOnChildrenCreate( tEntity& entity, const tEntityDefProperties& override ) const
	{
		if( override.mScriptFile )
			override.fEntityOnChildrenCreate( entity );
		else
			fEntityOnChildrenCreate( entity );
	}
	void tEntityDefProperties::fEntityOnSiblingsCreate( tEntity& entity ) const
	{
		if( mScriptFile )
			fInvokeEntityCreateFunction( entity, mScriptFile->fGetResourcePtr( )->fCast< tScriptFile >( )->fIndexStandardExportedFunction( tScriptFile::cEntityOnSiblingsCreated ) );
	}
	void tEntityDefProperties::fEntityOnSiblingsCreate( tEntity& entity, const tEntityDefProperties& override ) const
	{
		if( override.mScriptFile )
			override.fEntityOnSiblingsCreate( entity );
		else
			fEntityOnSiblingsCreate( entity );
	}
	void tEntityDefProperties::fApplyPropsAndSpawnWithScript( tEntity& entity, tEntity& parent, const tEntityCreationFlags& creationFlags ) const
	{
		// apply properties/creation flags before invoking scripts:
		fApplyProperties( entity, creationFlags );

		// invoke script spawn callback if we have a script file
		fEntityOnCreate( entity );

		// insert into scene graph hierarchy (if parent is already in scene graph, this will be delayed)
		entity.fSpawn( parent );

		// invoke script spawn callback if we have a script file
		fEntityOnChildrenCreate( entity );
	}
	void tEntityDefProperties::fAddBoneProxy( tEntity& parent, tAnimatedSkeleton& skeleton ) const
	{
		if( mBoneAttachment )
			skeleton.fAddBoneProxy( parent, mBoneAttachment->fGetStringPtr( ) );
	}
	tEntity& tEntityDefProperties::fInsertReferenceFrame( tEntity& parent, const Math::tMat3f& objectToWorld ) const
	{
		tReferenceFrameEntity* entity = NEW tReferenceFrameEntity( this, objectToWorld );
		entity->fSpawn( parent );
		return *entity;
	}


	///
	/// \section tEntityDef
	///

	tEntityDef::tEntityDef( )
		: mObjectToLocal( Math::tMat3f::cIdentity )
		, mLocalToObject( Math::tMat3f::cIdentity )
	{
		mBounds.fInvalidate( );
	}

	tEntityDef::tEntityDef( tNoOpTag )
		: tEntityDefProperties( cNoOpTag )
		, mBounds( cNoOpTag )
		, mObjectToLocal( cNoOpTag )
		, mLocalToObject( cNoOpTag )
	{
	}

	tEntityDef::~tEntityDef( )
	{
	}

}

