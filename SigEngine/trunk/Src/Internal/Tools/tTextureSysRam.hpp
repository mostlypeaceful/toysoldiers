#ifndef __tTextureSysRam__
#define __tTextureSysRam__
#include "tXmlSerializer.hpp"
#include "tXmlDeserializer.hpp"
#include "Gfx/tTextureFile.hpp"
#include "Gfx/tDynamicTextureVRam.hpp"
#include "Dx360Util.hpp"

namespace Sig
{
	class tFileWriter;

	///
	/// \brief Represents a "texture", either 2D or Cube, and all of its mip-maps, in a format
	/// that is relatively easy to manipulate in system ram (i.e., on the cpu, procedurally).
	class tools_export tTextureSysRam
	{
		friend class tTextureAtlasSysRam;
	public:

		enum tAlphaLevel
		{
			cAlphaNone,
			cAlpha1Bit,
			cAlpha8Bit,
			// last
			cAlphaLevelCount
		};

		class tools_export tSurface : public tRefCounter, public Gfx::tDynamicTextureVRam
		{
		public:
			tSurface( );
			~tSurface( );
			const Gfx::tDevicePtr& fGetReferenceDevice( ) const;
			Math::tVec2u fComputeLowerMipDimensions( b32 targetDxt ) const;
			
			// A8R8G8B8
			// NOTE: The unfiltered 32 bit functions do overlapped copies
			void fFilteredCopyA8R8G8B8( const tSurface& src );
			void fUnfilteredCopyA8R8G8B8( const tSurface& src ) { fUnfilteredCopyA8R8G8B8ToSubRect( src, 0, 0 ); }
			void fUnfilteredCopyA8R8G8B8ToSubRect( const tSurface& src, u32 dstX, u32 dstY );
			void fUnfilteredCopyA8R8G8B8FromSubRect( const tSurface & src, u32 srcX, u32 srcY );

			// R5G6B5
			void fUnfilteredCopyR5G6B5( const tSurface& src ); // Remapped copy
			void fUnfilteredCopyR5G6B5ToSubRect( const tSurface & src, u32 dstX, u32 dstY ); // Overlapped copy
			void fUnfilteredCopyR5G6B5FromSubRect( const tSurface & src, u32 srcX, u32 srcY ); // Overlapped copy

			void fConvertToNormalMapFormat( );
		};

		typedef tRefCounterPtr< tSurface > tSurfacePtr;
		typedef tDynamicArray< tSurfacePtr > tMipMapPtrArray;

		struct tools_export tImage
		{
			tMipMapPtrArray mMips;
		};

		typedef tDynamicArray< tImage > tImageArray;

	private:

		Gfx::tTextureFile::tType		mType;
		Gfx::tTextureFile::tSemantic	mSemantic;
		Gfx::tTextureFile::tFormat		mFormat;
		b32								mExplicitFormat;
		b32								mIsAtlas;
		u32								mNumSubTexX, mNumSubTexY;
		tAlphaLevel						mAlphaLevel;
		Math::tVec2u					mSize;
		tImageArray						mImages; ///< size should be == 1 for a simple 2D texture, or 6 for a cube-map

	public:

		///
		/// \brief Check if the extension at the end of 'path' is supported (e.g., bmp, jpg, png, tga, dds).
		static b32 fRecognizedExtension( const char* path );
		static tFilePathPtr fCreateBinaryPath( const tFilePathPtr& inputPath );
		static tFilePathPtr fTexturesIniPath( const tFilePathPtr& inputPath );

		inline Gfx::tTextureFile::tType			fType( ) const		{ return mType; }
		inline Gfx::tTextureFile::tSemantic		fSemantic( ) const	{ return mSemantic; }
		inline Gfx::tTextureFile::tFormat		fFormat( ) const	{ return mFormat; }
		inline b32								fIsAtlas( ) const	{ return mIsAtlas; }
		inline u32								fWidth( ) const		{ return mSize.x; }
		inline u32								fHeight( ) const	{ return mSize.y; }
		inline const Math::tVec2u&				fSize( ) const		{ return mSize; }
		inline tAlphaLevel						fAlphaLevel( ) const{ return mAlphaLevel; }
		inline u32								fNumMips( ) const	{ return ( mImages.fCount( ) == 0 ) ? 0 : mImages.fFront( ).mMips.fCount( ); }
		b32										fIsDXTFormat( ) const;
		const tImage&							fGetFace( u32 ithFace = 0 ) const { return mImages[ ithFace ]; }
		static u32								fNumFacesByType( Gfx::tTextureFile::tType type );


