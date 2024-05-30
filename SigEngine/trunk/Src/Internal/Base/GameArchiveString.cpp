#include "BasePch.hpp"
#include "GameArchiveString.hpp"

namespace Sig
{
	void fGameArchiveStandardSave( tGameArchiveSave& archive, std::string& s )
	{
		u32 count = s.length( );
		archive.fSaveLoad( count );
		archive.fPut( s.c_str( ), count );
	}
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, std::string& s )
	{
		u32 count = 0;
		archive.fSaveLoad( count );
		s = std::string( archive.fCurrentPos( ), archive.fCurrentPos( ) + count );
		archive.fAdvance( count );
	}
	void fGameArchiveStandardSave( tGameArchiveSave& archive, std::wstring& s )
	{
		u32 count = s.length( );
		archive.fSaveLoad( count );
		archive.fPut( s.c_str( ), sizeof( wchar_t ) * count );
	}
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, std::wstring& s )
	{
		u32 count = 0;
		archive.fSaveLoad( count );
		s = std::wstring( ( wchar_t* )archive.fCurrentPos( ), ( wchar_t* )archive.fCurrentPos( ) + count );
		archive.fAdvance( sizeof( wchar_t ) * count );
	}
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tLocalizedString& s )
	{
		u32 count = s.fLength( );
		archive.fSaveLoad( count );
		archive.fPut( s.fCStr( ), sizeof( u16 ) * count );
	}
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tLocalizedString& s )
	{
		u32 count = 0;
		archive.fSaveLoad( count );
		s.fFromCStr( ( u16* )archive.fCurrentPos( ), count );
		archive.fAdvance( sizeof( u16 ) * count );
	}
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tFilePathPtr& s )
	{
		if( tFilePathArchiver* filePathArchiver = archive.fFilePathArchiver( ) )
		{
			filePathArchiver->fSave( archive, s );
		}
		else
		{
			u32 count = s.fLength( );
			archive.fSaveLoad( count );
			archive.fPut( s.fCStr( ), sizeof( char ) * count );
		}
	}
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tFilePathPtr& s )
	{
		if( tFilePathArchiver* filePathArchiver = archive.fFilePathArchiver( ) )
		{
			filePathArchiver->fLoad( archive, s );
		}
		else
		{
			u32 count = 0;
			archive.fSaveLoad( count );
			s = tFilePathPtr( std::string( 
					( char* )archive.fCurrentPos( ), 
					( char* )archive.fCurrentPos( ) + count ) );
			archive.fAdvance( sizeof( char ) * count );
		}

	}
	void fGameArchiveStandardSave( tGameArchiveSave& archive, tStringPtr& s )
	{
		u32 count = s.fLength( );
		archive.fSaveLoad( count );
		archive.fPut( s.fCStr( ), sizeof( char ) * count );
	}
	void fGameArchiveStandardLoad( tGameArchiveLoad& archive, tStringPtr& s )
	{
		u32 count = 0;
		archive.fSaveLoad( count );
		if( count )
		{
			s = tStringPtr( std::string( 
				( char* )archive.fCurrentPos( ), 
				( char* )archive.fCurrentPos( ) + count ) );
		}
		else
		{
			s = tStringPtr::cNullPtr;
		}

		archive.fAdvance( sizeof( char ) * count );
	}
}