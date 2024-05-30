#include "BasePch.hpp"
#include "tText.hpp"
#include "Gfx/tDefaultAllocators.hpp"
#include "Gfx/tScreen.hpp"

namespace Sig { namespace Gui { namespace Detail
{
	b32 fIsWhiteSpace( wchar_t c );
	b32 fIsEastAsianChar( wchar_t c );
	b32 fIsNonBeginningChar( wchar_t c );
	b32 fIsNonEndingChar( wchar_t c );
	b32 fWordWrap_CanBreakLineAt( const wchar_t * psz, const wchar_t * pszStart );
}}}

namespace Sig { namespace Gui
{
	namespace { const u16 cAtSymbol = 64; }

	tText::tText( b32 dropShadow )
		: mAllocators( Gfx::tDefaultAllocators::fInstance( ) )
	{
		fCommonCtor( dropShadow );
	}
	tText::tText( Gfx::tDefaultAllocators& allocators, b32 dropShadow )
		: mAllocators( allocators )
	{
		fCommonCtor( dropShadow );
	}

	void tText::fCommonCtor( b32 dropShadow )
	{
		fSetDropShadowEnabled( dropShadow );

		mAlignment = cAlignLeft;
		mWidth = 0.f;
		mHeight = 0.f;
		fUpdateBounds( );

		fSetRenderBatch( Gfx::tRenderBatchPtr( ) );

		sigassert( mAllocators.mTextGeometryAllocator && mAllocators.mIndexAllocator );
		mGeometry.fResetDeviceObjects( mAllocators.mTextGeometryAllocator, mAllocators.mIndexAllocator );

		fRegisterWithDevice( Gfx::tDevice::fGetDefaultDevice( ).fGetRawPtr( ) );
	}
	void tText::fUpdateBounds( )
	{
		Gui::tRect rect;
		rect.mT = 0.f;
		rect.mB = mHeight;

		switch( mAlignment )
		{
		case cAlignLeft:
			rect.mL = 0.f;
			rect.mR = mWidth;
			break;
		case cAlignCenter:
			rect.mL = -0.5f * mWidth;
			rect.mR = +0.5f * mWidth;
			break;
		case cAlignRight:
			rect.mL = -mWidth;
			rect.mR = 0.f;
			break;
		}

		fSetBounds( rect );
	}
	void tText::fSetFont( const tResourcePtr& font )
	{
		mFont = font;
		sigassert( mFont );
		fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
	}
	void tText::fSetFont( const tFilePathPtr& font )
	{
		fSetFont( mAllocators.mResourceDepot->fQuery( tResourceId::fMake<Gui::tFont>( font ) ) );
	}
	void tText::fSetDevFont( )
	{
		fSetFont( mAllocators.mResourceDepot->fQuery( tResourceId::fMake<Gui::tFont>( Gui::tFont::fDevFontPath( ) ) ) );
	}
	void tText::fSetFontFromId( u32 gameSpecificId )
	{
		if( mAllocators.mFontFromId )
			fSetFont( mAllocators.mFontFromId( gameSpecificId ) );
		else
			log_warning( 0, "No font mapping function specified for tText object" );
	}
	const Gfx::tMaterial* tText::fMaterial( ) const
	{
		const tFont* font = mFont->fCast<tFont>( );
		sigassert( font && font->mPages.fCount( ) == 1 );
		return font->mPages.fFront( ).mMaterial;
	}
	f32 tText::fGetTextWidth( const char* text, u32 textCharCount )
	{
		const std::wstring ws = StringUtil::fStringToWString( text );
		return fGetTextWidth( ws.c_str( ), textCharCount ? textCharCount : ws.length( ) );
	}
	f32 tText::fGetTextWidth( const wchar_t* text, u32 textCharCount )
	{
		if( textCharCount == 0 )
			textCharCount = ( u32 )wcslen(text);

		sigassert( mFont );
		const tFont* font = mFont->fCast<tFont>( );
		sigassert( font );
		const tFont::tGlyphMap& glyphMap = font->fGetGlyphMap( );

		f32 x = 0;

		for( u32 i = 0; i < textCharCount; ++i )
		{
			const tFontGlyph* const*	find = glyphMap.fFind( ( u16 )text[ i ] );
			const tFontGlyph* const*	find2 = find ? find : glyphMap.fFind( cAtSymbol );
			const tFontGlyph*			glyph = find2 ? *find2 : &font->mDefaultGlyph;

			x += glyph->mXAdvance;

			if( i < textCharCount-1 )
				x += glyph->fGetKerningAmount( ( u16 )text[ i + 1 ] );
		}

		return x;
	}
	void tText::fBake( const char* text, u32 textCharCount, tAlignment mode )
	{
		const std::wstring ws = StringUtil::fStringToWString( text );
		return fBake( ws.c_str( ), textCharCount ? textCharCount : ws.length( ), mode );
	}
	void tText::fBake( const wchar_t* text, u32 textCharCount, tAlignment mode )
	{
		if( !text ) text = L"";

		f32 x = 0.f, y = 0.f, z = 0.f;

		if( textCharCount == 0 )
			textCharCount = ( u32 )wcslen(text);

		sigassert( mFont );
		const tFont* font = mFont->fCast<tFont>( );
		sigassert( font );

		mAlignment = mode;
		mWidth = fGetTextWidth( text, textCharCount );
		mHeight = ( f32 )font->mDesc.mLineHeight;
		fUpdateBounds( );

		if( mode == cAlignCenter )
			x -= mWidth/2;
		else if( mode == cAlignRight )
			x -= mWidth;

		mBake = tBakeData();
		mBake.mQuadVerts.fSetCapacity( 4 * textCharCount );

		fBakeInternal( mBake, x, y, z, text, textCharCount );

		if( mBake.mQuadVerts.fCount( ) > 0 )
			fFinishBake( mBake );
	}

