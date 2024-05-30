#include "ToolsPch.hpp"
#include "Tatml.hpp"
#include "tTextureSysRam.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"

namespace Sig { namespace Tatml
{
	const char* fGetFileExtension( )
	{
		return ".tatml";
	}

	b32 fIsTatmlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fTatmlPathToTatb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, ".tatb" );
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gTatmlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gTatmlVersion )
		{
			log_warning( "Tatml file format is out of date -> Please re-export." );
			return;
		}

		s( "SubWidth", o.mSubWidth );
		s( "SubHeight", o.mSubHeight );
		s( "Semantic", reinterpret_cast<u32&>( o.mSemantic ) );
		s( "Format", reinterpret_cast<u32&>( o.mFormat ) );
		s( "TexturePaths", o.mTexturePaths );
	}

	tFile::tFile( )
		: mVersion( gTatmlVersion )
		, mSubWidth( 512 )
		, mSubHeight( 512 )
		, mSemantic( Gfx::tTextureFile::cSemanticDiffuse )
		, mFormat( Gfx::tTextureFile::cFormatDXT1 )
	{
	}
	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Tatml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Tatml file [" << path << "]" );
			return false;
		}

		return true;
	}
	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Tatml", *this ) )
		{
			log_warning( "Couldn't load Tatml file [" << path << "]" );
			return false;
		}

		return true;
	}

	void tFile::fDetermineNumTexturesXandY( u32& numTexturesX, u32& numTexturesY ) const
	{
		const u32 maxTexW = 8*1024; const u32 maxTexH = 8*1024;
		const u32 numTextures = mTexturePaths.fCount( );
		const u32 maxNumTexX = maxTexW / mSubWidth;
		const u32 maxNumTexY = maxTexH / mSubHeight;

		numTexturesX = fMin( fMax( 1u, numTextures ), maxNumTexX );
		numTexturesY = fMin( fMax( 1u, ( numTextures + maxNumTexX - 1 ) / maxNumTexX ), maxNumTexY );
	}

	void tFile::fToTextureArray( tTextureSysRam& atlas )
	{
		atlas.fSetArrayTexture( mTexturePaths.fCount( ), mSubWidth, mSubHeight );

		for( u32 i = 0; i < mTexturePaths.fCount( ); ++i )
			atlas.fLoad( ToolsPaths::fMakeResAbsolute( mTexturePaths[ i ] ), mSemantic, mFormat, Gfx::tTextureFile::cType2d, false, i );
		atlas.fAfterImagesGenerated( true );
	}

	void tFile::fToTextureAtlas( tTextureAtlasSysRam& atlas )
	{
		u32 numTexturesX=1, numTexturesY=1;
		fDetermineNumTexturesXandY( numTexturesX, numTexturesY );

		atlas.fAllocate( numTexturesX, numTexturesY, mSubWidth, mSubHeight, mSemantic, mFormat );

		for( u32 y = 0; y < numTexturesY; ++y )
		{
			for( u32 x = 0; x < numTexturesX; ++x )
			{
				const u32 ithIndex = y * numTexturesX + x;
				if( ithIndex >= mTexturePaths.fCount( ) )
					continue;

				tTextureSysRam subTex;
				if( !subTex.fLoad( ToolsPaths::fMakeResAbsolute( mTexturePaths[ ithIndex ] ), mSemantic, mFormat, Gfx::tTextureFile::cType2d ) )
					continue;

				if( subTex.fWidth( ) == mSubWidth && subTex.fHeight( ) == mSubHeight )
					atlas.fUpdateSubTexture( x, y, subTex );
				else
				{
					tTextureSysRam subTexCopy;
					subTex.fScaleCopy( subTexCopy, mSubWidth, mSubHeight );
					atlas.fUpdateSubTexture( x, y, subTexCopy );
				}
			}
		}
	}

}}

