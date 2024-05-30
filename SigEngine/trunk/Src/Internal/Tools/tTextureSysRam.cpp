#include "ToolsPch.hpp"
#include "tFixedBitArray.hpp"
#include "tTextureSysRam.hpp"
#include "FileSystem.hpp"
#include "tFileWriter.hpp"
#include "tBase64.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "Gfx/tDevice.hpp"
#include "tProjectFile.hpp"
#include "tPlatform.hpp"

namespace Sig
{
	class tools_export tReferenceDevice
	{
		declare_singleton( tReferenceDevice );
	private:
		Gfx::tDevicePtr mDevice;
	public:
		void fAddClient( )
		{
			if( mDevice.fNull( ) )
				mDevice.fReset( new Gfx::tDevice( ( u64 )::GetConsoleWindow( ), true ) );
			else
				mDevice->fAddRef( );
		}
		void fRemoveClient( )
		{
			if( mDevice->fRefCount( ) == 1 )
				mDevice.fRelease( );
			else
				mDevice->fDecRef( );
		}

		inline operator const Gfx::tDevicePtr& ( ) const { return mDevice; }
		inline operator IDirect3DDevice9*( ) const { return mDevice->fGetDevice( ); }
	};

	namespace
	{
		D3DFORMAT fConvertFormat( Gfx::tTextureFile::tFormat fmt, Gfx::tTextureFile::tSemantic semantic, tTextureSysRam::tAlphaLevel alphaLevel )
		{
			D3DFORMAT o;

			switch( fmt )
			{
			case Gfx::tTextureFile::cFormatA8R8G8B8:	o = D3DFMT_A8R8G8B8; break;
			case Gfx::tTextureFile::cFormatDXT1:		o = D3DFMT_DXT1; break;
			case Gfx::tTextureFile::cFormatDXT3:		o = D3DFMT_DXT3; break;
			case Gfx::tTextureFile::cFormatDXT5:		o = D3DFMT_DXT5; break;
			case Gfx::tTextureFile::cFormatR5G6B5:		o = D3DFMT_R5G6B5; break;
			case Gfx::tTextureFile::cFormatA8:			o = D3DFMT_A8; break;
			default: sigassert( !"un-recognized texture format!" ); break;
			}

			return o;
		}

		u32 fConvertPitch( u32 inputPitch, D3DFORMAT d3dFormat )
		{
			u32 o = inputPitch;

			switch( d3dFormat )
			{
			case D3DFMT_DXT1: o = fMax( 2u, o / 4u ); break;
			case D3DFMT_DXT3: o = fMax( 4u, o / 4u ); break;
			case D3DFMT_DXT5: o = fMax( 4u, o / 4u ); break;
			}

			return o;
		}
	}

	tTextureSysRam::tSurface::tSurface( )
	{
		tReferenceDevice::fInstance( ).fAddClient( );
	}

	tTextureSysRam::tSurface::~tSurface( )
	{
		tReferenceDevice::fInstance( ).fRemoveClient( );
	}

	const Gfx::tDevicePtr& tTextureSysRam::tSurface::fGetReferenceDevice( ) const
	{
		return tReferenceDevice::fInstance( );
	}

	Math::tVec2u tTextureSysRam::tSurface::fComputeLowerMipDimensions( b32 targetDxt ) const
	{
		const u32 min = targetDxt ? 4u : 1u;
		return Math::tVec2u( fMax( min, fWidth( ) / 2 ), fMax( min, fHeight( ) / 2 ) );
	}

