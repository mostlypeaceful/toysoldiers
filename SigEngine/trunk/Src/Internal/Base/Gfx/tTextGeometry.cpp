//------------------------------------------------------------------------------
// \file tTextGeometry.cpp - 30 Nov 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTextGeometry.hpp"
#include "Gui\tFont.hpp"

namespace Sig { namespace Gfx { namespace TextDetail
{
	static b32 fIsWhiteSpace( wchar_t c );
	static b32 fIsEastAsianChar( wchar_t c );
	static b32 fIsNonBeginningChar( wchar_t c );
	static b32 fIsNonEndingChar( wchar_t c );
	static b32 fWordWrapCanBreakLineAt( const wchar_t * psz, const wchar_t * pszStart );

}}} // ::Sig::Gui::Detail

namespace Sig { namespace Gfx
{
	//------------------------------------------------------------------------------
	// tTextGeometry
	//------------------------------------------------------------------------------
	tTextGeometry::tTextGeometry( )
		: mAllocators( Gfx::tDefaultAllocators::fInstance( ) )
		, mAlignment( cAlignLeft )
		, mWidth( 0.f )
		, mHeight( 0.f )
	{
		fCommonCtor( );
	}

	//------------------------------------------------------------------------------
	tTextGeometry::tTextGeometry( Gfx::tDefaultAllocators& allocators )
		: mAllocators( allocators )
		, mAlignment( cAlignLeft )
		, mWidth( 0.f )
		, mHeight( 0.f )
	{
		fCommonCtor( );
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fSetFontDev( )
	{
		fSetFont( Gui::tFont::fDevFontPath( ) );
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fSetFont( const tFilePathPtr& font )
	{
		sigassert( mAllocators.mResourceDepot );

		fSetFont( mAllocators.mResourceDepot->fQuery( tResourceId::fMake<Gui::tFont>( font ) ) );
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fSetFont( u32 gameSpecificId )
	{
		if( !mAllocators.mFontFromId )
		{
			log_warning( "No font mapping function specified on default allocators for tTextGeometry" );
			return;
		}

		fSetFont( mAllocators.mFontFromId( gameSpecificId ) );
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fSetFont( const tResourcePtr& font )
	{
		sigassert( font );
		sigassert( font->fCast<Gui::tFont>( ) );

		mFont = font;

		// Reset geometry
		mGeometry.fCurrentState( ).fReleaseResources( );
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fBake( const char* text, u32 count, u32 alignment )
	{
		const std::wstring ws = StringUtil::fStringToWString( text );
		fBake( ws.c_str( ), count ? count : ws.length( ), alignment );
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fBake( const wchar_t* text, u32 count, u32 alignment )
	{
		sigassert( fIsValid( ) );

		// Default to empty string if it's null
		if( !text ) text = L"";

		// Calc the count if unspecifie
		if( !count ) count = ( u32 )wcslen( text );

		f32 x = 0.f, y = 0.f, z = 0.f;

		const Gui::tFont & font = *mFont->fCast<Gui::tFont>( );

		mAlignment = alignment;
		mWidth = font.fGetTextWidth( text, count );
		mHeight = ( f32 )font.mDesc.mLineHeight;

		if( alignment == cAlignCenter ) x -= mWidth/2;
		else if( alignment == cAlignRight ) x -= mWidth;

		tBakeData & bake = fGlobalBakeData( );
		bake.mQuadVerts.fSetCount( 0 );
		bake.mIndices.fSetCount( 0 );

		fBakeToData( bake, x, y, z, text, count );

		if( bake.mQuadVerts.fCount( ) > 0 )
			fBakeToGeometry( bake );
	}

	//------------------------------------------------------------------------------
	f32 tTextGeometry::fBakeBox( f32 width, const char* text, u32 count, u32 alignment )
	{
		const std::wstring ws = StringUtil::fStringToWString( text );
		return fBakeBox( width, ws.c_str( ), count ? count : ws.length( ), alignment );
	}

	//------------------------------------------------------------------------------
	f32 tTextGeometry::fBakeBox( f32 boxWidth, const wchar_t* text, u32 count, u32 alignment )
	{
		sigassert( fIsValid( ) );

		// Calc count if unspecified
		if( !count ) count = ( u32 )wcslen( text );

		tBakeData & bake = fGlobalBakeData( );
		bake.mQuadVerts.fSetCount( 0 );
		bake.mIndices.fSetCount( 0 );
		
		const Gui::tFont & font = *mFont->fCast<Gui::tFont>( );
		const f32 yDelta = (f32)font.mDesc.mLineHeight;

		f32 maxLineWidth = 0;
		f32 currY = 0.f;
		for( u32 lineStart = 0, lineEnd = 0; 
				 lineStart < count;
				 lineStart = lineEnd, currY += yDelta ) // for each line
		{
			f32 lineWidth = 0.f;

			for(;;) // determine the extent of the line
			{
				if( lineEnd >= count || text[ lineEnd ] == L'\n' )
					break;

				lineWidth += font.fGetTextWidth( &text[ lineEnd ], 1 );
				++lineEnd;

				if( lineWidth > boxWidth )
				{
					const u32 savedLineEnd = lineEnd;
					const f32 savedLineWidth = lineWidth;

					while( --lineEnd > lineStart )
					{
						if( TextDetail::fWordWrapCanBreakLineAt( &text[ lineEnd ], &text[ lineStart ] ) )
						{
							lineWidth = font.fGetTextWidth( &text[lineStart], lineEnd - lineStart );
							break;
						}
					}

					if( lineEnd <= lineStart )
					{
						lineEnd = savedLineEnd;
						lineWidth = savedLineWidth;
					}

					break;
				}
			}

			maxLineWidth = fMax( maxLineWidth, lineWidth );

			// Calculate the x pos of the line
			f32 x = 0.f;
			if( alignment == cAlignRight ) x = boxWidth - lineWidth;
			else if( alignment == cAlignCenter ) x = -0.5f * lineWidth;

			// Write the line...
			fBakeToData( bake, x, currY, 0.f, &text[ lineStart ], lineEnd - lineStart );

			// Skip the line break / white space
			if( lineEnd < count && ( TextDetail::fIsWhiteSpace( text[ lineEnd ] ) || text[ lineEnd ] == L'\n' ) )
				++lineEnd;
		}

		fBakeToGeometry( bake );

		mAlignment = alignment;
		if( alignment == cAlignLeft ) mWidth = maxLineWidth;
		else mWidth = boxWidth;

		mHeight = currY;
		return mHeight;
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fCommonCtor( )
	{
		sigassert( mAllocators.mTextGeometryAllocator && mAllocators.mIndexAllocator );
		mGeometry.fResetDeviceObjects( mAllocators.mTextGeometryAllocator, mAllocators.mIndexAllocator );
	}

	//------------------------------------------------------------------------------
	tTextGeometry::tBakeData & tTextGeometry::fGlobalBakeData( )
	{
		// all text baking stuff uses this bake data scratch buffer 
		// so we are gonna have a bad day if we start using this MT
		sigassert_is_main_thread( ); 

		static const u32 cPreAllocNumGlyphs = 512;
		static tBakeData gBakeData( cPreAllocNumGlyphs );

		return gBakeData;
	}

	//------------------------------------------------------------------------------
	template<class tIndex>
	static void fGenerateTriList( tIndex* ids, u32 numTris, u32 offset )
	{
		sigassert( sizeof( tIndex ) == sizeof( u16 ) || sizeof( tIndex ) == sizeof( u32 ) );

		for( u32 itri = 0; itri < numTris; itri += 2 )
		{
			const u32 iquad = itri / 2;

			ids[ ( itri + 0 ) * 3 + 0 ] = offset + iquad * 4 + 0;
			ids[ ( itri + 0 ) * 3 + 1 ] = offset + iquad * 4 + 1;
			ids[ ( itri + 0 ) * 3 + 2 ] = offset + iquad * 4 + 2;

			ids[ ( itri + 1 ) * 3 + 0 ] = offset + iquad * 4 + 0;
			ids[ ( itri + 1 ) * 3 + 1 ] = offset + iquad * 4 + 3;
			ids[ ( itri + 1 ) * 3 + 2 ] = offset + iquad * 4 + 2;
		}
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fBakeToData( 
		tBakeData & bakeData, 
		f32 x, f32 y, f32 z, 
		const wchar_t* text, u32 count, 
		f32 spacing )
	{
		// nothing to bake
		if( !count ) return; 

		sigassert( fIsValid( ) );

		// Cache the start quad index for building indices
		const u32 quadStart = bakeData.mQuadVerts.fCount( );

		// Reserve space for the new verts
		bakeData.mQuadVerts.fReserve( 4 * count );
		
		// Round the spacing to full pixels
		spacing = fRound<f32>( spacing );

		const Gui::tFont& font = *mFont->fCast<Gui::tFont>( );
		for( u32 i = 0; i < count; ++i )
		{
			// make sure we start on integer pixel values
			x = fRound<f32>( x );

			// Find the glyph
			const Gui::tFontGlyph & glyph = font.fFindGlyph( text[ i ] );

			// For now we only support single-page fonts
			sigassert( glyph.mPage == 0 );

			// Map the center of the texel to the corners in order to get pixel perfect mapping
			const f32 uvEpsilon = 0.000f; // use this to slightly widen uv window by a very small fudge factor
			const f32 u0	= ( f32( glyph.mX )		+ 0.5f - uvEpsilon ) / font.mDesc.mScaleW;
			const f32 v0	= ( f32( glyph.mY )		+ 0.5f - uvEpsilon ) / font.mDesc.mScaleH;
			const f32 u1	= u0 + f32( glyph.mWidth	+ 0.0f + uvEpsilon ) / font.mDesc.mScaleW;
			const f32 v1	= v0 + f32( glyph.mHeight	+ 0.0f + uvEpsilon ) / font.mDesc.mScaleH;

			const f32 a		= glyph.mXAdvance;
			const f32 w		= glyph.mWidth;
			const f32 h		= glyph.mHeight;
			const f32 ox	= glyph.mXOffset;
			const f32 oy	= glyph.mYOffset;

			// Push verts
			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox,     y + oy,     z ), Math::tVec2f( u0, v0 ) ) );
			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox + w, y + oy,     z ), Math::tVec2f( u1, v0 ) ) );
			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox + w, y + oy + h, z ), Math::tVec2f( u1, v1 ) ) );
			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox,     y + oy + h, z ), Math::tVec2f( u0, v1 ) ) );

			x += a;
			if( text[i] == L' ' ) x += spacing;
			else if( text[i] == L'\t' ) x += 4 * spacing;

			if( i < count-1 ) 
				x += glyph.fGetKerningAmount( ( u16 )text[ i + 1 ] );
		}

		// Push the indices
		const u32 indexStart = bakeData.mIndices.fCount( );
		bakeData.mIndices.fGrowCount( mGeometry.fIndexFormat( ).mSize * count * 6 );
		if( mGeometry.fIndexFormat( ).mStorageType == tIndexFormat::cStorageU16 )
		{
			fGenerateTriList( ( u16* )( bakeData.mIndices.fBegin( ) + indexStart ), count * 2, quadStart );
		}
		else
		{
			sigassert( mGeometry.fIndexFormat( ).mStorageType == tIndexFormat::cStorageU32 );
			fGenerateTriList( ( u32* )( bakeData.mIndices.fBegin( ) + indexStart ), count * 2, quadStart );
		}	
	}

	//------------------------------------------------------------------------------
	void tTextGeometry::fBakeToGeometry( const tBakeData & bakeData )
	{
		const u32 numVerts	= bakeData.mQuadVerts.fCount( );
		const u32 numQuads	= numVerts / 4;
		const u32 numTris	= numQuads * 2;
		const u32 numIds	= numTris * 3;

		const Gui::tFont & font = *mFont->fCast<Gui::tFont>( );
		if( !mGeometry.fAllocateGeometry( *font.mPages.fFront( ).mMaterial, numVerts, numIds, numTris ) )
		{
			if( numVerts > 0 )
				log_warning( "Couldn't allocate text geometry (numVerts=" << numVerts << ", numTris=" << numTris << ")" );
			return; // couldn't get geometry
		}

		// copy vert data to gpu
		mGeometry.fCopyVertsToGpu( bakeData.mQuadVerts.fBegin( ), numVerts );
		mGeometry.fCopyIndicesToGpu( bakeData.mIndices.fBegin( ), numIds );
	}

	//------------------------------------------------------------------------------
	// tTextGeometry::tBakeData
	//------------------------------------------------------------------------------
	tTextGeometry::tBakeData::tBakeData( u32 numGlyphs )
	{
		mQuadVerts.fReserve( 4 * numGlyphs );
		mIndices.fReserve( 6 * numGlyphs * sizeof( u16 ) );
	}

} } // ::Sig::Gfx


