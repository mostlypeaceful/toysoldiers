#include "BasePch.hpp"
#include "tTextureFile.hpp"
#include "tDevice.hpp"
#include "tProfiler.hpp"
#include "tGameAppBase.hpp"

namespace Sig { namespace Gfx
{
	const u32		tTextureFile::cVersion = 1;


	tTextureFile::tSurfacePointer::tSurfacePointer( )
		: mBufferOffset( 0 )
		, mBufferSize( 0 )
	{
	}

	tTextureFile::tSurfacePointer::tSurfacePointer( tNoOpTag )
	{
	}

	tTextureFile::tImage::tImage( )
	{
	}

	tTextureFile::tImage::tImage( tNoOpTag )
		: mMipMapBuffers( cNoOpTag )
	{
	}

	tTextureFile::tTextureFile( )
		: mPlatformHandle( 0 )
		, mHeaderSize( 0 )
		, mDiscardSurfaceBuffers( false )
		, mMipMapCount( 0 )
		, mWidth( 0 )
		, mHeight( 0 )
		, mType( cTypeInvalid )
		, mSemantic( cSemanticInvalid )
		, mFormat( cFormatInvalid )
		, mIsAtlas( false )
		, mSubTexWidth( 0 )
		, mSubTexHeight( 0 )
		, mSubTexCountX( 0 )
		, mSubTexCountY( 0 )
	{
	}

	tTextureFile::tTextureFile( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mImages( cNoOpTag )
	{
	}

	void tTextureFile::fOnFileLoaded( const tResource& ownerResource )
	{
#ifdef target_tools
		// TODO verifY that the corresponding res version of the resource is present, and warn if not
#endif//target_tools

		tDevicePtr device = tDevice::fGetDefaultDevice( );

		// low detail texture setting drops the top mipmap
		// don't drop the top mipmap for 1280x720 textures as they are all fullscreen background and look noticably low rez
		#define ISPOWEROF2(n) ( ((n)&((n)-1)) == 0 && (n) != 0 )
		b32 useLowDetailTextures = false;
#ifdef target_game
		useLowDetailTextures = tGameAppBase::fInstance( ).fUseLowDetailTextures(  ) && (mImages.fCount( ) == 1) && mImages[ 0 ].mMipMapBuffers.fCount( ) > 1 && ISPOWEROF2(mWidth) && ISPOWEROF2(mHeight);
#endif

		fCreateTextureInternal( device, useLowDetailTextures );

		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			for( u32 imip = 0; imip < mImages[ iface ].mMipMapBuffers.fCount( ); ++imip )
			{
				const Sig::byte* mipBuffer = ( const Sig::byte* )this + mImages[ iface ].mMipMapBuffers[ imip ].mBufferOffset;
				if (useLowDetailTextures)
				{
					if (imip > 0)
						fCopyMipInternal( device, iface, imip-1, mipBuffer, mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize );
				}
				else
				{
					fCopyMipInternal( device, iface, imip, mipBuffer, mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize );
				}
			}
		}

		profile_mem(cProfileMemTexture, +fComputeStorage( ));
	}

