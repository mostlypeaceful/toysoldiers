#include "ToolsPch.hpp"
#include "tSigFxMeshSystem.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Fxml.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "tSigFxParticleSystem.hpp"
#include "tResourceDepot.hpp"

#include "Gfx/tDefaultAllocators.hpp"

namespace Sig
{
	tSigFxMeshSystem::tSigFxMeshSystem( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fCommonCtor( *container.fGetResourceDepot( ) );
	}
	
	tSigFxMeshSystem::tSigFxMeshSystem( tEditableObjectContainer& container, const Fxml::tFxMeshSystemObject& fxmso )
		: tEditableObject( container )
	{
		fDeserializeBaseObject( &fxmso );
		fCommonCtor( *container.fGetResourceDepot( ) );
		
		mFxMeshSystem->fSetData( FX::FxMeshSystem::tMeshSystemDataPtr( new FX::FxMeshSystem::tData( fxmso.mFxMeshSystemData.fGetRawPtr( ) ) ) );
		fSetParticleSystemToSyncWith( fxmso.mParticleSystemToSyncWith );
		fSetMeshResourceFile( fxmso.mMeshResourceFile, true );

		fSetMeshSystemName( fxmso.mFxMeshSystemName );
	}

	void tSigFxMeshSystem::fClone( const tSigFxMeshSystem& meshSystem )
	{
		FX::tMeshSystemPtr data = meshSystem.fFxMeshSystem( );
		mFxMeshSystem->fSetData( FX::FxMeshSystem::tMeshSystemDataPtr( new FX::FxMeshSystem::tData( data->fFxMeshSystemData( ).fGetRawPtr( ) ) ) );
		fSetParticleSystemToSyncWith( meshSystem.fParticleSystemToSyncWith( ) );

		std::string cloneName = meshSystem.fFxMeshSystemName( ).fCStr( );
		cloneName += " Clone";
		mFxMeshSystem->fSetMeshSystemName( tStringPtr( cloneName ) );
	}

	tSigFxMeshSystem::~tSigFxMeshSystem( )
	{
		
	}

	void tSigFxMeshSystem::fCommonCtor( tResourceDepot& resDep )
	{
		mFxMeshSystem.fReset( new FX::tMeshSystem( ) );

		//mDummySphere->fSetRgbaTint( Math::tVec4f( 0.65f, 0.75f, 0.85f, 0.3f ) );
		mFxMeshSystem->fSpawnImmediate( *this );

		mHidden = false;
		mLastOpenGraphIdx = 0;
	}

	void tSigFxMeshSystem::fOnSpawn( )
	{
		fOnPause( false );
		tEditableObject::fOnSpawn( );
	}

	void tSigFxMeshSystem::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListThinkST );
		}
		else
		{
			fRunListInsert( cRunListThinkST );
		}
		tEditableObject::fOnPause( paused );
	}

	void tSigFxMeshSystem::fThinkST( f32 dt )
	{
		f32 scale = 1.f;
		Math::tAabbf bounds( -scale, scale );
		fSetLocalSpaceMinMax( bounds.mMin, bounds.mMax );

		Math::tMat3f xform( scale * fObjectToWorld( ).fGetScale( ) );
		
		if( mFxMeshSystem->fMeshCount( ) == 0 )
		{
			mDummySphere->fSetRgbaTint( Math::tVec4f( 1.f, 0.5f, 0.34f, 0.3f ) );
		}
		else
		{
			mDummySphere->fSetRgbaTint( Math::tVec4f( 1.f, 0.5f, 0.34f, 0.f ) );
		}

		//mDummySphere->fSetInvisible( mHidden );
		//mDummySphere->fMoveTo( xform );
		//mSelectionBox->fMoveTo( xform );
	}

	void tSigFxMeshSystem::fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max )
	{
		tEditableObject::fSetLocalSpaceMinMax( min, max );
		mDummySphere->fSetObjectSpaceBox( fObjectSpaceBox( ) );
		mDummySphere->fSetParentRelativeXform( fObjectSpaceBox( ).fAdjustMatrix( Math::tMat3f::cIdentity, 0.25f ) );
	}

	void tSigFxMeshSystem::fSetMeshResourceFile( const tFilePathPtr& meshFile, b32 nameOnly )
	{
		mFxMeshSystem->fSetMeshResourceFile( meshFile, nameOnly );

		Gfx::tDefaultAllocators& defFxAllocators = Gfx::tDefaultAllocators::fInstance( );
		tResourcePtr meshResource = defFxAllocators.mResourceDepot->fQuery( tResourceId::fMake< tSceneGraphFile >( meshFile ) );
		if( meshResource->fLoading( ) )
			meshResource->fBlockUntilLoaded( );
	}

	const tFilePathPtr& tSigFxMeshSystem::fMeshResourceFile( ) const
	{
		return mFxMeshSystem->fMeshResourceFile( );
	}

	void tSigFxMeshSystem::fSyncWithSystem( )
	{
		tGrowableArray< tSigFxParticleSystem* > systems;
		fGetContainer( ).fCollectAllByType< tSigFxParticleSystem >( systems );

		for( u32 i = 0; i < systems.fCount( ); ++i )
		{
			const tStringPtr& syncWith = fParticleSystemToSyncWith( );
			const tStringPtr& matchWith = systems[ i ]->fParticleSystemName( );

			if( syncWith == matchWith )
			{
				mFxMeshSystem->fSetRandomSeed( systems[ i ]->fGetParticleSystem( )->fRandomSeed( ) );
			}
		}
	}


	Sigml::tObjectPtr tSigFxMeshSystem::fSerialize( b32 clone ) const
	{
		Fxml::tFxMeshSystemObject* fxmso = new Fxml::tFxMeshSystemObject( );
		fSerializeBaseObject( fxmso, clone );
		fxmso->mName = fFxMeshSystemName( ).fCStr( );
		fxmso->mFxMeshSystemName = fFxMeshSystemName( );
		fxmso->mParticleSystemToSyncWith = fParticleSystemToSyncWith( );
		fxmso->mMeshResourceFile = fMeshResourceFile( );
		fxmso->mFxMeshSystemData.fReset( mFxMeshSystem->fFxMeshSystemData( ).fGetRawPtr( ) );
		return Sigml::tObjectPtr( fxmso );
	}
}

