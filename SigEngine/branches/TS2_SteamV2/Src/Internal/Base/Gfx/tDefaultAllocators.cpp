#include "BasePch.hpp"
#include "tDefaultAllocators.hpp"
#include "tMaterialFile.hpp"
#include "tFontMaterial.hpp"
#include "tSolidColorMaterial.hpp"
#include "tFullBrightMaterial.hpp"
#include "tParticleMaterial.hpp"
#include "tDecalMaterial.hpp"
#include "tProfiler.hpp"

namespace Sig { namespace Gfx
{
	void tDefaultAllocators::fCreateAllocatorsForGameApp( const tDevicePtr& device, const tDefaultAllocatorSettings& settings )
	{
		const f32 c1MB = (1024.f * 1024.f);

		// create allocator for text geometry
		{
			f32 textMB = settings.mMBGeomText;

			// This can make debugging confusing. but may prove helpful in the future.
//#ifdef sig_devmenu
//			textMB *= 3.f; // more text for dev menu
//#endif//sig_devmenu

			const tVertexFormat& vtxFormat = tFontMaterial::cVertexFormat;
			u32 count = u32( c1MB * textMB / vtxFormat.fVertexSize( ) );
			mTextGeometryAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			mTextGeometryAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			profile_geom_allocator( *mTextGeometryAllocator, "TextGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << textMB << "] megs for text geometry." );
		}

		// create allocator for full bright geometry
		{
			const tVertexFormat& vtxFormat = tFullBrightMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomFullBright / vtxFormat.fVertexSize( ) );
			mFullBrightGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			mFullBrightGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			profile_geom_allocator( *mFullBrightGeomAllocator, "FullBrightGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomFullBright << "] megs for full bright geometry." );
		}

		// create allocator for solid color geometry
		{
			const tVertexFormat& vtxFormat = tSolidColorMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomSolidColor / vtxFormat.fVertexSize( ) );
			mSolidColorGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			mSolidColorGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			profile_geom_allocator( *mSolidColorGeomAllocator, "SolidColorGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomSolidColor << "] megs for solid color geometry." );
		}

		// create allocator for particle geometry
		{
			const tVertexFormat& vtxFormat = tParticleMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomParticle / vtxFormat.fVertexSize( ) );
			mParticleGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			mParticleGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			profile_geom_allocator( *mParticleGeomAllocator, "ParticleGeometry", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomParticle << "] megs for particle geometry." );
		}

		// create allocators for decal geometry
		{
			const tVertexFormat& vtxFormat = tDecalMaterial::cVertexFormat;
			u32 count = u32( c1MB * settings.mMBGeomDecal / vtxFormat.fVertexSize( ) );
			mDecalGeomAllocator.fReset( NEW tGeometryBufferVRamAllocator );
			mDecalGeomAllocator->fAllocate( device, vtxFormat, count, tGeometryBufferVRam::cAllocDynamic );
			profile_geom_allocator( *mDecalGeomAllocator, "Decal", false );
			log_line( Log::cFlagGraphics, "Using [" << settings.mMBGeomDecal << "] megs for decals." );

		}

		// create allocator for indices
		{
			const tIndexFormat idxFormat( tIndexFormat::cStorageU16, tIndexFormat::cPrimitiveTriangleList );
			u32 count = u32( c1MB * settings.mMBIndices / idxFormat.mSize );
			u32 primitives = count / 3;
			count = primitives * 3;
			mIndexAllocator.fReset( NEW tIndexBufferVRamAllocator );
			mIndexAllocator->fAllocate( device, idxFormat, count, primitives, tGeometryBufferVRam::cAllocDynamic );
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
		mIndexAllocator->fDeallocate( );
		//mSolidColorMaterial;
	}
	void tDefaultAllocators::fLoadMaterials( const tResourceDepotPtr& resourceDepot, tResource::tLoadCallerId lcid )
	{
		mResourceDepot = resourceDepot;

		// load solid color material
		mSolidColorMaterialFile = mResourceDepot->fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tSolidColorMaterial::fMaterialFilePath( ) ), lcid );
		mSolidColorMaterial.fReset( NEW tSolidColorMaterial( mSolidColorMaterialFile ) );

		// load fullbright material
		mFullBrightMaterialFile = mResourceDepot->fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tFullBrightMaterial::fMaterialFilePath( ) ), lcid );

		// load decal material
		mDecalMaterialFile = mResourceDepot->fQueryLoadBlock( tResourceId::fMake<tMaterialFile>( tDecalMaterial::fMaterialFilePath( ) ), lcid );
	}
	void tDefaultAllocators::fUnloadMaterials( tResource::tLoadCallerId lcid )
	{
		mSolidColorMaterialFile->fUnload( lcid );
		mFullBrightMaterialFile->fUnload( lcid );
		mDecalMaterialFile->fUnload( lcid );
	}

}}
