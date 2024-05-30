#include "ToolsPch.hpp"
#include "WxUtil.hpp"
#include "tStrongPtr.hpp"

namespace Sig { namespace WxUtil
{
	b32 fDefaultBrowseForFile( std::string& pathOut, wxWindow* parent, const char* caption, const char* defDir, const char* defFile, const char* filter, u32 style )
	{
		// browse for a new path
		tStrongPtr<wxFileDialog> openFileDialog( new wxFileDialog( 
			parent, 
			caption,
			wxString( defDir ? defDir : "" ),
			wxString( defFile ? defFile : "" ),
			wxString( filter ? filter : "*.*" ),
			style ) );

		if( openFileDialog->ShowModal( ) == wxID_OK )
		{
			pathOut = openFileDialog->GetPath( );
			return true;
		}

		return false;
	}

	tBrowseForFile gBrowseForFile = &fDefaultBrowseForFile;

	b32 fBrowseForFile( std::string& pathOut, wxWindow* parent, const char* caption, const char* defDir, const char* defFile, const char* filter, u32 style /* = wxFD_OPEN */ )
	{
		return gBrowseForFile( pathOut, parent, caption, defDir, defFile, filter, style );
	}
}}