	void tTextureSysRam::tSurface::fFilteredCopyA8R8G8B8( const tSurface& src )
	{
		tSurface& dst = *this;

		// first check if we're identical size, in which case we go through the unfiltered route...
		if( dst.fWidth( ) == src.fWidth( ) && dst.fHeight( ) == src.fHeight( ) )
		{
			dst.fUnfilteredCopyA8R8G8B8( src );
			return;
		}

		sigassert( 
			this != &src &&
			dst.fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 &&
			src.fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 );

		Gfx::tTextureFile::tLockedMip srcMip = src.fLock( );
		Gfx::tTextureFile::tLockedMip dstMip = dst.fLock( );

		for( u32 yDst = 0; yDst < dst.fHeight( ); ++yDst )
		for( u32 xDst = 0; xDst < dst.fWidth( ); ++xDst )
		{
			const Math::tVec2f uv = dst.fUvForScaling( Math::tVec2u( xDst, yDst ) );
			const Math::tVec2f srcTexelIndex = src.fTexelIndexClampf( uv );

			const u32 l  = fRoundDown<u32>( srcTexelIndex.x );
			const u32 r  = fMin( src.fWidth( ) - 1, l + 1 );
			const u32 t  = fRoundDown<u32>( srcTexelIndex.y );
			const u32 b  = fMin( src.fHeight( ) - 1, t + 1 );
			const f32 dx = srcTexelIndex.x - l;
			const f32 dy = srcTexelIndex.y - t;

			Math::tVec4f tlSrcRgba;
			Math::tVec4f trSrcRgba;
			Math::tVec4f blSrcRgba;
			Math::tVec4f brSrcRgba;

			Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *srcMip.fGetTexel<u32>( l, t ), tlSrcRgba );
			Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *srcMip.fGetTexel<u32>( r, t ), trSrcRgba );
			Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *srcMip.fGetTexel<u32>( l, b ), blSrcRgba );
			Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *srcMip.fGetTexel<u32>( r, b ), brSrcRgba );

			const Math::tVec4f blend = Math::fLerp( 
				Math::fLerp( tlSrcRgba, trSrcRgba, dx ),
				Math::fLerp( blSrcRgba, brSrcRgba, dx ), dy );

			*dstMip.fGetTexel<u32>( xDst, yDst ) = Gfx::tTextureVRam::fPackColorR8G8B8A8( blend );
		}

		src.fUnlock( );
		dst.fUnlock( );
	}

	void tTextureSysRam::tSurface::fUnfilteredCopyA8R8G8B8ToSubRect( const tSurface& src, u32 dstX, u32 dstY )
	{
		tSurface& dst = *this;

		sigassert( 
			this != &src &&
			dst.fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 &&
			src.fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 );

		sigassert( 
			dstX + src.fWidth( ) <= dst.fWidth( ) &&
			dstY + src.fHeight( ) <= dst.fHeight( ) );

		Gfx::tTextureFile::tLockedMip srcMip = src.fLock( );
		Gfx::tTextureFile::tLockedMip dstMip = dst.fLock( );

		for( u32 yDst = dstY; yDst < dstY + src.fHeight( ); ++yDst )
		for( u32 xDst = dstX; xDst < dstX + src.fWidth( ); ++xDst )
		{
			*dstMip.fGetTexel<u32>( xDst, yDst ) = *srcMip.fGetTexel<u32>( xDst - dstX, yDst - dstY );
		}

		src.fUnlock( );
		dst.fUnlock( );
	}

	void tTextureSysRam::tSurface::fUnfilteredCopyA8R8G8B8FromSubRect( const tSurface & src, u32 srcX, u32 srcY )
	{
		tSurface& dst = *this;

		sigassert( 
			this != &src &&
			dst.fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 &&
			src.fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 );

		sigassert(
			srcX + dst.fWidth( ) <= src.fWidth( ) && 
			srcY + dst.fHeight( ) <= src.fHeight( ) );

		Gfx::tTextureFile::tLockedMip srcMip = src.fLock( );
		Gfx::tTextureFile::tLockedMip dstMip = dst.fLock( );

		for( u32 yDst = 0; yDst < dst.fHeight( ); ++yDst )
		for( u32 xDst = 0; xDst < dst.fWidth( ); ++xDst )
		{
			*dstMip.fGetTexel<u32>( xDst, yDst ) = *srcMip.fGetTexel<u32>( xDst + srcX, yDst + srcY );
		}

		src.fUnlock( );
		dst.fUnlock( );
	}

	void tTextureSysRam::tSurface::fUnfilteredCopyR5G6B5( const tSurface & src )
	{
		tSurface& dst = *this;

		sigassert( 
			this != &src &&
			dst.fFormat( ) == Gfx::tTextureFile::cFormatR5G6B5 &&
			src.fFormat( ) == Gfx::tTextureFile::cFormatR5G6B5 );

		Gfx::tTextureFile::tLockedMip srcMip = src.fLock( );
		Gfx::tTextureFile::tLockedMip dstMip = dst.fLock( );

		for( u32 yDst = 0; yDst < dst.fHeight( ); ++yDst )
		for( u32 xDst = 0; xDst < dst.fWidth( ); ++xDst )
		{
			const Math::tVec2f uv = dst.fUvForScaling( Math::tVec2u( xDst, yDst ) );
			const Math::tVec2f srcTexelIndex = src.fTexelIndexClampf( uv );

			const u32 l  = fRoundDown<u32>( srcTexelIndex.x );
			const u32 t  = fRoundDown<u32>( srcTexelIndex.y );

			*dstMip.fGetTexel<u16>( xDst, yDst ) = *srcMip.fGetTexel<u16>( l, t );
		}

		src.fUnlock( );
		dst.fUnlock( );
	}

	void tTextureSysRam::tSurface::fUnfilteredCopyR5G6B5ToSubRect( const tSurface & src, u32 dstX, u32 dstY )
	{
		tSurface& dst = *this;

		sigassert( 
			this != &src &&
			dst.fFormat( ) == Gfx::tTextureFile::cFormatR5G6B5 &&
			src.fFormat( ) == Gfx::tTextureFile::cFormatR5G6B5 );

		sigassert( 
			dstX + src.fWidth( ) <= dst.fWidth( ) &&
			dstY + src.fHeight( ) <= dst.fHeight( ) );

		Gfx::tTextureFile::tLockedMip srcMip = src.fLock( );
		Gfx::tTextureFile::tLockedMip dstMip = dst.fLock( );

		for( u32 yDst = dstY; yDst < dstY + src.fHeight( ); ++yDst )
		for( u32 xDst = dstX; xDst < dstX + src.fWidth( ); ++xDst )
		{
			*dstMip.fGetTexel<u16>( xDst, yDst ) = *srcMip.fGetTexel<u16>( xDst - dstX, yDst - dstY );
		}

		src.fUnlock( );
		dst.fUnlock( );
	}

	void tTextureSysRam::tSurface::fUnfilteredCopyR5G6B5FromSubRect( const tSurface & src, u32 srcX, u32 srcY )
	{
		tSurface& dst = *this;

		sigassert( 
			this != &src &&
			dst.fFormat( ) == Gfx::tTextureFile::cFormatR5G6B5 &&
			src.fFormat( ) == Gfx::tTextureFile::cFormatR5G6B5 );

		sigassert(
			srcX + dst.fWidth( ) <= src.fWidth( ) && 
			srcY + dst.fHeight( ) <= src.fHeight( ) );

		Gfx::tTextureFile::tLockedMip srcMip = src.fLock( );
		Gfx::tTextureFile::tLockedMip dstMip = dst.fLock( );

		for( u32 yDst = 0; yDst < dst.fHeight( ); ++yDst )
		for( u32 xDst = 0; xDst < dst.fWidth( ); ++xDst )
		{
			*dstMip.fGetTexel<u16>( xDst, yDst ) = *srcMip.fGetTexel<u16>( xDst + srcX, yDst + srcY );
		}

		src.fUnlock( );
		dst.fUnlock( );
	}

	void tTextureSysRam::tSurface::fConvertToNormalMapFormat( )
	{
		sigassert( fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 );
		Gfx::tTextureFile::tLockedMip mip = fLock( );

		for( u32 y = 0; y < fHeight( ); ++y )
		{
			for( u32 x = 0; x < fWidth( ); ++x )
			{
				u32* dstTexel = mip.fGetTexel<u32>( x, y );

				// unpack texel
				Math::tVec4f corrected;
				Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *dstTexel, corrected );

				// treat as a 3-component normal vector
				Math::tVec3f n = Math::tVec3f( corrected.x, corrected.y, corrected.z );

				// expand to [-1,+1]
				n = 2.0f * n - Math::tVec3f( 1.0f );

				// renormalize
				n.fNormalizeSafe( );

				// pack back to [0,1]
				n = 0.5f * n + Math::tVec3f( 0.5f );

				// pack y into green, x into alpha for DXT5n style compression
				corrected.x = 0.f;
				corrected.y = n.y;
				corrected.z = 0.f;
				corrected.w = n.x;

				// pack back into texture bits
				*dstTexel = Gfx::tTextureVRam::fPackColorR8G8B8A8( corrected );
			}
		}

		fUnlock( );
	}

	struct tTextureSurfaceSerializationData
	{
		Gfx::tTextureFile::tFormat mFormat;
		u32 mWidth;
		u32 mHeight;
		std::string mEncodedBits;

		tTextureSurfaceSerializationData( ) : mFormat( Gfx::tTextureFile::cFormatInvalid ), mWidth( 0 ), mHeight( 0 ) { }
	};

	template<class tSerializer>
	void fSerializeSurface( tSerializer& s, tTextureSurfaceSerializationData& object )
	{
		s( "Format", reinterpret_cast<int&>( object.mFormat ) );
		s( "Width", object.mWidth );
		s( "Height", object.mHeight );
		s( "EncodedBits", object.mEncodedBits );
	}

	template<>
	void fSerializeXmlObject<tXmlSerializer,tTextureSysRam::tSurface>( tXmlSerializer& s, tTextureSysRam::tSurface& object )
	{
		tTextureSurfaceSerializationData data;
		data.mFormat = object.fFormat( );
		data.mWidth = object.fWidth( );
		data.mHeight = object.fHeight( );

		Gfx::tTextureFile::tLockedMip mip = object.fLock( );
		tBase64::fEncode( mip.mBits, mip.mPitch * object.fHeight( ), data.mEncodedBits );
		object.fUnlock( );

		fSerializeSurface( s, data );
	}

	template<>
	void fSerializeXmlObject<tXmlDeserializer,tTextureSysRam::tSurface>( tXmlDeserializer& s, tTextureSysRam::tSurface& object )
	{
		tTextureSurfaceSerializationData data;
		fSerializeSurface( s, data );

		tGrowableArray<Sig::byte> decoded;
		if( data.mEncodedBits.length( ) > 0 )
		{
			object.fAllocate( tReferenceDevice::fInstance( ), data.mWidth, data.mHeight, data.mFormat );

			tBase64::fDecode( &data.mEncodedBits[ 0 ], ( u32 )data.mEncodedBits.length( ), decoded );

			Gfx::tTextureFile::tLockedMip mip = object.fLock( );
			sigassert( mip.mPitch * object.fHeight( ) == decoded.fCount( ) );
			fMemCpy( mip.mBits, decoded.fBegin( ), decoded.fCount( ) );
			object.fUnlock( );
		}
	}


	b32 tTextureSysRam::fRecognizedExtension( const char* path )
	{
		const char* textureExtensions[]=
		{
			".bmp",
			".jpg",
			".png",
			".tga",
			".dds",
		};

		// check input file against all possible texture extensions
		for( u32 ext = 0; ext < array_length( textureExtensions ); ++ext )
			if( StringUtil::fCheckExtension( path, textureExtensions[ ext ] ) )
				return true;

		return false;
	}

	tFilePathPtr tTextureSysRam::fCreateBinaryPath( const tFilePathPtr& inputPath )
	{
		const std::string outputExt = StringUtil::fGetExtension( inputPath.fCStr( ) ) + "b";
		return tFilePathPtr::fSwapExtension( ToolsPaths::fMakeResRelative( inputPath ), outputExt.c_str( ) );
	}

	tFilePathPtr tTextureSysRam::fTexturesIniPath( const tFilePathPtr& inputPath )
	{
		tFilePathPtr texturesIniFileName = tFilePathPtr::fConstructPath( 
			tFilePathPtr( StringUtil::fDirectoryFromPath( inputPath.fCStr( ) ) ),
			tFilePathPtr( "textures.ini" ) );
		return texturesIniFileName;
	}

	b32 tTextureSysRam::fIsDXTFormat( ) const
	{
		return 
			mFormat == Gfx::tTextureFile::cFormatDXT1 ||
			mFormat == Gfx::tTextureFile::cFormatDXT3 ||
			mFormat == Gfx::tTextureFile::cFormatDXT5;
	}

	u32 tTextureSysRam::fNumFacesByType( Gfx::tTextureFile::tType type )
	{
		u32 o = 0;

		switch( type )
		{
		case Gfx::tTextureFile::cType2d: o = 1; break;
		case Gfx::tTextureFile::cTypeCube: o = 6; break;
		default: sigassert( !"invalid case" ); break;
		}

		return o;
	}

	tTextureSysRam::tTextureSysRam( )
	{
		tReferenceDevice::fInstance( ).fAddClient( );

		fClear( );
	}

	tTextureSysRam::~tTextureSysRam( )
	{
		fClear( );

		tReferenceDevice::fInstance( ).fRemoveClient( );
	}

	tTextureSysRam::tTextureSysRam( const tTextureSysRam& other )
	{
		tReferenceDevice::fInstance( ).fAddClient( );

		// safe to copy directly as 'this' is fresh and new
		fCopy( other );
	}

	tTextureSysRam& tTextureSysRam::operator=( const tTextureSysRam& other )
	{
		if( this == &other )
			return *this;

		fClear( );
		fCopy( other );

		return *this;
	}

	void tTextureSysRam::fSetArrayTexture( u32 imgCount, u32 width, u32 height )
	{
		mIsAtlas = true;
		mImages.fNewArray( imgCount );
		mSize.x = width;
		mSize.y = height;
		mNumSubTexX = imgCount;
		mNumSubTexY = 1;
	}

	b32 tTextureSysRam::fLoad( const tFilePathPtr& fileName, b32 autoHandleMipsAndNormalMapFormat, u32 ithImage )
	{
		fClear( );

		fParseDetails( fileName );

		if( !fDetailsValid( ) )
		{
			log_warning( "No formatting was specified for this texture [" << fileName << "]; be sure to specify _d, _i, _n, etc. at the end of your texture name." );
			return false;
		}

		return fLoad( fileName, mSemantic, mFormat, mType, autoHandleMipsAndNormalMapFormat, ithImage );
	}

	b32 tTextureSysRam::fLoad( const tFilePathPtr& fileName, 
		Gfx::tTextureFile::tSemantic semantic,
		Gfx::tTextureFile::tFormat format,
		Gfx::tTextureFile::tType type,
		b32 autoHandleMipsAndNormalMapFormat,
		u32 ithImage )
	{
		mSemantic = semantic;
		mFormat = format;
		mType = type;

		switch( mType )
		{
		case Gfx::tTextureFile::cType2d:	fLoad2D( fileName, ithImage ); break;
		case Gfx::tTextureFile::cTypeCube:	fLoadCube( fileName ); break;
		default:							sigassert( !"invalid texture type" ); break;
		}

		if( !fImagesValid( ) )
			return false;

		fAfterImagesGenerated( autoHandleMipsAndNormalMapFormat );

		return true;
	}

	void tTextureSysRam::fFromSurface( 
		const tSurface& surface, 
		Gfx::tTextureFile::tSemantic semantic,
		Gfx::tTextureFile::tFormat format,
		b32 autoHandleMipsAndNormalMapFormat )
	{
		fClear( );

		mType		= Gfx::tTextureFile::cType2d; // surfaces are 2D only
		mSemantic	= semantic;
		mFormat		= format;

		sigassert( fDetailsValid( ) );

		mImages.fNewArray( 1 );
		mImages.fFront( ).mMips.fNewArray( 1 );
		mImages.fFront( ).mMips.fFront( ).fReset( new tSurface );

		// copy surface
		mImages.fFront( ).mMips.fFront( )->fCopyCpu( tReferenceDevice::fInstance( ), surface );

		sigassert( fImagesValid( ) );

		fAfterImagesGenerated( autoHandleMipsAndNormalMapFormat );
	}

	void tTextureSysRam::fGenerate( u32 width, u32 height, const Math::tVec4f& fillColorRgba,
		Gfx::tTextureFile::tSemantic semantic,
		Gfx::tTextureFile::tFormat format,
		Gfx::tTextureFile::tType type,
		b32 autoHandleMipsAndNormalMapFormat,
		const Math::tVec2u& lowestMipDims )
	{
		fClear( );

		mType		= type;
		mSemantic	= semantic;
		mFormat		= format;

		sigassert( fDetailsValid( ) );

		const u32 fillColorRgbaU32 = Gfx::tTextureVRam::fPackColorR8G8B8A8( fillColorRgba );

		mImages.fNewArray( mType == Gfx::tTextureFile::cType2d ? 1 : 6 );

		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			tImage& image = mImages[ iface ];
			image.mMips.fNewArray( 1 );
			image.mMips.fFront( ).fReset( new tSurface );
			image.mMips.fFront( )->fAllocate( tReferenceDevice::fInstance( ), width, height, Gfx::tTextureFile::cFormatA8R8G8B8 );

			Gfx::tTextureFile::tLockedMip mipData = image.mMips.fFront( )->fLock( );

			for( u32 y = 0; y < height; ++y )
			for( u32 x = 0; x < width; ++x )
			{
				*mipData.fGetTexel<u32>( x, y ) = fillColorRgbaU32;
			}

			image.mMips.fFront( )->fUnlock( );
		}

		sigassert( fImagesValid( ) );

		fAfterImagesGenerated( autoHandleMipsAndNormalMapFormat, lowestMipDims );
	}

	void tTextureSysRam::fScaleCopy( tTextureSysRam& scaledCopy, u32 newWidth, u32 newHeight ) const
	{
		scaledCopy.fGenerate( newWidth, newHeight, Math::tVec4f::cZeroVector, fSemantic( ), fFormat( ), fType( ), false );

		sigassert( scaledCopy.mImages.fCount( ) == mImages.fCount( ) );

		// copy top mip from each face
		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
			scaledCopy.mImages[ iface ].mMips.fFront( )->fFilteredCopyA8R8G8B8( *mImages[ iface ].mMips.fFront( ) );

		// now generate mips and handle normal map conversion
		scaledCopy.fAfterImagesGenerated( true );
	}

	void tTextureSysRam::fClear( )
	{
		mType		= Gfx::tTextureFile::cTypeInvalid;
		mSemantic	= Gfx::tTextureFile::cSemanticInvalid;
		mFormat		= Gfx::tTextureFile::cFormatInvalid;
		mExplicitFormat = false;
		mIsAtlas	= false;
		mNumSubTexX = 0;
		mNumSubTexY = 0;
		mAlphaLevel	= cAlphaNone;
		mSize		= Math::tVec2u::cZeroVector;

		mImages.fDeleteArray( );
	}

	void tTextureSysRam::fCopy( const tTextureSysRam& other )
	{
		// WARNING! assumes 'this' has already been fClear( )'d; this
		// is a fairly safe assumption as this method is private, but if
		// you're implementing any new functionality and using this method, be aware...

		mType		= other.mType;
		mSemantic	= other.mSemantic;
		mFormat		= other.mFormat;
		mExplicitFormat = other.mExplicitFormat;
		mIsAtlas	= other.mIsAtlas;
		mNumSubTexX	= other.mNumSubTexX;
		mNumSubTexY = other.mNumSubTexY;
		mAlphaLevel = other.mAlphaLevel;
		mSize		= other.mSize;

		mImages.fNewArray( other.mImages.fCount( ) );
		for( u32 i = 0; i < mImages.fCount( ); ++i )
		{
			mImages[ i ].mMips.fNewArray( other.mImages[ i ].mMips.fCount( ) );

			for( u32 imip = 0; imip < mImages[ i ].mMips.fCount( ); ++imip )
			{
				mImages[ i ].mMips[ imip ].fReset( new tSurface );

				// copy the underlying surface; this is a 'deep copy', everything
				// else has just been lightweight and structural; here we copy real bitmap bits
				mImages[ i ].mMips[ imip ]->fCopyCpu( tReferenceDevice::fInstance( ), *other.mImages[ i ].mMips[ imip ] );
			}
		}
	}

	b32 tTextureSysRam::fDetailsValid( ) const
	{
		if( mType		== Gfx::tTextureFile::cTypeInvalid ||
			mSemantic	== Gfx::tTextureFile::cSemanticInvalid ||
			mFormat		== Gfx::tTextureFile::cFormatInvalid )
		{
			return false;
		}

		return true;
	}

	void tTextureSysRam::fParseDetails( const tFilePathPtr& fileName )
	{
		// first look for and read the contents of "textures.ini" in
		// the same directory as the given file (if it exists; this allows
		// people to specify explicit formats)
		const tFilePathPtr texturesIniFileName = fTexturesIniPath( fileName );
		std::string texturesIniText = "";
		{
			tDynamicBuffer texturesIni;
			if( FileSystem::fReadFileToBuffer( texturesIni, texturesIniFileName, "" ) )
				texturesIniText = ( const char* )texturesIni.fBegin( );
		}

		// now we parse the filename for semantic, and textures ini text for specific format instructions
		mExplicitFormat = Gfx::tTextureFile::fParseDetails( fileName, texturesIniText, mType, mSemantic, mFormat );
	}

	b32 tTextureSysRam::fImagesValid( ) const
	{
		// ensure we have the proper number of images by type
		switch( mType )
		{
		case Gfx::tTextureFile::cType2d:
			if( mIsAtlas )
				return mImages.fCount( ) > 0;
			else
				return mImages.fCount( ) == 1;
			break;
		case Gfx::tTextureFile::cTypeCube:
			if( mImages.fCount( ) != 6 )
				return false;
			break;
		default:
			sigassert( !"invalid texture type" );
			return false;
		}

		// validate each image
		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			const tImage& image = mImages[ iface ];

			if( image.mMips.fCount( ) == 0 )
				return false;
			for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
				if( image.mMips[ imip ].fNull( ) )
					return false;

			// top mip dimensions must be multiples of 4
			if( fSemanticRequiresMipMaps( ) )
			{
				if( image.mMips[ 0 ]->fWidth( ) % 4 != 0 ||
					image.mMips[ 0 ]->fHeight( ) % 4 != 0 )
				{
					log_warning( "Image dimensions are not multiples of 4." );
					return false;
				}
			}
		}

		return true;
	}

	void tTextureSysRam::fLoad2D( const tFilePathPtr& fileName, u32 ithImage )
	{
		const u32 explicitW = ( mIsAtlas && mSize.x > 0 ) ? mSize.x : D3DX_DEFAULT_NONPOW2;
		const u32 explicitH = ( mIsAtlas && mSize.y > 0 ) ? mSize.y : D3DX_DEFAULT_NONPOW2;

		D3DXIMAGE_INFO imageInfo;
		IDirect3DTexture9* loadedTex = 0;
		const HRESULT hresult = D3DXCreateTextureFromFileEx(
			tReferenceDevice::fInstance( ),
			fileName.fCStr( ),
			explicitW,
			explicitH,
			D3DX_FROM_FILE,
			0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_SCRATCH,
			D3DX_FILTER_NONE,
			D3DX_FILTER_NONE,
			0,
			&imageInfo,
			0,
			&loadedTex );
		sigassert( SUCCEEDED( hresult ) && loadedTex );

		// top mip dimensions must be multiples of 4
		if( fSemanticRequiresMipMaps( ) )
		{
			if( imageInfo.Width % 4 != 0 || imageInfo.Height % 4 != 0 )
			{
				loadedTex->Release( );
				log_warning( "Image dimensions are not multiples of 4." );
				return;
			}
		}

		if( mImages.fCount( ) == 0 )
			mImages.fNewArray( 1 );
		sigassert( ithImage < mImages.fCount( ) && "invalid image index in tTextureSysRam::fLoad2D" );
		tImage& image = mImages[ ithImage ];
		image.mMips.fNewArray( imageInfo.MipLevels );

		for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
		{
			image.mMips[ imip ].fReset( new tSurface );

			// get description of this mip
			D3DSURFACE_DESC levelInfo;
			loadedTex->GetLevelDesc( imip, &levelInfo );

			// allocate my copy of this mip
			image.mMips[ imip ]->fAllocate( tReferenceDevice::fInstance( ), levelInfo.Width, levelInfo.Height, Gfx::tTextureFile::cFormatA8R8G8B8 );
			Gfx::tTextureFile::tLockedMip dst = image.mMips[ imip ]->fLock( );

			// get the bits for the loaded mip
			D3DLOCKED_RECT src;
			loadedTex->LockRect( imip, &src, 0, 0 );

			// copy the bits to my surface
			for( u32 j = 0; j < levelInfo.Height; ++j )
				fMemCpy( dst.mBits + j * dst.mPitch, ( Sig::byte* )src.pBits + j * src.Pitch, src.Pitch );

			// unlock both
			loadedTex->UnlockRect( imip );
			image.mMips[ imip ]->fUnlock( );
		}

		// check for alpha
		const b32 hasAlpha = ( imageInfo.Format == D3DFMT_A8R8G8B8 );
		fSetFormatWithAlpha( hasAlpha );

		// release d3d texture
		loadedTex->Release( );
	}

	void tTextureSysRam::fLoadCube( const tFilePathPtr& fileName )
	{
		D3DXIMAGE_INFO imageInfo;
		IDirect3DCubeTexture9* loadedTex = 0;
		const HRESULT hresult = D3DXCreateCubeTextureFromFileEx(
			tReferenceDevice::fInstance( ),
			fileName.fCStr( ),
			D3DX_DEFAULT_NONPOW2,
			D3DX_FROM_FILE,
			0,
			D3DFMT_A8R8G8B8,
			D3DPOOL_SCRATCH,
			D3DX_FILTER_NONE,
			D3DX_FILTER_NONE,
			0,
			&imageInfo,
			0,
			&loadedTex );
		sigassert( SUCCEEDED( hresult ) && loadedTex );

		// top mip dimensions must be multiples of 4
		if( fSemanticRequiresMipMaps( ) )
		{
			if( imageInfo.Width % 4 != 0 || imageInfo.Height % 4 != 0 )
			{
				loadedTex->Release( );
				log_warning( "Image dimensions are not multiples of 4." );
				return;
			}
		}

		mImages.fNewArray( 6 );

		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			tImage& image = mImages[ iface ];
			image.mMips.fNewArray( imageInfo.MipLevels );

			for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
			{
				image.mMips[ imip ].fReset( new tSurface );

				// get description of this mip
				D3DSURFACE_DESC levelInfo;
				loadedTex->GetLevelDesc( imip, &levelInfo );

				// allocate my copy of this mip
				image.mMips[ imip ]->fAllocate( tReferenceDevice::fInstance( ), levelInfo.Width, levelInfo.Height, Gfx::tTextureFile::cFormatA8R8G8B8 );
				Gfx::tTextureFile::tLockedMip dst = image.mMips[ imip ]->fLock( );

				// get the bits for the loaded mip
				D3DLOCKED_RECT src;
				loadedTex->LockRect( ( D3DCUBEMAP_FACES )iface, imip, &src, 0, 0 );

				// copy the bits to my surface
				for( u32 j = 0; j < levelInfo.Height; ++j )
					fMemCpy( dst.mBits + j * dst.mPitch, ( Sig::byte* )src.pBits + j * src.Pitch, src.Pitch );

				// unlock both
				loadedTex->UnlockRect( ( D3DCUBEMAP_FACES )iface, imip );
				image.mMips[ imip ]->fUnlock( );
			}
		}

		// check for alpha
		const b32 hasAlpha = ( imageInfo.Format == D3DFMT_A8R8G8B8 );
		fSetFormatWithAlpha( hasAlpha );

		// release d3d texture
		loadedTex->Release( );
	}

	void tTextureSysRam::fSetFormatWithAlpha( b32 hasAlpha )
	{
		if( !hasAlpha )
		{
			mAlphaLevel = cAlphaNone;
			return;
		}

		// we know we have at least one bit alpha, 
		// now run through the top mip to look for 8 bit
		mAlphaLevel = cAlpha1Bit;

		// for storing alpha levels
		u32 numAlphaBits = 0;
		tFixedBitArray<256,u32> alphaBits;
		fZeroOut( alphaBits );

		// we can stop as soon as we know we have more than one bit alpha
		for( u32 iface = 0; ( mAlphaLevel == cAlpha1Bit ) && ( iface < mImages.fCount( ) ); ++iface )
		{
			tImage& image = mImages[ iface ];

			// get top mip surface
			sigassert( image.mMips.fCount( ) >= 1 );
			Gfx::tTextureFile::tLockedMip mipBits = image.mMips.fFront( )->fLock( );

			for( u32 y = 0; y < image.mMips.fFront( )->fHeight( ); ++y )
			for( u32 x = 0; x < image.mMips.fFront( )->fWidth( ); ++x )
			{
				Math::tVec4u rgba;
				Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *mipBits.fGetTexel<u32>( x, y ), rgba );
				const u32 alpha = rgba.w;

				const u32 indistinctAlphaDiff = 3;
				if( fInBounds( alpha, 0+indistinctAlphaDiff, 255-indistinctAlphaDiff ) )
				{
					mAlphaLevel = cAlpha8Bit;
					break;
				}

				if( !alphaBits.fGetBit( alpha ) )
				{
					alphaBits.fSetBit( alpha, true );
					if( ++numAlphaBits >= 3 )
					{
						mAlphaLevel = cAlpha8Bit;
						break;
					}
				}
			}

			image.mMips.fFront( )->fUnlock( );
		}

		// now that we know whether we have 1bit or 8bit alpha, if the user
		// wanted default formatting, we can upgrade to dxt5 if appropriate
		if( ( mAlphaLevel == cAlpha8Bit ) && 
			 !mExplicitFormat && 
			( mFormat == Gfx::tTextureFile::cFormatDXT1 ) )
		{
			mFormat = Gfx::tTextureFile::cFormatDXT5;
		}
	}

	b32 tTextureSysRam::fSemanticRequiresMipMaps( ) const
	{
		// we used to disable mips for "gui" textures, but it turns out if you
		// do any scaling/rotating/etc., the textures look way better with mip-filtering
		// for most small images this is not a problem - however for full-screen images
		// that won't be scaled, it's kind of a waste of space - hence we need an
		// option in the textures.ini file to disable the mips (i.e., TODO) for those special cases
		//if( mSemantic == Gfx::tTextureFile::cSemanticGui )
		//	return false;

		if( mSemantic == Gfx::tTextureFile::cSemanticLookUpTable )
			return false;

		return true;
	}

	void tTextureSysRam::fAfterImagesGenerated( b32 autoHandleMipsAndNormalMapFormat, const Math::tVec2u& lowestMipDims )
	{
		sigassert( mImages.fCount( ) > 0 );

		// cache width and height so we don't have to follow the long pointer chain every time
		mSize.x = mImages.fFront( ).mMips.fFront( )->fWidth( );
		mSize.y = mImages.fFront( ).mMips.fFront( )->fHeight( );


		// check if we're a gui texture and premultiply alpha
		if( mSemantic == Gfx::tTextureFile::cSemanticGui )
			fPreMultiplyAlpha( );

		if( autoHandleMipsAndNormalMapFormat )
		{
			// check if we need mips, and generate them if so
			if( mImages.fFront( ).mMips.fCount( ) == 1 && fSemanticRequiresMipMaps( ) )
				fGenerateMipMaps( lowestMipDims );

			// check if we're a normal map and convert to proper format
			if( mSemantic == Gfx::tTextureFile::cSemanticNormal )
				fConvertToNormalMapFormat( );
		}
	}

	void tTextureSysRam::fGenerateMipMaps( const Math::tVec2u& lowestMipDims )
	{
		// generate mip maps for each face
		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			tImage& image = mImages[ iface ];

			// we should have at least one mip (the top one)
			sigassert( image.mMips.fCount( ) >= 1 );
			tSurfacePtr topMip = image.mMips.fFront( );
			sigassert( !topMip.fNull( ) );

			// if we have more than one, we need to get rid of the lower ones
			if( image.mMips.fCount( ) > 1 )
			{
				image.mMips.fNewArray( 1 );
				image.mMips.fFront( ) = topMip;
			}

			// generate mip maps recursively (all the way down to 1x1)
			fGenerateMipMaps( image, topMip->fWidth( ), topMip->fHeight( ), lowestMipDims );
		}
	}

	void tTextureSysRam::fGenerateMipMaps( tImage& image, u32 logicalWidth, u32 logicalHeight, const Math::tVec2u& lowestMipDims )
	{
		tSurfacePtr src = image.mMips.fBack( );
		if( logicalWidth <= lowestMipDims.x || logicalHeight <= lowestMipDims.y )
			return;

		const tProjectFile& projectFile = tProjectFile::fInstance( );
		if( image.mMips.fCount( ) >= tProjectFile::fInstance( ).mEngineConfig.mAssetGenConfig.mMaxMipCount )
			return; //no more mips please

		// currently we can only generate mips for 8888 argb format
		sigassert( src->fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 );

		// add lower level mip
		image.mMips.fPushBack( tSurfacePtr( new tSurface ) );
		tSurfacePtr dst = image.mMips.fBack( );

		// allocate lower level mip
		const Math::tVec2u mipDims = src->fComputeLowerMipDimensions( fIsDXTFormat( ) );
		dst->fAllocate( tReferenceDevice::fInstance( ), mipDims.x, mipDims.y, src->fFormat( ) );

		// perform a filtered copy of src into dst
		dst->fFilteredCopyA8R8G8B8( *src );

		// recurse
		fGenerateMipMaps( image, fMax( 1u, logicalWidth / 2 ), fMax( 1u, logicalHeight / 2 ), lowestMipDims );
	}

	void tTextureSysRam::fConvertToNormalMapFormat( )
	{
		// for normal maps, we renormalize and pack into special DXT5n format (alpha and green-channels)
		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			tImage& image = mImages[ iface ];
			for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
				image.mMips[ imip ]->fConvertToNormalMapFormat( );
		}
	}

	void tTextureSysRam::fPreMultiplyAlpha( )
	{
		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			const tImage& image = mImages[ iface ];

			for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
			{
				const tSurfacePtr src = image.mMips[ imip ];

				sigassert( src->fFormat( ) == Gfx::tTextureFile::cFormatA8R8G8B8 );

				Gfx::tTextureFile::tLockedMip mipBits = src->fLock( );

				const u32 width = src->fWidth( );
				const u32 height = src->fHeight( );

				// walk pixels
				for( u32 x = 0; x < width; ++x )
				for( u32 y = 0; y < height; ++y )
				{
					u32 * texel = mipBits.fGetTexel<u32>( x, y );

					Math::tVec4u rgba;
					Gfx::tTextureVRam::fUnpackColorR8G8B8A8( *texel, rgba );

					rgba.x = ( rgba.x * rgba.w ) / 255;
					rgba.y = ( rgba.y * rgba.w ) / 255;
					rgba.z = ( rgba.z * rgba.w ) / 255;

					*texel = Gfx::tTextureVRam::fPackColorR8G8B8A8( rgba.x, rgba.y, rgba.z, rgba.w ); 
				}

				src->fUnlock( );
			}
		}
	}

	void tTextureSysRam::fConvertRawFaceMips( tRawFaceMipMapSet& rawFaceMips, b32 noConvert ) const
	{
		rawFaceMips.fNewArray( mImages.fCount( ) );

		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			const tImage& image = mImages[ iface ];

			// set all mip-maps; we don't auto-generate them here, we use
			// what we've got (if they were needed, they should already have
			// been auto-generated during load)
			rawFaceMips[ iface ].fNewArray( image.mMips.fCount( ) );
			for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
			{
				// save current mip to file in memory
				ID3DXBuffer* fileInMemory = 0;
				D3DXSaveTextureToFileInMemory( &fileInMemory, D3DXIFF_DDS, ( IDirect3DTexture9* )image.mMips[ imip ]->fGetPlatformHandle( ), 0 );
				sigassert( fileInMemory );

				// compute binary texture format
				const D3DFORMAT d3dFormat = noConvert ? D3DFMT_A8R8G8B8 : fConvertFormat( mFormat, mSemantic, fAlphaLevel( ) );

				// now load back to a new texture
				IDirect3DTexture9* loadedTex = 0;
				const HRESULT hresult = D3DXCreateTextureFromFileInMemoryEx(
					tReferenceDevice::fInstance( ),
					fileInMemory->GetBufferPointer( ),
					fileInMemory->GetBufferSize( ),
					D3DX_DEFAULT_NONPOW2,
					D3DX_DEFAULT_NONPOW2,
					1,
					0,
					d3dFormat,
					D3DPOOL_SCRATCH,
					D3DX_FILTER_NONE,
					D3DX_FILTER_NONE,
					0,
					0,
					0,
					&loadedTex );
				sigassert( SUCCEEDED( hresult ) && loadedTex );

				// release in-memory buffer, we're done with it
				fileInMemory->Release( );
				fileInMemory = 0;

				// get level description for temp texture
				D3DSURFACE_DESC levelInfo;
				loadedTex->GetLevelDesc( 0, &levelInfo );

				// grab the raw bits from the temp texture
				D3DLOCKED_RECT loadedTexBits;
				loadedTex->LockRect( 0, &loadedTexBits, 0, 0 );
				const u32 totalMipSizeInBytes = fConvertPitch( loadedTexBits.Pitch, d3dFormat ) * levelInfo.Height;
				rawFaceMips[ iface ][ imip ].fNewArray( totalMipSizeInBytes );
				fMemCpy( rawFaceMips[ iface ][ imip ].fBegin( ), loadedTexBits.pBits, totalMipSizeInBytes );

				// cleanup
				loadedTex->UnlockRect( 0 );
				loadedTex->Release( );
				loadedTex = 0;
			}
		}
	}

	void tTextureSysRam::fConvertToVRamTexture( const Gfx::tDevicePtr& device, Gfx::tTextureVRam& vramTex ) const
	{
		sigassert( mImages.fCount( ) > 0 && mImages.fFront( ).mMips.fCount( )  > 0 );

		const tImage& face0 = fGetFace( );

		vramTex.fAllocate( device, fWidth( ), fHeight( ), face0.mMips.fCount( ), fFormat( ), fType( ) );

		tRawFaceMipMapSet rawFaceMips;

		fConvertRawFaceMips( rawFaceMips );

		for( u32 iface = 0; iface < rawFaceMips.fCount( ); ++iface )
		{
			for( u32 imip = 0; imip < rawFaceMips[ iface ].fCount( ); ++imip )
			{
				const tDynamicBuffer& rawBits = rawFaceMips[ iface ][ imip ];

				Gfx::tTextureFile::tLockedMip vramMip = vramTex.fLockMip( imip, iface );

				fMemCpy( vramMip.mBits, rawBits.fBegin( ), rawBits.fCount( ) );

				vramTex.fUnlockMip( imip, iface );
			}
		}
	}

	void tTextureSysRam::fSaveGameBinary( const tFilePathPtr& outputFileName, tPlatformId pid ) const
	{
		tRawFaceMipMapSet rawFaceMips;

		// convert/compress mip maps for each face

		b32 preConvertMips = true;
		if( pid == cPlatformXbox360 )
			preConvertMips = Dx360Util::fTextureConversionWantsPreConvertedMips( );

		fConvertRawFaceMips( rawFaceMips, !preConvertMips );

		// okay, we have all our raw mip-map data, time to output

		tFileWriter ofile( outputFileName );
		if( !ofile.fIsOpen( ) )
		{
			log_warning( "Couldn't open file [" << outputFileName << "] for writing." );
			return;
		}

		fSaveGameBinary( rawFaceMips, ofile, pid );
	}

	void tTextureSysRam::fSaveGameBinary( tRawFaceMipMapSet& rawFaceMips, tFileWriter& ofile, tPlatformId pid ) const
	{
		Gfx::tTextureFile texFile;
		texFile.mType = mType;
		texFile.mSemantic = mSemantic;
		texFile.mFormat = mFormat;
		texFile.mWidth = fWidth( );
		texFile.mHeight = fHeight( );
		texFile.mMipMapCount = fNumMips( );
		texFile.mIsAtlas = mIsAtlas;
		texFile.mSubTexWidth = mNumSubTexX > 0 ? ( fWidth( ) / mNumSubTexX ) : 0;
		texFile.mSubTexHeight = mNumSubTexY > 0 ? ( fHeight( ) / mNumSubTexY ) : 0;
		texFile.mSubTexCountX = mNumSubTexX;
		texFile.mSubTexCountY = mNumSubTexY;

		// For all semantics but render targets, build out the space for the mips
		if( mSemantic != Gfx::tTextureFile::cSemanticRenderTarget )
		{
			texFile.mImages.fNewArray( mImages.fCount( ) );
			sigassert( rawFaceMips.fCount( ) == mImages.fCount( ) );
			for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
			{
				const tImage& image = mImages[ iface ];
				texFile.mImages[ iface ].mMipMapBuffers.fNewArray( image.mMips.fCount( ) );

				sigassert( rawFaceMips[ iface ].fCount( ) == image.mMips.fCount( ) );
				for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
				{
					texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferOffset = 0;
					texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize = rawFaceMips[ iface ][ imip ].fCount( );
				}
			}
		}


		// make sure we set the signature prior to serializing
		texFile.fSetSignature<Gfx::tTextureFile>( pid );

		// create load in place serializer, and serialize the file header (persistent portion of the file)
		tLoadInPlaceSerializer ser;
		texFile.mHeaderSize = ser.fSave( texFile, ofile, pid );

		// write complete mip chain for all faces
		if( mSemantic != Gfx::tTextureFile::cSemanticRenderTarget )
			fOutputMipChain( texFile, rawFaceMips, ofile, pid );

		// seek back to the start of the file
		ofile.fSeek( 0 );

		// re-write the table of header information now that it has proper offsets
		const u32 headerSizeVerify = ser.fSave( texFile, ofile, pid );

		// sanity check
		sigassert( texFile.mHeaderSize == headerSizeVerify );
	}

	void tTextureSysRam::fOutputMipChain( Gfx::tTextureFile& texFile, tRawFaceMipMapSet& rawFaceMips, tFileWriter& ofile, tPlatformId pid ) const
	{
		// continue saving mip buffers from the current location of the output file;
		// we'll store the offset of each buffer in the header
		if( pid == cPlatformPcDx9 )
		{
			u32 offsetFromStartOfFile = texFile.mHeaderSize;
			for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
			{
				const tImage& image = mImages[ iface ];

				for( u32 imip = 0; imip < image.mMips.fCount( ); ++imip )
				{
					// store current file offset
					texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferOffset = offsetFromStartOfFile;

					// store buffer size (again, just in case)
					texFile.mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize = rawFaceMips[ iface ][ imip ].fCount( );

					// write mip buffer
					ofile( rawFaceMips[ iface ][ imip ].fBegin( ), rawFaceMips[ iface ][ imip ].fCount( ) );

					// increment current file offset
					offsetFromStartOfFile += rawFaceMips[ iface ][ imip ].fCount( );
				}
			}
		}
		else if( pid == cPlatformXbox360 )
		{
			Dx360Util::fOutputMipChain( texFile, rawFaceMips, ofile );
		}
		else
		{
			log_warning( "Unsupported platform (" << fPlatformIdString( pid ) << ") for texture generation." );
		}
	}



	tTextureAtlasSysRam::tTextureAtlasSysRam( )
		: mNumTexturesX( 0 ), mNumTexturesY( 0 )
		, mSubTexWidth( 0 ), mSubTexHeight( 0 )
	{
	}

	void tTextureAtlasSysRam::fAllocate( 
		u32 numTexturesX, 
		u32 numTexturesY, 
		u32 subTexWidth, 
		u32 subTexHeight,
		Gfx::tTextureFile::tSemantic semantic,
		Gfx::tTextureFile::tFormat format )
	{
		sigassert( numTexturesX * numTexturesY * subTexWidth * subTexHeight != 0 );

		mNumTexturesX = numTexturesX;
		mNumTexturesY = numTexturesY;
		mSubTexWidth = subTexWidth;
		mSubTexHeight = subTexHeight;

		const Math::tVec4f ogFillRgba( 
			0.5f, 
			0.5f, 
			semantic == Gfx::tTextureFile::cSemanticNormal ? 1.f : 0.5f,
			semantic == Gfx::tTextureFile::cSemanticNormal ? 0.f : 1.f );
		const Math::tVec2u lowestMipDims = Math::tVec2u( numTexturesX * 4, numTexturesY * 4 );
		mTexture.fGenerate( mNumTexturesX * mSubTexWidth, mNumTexturesY * mSubTexHeight, ogFillRgba, semantic, format, Gfx::tTextureFile::cType2d, true, lowestMipDims );
		mTexture.mIsAtlas = true;
		mTexture.mNumSubTexX = numTexturesX;
		mTexture.mNumSubTexY = numTexturesY;
	}

	void tTextureAtlasSysRam::fUpdateSubTexture( u32 xTexIndex, u32 yTexIndex, const tTextureSysRam& newTex )
	{
		sigassert( newTex.fType( ) == Gfx::tTextureFile::cType2d );
		sigassert( xTexIndex < mNumTexturesX );
		sigassert( yTexIndex < mNumTexturesY );
		sigassert( mSubTexWidth == newTex.fWidth( ) );
		sigassert( mSubTexHeight == newTex.fHeight( ) );

		const u32 numMips = mTexture.fGetFace( ).mMips.fCount( );
		for( u32 imip = 0; imip < numMips; ++imip )
		{
			const tTextureSysRam::tSurface& src = *newTex.fGetFace( ).mMips[ imip ];

			const u32 xPixIndex = xTexIndex * src.fWidth( );
			const u32 yPixIndex = yTexIndex * src.fHeight( );

			tTextureSysRam::tSurface& dst = *mTexture.fGetFace( ).mMips[ imip ];
			dst.fUnfilteredCopyA8R8G8B8ToSubRect( src, xPixIndex, yPixIndex );
		}
	}

	void tTextureAtlasSysRam::fConvertToVRamTexture( const Gfx::tDevicePtr& device, Gfx::tTextureVRam& vramTex ) const
	{
		mTexture.fConvertToVRamTexture( device, vramTex );
	}

	void tTextureAtlasSysRam::fConvertToSysRamTexture( tTextureSysRam& sysRamTex ) const
	{
		sysRamTex = mTexture;
	}

}


