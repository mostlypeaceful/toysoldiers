#include "ToolsPch.hpp"
#include "Locml.hpp"
#include "tLocalizationFile.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"

namespace Sig { namespace Locml
{
	const char* fGetFileExtension( )
	{
		return ".locml";
	}

	b32 fIsLocmlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	tFilePathPtr fLocmlPathToLocb( const tFilePathPtr& path )
	{
		return tFilePathPtr::fSwapExtension( path, tLocalizationFile::fGetFileExtension( ) );
	}

	///
	/// \section tStringEntry
	///


	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tStringEntry& o )
	{
		s.fAsAttribute( "name", o.mName );

		std::string tmp;
		if( s.fOut( ) )
			tmp = Win32Util::fWStringToMultiByte( o.mText );
		s( "Text", tmp );
		if( s.fIn( ) )
			o.mText = Win32Util::fMultiByteToWString( tmp );
	}

	///
	/// \section tPathEntry
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tPathEntry& o )
	{
		s.fAsAttribute( "name", o.mName );
		s( "Path", o.mPath );
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gLocmlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		if( o.mVersion != gLocmlVersion )
		{
			log_warning( "Locml file format is out of date -> Please re-export." );
			return;
		}

		s( "StringTable", o.mStringTable );
		s( "PathTable", o.mPathTable );
	}

	tFile::tFile( )
		: mVersion( gLocmlVersion )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Locml", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Locml file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Locml", *this ) )
		{
			log_warning( "Couldn't load Locml file [" << path << "]" );
			return false;
		}

		return true;
	}

}}

