//------------------------------------------------------------------------------
// \file tTextGeometry.hpp - 30 Nov 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTextGeometry__
#define __tTextGeometry__

#include "tDynamicGeometry.hpp"
#include "tDefaultAllocators.hpp"
#include "tFontMaterial.hpp"

namespace Sig { namespace Gfx
{
	///
	/// \class tTextGeometry
	/// \brief 
	class tTextGeometry
	{
	public:

		enum tAlignment
		{
			cAlignLeft,
			cAlignCenter,
			cAlignRight
		};

	public:

		explicit tTextGeometry( );
		explicit tTextGeometry( Gfx::tDefaultAllocators& allocators );

		b32 fIsValid( ) const { return !mFont.fNull( ); }

		// These will free the underlying geometry
		void fSetFontDev( );
		void fSetFont( const tFilePathPtr& font );
		void fSetFont( u32 gameSpecificId );
		void fSetFont( const tResourcePtr& font );
		const tResourcePtr & fFont( ) const { return mFont; }

		// Access to the underlying render batch
		const tRenderBatchPtr & fRenderBatch( ) const { return mGeometry.fGetRenderBatch( ); }

		// Access to baked size information
		f32 fWidth( ) const { return mWidth; }
		f32 fHeight( ) const { return mHeight; }
		u32 fAlignment( ) const { return mAlignment; }

		/// \brief "Bake" (generate the text geometry) from the specified string, using the specified alignment.
		void fBake( const char* text, u32 count, u32 alignment );
		void fBake( const wchar_t* text, u32 count, u32 alignment );

		///
		/// \brief "Bake" (generate the text geometry) from the specified string, using the specified alignment. The text
		/// will be wrapped using the specified width in order to constrain it to a box.
		/// \return The height of the box.
		f32 fBakeBox( f32 width, const char* text, u32 count, u32 alignment );
		f32 fBakeBox( f32 width, const wchar_t* text, u32 count, u32 alignment );

	private:

		struct tBakeData
		{
			tBakeData( u32 numGlyphs = 0 );

			tGrowableArray< Gfx::tGlyphRenderVertex > mQuadVerts;
			tGrowableBuffer mIndices;
		};

	private:

		void fCommonCtor( );

		static tBakeData & fGlobalBakeData( );
		void fBakeToData( 
			tBakeData & bakeData, 
			f32 x, f32 y, f32 z, 
			const wchar_t* text, u32 count, 
			f32 spacing = 0.f );
		void fBakeToGeometry( const tBakeData & bakeData );


	private:

		u32							mAlignment;
		f32							mWidth, mHeight;
		tResourcePtr				mFont;
		tDynamicGeometry			mGeometry;
		Gfx::tDefaultAllocators &	mAllocators;
	};

} } // ::Sig::Gfx

#endif//__tTextGeometry__