namespace Sig { namespace Gfx { namespace TextDetail
{
	namespace
	{
		struct tBreakInfo
		{ 
			wchar_t	wch; 
			b32		isNonBeginningChar; 
			b32		isNonEndingChar; 
		}; 

		static tBreakInfo cBreakArray[] = 
		{ 
			{0x0021, true,  false}, // 0021: !                                         fr, etc. 
			{0x0024, false, true},  // 0024: $                                         kr 
			{0x0025, true,  false}, // 0025: %                                         tc, kr 
			{0x0027, true,  true},  // 0027: '                                         fr 
			{0x0028, false, true},  // 0028: (                                         tc, kr 
			{0x0029, true,  false}, // 0029: )                                         fr, etc. 
			{0x002E, true,  false}, // 002E: .                                         fr 
			{0x002F, true,  true},  // 002F: /                                         fr, etc. 
			{0x003A, true,  false}, // 003A: :                                         tc, kr 
			{0x003B, true,  false}, // 003B: ;                                         tc, kr 
			{0x003F, true,  false}, // 003F: ?                                         fr 
			{0x005B, false, true},  // 005B: [                                         tc, kr 
			{0x005C, false, true},  // 005C: Reverse Solidus                           kr 
			{0x005D, true,  false}, // 005D: ]                                         tc, kr 
			{0x007B, false, true},  // 007B: {                                         tc, kr 
			{0x007D, true,  false}, // 007D: }                                         tc, kr 
			{0x00A2, true,  false}, // 00A2: ‘  Cent Sign                              etc. 
			{0x00A3, false, true},  // 00A3: Pound Sign                                tc 
			{0x00A5, false, true},  // 00A5: Yen Sign                                  tc 
			{0x00A7, false, true},  // 00A7: Section Sign                              ja 
			{0x00A8, true,  false}, // 00A8: N Diaeresis                              etc. 
			{0x00A9, true,  false}, // 00A9: ccopyright sign                           etc.
			{0x00AE, true,  false}, // 00AE: Rregistered sign                          etc.
			{0x00B0, true,  false}, // 00B0: Degree Sign                               kr 
			{0x00B7, true,  false}, // 00B7: Middle Dot                                tc 
			{0x02C7, true,  false}, // 02C7: ? Caron                                   etc. 
			{0x02C9, true,  false}, // 02C9: ? Modified Letter marcon                  etc. 
			{0x2013, true,  false}, // 2013: En Dash                                   tc 
			{0x2014, true,  false}, // 2014: Em Dash                                   tc 
			{0x2015, true,  false}, // 2015: \ Horizon bar                            etc. 
			{0x2016, true,  false}, // 2016: ? Double vertical line                   etc. 
			{0x2018, false, true},  // 2018: Left Single Quotation Mark                tc, kr 
			{0x2019, true,  false}, // 2019: Right Single Quotation Mark               tc, fr, kr 
			{0x201C, false, true},  // 201C: Left Double Quotation Mark                tc, kr 
			{0x201D, true,  false}, // 201D: Right Double Quotation Mark               de, kr 
			{0x2025, true,  false}, // 2025: Two Dot Leader                            tc 
			{0x2026, true,  false}, // 2026: Horizontal Ellpsis                        tc 
			{0x2027, true,  false}, // 2027: Hyphenation Point                         tc 
			{0x2032, true,  false}, // 2032: Prime                                     tc, kr 
			{0x2033, true,  false}, // 2033: Double Prime                              kr 
			{0x2035, false, true},  // 2035: Reversed Prime                            tc 
			{0x2103, true,  false}, // 2103: Degree Celsius                            ja, kr 
			{0x2122, true,  false}, // 2122: trade mark sign 
			{0x2236, true,  false}, // 2236: ? tilde operator                          etc. 
			{0x2574, true,  false}, // 2574: Box Drawings Light Left                   tc 
			{0x266F, false, true},  // 266F: Music Sharp Sign                          ja 
			{0x3001, true,  false}, // 3001: fullwidth ideographic comma               ja, tc 
			{0x3002, true,  false}, // 3002: fullwidth full stop                       ja, tc 
			{0x3005, true,  false}, // 3005: Ideographic Iteration Mark                ja 
			{0x3003, true,  false}, // 3003: VDitto Mark                              etc. 
			{0x3008, false, true},  // 3008: Left Angle Bracket                        tc, kr 
			{0x3009, true,  false}, // 3009: Right Angle Bracket                       tc, kr 
			{0x300A, false, true},  // 300A: Left Double Angle Bracket                 tc, kr 
			{0x300B, true,  false}, // 300B: Right Double Angle Bracket                tc, kr 
			{0x300C, false, true},  // 300C: Left Corner Bracket                       ja, tc, kr 
			{0x300D, true,  false}, // 300D: Right Corner Bracket                      ja, tc, kr 
			{0x300E, false, true},  // 300E: Left White Corner Bracket                 tc, kr 
			{0x300F, true,  false}, // 300F: Rignt White Corner Bracket                tc, kr 
			{0x3010, false, true},  // 3010: Left Black Lenticular Bracket             ja, tc, kr 
			{0x3011, true,  false}, // 3011: right black lenticular bracket            ja, tc, kr 
			{0x3012, false, true},  // 3012: Postal Mark                               ja 
			{0x3014, false, true},  // 3014: Left Tortoise Shell Bracket               tc, kr 
			{0x3015, true,  false}, // 3015: Right Tortoise Shell Bracket              tc, kr 
			{0x3016, false, true }, // 3016: ?                                        etc. 
			{0x3017, true,  false}, // 3017: ?                                        etc. 
			{0x301D, false, true},  // 301D: Reversed Double Prime Quotation Mark      tc 
			{0x301E, true,  false}, // 301E: Double Prime Quotation Mark               tc 
			{0x301F, true,  false}, // 301F: Low Double Prime Quotation Mark           tc 
			{0x3041, true,  false}, // 3041: Hiragana Letter Small A                   ja 
			{0x3043, true,  false}, // 3043: Hiragana Letter Small I                   ja 
			{0x3045, true,  false}, // 3045: Hiragana Letter Small U                   ja 
			{0x3047, true,  false}, // 3047: Hiragana Letter Small E                   ja 
			{0x3049, true,  false}, // 3049: Hiragana Letter Small O                   ja 
			{0x3063, true,  false}, // 3063: Hiragana Letter Small Tu                  ja 
			{0x3083, true,  false}, // 3083: Hiragana Letter Small Ya                  ja 
			{0x3085, true,  false}, // 3085: Hiragana Letter Small Yu                  ja 
			{0x3087, true,  false}, // 3087: Hiragana Letter Small Yo                  ja 
			{0x308E, true,  false}, // 308E: Hiragana Letter Small Wa                  ja 
			{0x3099, true,  false}, // 3099: Combining Katakana-Hiragana Voiced Sound Mark (if necessary) 
			{0x309A, true,  false}, // 309A: Combining Katakana-Hiragana Semi-Voiced Sound Mark (if necessary) 
			{0x309B, true,  false}, // 309B: Katakana-Hiragana Voiced Sound Mark       ja 
			{0x309C, true,  false}, // 309C: Katakana-Hiragana Semi-Voiced Sound Mark  ja 
			{0x309D, true,  false}, // 309D: Hiragana Iteration Mark                   ja 
			{0x309E, true,  false}, // 309E: Hiragana Voiced Iteration Mark            ja 
			{0x30A1, true,  false}, // 30A1: Katakana Letter Small A                   ja 
			{0x30A3, true,  false}, // 30A3: Katakana Letter Small I                   ja 
			{0x30A5, true,  false}, // 30A5: Katakana Letter Small U                   ja 
			{0x30A7, true,  false}, // 30A7: Katakana Letter Small E                   ja 
			{0x30A9, true,  false}, // 30A9: Katakana Letter Small O                   ja 
			{0x30C3, true,  false}, // 30C3: Katakana Letter Small Tu                  ja 
			{0x30E3, true,  false}, // 30E3: Katakana Letter Small Ya                  ja 
			{0x30E5, true,  false}, // 30E5: Katakana Letter Small Yu                  ja 
			{0x30E7, true,  false}, // 30E7: Katakana Letter Small Yo                  ja 
			{0x30EE, true,  false}, // 30EE: Katakana Letter Small Wa                  ja 
			{0x30F5, true,  false}, // 30F5: Katakana Letter Small Ka                  ja 
			{0x30F6, true,  false}, // 30F6: Katakana Letter Small Ke                  ja 
			{0x30FB, true,  false}, // 30FB: katakana middle dot                       ja 
			{0x30FC, true,  false}, // 30FC: Katakana-Hiragana Prolonged Sound Mark    ja 
			{0x30FD, true,  false}, // 30FD: Katakana Iteration Mark                   ja 
			{0x30FE, true,  false}, // 30FE: Katakana Voiced Iteration Mark            ja 
			{0xFE50, true,  false}, // FE50: Small Comma                               tc 
			{0xFE51, true,  false}, // FE51: Small Ideographic Comma                   tc 
			{0xFE52, true,  false}, // FE52: Small Full Stop                           tc 
			{0xFE54, true,  false}, // FE54: Small Semicolon                           tc 
			{0xFE55, true,  false}, // FE55: Small Colon                               tc 
			{0xFE56, true,  false}, // FE56: Small Question Mark                       tc 
			{0xFE57, true,  false}, // FE57: Small Exclamation Mark                    tc 
			{0xFE59, false, true},  // FE59: Small Left Parenthesis                    tc 
			{0xFE5A, true,  false}, // FE5A: Small Right Parenthesis                   tc 
			{0xFE5B, false, true},  // FE5B: Small Left Curly Bracket                  tc 
			{0xFE5C, true,  false}, // FE5C: Small Right Curly Bracket                 tc 
			{0xFE5D, false, true},  // FE5D: Small Left Tortoise Shell Bracket         tc 
			{0xFE5E, true,  false}, // FE5E: Small Right Tortoise Shell Bracket        tc 
			{0xFF01, true,  false}, // FF01: Fullwidth Exclamation Mark                ja, tc 
			{0xFF02, true,  false}, // FF02: úW Fullwidth quotation mark               etc. 
			{0xFF04, false, true},  // FF04: Fullwidth Dollar Sign                     kr 
			{0xFF05, true,  false}, // FF05: Fullwidth Percent Sign                    kr 
			{0xFF07, true,  false}, // FF07: úV Fullwidth Apos                         etc. 
			{0xFF08, false, true},  // FF08: Fullwidth Left Parenthesis                ja, tc 
			{0xFF09, true,  false}, // FF09: Fullwidth Right Parenthesis               ja, tc 
			{0xFF0C, true,  false}, // FF0C: Fullwidth Comma                           ja, tc, kr 
			{0xFF0E, true,  false}, // FF0E: Fullwidth Full Stop                       ja, tc, kr 
			{0xFF1A, true,  false}, // FF1A: Fullwidth Colon                           ja, tc, kr 
			{0xFF1B, true,  false}, // FF1B: Fullwidth Semicolon                       ja, tc, kr 
			{0xFF1F, true,  false}, // FF1F: Fullwidth Quation Mark                    ja, tc 
			{0xFF20, false, true},  // FF20: Fullwidth Commercial At                   ja 
			{0xFF3B, false, true},  // FF3B: Fullwidth Left Square Bracket             kr 
			{0xFF3D, true,  false}, // FF3D: Fullwidth Right Square Bracket            kr 
			{0xFF40, true,  false}, // FF40: M Fullwidth Grave accent                 etc. 
			{0xFF5B, false, true},  // FF5B: Fullwidth Left Curly Bracket              ja, tc 
			{0xFF5C, true,  false}, // FF5C: b Fullwidth Vertical line                etc. 
			{0xFF5D, true,  false}, // FF5D: Fullwidth Right Curly Bracket             kr 
			{0xFF5E, true,  false}, // FF5E: ` Fullwidth Tilda                        etc. 
			{0xFFE0, true,  true},  // FFE0: Fullwidth Cent Sign                       ja, kr 
			{0xFFE1, false, true},  // FFE1: Fullwidth Pound Sign                      fr, kr 
			{0xFFE5, false, true},  // FFE5: Fullwidth Yen Sign                        ja 
			{0xFFE6, false, true}   // FFE6: Fullwidth Won Sign                        kr 
		}; 
		static const int cNumBreakCharacters = array_length( cBreakArray );

	} // Anonymous

