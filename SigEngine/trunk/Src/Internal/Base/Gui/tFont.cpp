//------------------------------------------------------------------------------
// \file tFont.cpp - 01 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
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

	define_lip_version( tFont, 0, 0, 0 );

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
		mGlyphMap.fTreatAsObject( ).fSetCapacity( 2 * mRawGlyphs.fCount( ) + 1 );

		// initialize the glyph map (i.e., insert all glyphs)
		tGlyphMap& glyphMap = mGlyphMap.fTreatAsObject( );
		for( u32 i = 0; i < mRawGlyphs.fCount( ); ++i )
			glyphMap.fInsert( mRawGlyphs[ i ].mId, &mRawGlyphs[ i ] );
	}

	void tFont::fOnFileUnloading( const tResource& ownerResource )
	{
		mGlyphMap.fDestroy( );
	}

	//------------------------------------------------------------------------------
	const tFontGlyph & tFont::fFindGlyph( u16 character ) const
	{
		const tGlyphMap & glyphMap = mGlyphMap.fTreatAsObject( );
		const tFontGlyph* const*	find = glyphMap.fFind( character );
		const tFontGlyph* const*	find2 = find ? find : glyphMap.fFind( cAtSymbol );
		const tFontGlyph*			glyph = find2 ? *find2 : &mDefaultGlyph;

		return *glyph;
	}

	//------------------------------------------------------------------------------
	f32 tFont::fGetTextWidth( const char* text, u32 count ) const
	{
		const std::wstring ws = StringUtil::fStringToWString( text );
		return fGetTextWidth( ws.c_str( ), count ? count : ws.length( ) );
	}

	//------------------------------------------------------------------------------
	f32	tFont::fGetTextWidth( const wchar_t* text, u32 count ) const
	{
		const tFont::tGlyphMap& glyphMap = mGlyphMap.fTreatAsObject( );

		f32 x = 0;
		for( u32 i = 0; i < count; ++i )
		{
			const tFontGlyph & glyph = fFindGlyph( text[ i ] );

			x += glyph.mXAdvance;

			if( i < count-1 )
				x += glyph.fGetKerningAmount( ( u16 )text[ i + 1 ] );
		}

		return x;
	}

}}
