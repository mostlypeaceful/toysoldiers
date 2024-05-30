//------------------------------------------------------------------------------
// \file tSklmlBrowser.hpp - 07 Sep 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tSklmlBrowser__
#define __tSklmlBrowser__
#include "tConfigurableBrowserTree.hpp"
#include "Sklml.hpp"

namespace Sig
{
	///
	/// \class tSklmlBrowser
	/// \brief 
	class tSklmlBrowser : public tConfigurableBrowserTree
	{
	public:

		tSklmlBrowser( wxDialog * parent ) 
			: tConfigurableBrowserTree( parent, Sklml::fIsSklmlFile, 300, true )
			, mParentDialog( parent )
		{
			if( !parent->GetSizer( ) )
				parent->SetSizer( new wxBoxSizer( wxVERTICAL ) );

			parent->GetSizer( )->Add( this, 1, wxEXPAND | wxALL, 5 );
		}

		b32 fBrowse( ) 
		{
			int ret = mParentDialog->ShowModal( );
			return ret == wxID_OK;
		}

		const tFilePathPtr & fFilePath( ) const { return mFilePath; }

	private:

		virtual void fOpenDoc( const tFilePathPtr& file ) 
		{
			mFilePath = file;
			( (wxDialog *)GetParent( ) )->EndModal( wxID_OK );
		}

	private:

		wxDialog * mParentDialog;
		tFilePathPtr mFilePath;
	};
}

#endif//__tSklmlBrowser__
