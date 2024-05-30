#include "ToolsPch.hpp"
#include "tSigFxParticleSystem.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "Editor/tEditableObjectContainer.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "tShadeMaterialGen.hpp"

#include "FX/tQuadParticleList.hpp"
#include "FX/tMeshParticleList.hpp"

namespace Sig
{
	tSigFxParticleSystem::tSigFxParticleSystem( tEditableObjectContainer& container )
		: tEditableObject( container )
	{
		fCommonCtor( *container.fGetResourceDepot( ) );
		mParticleSystem->fSetLocalSpace( true );	// initial placement fx system needs to be in local space.
	}
	
	tSigFxParticleSystem::tSigFxParticleSystem( tEditableObjectContainer& container, const Fxml::tFxParticleSystemObject& fxo )
		: tEditableObject( container )
	{
		fDeserializeBaseObject( &fxo );

		tShadeMaterialGen* shadeMaterialGen = dynamic_cast< tShadeMaterialGen* >( fxo.mMaterial.fGetRawPtr( ) );
		if( shadeMaterialGen )
			mDermlMtlFile = shadeMaterialGen->mMtlFile;

		fCommonCtor( *container.fGetResourceDepot( ) );

		if( fxo.mToolState )
		{
			mToolState.fReset( NEW_TYPED( FX::tToolParticleSystemState )( *fxo.mToolState ) );
			mParticleSystem->fSetState( fxo.mToolState->fCreateBinaryState( ) );
		}
		else
		{
			sigassert( fxo.mStates.fCount( ) > 0 );
			
			// Convert old data to new system in an extremely complex and difficult process
			mToolState.fReset( NEW_TYPED( FX::tToolParticleSystemState )( fxo.mStates[ 0 ]->fToolState() ) );
		}

		fSetParticleSystemName( tStringPtr( fxo.mName ) );
		mParticleSystem->fSetLocalSpace( false );//fxo.mLocalSpace );	// after being placed, we can switch out of local space.
		mParticleSystem->fSetCameraDepthOffset( fxo.mCameraDepthOffset );
		mParticleSystem->fSetUpdateSpeedMultiplier( fxo.mUpdateSpeedMultiplier );
		mParticleSystem->fSetLodFactor( fxo.mLodFactor );
		mParticleSystem->fSetGhostParticleFrequency( fxo.mGhostParticleFrequency );
		mParticleSystem->fSetGhostParticleLifetime( fxo.mGhostParticleLifetime );
		mParticleSystem->fSetEmitterType( fxo.mEmitterType );
		mParticleSystem->fSetBlendOp( fxo.mBlendOp );
		mParticleSystem->fSetSrcBlend( fxo.mSrcBlend );
		mParticleSystem->fSetDstBlend( fxo.mDstBlend );
		mParticleSystem->fSetSortMode( fxo.mSortMode );

		if( fxo.mMeshResourcePath.fLength( ) > 0 )
			fSetMeshResourceFile( fxo.mMeshResourcePath );
	}

	void tSigFxParticleSystem::fClone( const tSigFxParticleSystem& system )
	{
		const FX::tParticleSystemPtr ps = system.fGetParticleSystem( );

		mToolState = FX::tToolParticleSystemStatePtr( NEW_TYPED( FX::tToolParticleSystemState )( *system.fGetToolState( ) ) );
		mParticleSystem->fClearAllStates( );
		mParticleSystem->fSetState( system.fGetToolState( )->fCreateBinaryState( ) );

		std::string cloneName = ps->fParticleSystemName( ).fCStr( );
		cloneName += " Clone";

		fSetParticleSystemName( tStringPtr( cloneName ) );
		mParticleSystem->fSetLocalSpace( ps->fLocalSpace( ) );
		mParticleSystem->fSetCameraDepthOffset( ps->fCameraDepthOffset( ) );
		mParticleSystem->fSetUpdateSpeedMultiplier( ps->fUpdateSpeedMultiplier( ) );
		mParticleSystem->fSetLodFactor( ps->fLodFactor( ) );
		mParticleSystem->fSetGhostParticleFrequency( ps->fGhostParticleFrequency( ) );
		mParticleSystem->fSetEmitterType( ps->fGetEmitterType( ) );
		mParticleSystem->fSetRenderState( ps->fRenderState( ) );
		mParticleSystem->fSetSortMode( ps->fSortMode( ) );

		if( system.mMeshResource )
			fSetMeshResourceFile( system.mMeshResource->fGetPath( ) );
	}

