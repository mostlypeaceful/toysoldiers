//------------------------------------------------------------------------------
// \file tTextureFile.hpp - 26 Aug 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTextureFile__
#define __tTextureFile__

namespace Sig { namespace Gfx
{
	class tDevicePtr;


	///
	/// TODO document
	class base_export tTextureFile : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tTextureFile, 0x84E879BE);
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathAddB( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path )
		{
			if( StringUtil::fCheckExtension( path.fCStr( ), ".texb" ) )
				return tFilePathPtr( ); // no related source file
			if( StringUtil::fCheckExtension( path.fCStr( ), ".tatb" ) )
				return tFilePathPtr::fSwapExtension( path, ".tatml" );
			return tResource::fConvertPathSubB( path );
		}

	public:

		enum tType
		{
			cType2d,
			cTypeCube,

			// last
			cTypeCount,
			cTypeInvalid = cTypeCount
		};

		enum tSemantic
		{
			cSemanticDiffuse,
			cSemanticSpecular,
			cSemanticEmissive,
			cSemanticGui,
			cSemanticNormal,
			cSemanticOpacity,
			cSemanticReflection,
			cSemanticLookUpTable,
			cSemanticFont,
			cSemanticRenderTarget, //For render to texture

			// last
			cSemanticCount,
			cSemanticInvalid
		};

		enum tFormat
		{
			cFormatA8R8G8B8,
			cFormatDXT1,
			cFormatDXT3,
			cFormatDXT5,
			cFormatR5G6B5,
			cFormatA8,
			cFormatA8B8G8R8, ///< iOS preferred "rgba" (byte order abgr)

			// last
			cFormatCount,
			cFormatInvalid
		};

		enum tAddressMode
		{
			cAddressModeWrap,
			cAddressModeClamp,
			cAddressModeMirror,
			cAddressModeBorderWhite,
			cAddressModeBorderBlack,

			// last
			cAddressModeCount,
			cAddressModeInvalid
		};
		
		enum tFilterMode
		{
			cFilterModeWithMip,
			cFilterModeNoMip,
			cFilterModeNone,

			// last
			cFilterModeCount,
			cFilterModeInvalid
		};

		enum tSurfaceState
		{
			cSurfaceStateNull = 0,
			cSurfaceStateLoading,
			cSurfaceStateReady
		};

		typedef Sig::byte* tPlatformHandle;

		class base_export tSurfacePointer
		{
			declare_reflector( );
		public:
			u32 mBufferOffset;
			u32 mBufferSize;
			u32 mState;
		public:
			tSurfacePointer( );
			tSurfacePointer( tNoOpTag );
		};

		class base_export tImage
		{
			declare_reflector( );
		public:
			tDynamicArray<tSurfacePointer> mMipMapBuffers;
		public:
			tImage( );
			tImage( tNoOpTag );
		};

		tDynamicArray<tImage>	mImages; ///< i.e., for a normal 2D texture, there should be just one image; for a cubemap, there should be six 
		tPlatformHandle			mPlatformHandle;
		u32						mHeaderSize; ///< size of the file up to but not including the surface buffers
		u16						mMipMapCount;
		u16						mWidth;
		u16						mHeight;
		tEnum<tType,u8>			mType;
		tEnum<tSemantic,u8>		mSemantic;
		tEnum<tFormat,u8>		mFormat;
		u8						mIsAtlas;
		u16						mSubTexWidth;
		u16						mSubTexHeight;
		u16						mSubTexCountX;
		u16						mSubTexCountY;

	public:

		///
		/// \brief Parse format, type, and semantic from a path. The path is expected to end in the form '*_L', where L stands
		/// for a semantic letter, such as 'd' for diffuse, 's' for spec, etc.
		/// \return true if the format was explicitly specified in the textures.ini file text (.e., if "texture_d.tga = dxt5" was found);
		/// false if the default formatting was desired (i.e., nothing was specified for "texture_d.tga" in textures.ini).
		static b32 fParseDetails( const tFilePathPtr& filePath, const std::string& iniText, tType& typeOut, tSemantic& semanticOut, tFormat& formatOut );

		tTextureFile( );
		tTextureFile( tNoOpTag );
		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( const tResource& ownerResource );

		inline tPlatformHandle fGetPlatformHandle( ) const { return mPlatformHandle; }

		///
		/// \brief This is mostly for debug purposes, actual VRAM usage might vary by platform.
		/// \return The storage in megabytes for all mips/faces/etc.
		u32 fComputeStorage( std::string & display ) const;
		u32 fVramUsage( ) const;
		u32 fMainUsage( ) const { return mHeaderSize; }
		u32 fLODMemoryInBytes( ) const { return 0; }

		void fApply( 
			const tDevicePtr& device, 
			u32 slot ) const;

		void fApply( 
			const tDevicePtr& device, 
			u32 slot, 
			tFilterMode filter, 
			tAddressMode u, 
			tAddressMode v, 
			tAddressMode w = cAddressModeClamp ) const;

		static void fApply( 
			tPlatformHandle rawTexHandle, 
			const tDevicePtr& device, 
			u32 slot, 
			tFilterMode filter, 
			tAddressMode u, 
			tAddressMode v, 
			tAddressMode w = cAddressModeClamp );

		static void fApplyFilterMode(
			const tDevicePtr& device, 
			u32 slot, 
			tFilterMode filter );

		static tPlatformHandle fCreateTexture( 
			const tDevicePtr& device,
			u32 width,
			u32 height,
			u32 mipCount,
			u32 semantic,
			tFormat format,
			tType type,
			u32 arrayCount = 0 );

		static void fDestroyTexture( 
			tPlatformHandle rawHandle );

		struct tLockedMip
		{
			declare_reflector( );

			Sig::byte*	mBits;
			s32			mPitch;

			inline tLockedMip( ) : mBits( 0 ), mPitch( 0 ) { }
			inline tLockedMip( Sig::byte* bits, s32 pitch ) : mBits( bits ), mPitch( pitch ) { }

			template<class tTexelType>
			inline tTexelType* fGetTexel( u32 x, u32 y ) const
			{
				return ( tTexelType* )( mBits + y * mPitch + x * sizeof( tTexelType ) );
			}
		};

		static tLockedMip fLockMip(
			tPlatformHandle rawHandle,
			u32 iface, 
			u32 imip,
			tType type,
			b32 isArray = false );

		static void fUnlockMip(
			tPlatformHandle rawHandle,
			u32 iface, 
			u32 imip,
			tType type,
			b32 isArray = false );

	private:

		void fCreateTextureInternal( const tResource& ownerResource, const tDevicePtr& device );
		void fDestroyTextureInternal( );

		Sig::byte* fLockForLoadInternal( u32 iface, u32 imip );
		void fUnlockForLoadInternal( u32 iface, u32 imip );

		
		void fSetTextureNameForProfiler( const char* name );
	};

}} // ::Sig::Gfx

#endif//__tTextureFile__
