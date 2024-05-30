#include "ToolsPch.hpp"
#include "Anifig.hpp"
#include "tFilePackageFile.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"
#include "tFilePackageCreator.hpp"

namespace Sig { namespace Anifig
{
	const char* fGetFileExtension( )
	{
		return ".anifig";
	}

	b32 fIsAnifigFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer & s, tObject & o )
	{
		s( "SgFile", o.mSgFile ); 
		s( "SkelFile", o.mSkelFile );
		s( "WorldXform", o.mWorldXform );
	}

	///
	/// \section tFile
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s( "Objects", o.mObjects );
	}

	tFile::tFile( )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Anifig", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Anifig file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Anifig", *this ) )
		{
			log_warning( "Couldn't load Anifig file [" << path << "]" );
			return false;
		}

		return true;
	}
}}