	tSigFxParticleSystem::~tSigFxParticleSystem( )
	{
	}

	void tSigFxParticleSystem::fCommonCtor( tResourceDepot& resDep )
	{
		mPreviewBundle.fReset( new tMaterialPreviewBundle( mContainer.fGetDevice( ), HlslGen::cVshFacingQuads ) );
		
		mMaterialFile = resDep.fQuery( tResourceId::fMake<Gfx::tMaterialFile>( Gfx::tParticleMaterial::fMaterialFilePath( ) ) );
		mMaterialFile->fLoadDefault( this );
		mMaterialFile->fBlockUntilLoaded( );

		if( mDermlMtlFile.mNodes.fCount( ) > 0 )
		{
			fGenerateShadersFromCurrentMtlFile( );
			mMaterial = mPreviewBundle->fMaterial( );
		}
		else
		{
			mMaterial.fReset( new Gfx::tParticleMaterial( mMaterialFile ) );

			/*
			const tFilePathPtr shaderPath = tFilePathPtr( "shaders\\particles\\standard.derml" );
			const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( shaderPath );
			Derml::tFile dermlFile;
			sigassert( dermlFile.fLoadXml( absolutePath ) );
			fChangeShader( dermlFile, shaderPath );
			mMaterial = mPreviewBundle->fMaterial( );
			*/
		}

		mToolState.fReset( NEW_TYPED( FX::tToolParticleSystemState )() );
		mParticleSystem.fReset( new FX::tParticleSystem( mMaterial, mToolState->fCreateBinaryState( ) ) );
		mParticleSystem->fSpawnImmediate( *this );

		mDummyBox->fSetRgbaTint( Math::tVec4f( 1.0f, 1.0f, 1.f, 0.f ) );
		mDummyBox->fSetInvisible( false );

		// non saved variables, editor related only
		mLastOpenGraphIdx = 0;

		for( u32 i = 0; i < mBubbles.fCount( ); ++i )
		{
			f32 x = tRandom::fSubjectiveRand( ).fFloatZeroToOne( );
			f32 y = tRandom::fSubjectiveRand( ).fFloatZeroToOne( );
			fSetBubble( i, x, y );	
		}
	}

	void tSigFxParticleSystem::fSetBubble( u32 idx, f32 i, f32 j )
	{
		const f32 u = i;
		const f32 v = j;

		const f32 theta = 2.f * Math::cPi * u;
		const f32 phi = std::acos( 2.f * v - 1 );
		const f32 r = tRandom::fSubjectiveRand( ).fFloatZeroToOne( ) * 0.15f + 0.20f;

		const f32 x = r * std::cos( theta ) * std::sin( phi );
		const f32 y = r * std::sin( theta ) * std::sin( phi );
		const f32 z = r * std::cos( phi );

		const f32 scale = tRandom::fSubjectiveRand( ).fFloatZeroToOne( ) * 0.1f + 0.03f;

		Math::tMat3f mat = Math::tMat3f::cIdentity;
		mat.fSetTranslation( mParticleSystem->fObjectToWorld( ).fGetTranslation( ) - Math::tVec3f( x, y, z ) );
		mat.fScaleGlobal( scale );

		mBubbles[ idx ].fReset( new tDummyObjectEntity( mContainer.fGetDummySphereTemplate( ).fGetRenderBatch( ), mContainer.fGetDummySphereTemplate( ).fGetBounds( ), true ) );
		mBubbles[ idx ]->fMoveTo( mat );
		mBubbles[ idx ]->fSpawnImmediate( *this );
	}

	void tSigFxParticleSystem::fOnSpawn( )
	{
		fRunListInsert( cRunListThinkST );
		tEditableObject::fOnSpawn( );
	}

	void tSigFxParticleSystem::fOnPause( b32 paused )
	{
		tEditableObject::fOnPause( paused );
	}

