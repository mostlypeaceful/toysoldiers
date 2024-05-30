#ifndef __tWxSlapOnCheckBox__
#define __tWxSlapOnCheckBox__
#include "tWxSlapOnControl.hpp"

class wxCheckBox;
class wxCommandEvent;
class wxToggleButton;

namespace Sig
{

	class tools_export tWxSlapOnCheckBox : public tWxSlapOnControl
	{
		wxCheckBox*		mCheckBox;
		wxToggleButton* mButton;

	public:

		static const s32 cFalse = false;
		static const s32 cTrue	= true;
		static const s32 cGray	= -1;

		tWxSlapOnCheckBox( wxWindow* parent, const char* label, b32 asButton = false, tWxSlapOnGridSizer* parentSizer = NULL );

		virtual void fEnableControl( );
		virtual void fDisableControl( );

		void fSetToolTip( const wxString& toolTip );

		void	fSetValue( s32 val );
		s32		fGetValue( );
		b32		fGetValueBool( ) { return ( fGetValue( ) == cTrue ) ? true : false; }

	private:

		void fOnControlUpdatedInternal( wxCommandEvent& );
	};

	class tools_export tWxSlapOnCheckBoxDataSync : public tWxSlapOnCheckBox
	{
		s32& mData;
	public:
		tWxSlapOnCheckBoxDataSync( wxWindow* parent, const char* label, s32& data );
		void	fSetValue( s32 val );
		virtual void fOnControlUpdated( );
	};

}

#endif//__tWxSlapOnCheckBox__