		tTextureSysRam( );
		~tTextureSysRam( );
		tTextureSysRam( const tTextureSysRam& other );
		tTextureSysRam& operator=( const tTextureSysRam& other );

		///
		/// \brief Set the number of images (becomes an array texture) and other array texture settings.
		/// \note Do this before loading images.
		void			fSetArrayTexture( u32 imgCount, u32 width, u32 height );

		///
		/// \brief Load from a recognized texture format, e.g. bmp, jpg, png, tga, dds
		b32				fLoad( const tFilePathPtr& fileName,
							b32 autoHandleMipsAndNormalMapFormat = true,
							u32 ithImage = 0 );

		///
		/// \brief Load from a recognized texture format, e.g. bmp, jpg, png, tga, dds, but force details.
		b32				fLoad( const tFilePathPtr& fileName, 
							Gfx::tTextureFile::tSemantic semantic,
							Gfx::tTextureFile::tFormat format,
							Gfx::tTextureFile::tType type,
							b32 autoHandleMipsAndNormalMapFormat = true,
							u32 ithImage = 0 );

		///
		/// \brief Initialize directly from a surface object (i.e., procedurally created texture).
		void			fFromSurface( const tSurface& surface, 
							Gfx::tTextureFile::tSemantic semantic,
							Gfx::tTextureFile::tFormat format,
							b32 autoHandleMipsAndNormalMapFormat = true );

		///
		/// \brief Create and initialize a texture
		void			fGenerate( u32 width, u32 height, const Math::tVec4f& fillColorRgba,
							Gfx::tTextureFile::tSemantic semantic,
							Gfx::tTextureFile::tFormat format,
							Gfx::tTextureFile::tType type = Gfx::tTextureFile::cType2d,
							b32 autoHandleMipsAndNormalMapFormat = true,
							const Math::tVec2u& lowestMipDims = Math::tVec2u(1,1) );

		///
		/// \brief Scale the current texture into a copy. This method will scale all mip levels of all faces.
		void			fScaleCopy( tTextureSysRam& scaledCopy, u32 newWidth, u32 newHeight ) const;

		///
		/// \brief If you created a texture array by loading each image one at a time, then you'll want to call this
		/// after loading all your images.
		void			fAfterImagesGenerated( b32 autoHandleMipsAndNormalMapFormat = true, const Math::tVec2u& lowestMipDims = Math::tVec2u(1,1) );

		///
		/// \brief If you've created a texture programatically (i.e., without calling fLoad), you
		/// can optionally call fGenerateMipMaps to have all mips auto-generated from the top one.
		void			fGenerateMipMaps( const Math::tVec2u& lowestMipDims = Math::tVec2u( 1, 1 ) );

		///
		/// \brief If you've created a normal map programatically using the standard x-y-z => r-g-b mapping,
		/// you should call this method after generating mip maps in order to convert to normalized alpha-green DXT5n style.
		void			fConvertToNormalMapFormat( );

		///
		/// \brief will multiply alpha across every pixel in the texture, effectively removing white edges and backgrounds where they should be alpha
		void			fPreMultiplyAlpha( );

		///
		/// \brief Deallocate and clear existing bits.
		void			fClear( );

		typedef Dx360Util::tRawFaceMipMapSet tRawFaceMipMapSet;

		///
		/// \brief Obtain the converted/formatted version of each mip as a raw buffer of bytes; i.e., this method
		/// will convert each mip of each face to the specified format (e.g. A8R8G8B8 => DXT5).
		/// \note This method does not generate mip-maps, it only converts them and spits them out into the supplied rawFaceMips argument.
		void			fConvertRawFaceMips( tRawFaceMipMapSet& rawFaceMips, b32 noConvert = false ) const;

