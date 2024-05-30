#ifndef __GameArchiveString__
#define __GameArchiveString__
#include "tGameArchive.hpp"
#include "tLocalizationFile.hpp"

namespace Sig
{
	base_export void fGameArchiveStandardSave( tGameArchiveSave& archive, std::string& s );
	base_export void fGameArchiveStandardLoad( tGameArchiveLoad& archive, std::string& s );
	base_export void fGameArchiveStandardSave( tGameArchiveSave& archive, std::wstring& s );
	base_export void fGameArchiveStandardLoad( tGameArchiveLoad& archive, std::wstring& s );
	base_export void fGameArchiveStandardSave( tGameArchiveSave& archive, tLocalizedString& s );
	base_export void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tLocalizedString& s );
	base_export void fGameArchiveStandardSave( tGameArchiveSave& archive, tFilePathPtr& s );
	base_export void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tFilePathPtr& s );
	base_export void fGameArchiveStandardSave( tGameArchiveSave& archive, tStringPtr& s );
	base_export void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tStringPtr& s );
}

#endif//__GameArchiveString__