	//------------------------------------------------------------------------------
	b32 fIsWhiteSpace( wchar_t c )
	{
		return ( c == L'\t' || c == L'\r' || c == L' ' || c == 0x3000 );
	}

	//------------------------------------------------------------------------------
	b32 fIsEastAsianChar( wchar_t c )
	{
		return
			( 0x1100 <= c ) && ( c <= 0x11FF ) ||		// Hangul Jamo
			( 0x3000 <= c ) && ( c <= 0xD7AF ) ||		// CJK symbols - Hangul Syllables
			( 0xF900 <= c ) && ( c <= 0xFAFF ) ||		// CJK compat
			( 0xFF00 <= c ) && ( c <= 0xFFDC );			// Halfwidth / Fullwidth
	}

	//------------------------------------------------------------------------------
	b32 fIsNonBeginningChar( wchar_t c )
	{
		int iLeft = 0;
		int iRight = cNumBreakCharacters;
		int iMid = 0;

		// Binary search through the array of break characters
		while( iLeft <= iRight )
		{
			iMid = ((iRight-iLeft)/2) + iLeft;
			if (cBreakArray[iMid].wch == c)
				return cBreakArray[iMid].isNonBeginningChar;
			if (c < cBreakArray[iMid].wch)
				iRight = iMid - 1;
			else
				iLeft = iMid + 1;
		}
		return false;
	}

