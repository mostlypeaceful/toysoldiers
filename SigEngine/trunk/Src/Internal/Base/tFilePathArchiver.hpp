#ifndef __tFilePathArchiver__
#define __tFilePathArchiver__

namespace Sig
{
	/// \class tFilePathArchiver
	/// \brief Base class for custom tFilePathPtr save/load
	class base_export tFilePathArchiver
	{
	public:
		virtual ~tFilePathArchiver( ) { }
		virtual void fSave( tGameArchiveSave& archive, tFilePathPtr& s ) = 0;
		virtual void fLoad( tGameArchiveLoad& archive, tFilePathPtr& s ) = 0;
	};
}

#endif //ndef __tFilePathArchiver__
