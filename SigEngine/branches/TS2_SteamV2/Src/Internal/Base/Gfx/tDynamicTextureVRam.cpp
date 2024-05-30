#include "BasePch.hpp"
#include "tDynamicTextureVRam.hpp"

namespace Sig { namespace Gfx
{
	tTextureVRam::tTextureVRam( )
		: mPlatformHandle( 0 )
		, mFormat( tTextureFile::cFormatInvalid )
		, mWidth( 0 )
		, mHeight( 0 )
	{
	}

	tTextureVRam::~tTextureVRam( )
	{
		fDeallocate( );
	}

	void tTextureVRam::fAllocate( const tDevicePtr& device, u32 width, u32 height, u32 numMips, tTextureFile::tFormat format, tTextureFile::tType type )
	{
		fDeallocate( );

		mPlatformHandle = tTextureFile::fCreateTexture( device, width, height, numMips, format, type );
		sigassert( mPlatformHandle );
		mFormat			= format;
		mType			= type;
		mWidth			= width;
		mHeight			= height;
	}

	void tTextureVRam::fDeallocate( )
	{
		tTextureFile::fDestroyTexture( mPlatformHandle );
		mPlatformHandle = 0;
		mFormat			= tTextureFile::cFormatInvalid;
		mType			= tTextureFile::cTypeInvalid;
		mWidth			= 0;
		mHeight			= 0;
	}

	tTextureFile::tLockedMip tTextureVRam::fLockMip( u32 ithMip, u32 ithFace ) const
	{
		return tTextureFile::fLockMip( mPlatformHandle, ithFace, ithMip, mType );
	}

	void tTextureVRam::fUnlockMip( u32 ithMip, u32 ithFace ) const
	{
		tTextureFile::fUnlockMip( mPlatformHandle, ithFace, ithMip, mType );
	}

	u32 tTextureVRam::fPackColorR8G8B8A8( const Math::tVec4f& rgbAIn0to1 )
	{
		const u32 r = fRound<u32>( fClamp( rgbAIn0to1.x, 0.f, 1.f ) * 255 );
		const u32 g = fRound<u32>( fClamp( rgbAIn0to1.y, 0.f, 1.f ) * 255 );
		const u32 b = fRound<u32>( fClamp( rgbAIn0to1.z, 0.f, 1.f ) * 255 );
		const u32 a = fRound<u32>( fClamp( rgbAIn0to1.w, 0.f, 1.f ) * 255 );

		return fPackColorR8G8B8A8( r, g, b, a );
	}

	u32 tTextureVRam::fPackColorR8G8B8A8( u32 rIn0to255, u32 gIn0to255, u32 bIn0to255, u32 aIn0to255 )
	{
		return ( aIn0to255 << 24 ) | ( rIn0to255 << 16 ) | ( gIn0to255 << 8 ) | ( bIn0to255 );
	}

	void tTextureVRam::fUnpackColorR8G8B8A8( u32 packedRgba, Math::tVec4f& rgbaIn0to1 )
	{
		Math::tVec4u rgbaIn0to255;
		fUnpackColorR8G8B8A8( packedRgba, rgbaIn0to255 );

		rgbaIn0to1 = Math::tVec4f( rgbaIn0to255.x / 255.f, rgbaIn0to255.y / 255.f, rgbaIn0to255.z / 255.f, rgbaIn0to255.w / 255.f );		
	}

	void tTextureVRam::fUnpackColorR8G8B8A8( u32 packedRgba, Math::tVec4u& rgbaIn0to255 )
	{
		rgbaIn0to255.w = packedRgba >> 24;
		rgbaIn0to255.x = u32( packedRgba << 8 ) >> 24;
		rgbaIn0to255.y = u32( packedRgba << 16 ) >> 24;
		rgbaIn0to255.z = u32( packedRgba << 24 ) >> 24;
	}

	u16 tTextureVRam::fPackColorR5G6B5( const Math::tVec3f& rgbIn0to1 )
	{
		const u32 r = fRound<u32>( fClamp( rgbIn0to1.x, 0.f, 1.f ) * 31 );
		const u32 g = fRound<u32>( fClamp( rgbIn0to1.y, 0.f, 1.f ) * 63 );
		const u32 b = fRound<u32>( fClamp( rgbIn0to1.z, 0.f, 1.f ) * 31 );

		return fPackColorR5G6B5( r, g, b );
	}

