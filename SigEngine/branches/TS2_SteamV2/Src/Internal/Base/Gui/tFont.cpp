#include "BasePch.hpp"
#include "tFont.hpp"

namespace Sig { namespace Gui
{
	tFontDesc::tFontDesc( )
		: mLineHeight( 0 )
		, mBase( 0) 
		, mScaleW( 0 )
		, mScaleH( 0 )
		, mPages( 0 )
		, mPacked( false )
		, mBold( false )
		, mItalic( false )
		, mOutline( false )
		, mAlphaChnl( 0 )
		, mRedChnl( 0 )
		, mGreenChnl( 0 )
		, mBlueChnl( 0 )
		, pad1( 0 )
		, pad2( 0 )
	{
	}

	tFontDesc::tFontDesc( tNoOpTag )
	{
	}


	tFontGlyphKerning::tFontGlyphKerning( )
		: mSecond( 0 )
		, mAmount( 0 )
	{
	}

	tFontGlyphKerning::tFontGlyphKerning( tNoOpTag )
	{
	}


	tFontGlyph::tFontGlyph( )
		: mId( 0 )
		, mX( 0 )
		, mY( 0 )
		, mWidth( 0 )
		, mHeight( 0 )
		, mXOffset( 0 )
		, mYOffset( 0 )
		, mXAdvance( 0 )
		, mPage( 0 )
		, mChannel( 0 )
	{
	}
	
	tFontGlyph::tFontGlyph( tNoOpTag )
		: mKernPairs( cNoOpTag )
	{
	}


	tFontPage::tFontPage( )
		: mMaterial( 0 )
	{
	}

	tFontPage::tFontPage( tNoOpTag )
	{
	}

	const u32	tFont::cVersion = 0;

	const char* tFont::fGetFileExtension( )
	{
		return ".fntb";
	}

	tFilePathPtr tFont::fDevFontPath( )
	{
		return tFilePathPtr( "gui\\font\\_devfont.fntb" );
	}

	tFont::tFont( )
		: mFaceName( 0 )
	{
	}

	tFont::tFont( tNoOpTag )
		: tLoadInPlaceFileBase( cNoOpTag )
		, mDesc( cNoOpTag )
		, mRawGlyphs( cNoOpTag )
		, mGlyphMap( cNoOpTag )
		, mDefaultGlyph( cNoOpTag )
		, mPages( cNoOpTag )
	{
	}

	tFont::~tFont( )
	{
	}

	void tFont::fOnFileLoaded( const tResource& ownerResource )
	{
		// construct glyph map with twice as many entries as
		// there are glyphs, just to maintain a fairly sparse harsh table
		mGlyphMap.fConstruct( 2 * mRawGlyphs.fCount( ) + 1 );

		// initialize the glyph map (i.e., insert all glyphs)
		tGlyphMap& glyphMap = mGlyphMap.fTreatAsObject( );
		for( u32 i = 0; i < mRawGlyphs.fCount( ); ++i )
			glyphMap.fInsert( mRawGlyphs[ i ].mId, &mRawGlyphs[ i ] );
	}

	void tFont::fOnFileUnloading( )
	{
		mGlyphMap.fDestroy( );
	}

}}