		///
		/// \brief Convert the texture to a form that is immediately usable for the gpu.
		void			fConvertToVRamTexture( const Gfx::tDevicePtr& device, Gfx::tTextureVRam& vramTex ) const;

		///
		/// \brief Convert texture to a game binary file (Gfx::tTextureFile).
		void			fSaveGameBinary( const tFilePathPtr& outputFileName, tPlatformId platform ) const;

	private:

		void fCopy( const tTextureSysRam& other );
		b32 fDetailsValid( ) const;
		void fParseDetails( const tFilePathPtr& fileName );
		b32 fImagesValid( ) const;
		void fLoad2D( const tFilePathPtr& fileName, u32 ithImage );
		void fLoadCube( const tFilePathPtr& fileName );
		void fSetFormatWithAlpha( b32 hasAlpha );
		b32 fSemanticRequiresMipMaps( ) const;
		void fGenerateMipMaps( tImage& image, u32 logicalWidth, u32 logicalHeight, const Math::tVec2u& lowestMipDims = Math::tVec2u( 1, 1 ) ); // recursive version
		void fSaveGameBinary( tRawFaceMipMapSet& rawFaceMips, tFileWriter& ofile, tPlatformId platform ) const;
		void fOutputMipChain( Gfx::tTextureFile& texFile, tRawFaceMipMapSet& rawFaceMips, tFileWriter& ofile, tPlatformId pid ) const;
	};

	template<>
	tools_export void fSerializeXmlObject<tXmlSerializer, tTextureSysRam::tSurface>( tXmlSerializer& s, tTextureSysRam::tSurface& object );
	template<>
	tools_export void fSerializeXmlObject<tXmlDeserializer, tTextureSysRam::tSurface>( tXmlDeserializer& s, tTextureSysRam::tSurface& object );


	///
	/// \brief Represents a tTextureSysRam (see above) that is being used to store a MxN grid
	/// of logical textures. Handles such things as proper mip-mapping, updating a sub-rectangle, etc.
	/// \note Proper mip-mapping means not filtering across sub-texture boundaries when mips are generated.
	class tools_export tTextureAtlasSysRam
	{
		tTextureSysRam mTexture;
		u32 mNumTexturesX, mNumTexturesY;
		u32 mSubTexWidth, mSubTexHeight;
	public:

		inline u32 fNumTexturesX( ) const { return mNumTexturesX; }
		inline u32 fNumTexturesY( ) const { return mNumTexturesY; }

		inline u32 fSubTexWidth( ) const  { return mSubTexWidth; }
		inline u32 fSubTexHeight( ) const { return mSubTexHeight; }

		inline u32 fTotalWidth( ) const { return mTexture.fWidth( ); }
		inline u32 fTotalHeight( ) const { return mTexture.fHeight( ); }

		inline Gfx::tTextureFile::tFormat fFormat( ) const { return mTexture.fFormat( ); }
		inline Gfx::tTextureFile::tSemantic fSemantic( ) const { return mTexture.fSemantic( ); }

		tTextureAtlasSysRam( );

		void fAllocate( 
			u32 numTexturesX, 
			u32 numTexturesY, 
			u32 subTexWidth, 
			u32 subTexHeight,
			Gfx::tTextureFile::tSemantic semantic,
			Gfx::tTextureFile::tFormat format );

		void fUpdateSubTexture( u32 xTexIndex, u32 yTexIndex, const tTextureSysRam& newTex );

		///
		/// \brief Convert the texture to a form that is immediately usable for the gpu.
		void			fConvertToVRamTexture( const Gfx::tDevicePtr& device, Gfx::tTextureVRam& vramTex ) const;

		///
		/// \brief Copy into a sys ram texture.
		void			fConvertToSysRamTexture( tTextureSysRam& sysRamTex ) const;
	};
}

#endif//__tTextureSysRam__
