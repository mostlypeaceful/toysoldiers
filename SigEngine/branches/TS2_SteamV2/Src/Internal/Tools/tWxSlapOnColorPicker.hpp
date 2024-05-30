#ifndef __tWxSlapOnColorPicker__
#define __tWxSlapOnColorPicker__
#include "tWxSlapOnControl.hpp"

class wxButton;
class wxCommandEvent;

namespace Sig
{
	struct tools_export tColorPickerData
	{
		Math::tVec3f mRgb;
		f32 mAlpha;
		f32 mRgbScale;
		tColorPickerData( ) : mRgb( 1.f ), mAlpha( 1.f ), mRgbScale( 1.f ) { }
		explicit tColorPickerData( const Math::tVec3f& rgb, f32 alpha = 1.f, f32 rgbScale = 1.f ) : mRgb( rgb ), mAlpha( alpha ), mRgbScale( rgbScale ) { }

		inline Math::tVec4f fExpandRgba( ) const { return Math::tVec4f( mRgb * mRgbScale, mAlpha ); }

		void fNormalize( );

		inline b32 operator==( const tColorPickerData& other ) const
		{
			return mRgb.fEqual( other.mRgb ) && fEqual( mAlpha, other.mAlpha ) && fEqual( mRgbScale, other.mRgbScale );
		}

		inline b32 operator!=( const tColorPickerData& other ) const
		{
			return !operator==( other );
		}

		template<class tSerializer>
		void fSerializeXml( tSerializer& s )
		{
			s( "Rgb", mRgb );
			s( "Alpha", mAlpha );
			s( "RgbScale", mRgbScale );
		}
	};

	inline tColorPickerData fClamp( const tColorPickerData& toClamp, const tColorPickerData& min, const tColorPickerData& max )
	{
		tColorPickerData v = toClamp; v.fNormalize( );
		tColorPickerData vMin = min; vMin.fNormalize( );
		tColorPickerData vMax = max; vMax.fNormalize( );
		return tColorPickerData( 
			fClamp( v.mRgb, vMin.mRgb, vMax.mRgb ), 
			fClamp( v.mAlpha, vMin.mAlpha, vMax.mAlpha ), 
			fClamp( v.mRgbScale, vMin.mRgbScale, vMax.mRgbScale ) );
	}

	class tools_export tWxSlapOnColorPicker : public tWxSlapOnControl
	{
		static tFixedArray<wxColour,16> gCustomColors;
	protected:
		wxButton*			mButton;
		tColorPickerData	mValue, mMin, mMax;

	public:

		static b32 fPopUpDialog( 
			wxButton* buttton, 
			tColorPickerData& value, 
			const tColorPickerData& min = tColorPickerData( 0.f, 0.f, 1.f ),
			const tColorPickerData& max = tColorPickerData( 1.f, 1.f, Math::cInfinity ));

	public:

		tWxSlapOnColorPicker( 
			wxWindow* parent, 
			const char* label,
			const tColorPickerData& initState = tColorPickerData( ), 
			const tColorPickerData& min = tColorPickerData( 0.f, 0.f, 1.f ),
			const tColorPickerData& max = tColorPickerData( 1.f, 1.f, Math::cInfinity ) );

		virtual void	fEnableControl( );
		virtual void	fDisableControl( );

		tColorPickerData fGetValue( ) const;
		void fSetValue( const tColorPickerData& colorData );

	private:

		void			fOnButtonPressedInternal( wxCommandEvent& );
	};

}

#endif//__tWxSlapOnColorPicker__
