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
		implement_rtti_serializable_base_class(tTextureFile, 0x84E879BE);
	public:
		static const u32		cVersion;

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
			cSemanticReflection,
			cSemanticLookUpTable,
			cSemanticFont,

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

		typedef Sig::byte* tPlatformHandle;

		class base_export tSurfacePointer
		{
			declare_reflector( );
		public:
			u32 mBufferOffset;
			u32 mBufferSize;
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
		b16						mDiscardSurfaceBuffers;
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
		virtual void fResizeAfterLoad( tGenericBuffer* fileBuffer );
		virtual void fOnFileUnloading( );

		inline tPlatformHandle fGetPlatformHandle( ) const { return mPlatformHandle; }

		///
		/// \brief This is mostly for debug purposes, actual VRAM usage might vary by platform.
		/// \return The storage in megabytes for all mips/faces/etc.
		s32 fComputeStorage( ) const;
		f32 fComputeStorageInMB( ) const;

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

		void fCreateTextureInternal( const tDevicePtr& device, b32 lowDetailTextures );
		void fCopyMipInternal( const tDevicePtr& device, u32 iface, u32 imip, const Sig::byte* mipBuffer, u32 bufferSize );
		void fDestroyTextureInternal( );
	};

}}


namespace Sig
{
	template<>
	class tResourceConvertPath<Gfx::tTextureFile>
	{
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
	};

	template< >
	class tResourceLoadBehavior<Gfx::tTextureFile>
	{
	public:
		static b32 fWillResizeAfterLoad( ) { return true; }
	};
}


#endif//__tTextureFile__
