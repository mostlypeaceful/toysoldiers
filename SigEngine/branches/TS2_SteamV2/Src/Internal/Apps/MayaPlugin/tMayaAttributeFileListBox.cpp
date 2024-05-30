//------------------------------------------------------------------------------
// \file tMayaAttributeFileListBox.cpp - 17 Aug 2010
// \author jwittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#include "MayaPluginPch.hpp"
#include "tWxSlapOnButton.hpp"
#include "tWxSlapOnGroup.hpp"
#include "tMayaAttributeFileListBox.hpp"
#include "MayaUtil.hpp"
#include "WxUtil.hpp"


namespace Sig
{
	//------------------------------------------------------------------------------
	tMayaAttributeFileListBox::tMayaAttributeFileListBox( 
		wxWindow* parent, 
		const char* label, 
		const char * openFileCaption, 
		const char * openFileFilter )
		: tWxSlapOnAddRemoveListBox( parent, label, true, true )
		, tMayaAttributeListBoxBase( dynamic_cast<tMayaGuiBase *>( parent ), this )
		, mFileCaption( openFileCaption )
		, mFileFilter( openFileFilter )
	{
		fDisableControl( );
	}

	//------------------------------------------------------------------------------
	void tMayaAttributeFileListBox::fOnAdd( )
	{
		std::string path;
		b32 fileFound = WxUtil::fBrowseForFile( 
			path, 
			fGetParentWindow( ), 
			mFileCaption.c_str( ), 
			0, // defDir
			0, // defFile
			mFileFilter.c_str( ) );

		if( !fileFound )
			return;

		tFilePathPtr ptr = ToolsPaths::fMakeResRelative( tFilePathPtr( path ), true );
		if( !ptr.fNull( ) )
			fAddItem( ptr.fCStr( ) );
	}
}