	void tTextureFile::fResizeAfterLoad( tGenericBuffer* fileBuffer )
	{
		// this is pretty ghetto and not for the faint of heart; ideally
		// the load-in-place serialization system could generalize a solution
		// to relocating memory; since we know we only have a few pointers
		// that need fixing up, this is pretty safe and easy...
		if( mDiscardSurfaceBuffers )
		{
			const Sig::byte* initialLocation = fileBuffer->fGetBuffer( );

			fileBuffer->fResize( mHeaderSize );

			const Sig::byte* newLocation = fileBuffer->fGetBuffer( );
			tTextureFile* newThis = ( tTextureFile* )newLocation;

			const ptrdiff_t delta = newLocation - initialLocation;

			newThis->fRelocateLoadInPlaceTables( delta );
			newThis->mImages.fRelocateInPlace( delta );
			for( u32 iface = 0; iface < newThis->mImages.fCount( ); ++iface )
			{
				newThis->mImages[ iface ].mMipMapBuffers.fRelocateInPlace( delta );
			}
		}
	}
	void tTextureFile::fOnFileUnloading( )
	{
		profile_mem(cProfileMemTexture, -fComputeStorage( ));

		fDestroyTextureInternal( );
		mPlatformHandle = 0;
	}
	s32 tTextureFile::fComputeStorage( ) const
	{
		u32 numBytes = 0;
		for( u32 iface = 0; iface < mImages.fCount( ); ++iface )
		{
			for( u32 imip = 0; imip < mImages[ iface ].mMipMapBuffers.fCount( ); ++imip )
				numBytes += mImages[ iface ].mMipMapBuffers[ imip ].mBufferSize;
		}
		return numBytes;
	}
	f32 tTextureFile::fComputeStorageInMB( ) const
	{
		return fComputeStorage( ) / ( 1024.f * 1024.f );
	}
	b32 tTextureFile::fParseDetails( 
		const tFilePathPtr& filePath, 
		const std::string& iniText, 
		tType& typeOut, 
		tSemantic& semanticOut, 
		tFormat& formatOut )
	{
		// start by setting details to invalid
		typeOut			= cTypeInvalid;
		semanticOut		= cSemanticInvalid;
		formatOut		= cFormatInvalid;

		// find last '_' before extension... 

		const char* lastDirectorySlash = strrchr( filePath.fCStr( ), fPlatformFilePathSlash( cCurrentPlatform ) );
		if( !lastDirectorySlash )
			return false; // no luck, bail
		const char* lastUnderScore = strrchr( lastDirectorySlash, '_' );
		if( !lastUnderScore || !*lastUnderScore || !*(lastUnderScore + 1) )
			return false; // no luck, bail

		const char* texDetailsString = lastUnderScore + 1;
		// the next character must be a dot
		if( *( texDetailsString + 1 ) != '.' )
			return false;

		switch( *texDetailsString )
		{
		// diffuse map
		case 'd':
		case 'D':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticDiffuse;
			formatOut = Gfx::tTextureFile::cFormatDXT1;
			break;

		// specular map
		case 's':
		case 'S':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticSpecular;
			formatOut = Gfx::tTextureFile::cFormatDXT1;
			break;

		// emissive ('i' is for self-Illumination)
		case 'i':
		case 'I':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticEmissive;
			formatOut = Gfx::tTextureFile::cFormatDXT1;
			break;

		// font map (no mips, 8-bit single channel, quality more important by default)
		case 'f':
		case 'F':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticFont;
			formatOut = Gfx::tTextureFile::cFormatA8;
			break;

		// gui map (no mips, quality more important by default)
		case 'g':
		case 'G':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticGui;
			formatOut = Gfx::tTextureFile::cFormatA8R8G8B8;
			break;

		// normal map
		case 'n':
		case 'N':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticNormal;
			formatOut = Gfx::tTextureFile::cFormatDXT5;
			break;

		// opacity map (mips, 8-bit alpha channel only)
		case 'o':
		case 'O':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticEmissive;
			formatOut = Gfx::tTextureFile::cFormatA8;
			break;

		// reflection (cube) map
		case 'r':
		case 'R':
			typeOut = Gfx::tTextureFile::cTypeCube;
			semanticOut = Gfx::tTextureFile::cSemanticReflection;
			formatOut = Gfx::tTextureFile::cFormatDXT1;
			break;

		// look up table ('t' is for table; values should be treated as integers, no mips, no filtering)
		case 't':
		case 'T':
			typeOut = Gfx::tTextureFile::cType2d;
			semanticOut = Gfx::tTextureFile::cSemanticLookUpTable;
			formatOut = Gfx::tTextureFile::cFormatA8R8G8B8;
			break;

		// TODO light map?
		// TODO ambient occlusion?
		// TODO opacity?

		default: // unrecognized texture type, bail
			return false;
		}

		const char formatStringStartChar = '=';
		const char formatStringEndChar = '\n';

		// now try and parse format, if specified
		const std::string simplePathName = StringUtil::fNameFromPath( filePath.fCStr( ) );
		const char* formatString = StringUtil::fStrStrI( iniText.c_str( ), simplePathName.c_str( ) );
		if( !formatString || !*formatString )
			return false; // false means format was not explicitly specified

		// start by finding the end of the line
		const char* formatStringEndOfLine = strchr( formatString, formatStringEndChar );
		if( !formatStringEndOfLine )
		{
			formatStringEndOfLine = strchr( formatString, '\0' );
			if( !formatStringEndOfLine )
				return false;
		}

		// now look for format start (equals sign)
		formatString += simplePathName.length( );
		formatString = strchr( formatString, formatStringStartChar );

		// no equals sign specified
		if( !formatString || formatString >= formatStringEndOfLine )
			return false;

		// advance past equals sign
		++formatString;

		// advance to first non-white-space after equals sign
		while( isspace( *formatString ) && formatString < formatStringEndOfLine )
			++formatString;

		if( formatString == formatStringEndOfLine )
			return false;// no non-white space after equals sign

		// this is the start of the format string
		const char* formatStringBegin = formatString;

		// find the end (first whitespace)
		while( !isspace( *formatString ) && formatString < formatStringEndOfLine )
			++formatString;

		// this is the end of the format string
		const char* formatStringEnd = formatString;

		const u32 len = fPtrDiff( formatStringEnd, formatStringBegin );

		Gfx::tTextureFile::tFormat parsedFormat = Gfx::tTextureFile::cFormatInvalid;

		if( StringUtil::fStrnicmp( formatStringBegin, "8888", len )==0 )
		{
			parsedFormat = Gfx::tTextureFile::cFormatA8R8G8B8;
			log_line( Log::cFlagGraphics, "\tFound texture format override: 8888" );
		}
		else if( StringUtil::fStrnicmp( formatStringBegin, "dxt1", len )==0 )
		{
			parsedFormat = Gfx::tTextureFile::cFormatDXT1;
			log_line( Log::cFlagGraphics, "\tFound texture format override: DXT1" );
		}
		else if( StringUtil::fStrnicmp( formatStringBegin, "dxt3", len )==0 )
		{
			parsedFormat = Gfx::tTextureFile::cFormatDXT3;
			log_line( Log::cFlagGraphics, "\tFound texture format override: DXT3" );
		}
		else if( StringUtil::fStrnicmp( formatStringBegin, "dxt5", len )==0 )
		{
			parsedFormat = Gfx::tTextureFile::cFormatDXT5;
			log_line( Log::cFlagGraphics, "\tFound texture format override: DXT5" );
		}
		else if( StringUtil::fStrnicmp( formatStringBegin, "565", len )==0 )
		{
			parsedFormat = Gfx::tTextureFile::cFormatR5G6B5;
			log_line( Log::cFlagGraphics, "\tFound texture format override: 565" );
		}
		else if( StringUtil::fStrnicmp( formatStringBegin, "a8", len )==0 )
		{
			parsedFormat = Gfx::tTextureFile::cFormatA8;
			log_line( Log::cFlagGraphics, "\tFound texture format override: A8" );
		}

		if( parsedFormat == Gfx::tTextureFile::cFormatInvalid )
			return false;

		formatOut = parsedFormat;
		return true;
	}

