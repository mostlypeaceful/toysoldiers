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
	public:

		tSigFxAttractor( tEditableObjectContainer& container );
		tSigFxAttractor( tEditableObjectContainer& container, const Fxml::tFxAttractorObject& fxa );

		virtual ~tSigFxAttractor( );

		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fThinkST( f32 dt );

		void fClone( const tSigFxAttractor& attractor );

		FX::tParticleAttractorPtr fGetAttractor( ) const { return mAttractor; }

		virtual Sigml::tObjectPtr fSerialize( b32 clone ) const;

		virtual void fSetLocalSpaceMinMax( const Math::tVec3f& min, const Math::tVec3f& max );

		void fSetAttractorName( const tStringPtr& name ) { mAttractor->fSetAttractorName( name ); }
		const tStringPtr& fAttractorName( ) const { return mAttractor->fAttractorName( ); }

		u32 mLastOpenGraphIdx;

		void fSetHidden( b32 hide ) { mHidden = hide; }
		b32 fHidden( ) const { return mHidden; }

	private:

		FX::tParticleAttractorPtr mAttractor;
		tSingleVectorRenderable* mSingleVector;

		b32	mHidden;
		void fCommonCtor( tResourceDepot& resDep );
	};


}

#endif // __tSigFxAttractor__