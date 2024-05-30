//------------------------------------------------------------------------------
// \file tText.hpp - 30 Nov 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tText__
#define __tText__
#include "tRenderableCanvas.hpp"
#include "tLocalizationFile.hpp"
#include "Gfx/tTextGeometry.hpp"


namespace Sig { namespace Gfx
{
	struct tSafeRect;
	struct tDefaultAllocators;
}}

namespace Sig { namespace Gui
{

	class base_export tText : public tRenderableCanvas
	{
		define_dynamic_cast( tText, tRenderableCanvas );
	public:
		
		enum tAlignment
		{
			cAlignLeft = Gfx::tTextGeometry::cAlignLeft,
			cAlignCenter = Gfx::tTextGeometry::cAlignCenter,
			cAlignRight = Gfx::tTextGeometry::cAlignRight,
		};

	public:
		
		explicit tText( b32 dropShadow = true );

		void fSetDevFont( ) { mGeometry.fSetFontDev( ); fSetRenderBatch( Gfx::tRenderBatchPtr( ) ); }
		void fSetFont( const tResourcePtr& font ) { mGeometry.fSetFont( font ); fSetRenderBatch( Gfx::tRenderBatchPtr( ) ); }
		void fSetFont( const tFilePathPtr& font ) { mGeometry.fSetFont( font ); fSetRenderBatch( Gfx::tRenderBatchPtr( ) ); }
		void fSetFontFromId( u32 gameSpecificId ) { mGeometry.fSetFont( gameSpecificId ); fSetRenderBatch( Gfx::tRenderBatchPtr( ) ); }

		// Font property access
		u32 fLineHeight( ) const;
		u32 fBase( ) const;

		b32 fValid( ) const { return mGeometry.fIsValid( ); }

		f32 fWidth( ) const { return mGeometry.fWidth( ); }
		f32 fHeight( ) const { return mGeometry.fHeight( ); }

		///
		/// \brief "Bake" (generate the text geometry) from the specified string, using the specified alignment.
		void fBake( const char* text, u32 count, u32 alignment );
		void fBake( const wchar_t* text, u32 count, u32 alignment );
		void fBake( const std::wstring& text, u32 alignment );
		void fBake( const tLocalizedString& text, u32 alignment );
		
		// Helpers for baking strings
		void fBakeCString( const char* text, u32 alignment ) { fBake( text, 0, alignment ); }
		void fBakeCString( const char* text ) { fBakeCString( text, cAlignLeft ); }
		void fBakeLocString( const tLocalizedString* text, u32 alignment ) 
			{ fBake( text ? *text : tLocalizedString::fNullString( ), alignment ); }
		void fBakeLocString( const tLocalizedString* text ) { fBakeLocString( text, cAlignLeft ); }

		///
		/// \brief "Bake" (generate the text geometry) from the specified string, using the specified alignment. The text
		/// will be wrapped using the specified width in order to constrain it to a box. If tBakeData is supplied than
		/// that buffer will be used as scratch memory for baking the data
		/// \return The height of the box.
		f32  fBakeBox( f32 width, const char* text, u32 count, u32 alignment );
		f32  fBakeBox( f32 width, const wchar_t* text, u32 count, u32 alignment );
		f32  fBakeBox( f32 width, const std::wstring& text, u32 alignment );
		f32  fBakeBox( f32 width, const tLocalizedString& text, u32 alignment );

		// Helpers for baking string
		void fBakeBoxCString( f32 width, const char* text, u32 alignment ) { fBakeBox( width, text, 0, alignment ); }
		void fBakeBoxCString( f32 width, const char* text ) { fBakeBoxCString( width, text, cAlignLeft ); }
		void fBakeBoxLocString( f32 width, const tLocalizedString* text, u32 alignment ) 
			{ fBakeBox( width, text ? *text : tLocalizedString::fNullString( ), alignment ); }
		void fBakeBoxLocString( f32 width, const tLocalizedString* text ) { fBakeBoxLocString( width, text, cAlignLeft ); }

		/// \brief Allows you to easily compact text because it does not fit in a space. Useful for very long usernames
		/// \return True if the compact occurred, false if the text was not wider than the space
		b32 fCompact( f32 availableSpace );
		/// \brief Compacts and returns the text scale
		f32 fCompactGetScale( f32 availableSpace );
		/// \brief Compacts the text, but will unscale if the text doesn't need compacting
		f32 fCompactAndUnscale( f32 availableSpace );
		/// \brief Compacts text that is already scaled, returns the final scale value
		f32 fCompactMaintainScale( f32 availableSpace );

		static void fExportScriptInterface( tScriptVm& vm );

	private:

		void fCommonCtor( b32 dropShadow );
		void fUpdateBounds( );

		void fOnBake( ) { fUpdateBounds( ); fSetRenderBatch( mGeometry.fRenderBatch( ) ); }

	private:

		Gfx::tTextGeometry mGeometry;
	};

	typedef tRefCounterPtr<tText> tTextPtr;
}}

#endif//__tText__
