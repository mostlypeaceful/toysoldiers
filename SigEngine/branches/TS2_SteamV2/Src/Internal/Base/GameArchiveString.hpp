#ifndef __GameArchiveString__
#define __GameArchiveString__
#include "tGameArchive.hpp"
#include "tLocalizationFile.hpp"

namespace Sig
{
	void fGameArchiveStandardSave( tGameArchiveSave& archive, std::string& s );
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, std::string& s );
	void fGameArchiveStandardSave( tGameArchiveSave& archive, std::wstring& s );
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, std::wstring& s );
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tLocalizedString& s );
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tLocalizedString& s );
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tFilePathPtr& s );
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tFilePathPtr& s );
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tStringPtr& s );
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tStringPtr& s );
}

#endif//__GameArchiveString__