	void tText::fOnDeviceReset( Gfx::tDevice* device )
	{
		if( mBake.mQuadVerts.fCount( ) > 0 )
			fFinishBake( mBake );
	}

	void tText::fBakeCString( const char* text, u32 alignment  )
	{
		fBake( text, 0, ( tAlignment )alignment );
	}
	void tText::fBakeCString( const char* text )
	{
		fBakeCString( text, cAlignLeft );
	}
	void tText::fBakeLocString( const tLocalizedString* text, u32 alignment )
	{
		if( !text )
		{
			log_warning( 0, "NULL text passed into tText::fBakeLocString. Don't forget to convert your string using GameApp.LocString( str )" );
			text = &tLocalizedString::fNullString( );
		}
		fBake( *text, ( tAlignment )alignment );
	}
	void tText::fBakeLocString( const tLocalizedString* text )
	{
		fBakeLocString( text, cAlignLeft );
	}
	f32 tText::fBakeBox( f32 boxWidth, const char* text, u32 textCharCount, tAlignment mode )
	{
		const std::wstring ws = StringUtil::fStringToWString( text );
		return fBakeBox( boxWidth, ws.c_str( ), textCharCount ? textCharCount : ws.length( ), mode );
	}
	f32 tText::fBakeBox( f32 boxWidth, const wchar_t* text, u32 textCharCount, tAlignment mode )
	{
		const f32 x = 0.f;
		const f32 z = 0.f;
		if( textCharCount == 0 )
			textCharCount = ( u32 )wcslen(text);

		mBake = tBakeData( );
		mBake.mQuadVerts.fSetCapacity( 4 * textCharCount );

		sigassert( mFont );
		const tFont* font = mFont->fCast<tFont>( );
		sigassert( font );
		const u16 fontHeight = font->mDesc.mLineHeight;
		const f32 yDelta = f32( fontHeight );

		f32 currY = 0.f;
		for( u32 lineStart = 0, lineEnd = 0; 
				 lineStart < textCharCount;
				 lineStart = lineEnd, currY += yDelta ) // for each line
		{
			f32 lineWidth = 0.f;

			for(;;) // determine the extent of the line
			{
				if( lineEnd >= textCharCount || text[lineEnd] == L'\n' )
					break;

				const f32 charWidth = fGetTextWidth( &text[lineEnd], 1 );
				lineWidth += charWidth;
				++lineEnd;

				if( lineWidth > boxWidth )
				{
					const u32 savedLineEnd = lineEnd;
					const f32 savedLineWidth = lineWidth;

					while( --lineEnd > lineStart )
					{
						if( Detail::fWordWrap_CanBreakLineAt( &text[lineEnd], &text[lineStart] ) )
						{
							lineWidth = fGetTextWidth( &text[lineStart], lineEnd - lineStart );
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

			// Write the line...
			f32 cx = x;
			f32 spacing = 0.f;
			if( mode == cAlignRight )
				cx = x + boxWidth - lineWidth;
			else if( mode == cAlignCenter )
				cx = x - 0.5f * lineWidth;
			fBakeInternal( mBake, cx, currY, z, &text[lineStart], lineEnd - lineStart, spacing );

			// Skip the line break / white space
			if( lineEnd < textCharCount && ( Detail::fIsWhiteSpace( text[ lineEnd ] ) || text[ lineEnd ] == L'\n' ) )
				++lineEnd;
		}

		if( mBake.mQuadVerts.fCount( ) > 0 )
		{
			fFinishBake( mBake );
		}

		mAlignment = mode;
		mWidth = boxWidth;
		mHeight = currY;
		fUpdateBounds( );

		return mHeight;
	}
	void tText::fBakeBoxLocString( f32 width, const tLocalizedString* text, u32 alignment )
	{
		if( !text )
		{
			log_warning( 0, "NULL text passed into tText::fBakeBoxLocString. Don't forget to convert your string using GameApp.LocString( str )" );
			text = &tLocalizedString::fNullString( );
		}
		fBakeBox( width, *text, ( tAlignment )alignment );
	}
	void tText::fBakeInternal( tBakeData& bakeData, f32 x, f32 y, f32 z, const wchar_t* text, u32 textCharCount, f32 spacing )
	{
		if( textCharCount == 0 )
			return; // nothing to bake

		spacing = fRound<f32>( spacing );

		sigassert( mFont );
		sigassert( textCharCount > 0 );
//		sigassert( bakeData.mQuadVerts.fCapacity( ) >= bakeData.mQuadVerts.fCount( ) + 4 * textCharCount );

		const tFont* font = mFont->fCast<tFont>( );
		if( !font ) return;
		sigassert( font );
		const tFont::tGlyphMap& glyphMap = font->fGetGlyphMap( );

		for( u32 i = 0; i < textCharCount; ++i )
		{
			// make sure we start on integer pixel values
			x = fRound<f32>( x );

			const tFontGlyph* const*	find = glyphMap.fFind( ( u16 )text[ i ] );
			const tFontGlyph* const*	find2 = find ? find : glyphMap.fFind( cAtSymbol );
			const tFontGlyph*			glyph = find2 ? *find2 : &font->mDefaultGlyph;

			// for now we only support single-page fonts (fonts that span across multiple
			// texture pages will be less efficient and more of a pain, and i can't see any
			// great reason for it)
			sigassert( glyph->mPage == 0 );

			// Map the center of the texel to the corners in order to get pixel perfect mapping
			const f32 uvEpsilon = 0.000f; // use this to slightly widen uv window by a very small fudge factor
			const f32 u0	= ( f32( glyph->mX )		+ 0.5f - uvEpsilon ) / font->mDesc.mScaleW;
			const f32 v0	= ( f32( glyph->mY )		+ 0.5f - uvEpsilon ) / font->mDesc.mScaleH;
			const f32 u1	= u0 + f32( glyph->mWidth	+ 0.0f + uvEpsilon ) / font->mDesc.mScaleW;
			const f32 v1	= v0 + f32( glyph->mHeight	+ 0.0f + uvEpsilon ) / font->mDesc.mScaleH;

			const f32 a		= glyph->mXAdvance;
			const f32 w		= glyph->mWidth;
			const f32 h		= glyph->mHeight;
			const f32 ox	= glyph->mXOffset;
			const f32 oy	= glyph->mYOffset;

			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox,     y + oy,     z ), Math::tVec2f( u0, v0 ) ) );
			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox + w, y + oy,     z ), Math::tVec2f( u1, v0 ) ) );
			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox + w, y + oy + h, z ), Math::tVec2f( u1, v1 ) ) );
			bakeData.mQuadVerts.fPushBack( Gfx::tGlyphRenderVertex( Math::tVec3f( x + ox,     y + oy + h, z ), Math::tVec2f( u0, v1 ) ) );

			x += a;
			if( text[i] == L' ' )
				x += spacing;
			if( text[i] == L'\t' )
				x += 4 * spacing;

			if( i < textCharCount-1 )
				x += glyph->fGetKerningAmount( ( u16 )text[ i + 1 ] );
		}
	}
	template<class tIndex>
	void fGenerateTriList( tIndex* ids, u32 numTris )
	{
		sigassert( sizeof( tIndex ) == sizeof( u16 ) || sizeof( tIndex ) == sizeof( u32 ) );

		for( u32 itri = 0; itri < numTris; itri += 2 )
		{
			const u32 iquad = itri / 2;

			ids[ ( itri + 0 ) * 3 + 0 ] = iquad * 4 + 0;
			ids[ ( itri + 0 ) * 3 + 1 ] = iquad * 4 + 1;
			ids[ ( itri + 0 ) * 3 + 2 ] = iquad * 4 + 2;

			ids[ ( itri + 1 ) * 3 + 0 ] = iquad * 4 + 0;
			ids[ ( itri + 1 ) * 3 + 1 ] = iquad * 4 + 3;
			ids[ ( itri + 1 ) * 3 + 2 ] = iquad * 4 + 2;
		}
	}
	void tText::fFinishBake( const tBakeData& bakeData )
	{
		const u32 numVerts	= bakeData.mQuadVerts.fCount( );
		const u32 numQuads	= numVerts / 4;
		const u32 numTris	= numQuads * 2;
		const u32 numIds	= numTris * 3;

		if( !mGeometry.fAllocateGeometry( *fMaterial( ), numVerts, numIds, numTris ) )
		{
			if( numVerts > 0 )
				log_warning( Log::cFlagGraphics, "Couldn't allocate geometry for renderable Text (numVerts=" << numVerts << ", numTris=" << numTris << ")" );
			fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
			return; // couldn't get geometry
		}

		// copy vert data to gpu
		mGeometry.fCopyVertsToGpu( bakeData.mQuadVerts.fBegin( ), numVerts );

		// copy index data to gpu
		if( mGeometry.fIndexFormat( ).mStorageType == Gfx::tIndexFormat::cStorageU16 )
		{
			tDynamicArray<u16> ids( numIds );
			fGenerateTriList( ids.fBegin( ), numTris );
			mGeometry.fCopyIndicesToGpu( ids.fBegin( ), numIds );
		}
		else
		{
			tDynamicArray<u32> ids( numIds );
			fGenerateTriList( ids.fBegin( ), numTris );
			mGeometry.fCopyIndicesToGpu( ids.fBegin( ), numIds );
		}

		fSetRenderBatch( mGeometry.fGetRenderBatch( ) );
	}

	b32 tText::fCompact( f32 availableSpace )
	{
		return fCompactGetScale( availableSpace ) == 1.0f;
	}

	f32 tText::fCompactGetScale( f32 availableSpace )
	{
		const f32 textWidth = fWidth( );
		if( textWidth != 0 && textWidth > availableSpace )
		{
			const f32 textScale = availableSpace / textWidth;
			fSetScale( textScale, 1.0f );
			return textScale;
		}
		else
			return 1.0f;
	}

	f32 tText::fCompactAndUnscale( f32 availableSpace )
	{
		const f32 textScale = fCompactGetScale( availableSpace );
		if( textScale == 1.0f )
			fSetScale( textScale, 1.0f );
		return textScale;
	}

	f32 tText::fCompactMaintainScale( f32 availableSpace )
	{
		const f32 textWidth = fWidth( ) * fScale( ).x;
		if( textWidth != 0 && textWidth > availableSpace )
		{
			const f32 textScale = availableSpace / textWidth;
			fSetScale( textScale * fScale( ).x, fScale( ).y );
			return textScale;
		}
		else
			return fScale( ).x;
	}

}}


