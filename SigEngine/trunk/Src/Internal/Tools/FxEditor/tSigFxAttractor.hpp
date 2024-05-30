#ifndef __tSigFxAttractor__
#define __tSigFxAttractor__

#include "Editor/tEditableObject.hpp"
#include "FX/tParticleAttractor.hpp"
#include "Fxml.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "Gfx/tMaterial.hpp"
#include "Gfx/tGeometryBufferVRamSlice.hpp"
#include "Gfx/tIndexBufferVRamSlice.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig
{
	class tools_export tSingleVectorRenderable : public Gfx::tRenderableEntity
	{
	public:

		tSingleVectorRenderable( 
		const Gfx::tMaterialPtr& material, 
		const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
		const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );

		virtual ~tSingleVectorRenderable( );

		void fResetDeviceObjects( 
			const Gfx::tMaterialPtr& material,
			const Gfx::tGeometryBufferVRamAllocatorPtr& geometryAllocator, 
			const Gfx::tIndexBufferVRamAllocatorPtr& indexAllocator );

		void fGenerate( f32 scale );
		void fBake( const tGrowableArray<Gfx::tSolidColorRenderVertex>& verts, const tGrowableArray<u16>& ids );

		f32 fCurrentScale( ) const { return mCurrentScale; }
	private:

		Gfx::tDynamicGeometry mGeometry;
		Gfx::tMaterialPtr mMaterial;
		f32 mCurrentScale;
	};



	class tools_export tSigFxAttractor : public tEditableObject
	{
		define_dynamic_cast( tSigFxAttractor, tEditableObject );

		FX::tParticleAttractorPtr mAttractor;
		FX::tToolAttractorDataPtr mToolData;
		tSingleVectorRenderable* mSingleVector;

		b32	mHidden;

	public:
		u32 mLastOpenGraphIdx;

		tSigFxAttractor( tEditableObjectContainer& container );
		tSigFxAttractor( tEditableObjectContainer& container, const Fxml::tFxAttractorObject& fxa );

		virtual ~tSigFxAttractor( );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		void fClone( const tSigFxAttractor& attractor );

		FX::tParticleAttractorPtr fGetAttractor( ) const { return mAttractor; }
		FX::tToolAttractorDataPtr fGetToolData( ) { return mToolData; }

		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		virtual void fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max );

		void fSetAttractorName( const tStringPtr& name ) { mAttractor->fSetAttractorName( name ); }
		const tStringPtr& fAttractorName( ) const { return mAttractor->fAttractorName( ); }

		// TODO
		void fSetForceType( FX::tForceType type ) { mToolData->fSetForceType( type ); }	
		void fSetAffectParticlesDirection( b32 b ) { mToolData->fSetAffectParticlesDirection( b ); }
		void fSetParticleMustBeInRadius( b32 b ) { mToolData->fSetParticleMustBeInRadius( b ); }

		// Use this editor object to manage flags while in the editor. It will be copied
		// over to the object in the editor.
		b32 fHasFlag( u32 flag ) { return mToolData->fHasFlag( flag ); }
		void fRemoveFlag( u32 flag ) { mToolData->fRemoveFlag( flag ); fSyncToBinary(); }
		void fAddFlag( u32 flag ) { mToolData->fAddFlag( flag ); fSyncToBinary(); }

		void fSetHidden( b32 hide ) { mHidden = hide; }
		b32 fHidden( ) const { return mHidden; }

	private:
		void fCommonCtor( tResourceDepot& resDep );
		void fSyncToBinary( );
	};


}

#endif // __tSigFxAttractor__