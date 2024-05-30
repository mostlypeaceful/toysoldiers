#ifndef __tWxSlapOnDialog__
#define __tWxSlapOnDialog__
#include "tWxSavedLayout.hpp"

namespace Sig
{

	class tools_export tWxSlapOnDialog : public wxFrame, public tWxSavedLayout
	{
	public:
		static const u32 cDefaultStyles;
	private:
		HWND mRawParent;
		b32	mWasTopMost; // TODO: remove these, need to ensure MatEd stuff in the plugin will operate without it.
		std::string	mRegKeyName;
	public:
		explicit tWxSlapOnDialog( const char* title, wxWindow* parent=0, const std::string regKeyName="" );
		virtual std::string fRegistryKeyName( ) const;
		void fSetRawParent( HWND rawParent );
		void fSave( );
		void fLoad( );
		b32 fIsActive( ) const;
		b32 fIsMinimized( ) const;
		void fRestoreFromMinimized( );
		void fShow( );
		void fToggle( );

		// TODO: remove these 
		void fSetTopMost( b32 makeTopMost );
		b32  fAutoHandleTopMost( HWND parent = NULL );
	protected:
		virtual void fOnClose( wxCloseEvent& event );
	};

}


#endif//__tWxSlapOnDialog__
