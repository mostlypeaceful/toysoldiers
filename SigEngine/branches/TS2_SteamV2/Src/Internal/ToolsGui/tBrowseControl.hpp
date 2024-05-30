//------------------------------------------------------------------------------
// \file tBrowseContrl.hpp - 19 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tBrowseContrl__
#define __tBrowseContrl__

namespace Sig
{

	///
	/// \class tBrowseControl
	/// \brief Represents a button and text box for standard file system browsing
	class toolsgui_export tBrowseControl : public tUncopyable, public wxEvtHandler
	{
		wxWindow* mParent;
		wxTextCtrl* mTextBox;

	public:

		tBrowseControl( wxWindow* parent, wxBoxSizer* ownerSizer, const char* name );
		virtual ~tBrowseControl( ) { }
		
		void fSetText( const wxString& text );
		wxString fGetText( ) const;

	protected:

		wxWindow * fParentWindow( ) { return mParent; }
		virtual void fOnBrowse( ) = 0;

	private:
		
		void fOnBrowseInternal( wxCommandEvent & );
	};

	///
	/// \class tFolderBrowseControl
	/// \brief Browse for folder ui
	class toolsgui_export tFolderBrowseControl : public tBrowseControl
	{
	public:

		tFolderBrowseControl( wxWindow* parent, wxBoxSizer* ownerSizer, const char* name );
		virtual ~tFolderBrowseControl( ) { }

	protected:
		
		virtual void fOnBrowse( );

	};

	///
	/// \class tFileBrowseControl
	/// \brief Browse for file ui
	class toolsgui_export tFileBrowseControl : public tBrowseControl
	{
	public:

		tFileBrowseControl( 
			wxWindow * parent, 
			wxBoxSizer * ownerSizer, 
			const char * name, 
			const char * fileFilter );

	protected:

		virtual void fOnBrowse( );

	private:

		std::string mFileFilter;
	};
}

#endif//__tBrowseContrl__
