#ifndef __tSigFxParticleSystem__
#define __tSigFxParticleSystem__
#include "Fxml.hpp"
#include "Derml.hpp"
#include "tAttachmentEntity.hpp"
#include "FX/tParticleSystem.hpp"
#include "Editor/tEditableObject.hpp"
#include "tMaterialPreviewBundle.hpp"

namespace Sig
{
	class tools_export tSigFxParticleSystem : public tEditableObject
	{
		define_dynamic_cast( tSigFxParticleSystem, tEditableObject );
	private:
		Derml::tMtlFile					mDermlMtlFile;
		tMaterialPreviewBundlePtr		mPreviewBundle;
		FX::tParticleSystemPtr			mParticleSystem;
		FX::tToolParticleSystemStatePtr mToolState;

		tResourcePtr					mMaterialFile;
		Gfx::tMaterialPtr				mMaterial;

		tResourcePtr					mMeshResource;

		tAttachmentEntityPtr			mAttachmentEntity;

		tFixedArray< Gfx::tRenderableEntityPtr, 50 > mBubbles;

	public:

		tSigFxParticleSystem( tEditableObjectContainer& container );
		tSigFxParticleSystem( tEditableObjectContainer& container, const Fxml::tFxParticleSystemObject& fxo );
		~tSigFxParticleSystem( );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		void fClone( const tSigFxParticleSystem& system );

		void fSetAttachmentEntity( tAttachmentEntity* attachment ) { mAttachmentEntity.fReset( attachment ); }

		const Derml::tMtlFile& fMaterialFile( ) const { return mDermlMtlFile; }
		const tMaterialPreviewBundlePtr& fPreviewBundle( ) const { return mPreviewBundle; }

		FX::tParticleSystemPtr fGetParticleSystem( ) const { return mParticleSystem; }
		FX::tToolParticleSystemStatePtr fGetToolState( ) { return mToolState; }
		const FX::tToolParticleSystemStatePtr fGetToolState( ) const { return mToolState; } // This is stupid, but I am just a man

		// TODO REFACTOR
		//virtual const u32 fRunListFlags( ) const { return ( 1u << tEntity::cRunListMove ); }
		//virtual void fOnTick( tRunListName runList, f32 dt );

		virtual void fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max );

		b32 fChangeShader( Derml::tFile& dermlFile, const tFilePathPtr& shaderPath );
		void fUpdateAgainstShaderFile( );
		void fGenerateShadersFromCurrentMtlFile( );
		void fGenerateShaders( const Derml::tFile& dermlFile );

		void fSetMeshResourceFile( const tFilePathPtr& meshPath );
		tFilePathPtr fMeshResourcePath( ) const { return mMeshResource ? mMeshResource->fGetPath( ) : tFilePathPtr( ); }

		// editor related that doesn't need to be saved
		u32 mLastOpenGraphIdx;

		void fSetParticleSystemName( const tStringPtr& name ) { mParticleSystem->fSetParticleSystemName( name ); }
		tStringPtr fParticleSystemName( ) const { return mParticleSystem->fParticleSystemName( ); }

		// These need to trigger a sync because a lot of places assume these are updated immediately on the
		// actual particle. Important!
		void fAddAttractorIgnoreId( u32 id ) { mToolState->fAddAttractorIgnoreId( id ); fSyncStateToBinary( ); }
		void fClearAttractorIgnoreIds( ) { mToolState->fClearAttractorIgnoreIds( ); fSyncStateToBinary( ); }

		b32 fHasFlag( u32 flag ) const { return mToolState->fHasFlag( flag ); }
		void fAddFlag( u32 flag ) { mToolState->fAddFlag( flag ); fSyncStateToBinary( ); }
		void fRemoveFlag( u32 flag ) { mToolState->fRemoveFlag( flag ); fSyncStateToBinary( ); }

	private:

		void fSyncStateToBinary( );
		void fCommonCtor(  tResourceDepot& resDep );
		void fSetBubble( u32 idx, f32 i, f32 j );
	};

}

#endif // __tSigFxParticleSystem__

