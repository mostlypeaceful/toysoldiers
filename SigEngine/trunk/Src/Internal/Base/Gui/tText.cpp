//------------------------------------------------------------------------------
// \file tText.cpp - 30 Nov 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tText.hpp"
#include "Gfx/tScreen.hpp"
#include "tFont.hpp"

namespace Sig { namespace Gui
{

	//------------------------------------------------------------------------------
	// tText
	//------------------------------------------------------------------------------
	tText::tText( b32 dropShadow )
	{
		fCommonCtor( dropShadow );
	}

	//------------------------------------------------------------------------------
	u32 tText::fLineHeight( ) const 
	{
		if( const tResourcePtr & font = mGeometry.fFont( ) )
			return font->fCast<tFont>( )->mDesc.mLineHeight;
		return 0;
	}

	//------------------------------------------------------------------------------
	u32 tText::fBase( ) const 
	{ 
		if( const tResourcePtr & font = mGeometry.fFont( ) )
			return font->fCast<tFont>( )->mDesc.mBase;
		return 0;
	}

	//------------------------------------------------------------------------------
	void tText::fBake( const char* text, u32 count, u32 alignment ) 
	{
		mGeometry.fBake( text, count, alignment ); 
		fOnBake( ); 
	}

	//------------------------------------------------------------------------------
	void tText::fBake( const wchar_t* text, u32 count, u32 alignment ) 
	{ 
		mGeometry.fBake( text, count, alignment ); 
		fOnBake( ); 
	}

	//------------------------------------------------------------------------------
	void tText::fBake( const std::wstring& text, u32 alignment ) 
	{ 
		mGeometry.fBake( text.c_str( ), text.length( ), alignment ); 
		fOnBake( ); 
	}

	//------------------------------------------------------------------------------
	void tText::fBake( const tLocalizedString& text, u32 alignment ) 
	{ 
		mGeometry.fBake( text.fCStr( ), text.fLength( ), alignment ); 
		fOnBake( ); 
	}

	//------------------------------------------------------------------------------
	f32  tText::fBakeBox( f32 width, const char* text, u32 count, u32 alignment ) 
	{ 
		const f32 height = mGeometry.fBakeBox( width, text, count, alignment ); 
		fOnBake( );
		return height;
	}

	//------------------------------------------------------------------------------
	f32  tText::fBakeBox( f32 width, const wchar_t* text, u32 count, u32 alignment ) 
	{ 
		const f32 height = mGeometry.fBakeBox( width, text, count, alignment ); 
		fOnBake( );
		return height;
	}

	//------------------------------------------------------------------------------
	f32  tText::fBakeBox( f32 width, const std::wstring& text, u32 alignment ) 
	{ 
		const f32 height = mGeometry.fBakeBox( width, text.c_str( ), text.length( ), alignment );
		fOnBake( );
		return height;
	}

	//------------------------------------------------------------------------------
	f32  tText::fBakeBox( f32 width, const tLocalizedString& text, u32 alignment ) 
	{ 
		const f32 height = mGeometry.fBakeBox( width, text.fCStr( ), text.fLength( ), alignment );
		fOnBake( );
		return height;
	}

	//------------------------------------------------------------------------------
	void tText::fCommonCtor( b32 dropShadow )
	{
		fSetDropShadowEnabled( dropShadow );
		
		fUpdateBounds( );

		fSetRenderBatch( Gfx::tRenderBatchPtr( ) );
	}

	//------------------------------------------------------------------------------
	void tText::fUpdateBounds( )
	{
		const f32 width = fWidth( );
		const f32 height = fHeight( );

		Math::tRect rect;
		rect.mT = 0.f;
		rect.mB = height;

		switch( mGeometry.fAlignment( ) )
		{
		case cAlignLeft:
			rect.mL = 0.f;
			rect.mR = width;
			break;
		case cAlignCenter:
			rect.mL = -0.5f * width;
			rect.mR = +0.5f * width;
			break;
		case cAlignRight:
			rect.mL = -width;
			rect.mR = 0.f;
			break;
		}

		fSetBounds( rect );
	}

	//------------------------------------------------------------------------------
	b32 tText::fCompact( f32 availableSpace )
	{
		return fCompactGetScale( availableSpace ) == 1.0f;
	}

	//------------------------------------------------------------------------------
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

	//------------------------------------------------------------------------------
	f32 tText::fCompactAndUnscale( f32 availableSpace )
	{
		const f32 textScale = fCompactGetScale( availableSpace );
		if( textScale == 1.0f )
			fSetScale( textScale, 1.0f );
		return textScale;
	}

	//------------------------------------------------------------------------------
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
	namespace
	{
		static const tStringPtr cTextString( "Gui.Text" );
		static const char* fTypeOfText( )
		{
			return cTextString.fCStr( );
		}
	}

	void tText::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tText, tRenderableCanvas, Sqrat::NoCopy<tText> > classDesc( vm.fSq( ) );

		classDesc
			.Func<void (tText::*)(const tFilePathPtr&)>( _SC("SetFont"), &tText::fSetFont )
			.Func( _SC("SetDevFont"), &tText::fSetDevFont )
			.Func( _SC("SetFontById"), &tText::fSetFontFromId )
			.Prop( _SC("LineHeight"), &tText::fLineHeight )
			.Prop( _SC("Base"), &tText::fBase )
			.Overload<void (tText::*)(const char*, u32)>					( _SC("BakeCString"), &tText::fBakeCString )
			.Overload<void (tText::*)(const char*)>							( _SC("BakeCString"), &tText::fBakeCString )
			.Overload<void (tText::*)(const tLocalizedString*, u32)>		( _SC("BakeLocString"), &tText::fBakeLocString )
			.Overload<void (tText::*)(const tLocalizedString*)>				( _SC("BakeLocString"), &tText::fBakeLocString )
			.Overload<void (tText::*)(f32, const char*, u32)>				( _SC("BakeBoxCString"), &tText::fBakeBoxCString )
			.Overload<void (tText::*)(f32, const char*)>					( _SC("BakeBoxCString"), &tText::fBakeBoxCString )
			.Overload<void (tText::*)(f32, const tLocalizedString*, u32)>	( _SC("BakeBoxLocString"), &tText::fBakeBoxLocString )
			.Overload<void (tText::*)(f32, const tLocalizedString*)>		( _SC("BakeBoxLocString"), &tText::fBakeBoxLocString )
			.Prop( _SC("Width"), &tText::fWidth )
			.Prop( _SC("Height"), &tText::fHeight )
			.Func( _SC("Compact"), &tText::fCompact )
			.Func( _SC("CompactGetScale"), &tText::fCompactGetScale )
			.Func( _SC("CompactAndUnscale"), &tText::fCompactAndUnscale )
			.Func( _SC("CompactMaintainScale"), &tText::fCompactMaintainScale )
			.StaticFunc(_SC("_typeof"), &fTypeOfText)
			;

		vm.fNamespace(_SC("Gui")).Bind(_SC("Text"), classDesc);

		vm.fConstTable( ).Const( "TEXT_ALIGN_LEFT", ( int )cAlignLeft );
		vm.fConstTable( ).Const( "TEXT_ALIGN_CENTER", ( int )cAlignCenter );
		vm.fConstTable( ).Const( "TEXT_ALIGN_RIGHT", ( int )cAlignRight );
	}
}}