	void tTextureFile::fApply( const tDevicePtr& device, u32 slot ) const
	{
		fApply(
			mPlatformHandle,
			device, 
			slot, 
			( mSemantic == tTextureFile::cSemanticGui ) ? tTextureFile::cFilterModeNone : tTextureFile::cFilterModeWithMip,
			tTextureFile::cAddressModeWrap,
			tTextureFile::cAddressModeWrap,
			tTextureFile::cAddressModeWrap );
	}

	void tTextureFile::fApply( 
		const tDevicePtr& device, 
		u32 slot, 
		tFilterMode filter, 
		tAddressMode u, 
		tAddressMode v, 
		tAddressMode w ) const
	{
		fApply(
			mPlatformHandle,
			device, 
			slot, 
			filter,
			u,
			v,
			w );
	}

	void tTextureFile::fCreateTextureInternal( const tDevicePtr& device, b32 lowDetailTextures )
	{
		const u32 arrayCount = ( mIsAtlas ? mImages.fCount( ) : 0 );
		if( lowDetailTextures )
			mPlatformHandle = fCreateTexture( device, mWidth/2, mHeight/2, mMipMapCount-1, mFormat, mType, arrayCount );
		else
			mPlatformHandle = fCreateTexture( device, mWidth, mHeight, mMipMapCount, mFormat, mType, arrayCount );
	}

	void tTextureFile::fDestroyTextureInternal( )
	{
		fDestroyTexture( mPlatformHandle );
		mPlatformHandle = 0;
	}

}}
