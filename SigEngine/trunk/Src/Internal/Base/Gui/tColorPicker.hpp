#ifndef __tColorPicker__
#define __tColorPicker__
#include "tRenderableCanvas.hpp"
#include "Gfx/tSolidColorMaterial.hpp"
#include "Gfx/tDynamicGeometry.hpp"
#include "tColoredQuad.hpp"
#include "tText.hpp"

namespace Sig { namespace Gfx
{
	class tDevice;
	struct tDefaultAllocators;
}}

namespace Sig { namespace Gui
{
	class tColorPickerQuad : public tRenderableCanvas
	{
		define_dynamic_cast( tColorPickerQuad, tRenderableCanvas );
	public:
		tColorPickerQuad( );
		explicit tColorPickerQuad( Gfx::tDefaultAllocators& allocators );
		virtual ~tColorPickerQuad( );

		virtual void fOnDeviceReset( Gfx::tDevice* device );
		
		void fSetRect( const Math::tVec2f& topLeft, const Math::tVec2f& widthHeight );
		void fSetRect( const Math::tVec2f& widthHeight );
		void fSetRect( const Math::tRect& rect );
		
		const Gfx::tRenderState&	fRenderState( ) const { return mRenderState; }
		void						fSetRenderState( const Gfx::tRenderState& state ) { mRenderState = state; }

		// x: hue, [0,1] left to right x direction.
		// y: value, [0,1], bottom to top, black to color
		// z: saturation, [0,1] white to color
		static Math::tVec3f fRGBFromHSV( const Math::tVec3f& hsv );
		static Math::tVec3f fHSVFromRGB( const Math::tVec3f& rgb );

	protected:

		void fCommonCtor( );
		void fResetDeviceObjects( Gfx::tDevice& device );
		void fUpdateGeometry( );

	private:
		Gfx::tSolidColorMaterialPtr	mMaterial;
		tResourcePtr				mColorMap;
		Gfx::tDynamicGeometry		mGeometry;
		Gfx::tRenderState			mRenderState;
		Gfx::tDefaultAllocators&	mAllocators;
	};

	class tColorPicker
	{
		Gui::tColorPickerQuad	mColorPicker;
		Gui::tColoredQuad		mSaturation;
		Gui::tColoredQuad		mPreview;
		Gui::tColoredQuad		mSaturationCursor;
		Gui::tColoredQuad		mColorCursor;
		Gui::tColoredQuad		mAlphaBar;
		Gui::tColoredQuad		mAlphaCursor;
		Gui::tColoredQuad		mIntensityBar;
		Gui::tColoredQuad		mIntensityCursor;
		Gui::tText				mInstructions;
		Gui::tText				mIntensityText;
		tFixedArray<Gui::tText,5> mValues;
		b32 mHasFont;
		u32 mLineHeight;
		b32 mShowAlpha;
		f32 mMin, mMax;

		Math::tVec3f mPos;
		Math::tVec2f mRect;
		Math::tVec3f mSatPos;
		Math::tVec2f mSatRect;
		Math::tVec3f mAlphaPos;
		Math::tVec3f mIntensityPos;

		Math::tVec4f mRGBA;
		f32 mIntensity;


		static const u32 cDefaultSize = 150;
		static const u32 cSpacer = 10;
		static const u32 cSaturationWidth = 25;
		static const u32 cPreviewSize = 40;
		static const u32 cCursorWidth = 3;

	public:
		tColorPicker( );

		void fSetPosition( const Math::tVec3f& pos );
		void fSetRect( const Math::tVec2f& rect );
		void fSetFont( const tResourcePtr& smallFont );
		void fResize( );

		void fDraw( Gfx::tScreen& screen, b32 showAlpha );
		void fSetColor( const Math::tVec4f& rgba, f32 min, f32 max );
		void fSetColor( const Math::tVec3f& rgb, f32 min, f32 max );

		static void fIncrementColor( Math::tVec3f& rgb, const Math::tVec3f& hsvDelta, f32 min, f32 max );
		void fRefreshColor( );
	};

}}

#endif//__tColorPicker__