namespace Sig { namespace Gui
{
	void tText::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tText, tRenderableCanvas, Sqrat::NoCopy<tText> > classDesc( vm.fSq( ) );

		classDesc
			.Func<void (tText::*)(const tFilePathPtr&)>( _SC("SetFont"), &tText::fSetFont )
			.Func( _SC("SetDevFont"), &tText::fSetDevFont )
			.Func( _SC("SetFontById"), &tText::fSetFontFromId )
			.Prop( _SC("LineHeight"), &tText::fLineHeight )
			.Prop( _SC("Base"), &tText::fBase )
			.Overload<void (tText::*)(const char*, u32)>( _SC("BakeCString"), &tText::fBakeCString )
			.Overload<void (tText::*)(const char*)>( _SC("BakeCString"), &tText::fBakeCString )
			.Overload<void (tText::*)(const tLocalizedString*, u32)>( _SC("BakeLocString"), &tText::fBakeLocString )
			.Overload<void (tText::*)(const tLocalizedString*)>( _SC("BakeLocString"), &tText::fBakeLocString )
			.Func( _SC("BakeBoxCString"), &tText::fBakeBoxCString )
			.Func( _SC("BakeBoxLocString"), &tText::fBakeBoxLocString )
			.Prop( _SC("Width"), &tText::fWidth )
			.Prop( _SC("Height"), &tText::fHeight )
			.Func( _SC("Compact"), &tText::fCompact )
			.Func( _SC("CompactGetScale"), &tText::fCompactGetScale )
			.Func( _SC("CompactAndUnscale"), &tText::fCompactAndUnscale )
			.Func( _SC("CompactMaintainScale"), &tText::fCompactMaintainScale )
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("Text"), classDesc);

