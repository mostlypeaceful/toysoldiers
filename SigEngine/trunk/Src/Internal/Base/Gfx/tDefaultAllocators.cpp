#include "BasePch.hpp"
#include "tDefaultAllocators.hpp"
#include "tMaterialFile.hpp"
#include "tFontMaterial.hpp"
#include "tSolidColorMaterial.hpp"
#include "tFullBrightMaterial.hpp"
#include "tParticleMaterial.hpp"
#include "tDecalMaterial.hpp"
#include "tDeferredShadingMaterial.hpp"
#include "tPostEffectsMaterial.hpp"
#include "tProfiler.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	tDefaultAllocators& tDefaultAllocators::fInstance( )
	{
		// Continue playing singleton whack-a-mole.

		// Fix maya crash where the class pool, and thus the relevant memory page, deallocates before the
		// tGeometryBufferVRamDeviceResources allocate by tDefaultAllocators do and thus stomps their memory
		// before they get deleted.  --mrickert
		fPreinitGeometryBufferVRamDeviceResourcePool( );

		static tDefaultAllocators a;
		return a;
	}

	void tDefaultAllocators::fCreateAllocators( const tDevicePtr& device, const tDefaultAllocatorSettings& settings )
	{
		sigassert( device );

		if( mTextGeometryAllocator )
			return; // must have already been initialized.

		const f32 c1MB = (1024.f * 1024.f);

		// create allocator for text geometry
		{
			f32 textMB = settings.mMBGeomText;

#ifdef target_tools
			textMB += 3.f; // lots more text for tools
#endif//target_tools

			const tVertexFormat& vtxFormat = tFontMaterial::cVertexFormat;
			u32 count = u32( c1MB * textMB / vtxFormat.fVertexSize( ) );
			mTextGeometryAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
			mTextGeometryAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			Memory::tHeap::fResetVramContext( );
			profile_geom_allocator( *mTextGeometryAllocator, "TextGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << textMB << "] megs for text geometry." );
		}

		// create allocator for full bright geometry
		{
			const tVertexFormat& vtxFormat = tFullBrightMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomFullBright / vtxFormat.fVertexSize( ) );
			mFullBrightGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
			mFullBrightGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			Memory::tHeap::fResetVramContext( );
			profile_geom_allocator( *mFullBrightGeomAllocator, "FullBrightGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomFullBright << "] megs for full bright geometry." );
		}

		// create allocator for solid color geometry
		{
			f32 sizeMB = settings.mMBGeomSolidColor;

#ifdef target_tools
			sizeMB *= 10.f; // lots more lines for tools
#endif//target_tools

			const tVertexFormat& vtxFormat = tSolidColorMaterial::cVertexFormat;
			u32 count = u32( c1MB * sizeMB / vtxFormat.fVertexSize( ) );
			mSolidColorGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
			mSolidColorGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			Memory::tHeap::fResetVramContext( );
			profile_geom_allocator( *mSolidColorGeomAllocator, "SolidColorGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomSolidColor << "] megs for solid color geometry." );
		}

		// create allocator for particle geometry
		{
			const tVertexFormat& vtxFormat = tParticleMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomParticle / vtxFormat.fVertexSize( ) );
			mParticleGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
			mParticleGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			Memory::tHeap::fResetVramContext( );
			profile_geom_allocator( *mParticleGeomAllocator, "ParticleGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomParticle << "] megs for particle geometry." );
		}

		// create allocators for decal geometry
		{
			const tVertexFormat& vtxFormat = tDecalMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomDecal / vtxFormat.fVertexSize( ) );
			mDecalGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
			mDecalGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			Memory::tHeap::fResetVramContext( );
			profile_geom_allocator( *mDecalGeomAllocator, "Decal", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomDecal << "] megs for decals." );
		}

		// create allocators for post processsing and deferred shading
		{
			const tVertexFormat& vtxFormat = tPostEffectsMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomPostEffectAndDeferredShading / vtxFormat.fVertexSize( ) );
			mPostEffectsAndDeferredGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
			mPostEffectsAndDeferredGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			Memory::tHeap::fResetVramContext( );
			profile_geom_allocator( *mPostEffectsAndDeferredGeomAllocator, "PostEffects", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomPostEffectAndDeferredShading << "] megs for post effects and deferred shading." );
		}		

		// create allocator for indices
		{
			f32 sizeMB = settings.mMBIndices;

#ifdef target_tools
			sizeMB *= 10.f; // lots more indices for tools
#endif//target_tools

			const tIndexFormat idxFormat( tIndexFormat::cStorageU16, tIndexFormat::cPrimitiveTriangleList );
			u32 count = u32( c1MB * sizeMB / idxFormat.mSize );
			u32 primitives = count / 3;
			count = primitives * 3;
			mIndexAllocator.fReset( NEW tIndexBufferVRamAllocator );
			Memory::tHeap::fSetVramContext( vram_alloc_stamp( cAllocStampContextSysGeometry ) );
			mIndexAllocator->fAllocate( device, idxFormat, count, primitives, tGeometryBufferVRam::cAllocDynamic );
			Memory::tHeap::fResetVramContext( );
			profile_geom_allocator( *mIndexAllocator, "Indices", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBIndices << "] megs for dynamic indices." );
		}
	}
	void tDefaultAllocators::fDeallocate( )
	{
		mSolidColorGeomAllocator->fDeallocate( );
		mTextGeometryAllocator->fDeallocate( );
		mFullBrightGeomAllocator->fDeallocate( );
		mParticleGeomAllocator->fDeallocate( );
		mDecalGeomAllocator->fDeallocate( );
		mPostEffectsAndDeferredGeomAllocator->fDeallocate( );
		mIndexAllocator->fDeallocate( );
	}
	void tDefaultAllocators::fLoadMaterials( const tResourceDepotPtr& resourceDepot, tResource::tLoadCallerId lcid )
	{
		sigassert( resourceDepot );

		tDevicePtr device = tDevice::fGetDefaultDevice( );
		sigassert( device && "Default device is needed to load material resources." );

		if( mResourceDepot )
			return; // must have already been initialized.

		mResourceDepot = resourceDepot;

		mSolidColorMaterialFile = mResourceDepot->fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tSolidColorMaterial::fMaterialFilePath( ) ), lcid );
		mSolidColorMaterial.fReset( NEW tSolidColorMaterial( mSolidColorMaterialFile ) );
		mFullBrightMaterialFile = mResourceDepot->fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tFullBrightMaterial::fMaterialFilePath( ) ), lcid );
		mDecalMaterialFile = mResourceDepot->fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tDecalMaterial::fMaterialFilePath( ) ), lcid );
		mDeferredShadingMaterialFile = mResourceDepot->fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tDeferredShadingMaterial::fMaterialFilePath( ) ), lcid );
	}
	void tDefaultAllocators::fUnloadMaterials( tResource::tLoadCallerId lcid )
	{
		mSolidColorMaterialFile.fRelease( );
		mFullBrightMaterialFile.fRelease( );
		mDecalMaterialFile.fRelease( );
		mDeferredShadingMaterialFile.fRelease( );
	}

}}