	u16 tTextureVRam::fPackColorR5G6B5( u32 rIn0to31, u32 gIn0to63, u32 bIn0to31 )
	{
		return ( rIn0to31 << 11 ) | ( gIn0to63 << 5 ) | ( bIn0to31 );
	}

	void tTextureVRam::fUnpackColorR5G6B5( u16 packedRgb, Math::tVec3f& rgbIn0to1 )
	{
		Math::tVec3u rgbIn0to31and0to63;
		fUnpackColorR5G6B5( packedRgb, rgbIn0to31and0to63 );

		rgbIn0to1 = Math::tVec3f( rgbIn0to31and0to63.x / 31.f, rgbIn0to31and0to63.y / 63.f, rgbIn0to31and0to63.z / 31.f );
	}

	void tTextureVRam::fUnpackColorR5G6B5( u16 packedRgb, Math::tVec3u& rgbIn0to31and0to63 )
	{
		rgbIn0to31and0to63.x = packedRgb >> 11;
		rgbIn0to31and0to63.y = u16( packedRgb << 5 ) >> 10;
		rgbIn0to31and0to63.z = u16( packedRgb << 11 ) >> 11;
	}




	tDynamicTextureVRam::tDynamicTextureVRam( )
		: mLocked( false )
	{
	}

	void tDynamicTextureVRam::fAllocate( const tDevicePtr& device, u32 width, u32 height, tTextureFile::tFormat format )
	{
		fDeallocate( );
		tTextureVRam::fAllocate( device, width, height, 1, format, tTextureFile::cType2d );
	}

	void tDynamicTextureVRam::fDeallocate( )
	{
		sigassert( !mLocked );
		tTextureVRam::fDeallocate( );
	}

	void tDynamicTextureVRam::fChangeDevice( const tDevicePtr& newDevice )
	{
		sigassert( fAllocated( ) );

		// copy bits out to temp buffer
		tTextureFile::tLockedMip mip = fLock( );
		tDynamicBuffer savedBits( mip.mPitch * fHeight( ) );
		fMemCpy( savedBits.fBegin( ), mip.mBits, savedBits.fCount( ) );
		fUnlock( );
		
		// allocate with new device, but same width/height/format
		fAllocate( newDevice, fWidth( ), fHeight( ), fFormat( ) );

		// copy bits back
		mip = fLock( );
		fMemCpy( mip.mBits, savedBits.fBegin( ), savedBits.fCount( ) );
		fUnlock( );
	}

	void tDynamicTextureVRam::fCopyCpu( const tDevicePtr& device, const tDynamicTextureVRam& copyFrom )
	{
		sigassert( &copyFrom != this );
		sigassert( copyFrom.fAllocated( ) );

		fAllocate( device, copyFrom.fWidth( ), copyFrom.fHeight( ), copyFrom.fFormat( ) );

		fCopyCpu( copyFrom );
	}

	void tDynamicTextureVRam::fCopyCpu( const tDynamicTextureVRam& copyFrom )
	{
		sigassert( &copyFrom != this );
		sigassert( fAllocated( ) );
		sigassert( copyFrom.fAllocated( ) );
		sigassert( fFormat( ) == copyFrom.fFormat( ) && fWidth( ) == copyFrom.fWidth( ) && fHeight( ) == copyFrom.fHeight( ) );

		tTextureFile::tLockedMip dst = fLock( );
		tTextureFile::tLockedMip src = copyFrom.fLock( );

		sigassert( dst.mPitch == src.mPitch );
		sigassert( fHeight( ) == copyFrom.fHeight( ) );

		fMemCpy( dst.mBits, src.mBits, fHeight( ) * dst.mPitch );

		fUnlock( );
		copyFrom.fUnlock( );
	}

	tTextureFile::tLockedMip tDynamicTextureVRam::fLock( ) const
	{
		sigassert( !mLocked );
		mLocked = true;
		return tTextureVRam::fLockMip( 0 );
	}

	void tDynamicTextureVRam::fUnlock( ) const
	{
		sigassert( mLocked );
		mLocked = false;
		tTextureVRam::fUnlockMip( 0 );
	}

}}

