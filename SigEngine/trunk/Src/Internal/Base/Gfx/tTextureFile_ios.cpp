#include "BasePch.hpp"
#if defined( platform_ios )
#include "tTextureFile.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	devvar_clamp( u32, Renderer_Settings_MaxAnisotropy, 4, 1, 8, 0 );

	//------------------------------------------------------------------------------
	u32 tTextureFile::fComputeStorage( std::string & display ) const
	{
		//NOTE: These numbers aren't very accurate. 

		u32 numBytes = fVramUsage( );

		std::stringstream ss;
		ss << std::fixed << std::setprecision( 2 ) << 
			"Total - " << Memory::fToMB<f32>( mHeaderSize + numBytes ) << " MB " <<
			"Main - " << Memory::fToKB<f32>( mHeaderSize ) << " KB " <<
			"Vid - "  << Memory::fToKB<f32>( numBytes ) << " / " << Memory::fToKB<f32>( numBytes ) << " KB";

		display = ss.str( );

		return numBytes;
	}

	u32 tTextureFile::fVramUsage( ) const
	{
		u32 numBytes = 0;
		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			for( u32 imip = 0; imip < mImages[ iface ].mMipMapBuffers.fCount( ); ++imip )
				numBytes += mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize;
		}

		return numBytes;
	}

	tTextureFile::tPlatformHandle tTextureFile::fCreateTexture( 
		const tDevicePtr& device,
		u32 width,
		u32 height,
		u32 mipCount,
		tFormat format,
		tType type,
		u32 arrayCount )
	{
		return ( tPlatformHandle )0;
	}

	void tTextureFile::fDestroyTexture( 
		tPlatformHandle rawHandle )
	{
	}

	tTextureFile::tLockedMip tTextureFile::fLockMip(
		tPlatformHandle rawHandle,
		u32 iface, 
		u32 imip,
		tType type,
		b32 isArray )
	{
		return tLockedMip( );
	}

	void tTextureFile::fUnlockMip(
		tPlatformHandle rawHandle,
		u32 iface, 
		u32 imip,
		tType type,
		b32 isArray )
	{
	}

	void tTextureFile::fApply( 
		tPlatformHandle rawTexHandle, 
		const tDevicePtr& device, 
		u32 slot, 
		tFilterMode filter, 
		tAddressMode u, 
		tAddressMode v, 
		tAddressMode w )
	{
	}
	
	void tTextureFile::fApplyFilterMode(
		const tDevicePtr& device, 
		u32 slot, 
		tFilterMode filter )
	{
	}
	Sig::byte* tTextureFile::fLockForLoadInternal( u32 iface, u32 imip )
	{
		return NULL;
	}
	void tTextureFile::fUnlockForLoadInternal( u32 iface, u32 imip )
	{
		
	}
	void tTextureFile::fSetTextureNameForProfiler( const char* name )
	{
	}
}}

#include "tTextureReference.hpp"

namespace Sig { namespace Gfx
{
	void tTextureReference::fClearBoundTextures( const tDevicePtr& device, u32 begin, u32 end )
	{
	}
}}
#endif//#if defined( platform_ios )

