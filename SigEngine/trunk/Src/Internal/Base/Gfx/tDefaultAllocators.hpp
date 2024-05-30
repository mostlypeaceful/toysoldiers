#ifndef __Gfx_tDefaultAllocators__
#define __Gfx_tDefaultAllocators__
#include "tResourceDepot.hpp"
#include "tMaterial.hpp"
#include "tGeometryBufferVRamSlice.hpp"
#include "tIndexBufferVRamSlice.hpp"

namespace Sig { namespace Gfx
{
	typedef tResourcePtr (*tFontFromId)( u32 id );

	struct tDefaultAllocatorSettings
	{
		f32 mMBGeomText;
		f32 mMBGeomFullBright;
		f32 mMBGeomSolidColor;
		f32 mMBGeomParticle;
		f32 mMBGeomDecal;
		f32 mMBGeomPostEffectAndDeferredShading;
		f32 mMBIndices;

		// default settings
		tDefaultAllocatorSettings( )
			: mMBGeomText( 1.5f )
			, mMBGeomFullBright( 4.7f )	
			, mMBGeomSolidColor( 3.0f )	
			, mMBGeomParticle( 6.0f )
			, mMBGeomDecal( 5.0f )
			, mMBGeomPostEffectAndDeferredShading( 0.2f )
			, mMBIndices( 3.0f )
		{ }
	};

	struct base_export tDefaultAllocators
	{
		static tDefaultAllocators& fInstance( );

		tDefaultAllocators( ) : mFontFromId( 0 ) { }

		void fCreateAllocators( const tDevicePtr& device, const tDefaultAllocatorSettings& settings );
		void fDeallocate( );
		void fLoadMaterials( const tResourceDepotPtr& resourceDepot, tResource::tLoadCallerId lcid );
		void fUnloadMaterials( tResource::tLoadCallerId lcid );

		tResourceDepotPtr						mResourceDepot;
		tGeometryBufferVRamAllocatorPtr			mSolidColorGeomAllocator;
		tGeometryBufferVRamAllocatorPtr			mTextGeometryAllocator;
		tGeometryBufferVRamAllocatorPtr			mFullBrightGeomAllocator;
		tGeometryBufferVRamAllocatorPtr			mParticleGeomAllocator;
		tGeometryBufferVRamAllocatorPtr			mDecalGeomAllocator;
		tGeometryBufferVRamAllocatorPtr			mPostEffectsAndDeferredGeomAllocator;
		tIndexBufferVRamAllocatorPtr			mIndexAllocator;
		tMaterialPtr							mSolidColorMaterial;
		tResourcePtr							mSolidColorMaterialFile;
		tResourcePtr							mFullBrightMaterialFile;
		tResourcePtr							mDecalMaterialFile;
		tResourcePtr							mDeferredShadingMaterialFile;
		tFontFromId								mFontFromId;
	};
}}

#endif//__Gfx_tDefaultAllocators__
