#ifndef __tDynamicTextureVRam__
#define __tDynamicTextureVRam__
#include "tTextureFile.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \brief Represents a 2D texture map that can be modified on the CPU while
	/// still being used as a texture map in VRAM.
	class base_export tTextureVRam : public tUncopyable
	{
	private:
		tTextureFile::tPlatformHandle	mPlatformHandle;
		tTextureFile::tFormat			mFormat;
		tTextureFile::tType				mType;
		u32								mWidth, mHeight;

	public:
		tTextureVRam( );
		~tTextureVRam( );

		void fAllocate( const tDevicePtr& device, u32 width, u32 height, u32 numMips, tTextureFile::tFormat format, tTextureFile::tType type );
		void fDeallocate( );

		tTextureFile::tLockedMip fLockMip( u32 ithMip, u32 ithFace = 0 ) const;
		void fUnlockMip( u32 ithMip, u32 ithFace = 0 ) const;

		inline tTextureFile::tFormat fFormat( ) const { return mFormat; }
		inline tTextureFile::tType fType( ) const { return mType; }
		inline u32 fWidth( ) const { return mWidth; }
		inline u32 fHeight( ) const { return mHeight; }
		inline b32 fAllocated( ) const { return mFormat != tTextureFile::cFormatInvalid && mWidth > 0 && mHeight > 0 && mPlatformHandle; }

		inline Math::tVec2f fUv( const Math::tVec2u& texelIndex ) const
		{
			return Math::tVec2f(
				texelIndex.x / fMax( 1.f, mWidth - 1.f ),
				texelIndex.y / fMax( 1.f, mHeight - 1.f ) );
		}

		///
		/// \brief Computes a slightly altered "uv" value based on texel index.
		/// This uv will step through the middle of texels in the source texture; this
		/// will create for a good averaging of neighboring texels. In the case of a square src texture
		/// and dst texture half the size, this will result in a perfect averaging of all texels
		inline Math::tVec2f fUvForScaling( const Math::tVec2u& texelIndex ) const
		{
			return Math::tVec2f(
				( texelIndex.x + 0.5f ) / mWidth,
				( texelIndex.y + 0.5f ) / mHeight );
		}

		inline Math::tVec2u fTexelIndexClamp( const Math::tVec2f& uv ) const
		{
			return Math::tVec2u( 
				fRoundDown<u32>( fClamp( uv.x, 0.f, 1.f ) * ( mWidth - 1.f ) ), 
				fRoundDown<u32>( fClamp( uv.y, 0.f, 1.f ) * ( mHeight - 1.f ) ) );
		}

		inline Math::tVec2f fTexelIndexClampf( const Math::tVec2f& uv ) const
		{
			return Math::tVec2f( 
				fClamp( uv.x, 0.f, 1.f ) * ( mWidth - 1.f ), 
				fClamp( uv.y, 0.f, 1.f ) * ( mHeight - 1.f ) );
		}

		inline Math::tVec2u fTexelIndexWrap( const Math::tVec2f& uv ) const
		{
			return Math::tVec2u( 
				fRoundDown<u32>( std::fmod( uv.x, 1.f ) * ( mWidth - 1.f ) ), 
				fRoundDown<u32>( std::fmod( uv.y, 1.f ) * ( mHeight - 1.f ) ) );
		}

		inline Math::tVec2f fTexelIndexWrapf( const Math::tVec2f& uv ) const
		{
			return Math::tVec2f( 
				std::fmod( uv.x, 1.f ) * ( mWidth - 1.f ), 
				std::fmod( uv.y, 1.f ) * ( mHeight - 1.f ) );
		}

		inline tTextureFile::tPlatformHandle fGetPlatformHandle( ) const { return mPlatformHandle; }

		static u32		fPackColorR8G8B8A8( const Math::tVec4f& rgbAIn0to1 );
		static u32		fPackColorR8G8B8A8( u32 rIn0to255, u32 gIn0to255, u32 bIn0to255, u32 aIn0to255 = 0xff );
		static void		fUnpackColorR8G8B8A8( u32 packedRgba, Math::tVec4f& rgbaIn0to1 );
		static void		fUnpackColorR8G8B8A8( u32 packedRgba, Math::tVec4u& rgbaIn0to255 );

		static u16		fPackColorR5G6B5( const Math::tVec3f& rgbIn0to1 );
		static u16		fPackColorR5G6B5( u32 rIn0to31, u32 gIn0to63, u32 bIn0to31 );
		static void		fUnpackColorR5G6B5( u16 packedRgb, Math::tVec3f& rgbIn0to1 );
		static void		fUnpackColorR5G6B5( u16 packedRgb, Math::tVec3u& rgbIn0to31and0to63 );
	};


	///
	/// \brief Represents a 2D texture map that can be modified on the CPU while
	/// still being used as a texture map in VRAM.
	class base_export tDynamicTextureVRam : public tTextureVRam
	{
	private:
		mutable b32						mLocked;

	public:
		tDynamicTextureVRam( );

		void fAllocate( const tDevicePtr& device, u32 width, u32 height, tTextureFile::tFormat format );
		void fDeallocate( );

		///
		/// \brief Reallocates with the new device and copies previous data.
		void fChangeDevice( const tDevicePtr& newDevice );

		///
		/// \brief Performs a cpu copy of the supplied texture (effectively a memcpy of the bits).
		/// This version will re-allocate the current texture.
		void fCopyCpu( const tDevicePtr& device, const tDynamicTextureVRam& copyFrom );

		///
		/// \brief Performs a direct byte-copy of the supplied texture. Assumes 'this' has already
		/// been allocated with identical formats and width/height.
		void fCopyCpu( const tDynamicTextureVRam& copyFrom );

		tTextureFile::tLockedMip fLock( ) const;
		void fUnlock( ) const;

	private:
		// redeclare so as to be inaccessible
		tTextureFile::tLockedMip fLockMip( u32 ithMip, u32 ithFace = 0 ) const;
		void fUnlockMip( u32 ithMip, u32 ithFace = 0 ) const;
	};

}}

#endif//__tDynamicTextureVRam__