		vm.fConstTable( ).Const( "TEXT_ALIGN_LEFT", ( int )cAlignLeft );
		vm.fConstTable( ).Const( "TEXT_ALIGN_CENTER", ( int )cAlignCenter );
		vm.fConstTable( ).Const( "TEXT_ALIGN_RIGHT", ( int )cAlignRight );
	}
}}

namespace Sig { namespace Gui { namespace Detail
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
		{0x00A2, true,  false}, // 00A2: Åë  Cent Sign                              etc. 
		{0x00A3, false, true},  // 00A3: Pound Sign                                tc 
		{0x00A5, false, true},  // 00A5: Yen Sign                                  tc 
		{0x00A7, false, true},  // 00A7: Section Sign                              ja 
		{0x00A8, true,  false}, // 00A8: ÅN Diaeresis                              etc. 
		{0x00A9, true,  false}, // 00A9: ccopyright sign                           etc.
		{0x00AE, true,  false}, // 00AE: Rregistered sign                          etc.
		{0x00B0, true,  false}, // 00B0: Degree Sign                               kr 
		{0x00B7, true,  false}, // 00B7: Middle Dot                                tc 
		{0x02C7, true,  false}, // 02C7: ? Caron                                   etc. 
		{0x02C9, true,  false}, // 02C9: ? Modified Letter marcon                  etc. 
		{0x2013, true,  false}, // 2013: En Dash                                   tc 
		{0x2014, true,  false}, // 2014: Em Dash                                   tc 
		{0x2015, true,  false}, // 2015: Å\ Horizon bar                            etc. 
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
		{0x3003, true,  false}, // 3003: ÅVDitto Mark                              etc. 
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
		{0xFF02, true,  false}, // FF02: ˙W Fullwidth quotation mark               etc. 
		{0xFF04, false, true},  // FF04: Fullwidth Dollar Sign                     kr 
		{0xFF05, true,  false}, // FF05: Fullwidth Percent Sign                    kr 
		{0xFF07, true,  false}, // FF07: ˙V Fullwidth Apos                         etc. 
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
		{0xFF40, true,  false}, // FF40: ÅM Fullwidth Grave accent                 etc. 
		{0xFF5B, false, true},  // FF5B: Fullwidth Left Curly Bracket              ja, tc 
		{0xFF5C, true,  false}, // FF5C: Åb Fullwidth Vertical line                etc. 
		{0xFF5D, true,  false}, // FF5D: Fullwidth Right Curly Bracket             kr 
		{0xFF5E, true,  false}, // FF5E: Å` Fullwidth Tilda                        etc. 
		{0xFFE0, true,  true},  // FFE0: Fullwidth Cent Sign                       ja, kr 
		{0xFFE1, false, true},  // FFE1: Fullwidth Pound Sign                      fr, kr 
		{0xFFE5, false, true},  // FFE5: Fullwidth Yen Sign                        ja 
		{0xFFE6, false, true}   // FFE6: Fullwidth Won Sign                        kr 
	}; 
	static const int cNumBreakCharacters = array_length( cBreakArray );

	b32 fIsWhiteSpace( wchar_t c )
	{
		return ( c == L'\t' || c == L'\r' || c == L' ' || c == 0x3000 );
	}

	b32 fIsEastAsianChar( wchar_t c )
	{
		return
			( 0x1100 <= c ) && ( c <= 0x11FF ) ||		// Hangul Jamo
			( 0x3000 <= c ) && ( c <= 0xD7AF ) ||		// CJK symbols - Hangul Syllables
			( 0xF900 <= c ) && ( c <= 0xFAFF ) ||		// CJK compat
			( 0xFF00 <= c ) && ( c <= 0xFFDC );			// Halfwidth / Fullwidth
	}

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

	b32 fWordWrap_CanBreakLineAt( const wchar_t * psz, const wchar_t * pszStart )
	{
		if( psz == pszStart )
			return false;	// leave at least one character in a line
		if( fIsWhiteSpace( *psz ) && fIsNonBeginningChar( psz[1] ) )
			return 	false;	

		if(psz - pszStart >1)	// Do not word wrap when current character is IsEastAsianChar , the previus character as " and before it was space.
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

