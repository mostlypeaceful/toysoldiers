#include "BasePch.hpp"
#include "tPlatform.hpp"

#include "Gfx/tVertexFormat.hpp"
#include "Gfx/tTextureFile.hpp"

namespace Sig
{
	///
	/// BuildConfig.hpp functions implemented for backwards compatability
	///

	const char* fPlatformIdString( tPlatformId pid )
	{
		return tPlatformInfo(pid).fFolderName().fCStr();
	}

	char fPlatformFilePathSlash( tPlatformId pid )
	{
		//Bugfix for nasty "static order initilization fiasco"
		//  a real fix would be to remove all statics that do any sort of work
		//return tPlatformInfo(pid).fFilesystemSeperator();
		
		const char cNoClue = '?';
		switch( pid )
		{
		case cPlatformWii:		sigassert( !"no clue" ); return cNoClue;
		case cPlatformPcDx9:	return '\\';
		case cPlatformPcDx10:	return '\\';
		case cPlatformXbox360:	return '\\';
		case cPlatformPs3Ppu:	sigassert( !"no clue" ); return cNoClue;
		case cPlatformPs3Spu:	sigassert( !"no clue" ); return cNoClue;
		case cPlatformiOS:		return '/';
		case cPlatformAndroid:	return '/';
		case cPlatformPcDx11:	return '\\';
		case cPlatformMetro:	return '\\';
		default: sig_nodefault( );
		}
		return cNoClue;

	}
	
	b32 fPlatformFileCaseSensitive( tPlatformId pid )
	{
		return tPlatformInfo(pid).fCaseSensitiveFilesystem();
	}

	tEndianType fPlatformEndianType( tPlatformId pid )
	{
		return tPlatformInfo(pid).fEndian();
	}

	class tPlatformInfo::tImpl : public tRefCounter
	{
	public:
		u32 mSupportedTextureFormats;
		u32 mSupportedVertexFormats;
		tStringPtr mSystemName;
		tStringPtr mShortName;
		tStringPtr mLongName;
		char mPathSeperator;
		b32  mPathCaseSensitive;
		tEndianType mEndian;

		tShaderFamily mShaderFamily;
		tShaderModel mShaderModelPrefered;

		void fMetadata(
			const char* flagname,
			const char* shortname,
			const char* longname )
		{
			mSystemName = tStringPtr( flagname );
			mShortName = tStringPtr( shortname );
			mLongName = tStringPtr( longname );
		}

		void fCore(
			const char* pathSeperator,
			b32 caseSensitive,
			tEndianType endian )
		{
			mPathSeperator = pathSeperator[0];
			mPathCaseSensitive = caseSensitive;
			mEndian = endian;
		}

		void fGraphics(
			tShaderModel preferredSm,
			u32 guaranteedTextureExt,
			b32 dxt,
			b32 argb,
			b32 sixteenBitTextures,
			b32 sixteenBitTexcoords )
		{
			// Texture formats

			mSupportedTextureFormats = (1<<Gfx::tTextureFile::cFormatA8);

			if( dxt ) mSupportedTextureFormats |=
				(1<<Gfx::tTextureFile::cFormatDXT1) |
				(1<<Gfx::tTextureFile::cFormatDXT3) |
				(1<<Gfx::tTextureFile::cFormatDXT5) ;

			if( argb )
				mSupportedTextureFormats |= (1<<Gfx::tTextureFile::cFormatA8R8G8B8);
			else
				mSupportedTextureFormats |= (1<<Gfx::tTextureFile::cFormatA8B8G8R8);

			if( sixteenBitTextures )
				mSupportedTextureFormats |= (1<<Gfx::tTextureFile::cFormatR5G6B5);

			// Vertex formats

			mSupportedVertexFormats =
				(1<<Gfx::tVertexElement::cFormat_f32_1) |
				(1<<Gfx::tVertexElement::cFormat_f32_2) |
				(1<<Gfx::tVertexElement::cFormat_f32_3) |
				(1<<Gfx::tVertexElement::cFormat_f32_4) |
				(1<<Gfx::tVertexElement::cFormat_u8_4) |
				(1<<Gfx::tVertexElement::cFormat_u8_4_Color) |
				(1<<Gfx::tVertexElement::cFormat_u8_4_Normalized) ;

			if( sixteenBitTexcoords ) mSupportedVertexFormats |=
				(1<<Gfx::tVertexElement::cFormat_f16_2) |
				(1<<Gfx::tVertexElement::cFormat_f16_4) ;
		}
	};

	define_static_function( fRegisterAssetGenPlatformData )
	{
		tPlatformInfo::fInitAll( );
	}

	tPlatformInfo::tPlatformInfo( tPlatformId pid )
	{
		static tFixedArray<tRefCounterPtr<tPlatformInfo::tImpl>,cPlatformLastPlusOne> gPlatformInfoRegistry;

		if( !gPlatformInfoRegistry[pid] )
			gPlatformInfoRegistry[pid].fReset( NEW tImpl() );

		mImpl = gPlatformInfoRegistry[pid];

		fInitAll();
	}

	tPlatformInfo::tPlatformInfo()
	{
	}

	tPlatformInfo::~tPlatformInfo()
	{
	}

	b32 tPlatformInfo::fIsTextureFormatSupported( u32 format ) const
	{
		return mImpl->mSupportedTextureFormats & (1<<format);
	}

