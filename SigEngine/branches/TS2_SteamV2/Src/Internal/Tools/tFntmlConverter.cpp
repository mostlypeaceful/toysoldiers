#include "ToolsPch.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "tFntmlConverter.hpp"
#include "tFontGen.hpp"
#include "tFontMaterialGen.hpp"
#include "Gfx/tFontMaterial.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"

namespace Sig
{
	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, Gui::tFontDesc& o )
	{
		s.fAsAttribute( "lineHeight", o.mLineHeight );
		s.fAsAttribute( "base", o.mBase );
		s.fAsAttribute( "scaleW", o.mScaleW );
		s.fAsAttribute( "scaleH", o.mScaleH );
		s.fAsAttribute( "pages", o.mPages );
		s.fAsAttribute( "packed", o.mPacked );
		s.fAsAttribute( "alphaChnl", o.mAlphaChnl );
		s.fAsAttribute( "redChnl", o.mRedChnl );
		s.fAsAttribute( "greenChnl", o.mGreenChnl );
		s.fAsAttribute( "blueChnl", o.mBlueChnl );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, Gui::tFontGlyph& o )
	{
		s.fAsAttribute( "id", o.mId );
		s.fAsAttribute( "x", o.mX );
		s.fAsAttribute( "y", o.mY );
		s.fAsAttribute( "width", o.mWidth );
		s.fAsAttribute( "height", o.mHeight );
		s.fAsAttribute( "xoffset", o.mXOffset );
		s.fAsAttribute( "yoffset", o.mYOffset );
		s.fAsAttribute( "xadvance", o.mXAdvance );
		s.fAsAttribute( "page", o.mPage );
		s.fAsAttribute( "chnl", o.mChannel );
	}
}



namespace Sig { namespace Fntml
{
	const char* fGetFileExtension( )
	{
		return ".Fntml";
	}

	struct tInfo
	{
		std::string mFace;
		u32 mSize;
		b32 mBold;
		b32 mItalic;
		std::string mCharSet;
		b32 mUnicode;
		u32 mStretchH;
		b32 mSmooth;
		b32 mAA;
		Math::tVec4u mPadding;
		Math::tVec2u mSpacing;
		b32 mOutline;

		tInfo( )
			: mFace( "" )
			, mSize( 0 )
			, mBold( false )
			, mItalic( false )
			, mCharSet( "" )
			, mUnicode( false )
			, mStretchH( 0 )
			, mSmooth( false )
			, mAA( false )
			, mPadding( 0, 0, 0, 0 )
			, mSpacing( 0, 0 )
			, mOutline( false )
		{
		}

		template<class tVec>
		void fParseVector( std::string s, tVec& v )
		{
			for( u32 i = 0; i < s.length( ); ++i )
				if( s[ i ] == ',' )
					s[ i ] = ' ';

			std::stringstream ss;
			ss << s;

			for( u32 i = 0; i < v.cDimension; ++i )
				ss >> v.fAxis( i );
		}
	};

	struct tPage
	{
		u32				mId;
		std::string		mFile;

		tPage( )
			: mId( 0 )
			, mFile( "" )
		{
		}
	};

	struct tKerning
	{
		u16 mFirst;
		u16 mSecond;
		s16 mAmount;

		tKerning( )
			: mFirst( 0 )
			, mSecond( 0 )
			, mAmount( 0 )
		{
		}
	};

	typedef tGrowableArray<tPage>				tPageArray;
	typedef tGrowableArray<Gui::tFontGlyph>		tCharacterArray;
	typedef tGrowableArray<tKerning>			tKerningArray;

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tInfo& o )
	{
		s.fAsAttribute( "face", o.mFace );
		s.fAsAttribute( "size", o.mSize );
		s.fAsAttribute( "bold", o.mBold );
		s.fAsAttribute( "italic", o.mItalic );
		s.fAsAttribute( "charset", o.mCharSet );
		s.fAsAttribute( "unicode", o.mUnicode );
		s.fAsAttribute( "stretchH", o.mStretchH );
		s.fAsAttribute( "smooth", o.mSmooth );
		s.fAsAttribute( "aa", o.mAA );
		std::string padding, spacing;
		s.fAsAttribute( "padding", padding );
		s.fAsAttribute( "spacing", spacing );
		o.fParseVector( padding, o.mPadding );
		o.fParseVector( spacing, o.mSpacing );
		s.fAsAttribute( "outline", o.mOutline );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPage& o )
	{
		s.fAsAttribute( "id", o.mId );
		s.fAsAttribute( "file", o.mFile );

		if( s.fIn( ) )
		{
			// convert _0 part of filename to _f automatically
			char* replace = ( char* )StringUtil::fStrStrI( o.mFile.c_str( ), "_0.tga" );
			if( replace )
				replace[1] = 'f';
		}
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tKerning& o )
	{
		s.fAsAttribute( "first", o.mFirst );
		s.fAsAttribute( "second", o.mSecond );
		s.fAsAttribute( "amount", o.mAmount );
	}

