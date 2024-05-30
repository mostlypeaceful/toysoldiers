#ifndef __tText__
#define __tText__
#include "tRenderableCanvas.hpp"
#include "tLocalizationFile.hpp"
#include "Gui/tFont.hpp"
#include "Gfx/tDynamicGeometry.hpp"

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
			cAlignLeft,
			cAlignCenter,
			cAlignRight,
			cAlignMax
		};

	public:
		explicit tText( b32 dropShadow = true );
		explicit tText( Gfx::tDefaultAllocators& allocators, b32 dropShadow = true );

		// need to rebake text after a device reset
		virtual void fOnDeviceReset( Gfx::tDevice* device );

		void fSetFont( const tResourcePtr& font );
		void fSetFont( const tFilePathPtr& font );
		void fSetDevFont( );
		void fSetFontFromId( u32 gameSpecificId );

		b32 fValid( ) const { return !mFont.fNull( ); }

		const Gui::tFont* fGetFont( ) const { return mFont.fNull( ) ? 0 : mFont->fCast<Gui::tFont>( ); }

		f32 fWidth( ) const { return mWidth; }
		f32 fHeight( ) const { return mHeight; }
		u32 fLineHeight( ) const { return mFont.fNull( ) ? 0 : mFont->fCast<Gui::tFont>( )->mDesc.mLineHeight; }
		u32 fBase( ) const { return mFont.fNull( ) ? 0 : mFont->fCast<Gui::tFont>( )->mDesc.mBase; }

		///
		/// \brief Allows you to compute the width of a string of text without printing it
		f32	 fGetTextWidth( const char* text, u32 count );
		f32	 fGetTextWidth( const wchar_t* text, u32 count );
		f32	 fGetTextWidth( const std::wstring& text ) { return fGetTextWidth( text.c_str( ), text.length( ) ); }
		f32	 fGetTextWidth( const tLocalizedString& text ) { return fGetTextWidth( text.c_str( ), text.length( ) ); }

		///
		/// \brief "Bake" (generate the text geometry) from the specified string, using the specified alignment.
		void fBake( const char* text, u32 count, tAlignment mode );
		void fBake( const wchar_t* text, u32 count, tAlignment mode );
		void fBake( const std::wstring& text, tAlignment mode ) { fBake( text.c_str( ), text.length( ), mode ); }
		void fBake( const tLocalizedString& text, tAlignment mode ) { fBake( text.c_str( ), text.length( ), mode ); }
		void fBakeCString( const char* text, u32 alignment );
		void fBakeCString( const char* text );
		void fBakeLocString( const tLocalizedString* text, u32 alignment );
		void fBakeLocString( const tLocalizedString* text );

		///
		/// \brief "Bake" (generate the text geometry) from the specified string, using the specified alignment. The text
		/// will be wrapped using the specified width in order to constrain it to a box.
		/// \return The height of the box.
		f32  fBakeBox( f32 width, const char* text, u32 count, tAlignment mode );
		f32  fBakeBox( f32 width, const wchar_t* text, u32 count, tAlignment mode );
		f32  fBakeBox( f32 width, const std::wstring& text, tAlignment mode ) { return fBakeBox( width, text.c_str( ), text.length( ), mode ); }
		f32  fBakeBox( f32 width, const tLocalizedString& text, tAlignment mode ) { return fBakeBox( width, text.c_str( ), text.length( ), mode ); }
		void fBakeBoxCString( f32 width, const char* text, u32 alignment ) { fBakeBox( width, text, 0, ( tAlignment )alignment ); }
		void fBakeBoxLocString( f32 width, const tLocalizedString* text, u32 alignment );

		/// \brief Allows you to easily compact text because it does not fit in a space. Useful for very long usernames
		/// \return True if the compact occurred, false if the text was not wider than the space
		b32 fCompact( f32 availableSpace );
		/// \brief Compacts and returns the text scale
		f32 fCompactGetScale( f32 availableSpace );
		/// \brief Compacts the text, but will unscale if the text doesn't need compacting
		f32 fCompactAndUnscale( f32 availableSpace );
		/// \brief Compacts text that is already scaled, returns the final scale value
		f32 fCompactMaintainScale( f32 availableSpace );

	private:

		void fCommonCtor( b32 dropShadow );
		void fUpdateBounds( );

		const Gfx::tMaterial* fMaterial( ) const;
	
		struct base_export tBakeData
		{
			tGrowableArray< Gfx::tGlyphRenderVertex > mQuadVerts;
		};

		void fBakeInternal( tBakeData& bakeData, f32 x, f32 y, f32 z, const wchar_t* text, u32 count, f32 spacing = 0.f );
		void fFinishBake( const tBakeData& bakeData );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	private:
		Gfx::tDynamicGeometry				mGeometry;
		tResourcePtr						mFont;
		tAlignment							mAlignment;
		f32									mWidth;
		f32									mHeight;
		Gfx::tDefaultAllocators&			mAllocators;

		// need to keep this around so we can re-bake after a device reset
		tBakeData mBake;
	};

	typedef tRefCounterPtr<tText> tTextPtr;
}}

#endif//__tText__
