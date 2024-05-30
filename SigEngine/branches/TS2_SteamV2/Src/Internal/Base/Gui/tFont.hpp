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
		implement_rtti_serializable_base_class(tFont, 0xB164C06C);
	public:
		static const u32		cVersion;
		static const char*		fGetFileExtension( );
		static tFilePathPtr		fDevFontPath( );
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

		inline const tGlyphMap& fGetGlyphMap( ) const { return mGlyphMap.fTreatAsObject( ); }

		virtual void fOnFileLoaded( const tResource& ownerResource );
		virtual void fOnFileUnloading( );
	};
}}

namespace Sig
{
	template<>
	class tResourceConvertPath<Gui::tFont>
	{
	public:
		static tFilePathPtr fConvertToBinary( const tFilePathPtr& path ) { return tResource::fConvertPathML2B( path ); }
		static tFilePathPtr fConvertToSource( const tFilePathPtr& path ) { return tResource::fConvertPathB2ML( path ); }
	};
}

#endif//__tFont__
