//------------------------------------------------------------------------------
// \file tMayaAttributeListBox.hpp - 17 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMayaAttributeListBox__
#define __tMayaAttributeListBox__
#include "tWxSlapOnListBox.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{

	///
	/// \class tMayaAttributeListBoxBase
	/// \brief 
	class tMayaAttributeListBoxBase : public tMayaAttributeControlBase
	{
	public:

		tMayaAttributeListBoxBase( tMayaGuiBase * parent, tWxSlapOnListBox * listBox );
		virtual void fOnControlUpdated( );

		virtual void fDisableControl( ) = 0;
		virtual void fEnableControl( ) = 0;

	private:
		
		virtual void fOnMayaSelChanged( );
		b32 fOnMayaSelChangedEachObject( MDagPath& path, MObject& component );
		b32 fSetEachSelectedObject( MDagPath& path, MObject& component );

	private:

		tWxSlapOnListBox * mListBox;
		bool mUpdatingSelection;
	};

	///
	/// \class tMayaAttributeListBox
	/// \brief 
	class tMayaAttributeListBox : 
		public tWxSlapOnListBox,
		public tMayaAttributeListBoxBase
	{
	public:

		tMayaAttributeListBox( wxWindow* parent, const char* label );

		virtual void fOnControlUpdated( ) 
		{
			tMayaAttributeListBoxBase::fOnControlUpdated( );
		}

		virtual void fDisableControl( )
		{
			tWxSlapOnListBox::fDisableControl( );
		}

		virtual void fEnableControl( )
		{
			tWxSlapOnListBox::fEnableControl( );
		}

	private:
	};
}

#endif//__tMayaAttributeListBox__
