//------------------------------------------------------------------------------
// \file tFont.hpp - 01 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tFont__
#define __tFont__
#include "Gfx/tFontMaterial.hpp"

namespace Sig { namespace Gui
{
	struct base_export tFontDesc
	{
		declare_reflector( );

		u16 mLineHeight;
		u16 mBase;
		u16 mScaleW;
		u16 mScaleH;
		u16 mPages;
		b8  mPacked;
		b8	mBold;
		b8	mItalic;
		b8	mOutline;
		u8  mAlphaChnl;
		u8	mRedChnl;
		u8	mGreenChnl;
		u8	mBlueChnl;
		u8  pad1,pad2;

		tFontDesc( );
		tFontDesc( tNoOpTag );
	};

	struct base_export tFontGlyphKerning
	{
		declare_reflector( );

		u16 mSecond;
		s16 mAmount;

		tFontGlyphKerning( );
		tFontGlyphKerning( tNoOpTag );
	};

	struct base_export tFontGlyph
	{
		declare_reflector( );

		typedef tDynamicArray<tFontGlyphKerning> tKernPairs;

		u16 mId;
		u16 mX;
		u16 mY;
		u16 mWidth;
		u16 mHeight;
		s16 mXOffset;
		s16 mYOffset;
		s16 mXAdvance;
		u16 mPage;
		u16 mChannel;
		tKernPairs mKernPairs;

		tFontGlyph( );
		tFontGlyph( tNoOpTag );

		inline s16 fGetKerningAmount( u16 nextGlyph ) const
		{
			for( u32 i = 0; i < mKernPairs.fCount( ); ++i )
				if( mKernPairs[ i ].mSecond == nextGlyph )
					return mKernPairs[ i ].mAmount;
			return 0;
		}
	};

	struct base_export tFontPage
	{
		declare_reflector( );

		Gfx::tFontMaterial* mMaterial;

		tFontPage( );
		tFontPage( tNoOpTag );
	};

	class base_export tFont : public tLoadInPlaceFileBase
	{
		declare_reflector( );
		declare_lip_version( );
		implement_rtti_serializable_base_class(tFont, 0xB164C06C);
	public:
		static const u16		cAtSymbol = 64;
		static const char*		fGetFileExtension( );
		static tFilePathPtr		fDevFontPath( );

		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }
	public:
		typedef tHashTable<u16, const tFontGlyph*, tHashTableNoResizePolicy>	tGlyphMap;
		typedef tLoadInPlaceRuntimeObject<tGlyphMap>							tGlyphMapStorage;
		typedef tDynamicArray< tFontGlyph >										tGlyphArray;
		typedef tDynamicArray< tFontPage >										tFontPageArray;


		tFontDesc				mDesc;
		tLoadInPlaceStringPtr*	mFaceName;
		tGlyphArray				mRawGlyphs;
		tGlyphMapStorage		mGlyphMap;
		tFontGlyph				mDefaultGlyph;
		tFontPageArray			mPages;

		tFont( );
		tFont( tNoOpTag );
		~tFont( );
		
		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( const tResource& ownerResource );

		const tGlyphMap& fGetGlyphMap( ) const { return mGlyphMap.fTreatAsObject( ); }
		const tFontGlyph & fFindGlyph( u16 character ) const;
		const tFontGlyph & fFindGlyhp( wchar_t character ) const { return fFindGlyph( ( u16 )character ); }

		f32	fGetTextWidth( const char* text, u32 count ) const;
		f32	fGetTextWidth( const wchar_t* text, u32 count ) const;

		template<class T>
		f32	fGetTextWidth( const T& text ) { return fGetTextWidth( text.c_str( ), text.length( ) ); }
	};
}} // ::Sig::Gui

#endif//__tFont__
