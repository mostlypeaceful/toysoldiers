#ifndef __tWxSlapOnMask__
#define __tWxSlapOnMask__
#include "tWxSlapOnCheckbox.hpp"

namespace Sig
{

	class tools_export tWxSlapOnMask : public tWxSlapOnControl
	{
		u32 mMaskValue;
		u32 mNumBits;

		class tools_export tCheckBox : public tWxSlapOnCheckBox
		{
			tWxSlapOnMask* mOwnerProp;
			u32	mIndex;
		public:
			tCheckBox( tWxSlapOnMask* ownerProp, wxWindow* parent, u32 index, tWxSlapOnGridSizer* parentSizer ) 
				: tWxSlapOnCheckBox( parent , StringUtil::fToString( index ).c_str( ), true, parentSizer )
				, mOwnerProp( ownerProp ), mIndex( index ) 
			{ }
			virtual void fOnControlUpdated( );
		};
		class tools_export tAllButton : public tWxSlapOnCheckBox
		{
			tWxSlapOnMask* mOwnerProp;
		public:
			tAllButton( tWxSlapOnMask* ownerProp, wxWindow* parent, tWxSlapOnGridSizer* parentSizer ) 
				: tWxSlapOnCheckBox( parent, "All", true, parentSizer )
				, mOwnerProp( ownerProp )
			{ }
			virtual void fOnControlUpdated( );
		};

		tGrowableArray<tCheckBox*> mCheckBoxes;
		tAllButton* mAllButton;

	public:
		tWxSlapOnMask( wxWindow* parent, const char* label, u32 numBits );

		virtual void fEnableControl( );
		virtual void fDisableControl( );

		void fSetToolTip( const wxString& toolTip );

		void fSetValue( u32 val, b32 changed );
		u32  fGetValue( );

		void fGrayOut( b32 gray );

	private:

		void fUpdateAllButton( );
		void fOnControlUpdatedInternal( wxCommandEvent& );
	};

}

#endif//__tWxSlapOnMask__
