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
		f32 mMBIndices;

		// default settings
		tDefaultAllocatorSettings( )
			: mMBGeomText( 1.5f )
			, mMBGeomFullBright( 4.7f )	
			, mMBGeomSolidColor( 3.0f )	
			, mMBGeomParticle( 11.6f )
			, mMBGeomDecal( 11.5f )
			, mMBIndices( 3.0f )
		{ }
	};

	struct base_export tDefaultAllocators
	{
		declare_global_object( tDefaultAllocators );

		tDefaultAllocators( ) : mFontFromId( 0 ) { }
		void fCreateAllocatorsForGameApp( const tDevicePtr& device, const tDefaultAllocatorSettings& settings );
		void fDeallocate( );
		void fLoadMaterials( const tResourceDepotPtr& resourceDepot, tResource::tLoadCallerId lcid );
		void fUnloadMaterials( tResource::tLoadCallerId lcid );

		tResourceDepotPtr						mResourceDepot;
		tGeometryBufferVRamAllocatorPtr			mSolidColorGeomAllocator;
		tGeometryBufferVRamAllocatorPtr			mTextGeometryAllocator;
		tGeometryBufferVRamAllocatorPtr			mFullBrightGeomAllocator;
		tGeometryBufferVRamAllocatorPtr			mParticleGeomAllocator;
		tGeometryBufferVRamAllocatorPtr			mDecalGeomAllocator;
		tIndexBufferVRamAllocatorPtr			mIndexAllocator;
		tMaterialPtr							mSolidColorMaterial;
		tResourcePtr							mSolidColorMaterialFile;
		tResourcePtr							mFullBrightMaterialFile;
		tResourcePtr							mDecalMaterialFile;
		tFontFromId								mFontFromId;
	};
}}

#endif//__Gfx_tDefaultAllocators__
