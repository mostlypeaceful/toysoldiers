#include "ToolsPch.hpp"
#include "tWxSlapOnMask.hpp"
#include "wx/tglbtn.h"

namespace Sig
{

	tWxSlapOnMask::tWxSlapOnMask( wxWindow* parent, const char* label, u32 numBits )
		: tWxSlapOnControl( parent, label )
		, mMaskValue( ~0 )
		, mNumBits( numBits )
		, mAllButton( NULL )
	{
		const u32 cColumns = 4;
		const u32 cLeftPadding = tWxSlapOnControl::fGlobalLabelWidth( ) + tWxSlapOnControl::fGlobalSizerPadding( );
		tWxSlapOnGridSizer* sizer = new tWxSlapOnGridSizer( parent, cColumns, cLeftPadding );

		for( u32 i = 0; i < mNumBits; ++i )
			mCheckBoxes.fPushBack( new tCheckBox( this, parent, i, sizer ) );

		mAllButton = new tAllButton( this, parent, sizer );
	}

	void tWxSlapOnMask::fEnableControl( )
	{
		for( u32 i = 0; i < mCheckBoxes.fCount( ); ++i )
			mCheckBoxes[ i ]->fEnableControl( );
		mAllButton->fEnableControl( );
	}
	
	void tWxSlapOnMask::fDisableControl( )
	{
		for( u32 i = 0; i < mCheckBoxes.fCount( ); ++i )
			mCheckBoxes[ i ]->fDisableControl( );
		mAllButton->fDisableControl( );
	}

	void tWxSlapOnMask::fSetToolTip( const wxString& toolTip )
	{
		for( u32 i = 0; i < mCheckBoxes.fCount( ); ++i )
			mCheckBoxes[ i ]->fSetToolTip( toolTip );
		mAllButton->fSetToolTip( toolTip );

		tWxSlapOnControl::SetToolTip( toolTip );
	}

	void tWxSlapOnMask::fOnControlUpdatedInternal( wxCommandEvent& )
	{
		fOnControlUpdated( );
		fUpdateAllButton( );
	}

	void tWxSlapOnMask::fSetValue( u32 val, b32 changed )
	{
		mMaskValue = val;
		for( u32 i = 0; i < mCheckBoxes.fCount( ); ++i )
			mCheckBoxes[ i ]->fSetValue( fTestBits( val, (1<<i) ) ? tWxSlapOnCheckBox::cTrue : tWxSlapOnCheckBox::cFalse  );
		fUpdateAllButton( );

		if( changed )
			fOnControlUpdated( );
	}

	u32 tWxSlapOnMask::fGetValue( )
	{
		return mMaskValue;
	}

	void tWxSlapOnMask::fUpdateAllButton( )
	{
		b32 all = true;
		for( u32 i = 0; i < mCheckBoxes.fCount( ); ++ i )
		{
			b32 val = fTestBits( mMaskValue, (1<<i) );
			if( !val ) 
				all = false;
		}
		mAllButton->fSetValue( all ? tWxSlapOnCheckBox::cTrue : tWxSlapOnCheckBox::cFalse  );
	}

	void tWxSlapOnMask::tAllButton::fOnControlUpdated( ) 
	{ 
		u32 val = mOwnerProp->fGetValue( );

		if( fGetValue( ) == cTrue )
			val = ~0;
		else if( fGetValue( ) == cFalse )
			val = 0;

		mOwnerProp->fSetValue( val, true );
	}

	void tWxSlapOnMask::tCheckBox::fOnControlUpdated( ) 
	{ 
		u32 val = mOwnerProp->fGetValue( );

		if( fGetValue( ) == cTrue )
			val = fSetBits( val, (1<<mIndex) );
		else if( fGetValue( ) == cFalse )
			val = fClearBits( val, (1<<mIndex) );

		mOwnerProp->fSetValue( val, true );
	}

	void tWxSlapOnMask::fGrayOut( b32 gray )
	{
		if( gray )
		{
			for( u32 i = 0; i < mCheckBoxes.fCount( ); ++ i )
				mCheckBoxes[ i ]->fSetValue( tWxSlapOnCheckBox::cGray );
			mAllButton->fSetValue( tWxSlapOnCheckBox::cGray );
		}
	}
}