	b32 tPlatformInfo::fIsVertexFormatSupported( u32 format ) const
	{
		return mImpl->mSupportedVertexFormats & (1<<format);
	}

	b32 tPlatformInfo::fIsVertexShaderModelSupported( tShaderModel sm ) const { return mImpl->mShaderModelPrefered == sm; }
	b32 tPlatformInfo::fIsPixelShaderModelSupported( tShaderModel sm ) const { return mImpl->mShaderModelPrefered == sm; }
	b32 tPlatformInfo::fIsCommonShaderModelSupported( tShaderModel sm ) const { return mImpl->mShaderModelPrefered == sm; }

	tStringPtr tPlatformInfo::fFolderName() const { return mImpl->mSystemName; }
	tStringPtr tPlatformInfo::fFlagName() const { return mImpl->mSystemName; }
	tStringPtr tPlatformInfo::fShortDisplayName() const { return mImpl->mShortName; }
	tStringPtr tPlatformInfo::fLongDisplayName() const { return mImpl->mLongName; }

	tEndianType tPlatformInfo::fEndian() const
	{
		return mImpl->mEndian;
	}

	b32 tPlatformInfo::fCaseSensitiveFilesystem() const
	{
		return mImpl->mPathCaseSensitive;
	}

	char tPlatformInfo::fFilesystemSeperator() const
	{
		return mImpl->mPathSeperator;
	}

	void tPlatformInfo::fInitAll( )
	{
		static b32 once=false;
		if( once )
			return;
		once = true;

		tImpl* pcdx9	= tPlatformInfo(cPlatformPcDx9).mImpl.fGetRawPtr();
		tImpl* pcdx10	= tPlatformInfo(cPlatformPcDx10).mImpl.fGetRawPtr();
		tImpl* pcdx11	= tPlatformInfo(cPlatformPcDx11).mImpl.fGetRawPtr();
		tImpl* metro	= tPlatformInfo(cPlatformMetro).mImpl.fGetRawPtr();
		tImpl* xbox360	= tPlatformInfo(cPlatformXbox360).mImpl.fGetRawPtr();
		tImpl* ios		= tPlatformInfo(cPlatformiOS).mImpl.fGetRawPtr();
		tImpl* android	= tPlatformInfo(cPlatformAndroid).mImpl.fGetRawPtr();
		tImpl* wii		= tPlatformInfo(cPlatformWii).mImpl.fGetRawPtr();
		tImpl* ps3ppu	= tPlatformInfo(cPlatformPs3Ppu).mImpl.fGetRawPtr();
		tImpl* ps3spu	= tPlatformInfo(cPlatformPs3Spu).mImpl.fGetRawPtr();

		//						flag		short		long
		pcdx9	->fMetadata( "pcdx9",	"PC/DX9",	"Windows (DirectX 9)" );
		pcdx10	->fMetadata( "pcdx10",	"PC/DX10",	"Windows (DirectX 10)" );
		pcdx11	->fMetadata( "pcdx11",	"PC/DX11",	"Windows (DirectX 11)" );
		metro	->fMetadata( "metro",	"Metro",	"Metro (DirectX 11)" );
		xbox360	->fMetadata( "xbox360",	"XBox 360",	"XBox 360 (DirectX 9)" );
		ios		->fMetadata( "ios",		"iOS",		"iOS" );
		android	->fMetadata( "android",	"Android",	"Android" );
		wii		->fMetadata( "wii",		"Wii",		"Wii" );
		ps3ppu	->fMetadata( "ps3ppu",	"PS3/PPU",	"PS3 (PPU)" );
		ps3spu	->fMetadata( "ps3spu",	"PS3/SPU",	"PS3 (SPU)" );


		//			seperator, case sensitive, endian
		pcdx9	->fCore( "\\", false, cLittleEndian	); // x86
		pcdx10	->fCore( "\\", false, cLittleEndian	);
		pcdx11	->fCore( "\\", false, cLittleEndian	);
		metro	->fCore( "\\", false, cLittleEndian	); // x86 and little-endian arm
		xbox360	->fCore( "\\", false, cBigEndian	); // ppc
		ios		->fCore( "/" , true , cLittleEndian	); // arm is bi-endian but ios chooses little endian.
		android	->fCore( "/" , true , cLittleEndian	); // likely arm in little-endian mode.  Here's hoping!
		// N.B. OS X, if we ever chose to target it, can be ppc (big endian)
		// or x86 (little endian) so this "one endian per platform" thing may
		// eventually break down.


		//	largely untested bullshit:
		//																	tex		texcoord
		//					preferred		ext		dxt?	argb?	16?		16?
		pcdx9	->fGraphics( cHlsl3,		2048,	true,	true,	true,	true );
		pcdx10	->fGraphics( cHlsl4,		2048,	true,	true,	true,	false );
		pcdx11	->fGraphics( cHlsl4,		2048,	true,	true,	true,	false );
		metro	->fGraphics( cHlsl2,		2048,	true,	true,	true,	false ); // if we want to target low end / arm, 
		xbox360	->fGraphics( cHlsl3,		4096,	true,	true,	true,	true );
		ios		->fGraphics( cGlslEs1_1,	4096,	false,	true,	false,	false );
		android	->fGraphics( cGlslEs1_1,	1024,	false,	true,	false,	false );
	}
}