	class tFile
	{
	public:
		tInfo				mInfo;
		Gui::tFontDesc		mCommon;
		tPageArray			mPages;
		tCharacterArray		mCharacters;
		tKerningArray		mKernings;
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s( "info", o.mInfo );
		s( "common", o.mCommon );
		s( "pages", o.mPages );
		s( "chars", o.mCharacters );
		s( "kernings", o.mKernings );

		// transfer some attributes from "info" to "common"
		o.mCommon.mBold = o.mInfo.mBold;
		o.mCommon.mItalic = o.mInfo.mItalic;
		o.mCommon.mOutline = o.mInfo.mOutline;
	}

}}


namespace Sig
{
	tFntmlConverter::tFntmlConverter( )
	{
	}

	tFntmlConverter::~tFntmlConverter( )
	{
	}

	b32 tFntmlConverter::fLoad( const tFilePathPtr& path )
	{
		mInputFont.fReset( new Fntml::tFile );

		// load the font xml file
		tXmlDeserializer des;
		if( !des.fLoad( path, "font", *mInputFont ) )
		{
			mInputFont.fRelease( );
			return false;
		}

		// check for multiple font pages
		if( mInputFont->mPages.fCount( ) != 1 )
		{
			log_warning( 0, "Fonts only support a single page (1 font texture map) - in the export settings for the font, adjust the output texture size to ensure it is big enough." );
			return false;
		}

		// now we can safely store reference to first (and only) font page
		const Fntml::tPage& fntmlPage = mInputFont->mPages.fFront( );

		// convert the font xml file to binary representation
		mOutputFont.mDesc = mInputFont->mCommon;
		mOutputFont.mFaceName = mOutputFont.fAddLoadInPlaceStringPtr( mInputFont->mInfo.mFace.c_str( ) );
		mOutputFont.mRawGlyphs.fNewArray( mInputFont->mCharacters.fCount( ) );
		for( u32 i = 0; i < mOutputFont.mRawGlyphs.fCount( ); ++i )
			mOutputFont.mRawGlyphs[ i ] = mInputFont->mCharacters[ i ];
		for( u32 i = 0; i < mInputFont->mKernings.fCount( ); ++i )
		{
			for( u32 j = 0; j < mOutputFont.mRawGlyphs.fCount( ); ++j )
			{
				if( mOutputFont.mRawGlyphs[ j ].mId == mInputFont->mKernings[ i ].mFirst )
				{
					Gui::tFontGlyphKerning k;
					k.mSecond = mInputFont->mKernings[ i ].mSecond;
					k.mAmount = mInputFont->mKernings[ i ].mAmount;
					mOutputFont.mRawGlyphs[ j ].mKernPairs.fPushBack( k );
					break;
				}
			}
		}


		// generate font material
		tFontMaterialGen fontMtlGen;

		const std::string fntmlDir = StringUtil::fDirectoryFromPath( path.fCStr( ) );
		const tFilePathPtr fontMapInputPath = tFilePathPtr::fConstructPath( tFilePathPtr( fntmlDir.c_str( ) ), tFilePathPtr( fntmlPage.mFile.c_str( ) ) );
		fontMtlGen.mFontMap = ToolsPaths::fMakeResRelative( fontMapInputPath );
		fontMtlGen.mOutline = mInputFont->mCommon.mOutline;
		mAutoDeleteMaterial.fReset( fontMtlGen.fCreateFontMaterial( mOutputFont ) );

		// add font page
		mOutputFont.mPages.fPushBack( Gui::tFontPage( ) );
		mOutputFont.mPages.fBack( ).mMaterial = mAutoDeleteMaterial.fGetRawPtr( );

		return true;
	}

	void tFntmlConverter::fSaveGameBinary( const tFilePathPtr& path, tPlatformId pid )
	{
		// create output file
		tFileWriter ofile( path );
		if( !ofile.fIsOpen( ) )
		{
			log_warning( 0, "Couldn't open output file [" << path << "] for writing (it's probably read-only for some reason." );
			return;
		}

		mOutputFont.fSetSignature( pid, Rtti::fGetClassId<Gui::tFont>( ), Gui::tFont::cVersion );
		tLoadInPlaceSerializer ser;
		ser.fSave( mOutputFont, ofile, pid );
	}

}


