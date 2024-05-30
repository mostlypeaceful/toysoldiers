//------------------------------------------------------------------------------
// \file tMayaAttributeFileListBox.hpp - 17 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tMayaAttributeFileListBox__
#define __tMayaAttributeFileListBox__
#include "tWxSlapOnAddRemoveListBox.hpp"
#include "tMayaAttributeListBox.hpp"

namespace Sig
{
	class tWxSlapOnButton;

	///
	/// \class tMayaAttributeFileListBox
	/// \brief 
	class tMayaAttributeFileListBox : 
		public tWxSlapOnAddRemoveListBox,
		public tMayaAttributeListBoxBase
	{
	public:

		tMayaAttributeFileListBox( 
			wxWindow* parent, 
			const char* label, 
			const char * openFileCaption, 
			const char * openFileFilter );

		virtual void fOnControlUpdated( ) 
		{
			tMayaAttributeListBoxBase::fOnControlUpdated( );
		}

		virtual void fDisableControl( )
		{
			tWxSlapOnAddRemoveListBox::fDisableControl( );
		}

		virtual void fEnableControl( )
		{
			tWxSlapOnAddRemoveListBox::fEnableControl( );
		}

	protected:

		virtual void fOnAdd( );

	private:

		std::string mFileCaption;
		std::string mFileFilter;
	};
}

#endif//__tMayaAttributeFileListBox__
