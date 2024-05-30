#ifndef __WxUtil__
#define __WxUtil__

namespace Sig { namespace WxUtil
{
	typedef tools_export b32 (*tBrowseForFile)( std::string& pathOut, wxWindow* parent, const char* caption, const char* defDir, const char* defFile, const char* filter, u32 style );
	extern tools_export tBrowseForFile gBrowseForFile;
	tools_export b32 fBrowseForFile( std::string& pathOut, wxWindow* parent, const char* caption, const char* defDir, const char* defFile, const char* filter, u32 style = wxFD_OPEN );
}}


#endif//__WxUtil__
