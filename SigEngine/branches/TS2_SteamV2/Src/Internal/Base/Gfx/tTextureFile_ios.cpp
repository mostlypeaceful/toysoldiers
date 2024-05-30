#include "BasePch.hpp"
#if defined( platform_ios )
#include "tTextureFile.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	devvar_clamp( u32, Renderer_Settings_MaxAnisotropy, 4, 1, 8, 0 );

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

	void tTextureFile::fCopyMipInternal( const tDevicePtr& device, u32 iface, u32 imip, const Sig::byte* mipBuffer, u32 bufferSize )
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