	void tSigFxParticleSystem::fThinkST( f32 dt )
	{
		// Sync the tool graphs to the actual particle system.
		fSyncStateToBinary( );

		Math::tAabbf bounds = mParticleSystem->fObjectSpaceBox( );
		fSetLocalSpaceMinMax( bounds.mMin, bounds.mMax );
		mDummyBox->fSetInvisible( mParticleSystem->fDisabled( ) );

		for( u32 i = 0; i < mBubbles.fCount( ); ++i )
		{
			if( mParticleSystem->fParticleCount( ) < 1 )
			{
				mBubbles[ i ]->fSetInvisible( false  );
				const Math::tVec4f color = mParticleSystem->fState( ).fSamplePerParticleGraph< FX::tBinaryV4Graph >( mParticleSystem->fSystemRandomNumberGenerator( ), FX::cColorGraph, 0.f );
				mBubbles[ i ]->fSetRgbaTint( Math::tVec4f( color.x, color.y, color.z, 0.25f ) );
			}
			else
			{
				if( mBubbles[ i ]->fInvisible( ) )
					break;
				mBubbles[ i ]->fSetInvisible( true  );
			}
		}

		if( mAttachmentEntity )
			fMoveTo( mAttachmentEntity->fObjectToWorld( ).fGetTranslation( ) );
	
		//fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mDummyBox->fObjectSpaceBox( ), Math::tVec4f( 1.f, 0.f, 0.f, 1.f ) );
		//fSceneGraph( )->fDebugGeometry( ).fRenderOnce( mSelectionBox->fObjectSpaceBox( ), Math::tVec4f( 0.f, 0.f, 1.f, 1.f ) );
	}