	//------------------------------------------------------------------------------
	b32 fIsNonEndingChar( wchar_t c )
	{
		int iLeft = 0;
		int iRight = cNumBreakCharacters;
		int iMid = 0;

		// Binary search through the array of break characters
		while( iLeft <= iRight )
		{
			iMid = ((iRight-iLeft)/2) + iLeft;
			if (cBreakArray[iMid].wch == c)
				return cBreakArray[iMid].isNonEndingChar;
			if (c < cBreakArray[iMid].wch)
				iRight = iMid - 1;
			else
				iLeft = iMid + 1;
		}
		return false;
	}

	//------------------------------------------------------------------------------
	b32 fWordWrapCanBreakLineAt( const wchar_t * psz, const wchar_t * pszStart )
	{
		if( psz == pszStart )
			return false;	// leave at least one character in a line

		if( fIsWhiteSpace( *psz ) && fIsNonBeginningChar( psz[1] ) )
			return 	false;	

		// Do not word wrap when current character is IsEastAsianChar, 
		// the previus character as " and before it was space.
		if(psz - pszStart >1)	
		{
			if(fIsWhiteSpace( psz[-2] ) && psz[-1]==L'\"' && !fIsWhiteSpace( *psz ) ) 
			// Do not leave " at end of line when the leading character is not space.
			{
				return false;	
			}
		}
		if(!fIsWhiteSpace( psz[-1] ) && *psz==L'\"' && fIsWhiteSpace( psz[1] ) )
			// Do not put " at top of line when the " is closing word and leading space.
		{
			return false;
		}

		return
			( fIsWhiteSpace( *psz ) || fIsEastAsianChar( *psz ) || fIsEastAsianChar( psz[-1] ) || psz[-1] == L'-') &&
			!fIsNonBeginningChar( *psz ) && !fIsNonEndingChar( psz[-1] );
	}

}}}