	void tSigFxParticleSystem::fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max )
	{
		tEditableObject::fSetLocalSpaceMinMax( min, max );
		mDummyBox->fSetObjectSpaceBox( fObjectSpaceBox( ) );
		mDummyBox->fSetParentRelativeXform( fObjectSpaceBox( ).fAdjustMatrix( Math::tMat3f::cIdentity, 0.25f ) );
	}

	Sigml::tObjectPtr tSigFxParticleSystem::fSerialize( b32 clone ) const
	{
		Fxml::tFxParticleSystemObject* fxo = new Fxml::tFxParticleSystemObject( );
		fSerializeBaseObject( fxo, clone );

		fxo->mName = fParticleSystemName( ).fCStr( );
		fxo->mParticleSystemName = fParticleSystemName( );
		fxo->mMeshResourcePath = mMeshResource ? Sigml::fSigbPathToSigml( mMeshResource->fGetPath( ) ) : tFilePathPtr( );
		fxo->mLocalSpace = mParticleSystem->fLocalSpace( );
		fxo->mCameraDepthOffset = mParticleSystem->fCameraDepthOffset( );
		fxo->mUpdateSpeedMultiplier = mParticleSystem->fUpdateSpeedMultiplier( );
		fxo->mLodFactor = mParticleSystem->fLodFactor( );
		fxo->mGhostParticleFrequency = mParticleSystem->fGhostParticleFrequency( );
		fxo->mGhostParticleLifetime = mParticleSystem->fGhostParticleLifetime( );
		fxo->mEmitterType = mParticleSystem->fGetEmitterType( );
		fxo->mBlendOp = mParticleSystem->fRenderState( ).fGetBlendOpFlags( );
		fxo->mSrcBlend = mParticleSystem->fRenderState( ).fGetSrcBlendMode( );
		fxo->mDstBlend = mParticleSystem->fRenderState( ).fGetDstBlendMode( );
		fxo->mSortMode = mParticleSystem->fSortMode( );

		tShadeMaterialGen* shadeMaterialGen = new tShadeMaterialGen( );
		fxo->mMaterial.fReset( shadeMaterialGen );
		
		if( !shadeMaterialGen->fFromDermlMtlFile( mDermlMtlFile ) )
			fxo->mMaterial.fRelease( );

		fxo->mToolState = mToolState;

		return Sigml::tObjectPtr( fxo );
	}

	b32 tSigFxParticleSystem::fChangeShader( Derml::tFile& dermlFile, const tFilePathPtr& shaderPath )
	{
		mDermlMtlFile.fFromShaderFile( dermlFile, shaderPath );
		fGenerateShaders( dermlFile );
		return true;
	}
	void tSigFxParticleSystem::fUpdateAgainstShaderFile( )
	{
		Derml::tFile dermlFile;
		if( mDermlMtlFile.fUpdateAgainstShaderFile( dermlFile ) )
			fGenerateShaders( dermlFile );
		else if( mPreviewBundle && !mPreviewBundle->fHasShaders( ) )
			fGenerateShadersFromCurrentMtlFile( );
	}
	void tSigFxParticleSystem::fGenerateShadersFromCurrentMtlFile( )
	{
		const tFilePathPtr absolutePath = ToolsPaths::fMakeResAbsolute( mDermlMtlFile.mShaderPath );
		Derml::tFile dermlFile;
		if( dermlFile.fLoadXml( absolutePath ) )
			fGenerateShaders( dermlFile );
	}
	void tSigFxParticleSystem::fGenerateShaders( const Derml::tFile& dermlFile )
	{
		if( dermlFile.mGeometryStyle != HlslGen::cVshFacingQuads )
		{
			log_warning( "Invalid geometry style on shader [" << mDermlMtlFile.mShaderPath << "]; expected GeometryType: 'FacingParticleQuads'" );
		}

		if( mPreviewBundle )
		{
			mPreviewBundle->fGenerateShaders( dermlFile, HlslGen::cToolTypeDefault );
			mPreviewBundle->fUpdateMaterial( mDermlMtlFile, *mContainer.fTextureCache( ) );
			if( mParticleSystem )
				mParticleSystem->fChangeMaterial( mPreviewBundle->fMaterial( ) );
		}
	}
	void tSigFxParticleSystem::fSetMeshResourceFile( const tFilePathPtr& meshPath )
	{
		if( mMeshResource )
			mMeshResource.fRelease( );

		b32 usingMesh = false;

		if( meshPath.fLength( ) > 0 )
		{
			Gfx::tDefaultAllocators& defFxAllocators = Gfx::tDefaultAllocators::fInstance( );
			mMeshResource = defFxAllocators.mResourceDepot->fQuery( tResourceId::fMake< tSceneGraphFile >( meshPath ) );
			mMeshResource->fLoadDefault( this );
			if( mMeshResource->fLoaded( ) || mMeshResource->fLoading( ) )
			{
				usingMesh = true;
				mMeshResource->fBlockUntilLoaded( );
				mParticleSystem->fChangeParticleList( new FX::tMeshParticleList( mMeshResource ) );
			}
			else
				mMeshResource.fRelease( );
		}

		if( !usingMesh )
			mParticleSystem->fChangeParticleList( new FX::tQuadParticleList( ) );
	}

	void tSigFxParticleSystem::fSyncStateToBinary( )
	{
		FX::tBinaryParticleSystemState& state = mParticleSystem->fState( );
		state.mSystemFlags = mToolState->mSystemFlags;

		state.mAttractorIgnoreIds.fNewArray( mToolState->mAttractorIgnoreIds.fCount( ) );
		for( u32 i = 0; i < state.mAttractorIgnoreIds.fCount( ); ++i )
			state.mAttractorIgnoreIds[ i ] = mToolState->mAttractorIgnoreIds[ i ];

		// Check for changes to the graph shapes and update the built versions on the particle system if necessary.
		for( u32 i = 0; i < mToolState->mEmissionGraphs.fCount( ); ++i )
		{
			if( !mToolState->mEmissionGraphs[ i ]->fGetDirty( ) )
				continue;

			mParticleSystem->fState( ).mEmissionGraphs[ i ]->fCopyFromGraph( mToolState->mEmissionGraphs[ i ] );

			mToolState->mEmissionGraphs[ i ]->fClean( );
		}

		// Check a per particle graph!
		for( u32 i = 0; i < mToolState->mPerParticleGraphs.fCount( ); ++i )
		{
			if( !mToolState->mPerParticleGraphs[ i ]->fGetDirty( ) )
				continue;

			mParticleSystem->fState( ).mPerParticleGraphs[ i ]->fCopyFromGraph( mToolState->mPerParticleGraphs[ i ] );

			mToolState->mPerParticleGraphs[ i ]->fClean( );
		}

		// Maybe a mesh?
		for( u32 i = 0; i < mToolState->mMeshGraphs.fCount( ); ++i )
		{
			if( !mToolState->mMeshGraphs[ i ]->fGetDirty( ) )
				continue;

			mParticleSystem->fState( ).mMeshGraphs[ i ]->fCopyFromGraph( mToolState->mMeshGraphs[ i ] );

			mToolState->mMeshGraphs[ i ]->fClean( );
		}
	}
}

