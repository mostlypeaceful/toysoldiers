#include "BasePch.hpp"
#include "tBinaryFileBase.hpp"
#include "tResource.hpp"
#include "FileSystem.hpp"

namespace Sig
{
	// 20 bytes bcz of 4-byte vtable
	static_assert( sizeof( tBinaryFileBase ) == 20 );

	tBinaryFileBase::tBinaryFileBase( )
		: mFileType( Rtti::cInvalidClassId )
		, mVersion( 0 )
	{
		fZeroOut( mSignature );
	}

	tBinaryFileBase::tBinaryFileBase( tNoOpTag )
	{
	}

	void tBinaryFileBase::fCreateSignature( tSignature& sig, tPlatformId pid )
	{
		fZeroOut( sig );
		sig[0] = '!';
		sig[1] = 'S';
		sig[2] = 'i';
		sig[3] = 'g';
		sig[4] = 'B';
		sig[5] = '!';
		sig[6] = ( u8 )pid;// treat the platform id as a raw unsigned byte
		sig[7] = '\0';
	}

	b32 tBinaryFileBase::fVerifyResource( 
		const tResource& resource, 
		tFilePathPtr (*fConvertToSource)( const tFilePathPtr& path ), 
		u32 minVersion,
		u32 maxVersion,
		Rtti::tClassId classId )
	{
		tSignature expectedSig;
		fCreateSignature( expectedSig, cCurrentPlatform );

		// Signature
		if( fMemCmp( &mSignature, &expectedSig, sizeof( mSignature ) ) != 0 )
		{
			log_warning( 
				"Corrupt file [" << resource.fGetPath( ) << "], try re-converting the file using AssetGen." );
			return false;
		}

		// File Type
		if( mFileType != classId )
		{
			log_warning(  
				"The file [" << resource.fGetPath( ) << "] of type " << std::hex << mFileType << 
				" is not of expected type " << std::hex << classId << "." );

			return false;
		}

		// Version
		if( mVersion < minVersion || mVersion > maxVersion )
		{
			log_warning( "[" << resource.fGetPath( ) << "] failed to load because the version[" << mVersion << "] is not in the expected range [" << minVersion << ".." << maxVersion << "]. Try re-converting the file using AssetGen." );
			return false;
		}

#ifdef target_tools
		const tFilePathPtr fileInRes = fConvertToSource( resource.fGetPath( ) );
		if( !fileInRes.fNull( ) )
		{
			if( !FileSystem::fFileExists( ToolsPaths::fMakeResAbsolute( fileInRes ) ) )
			{
				// TODO LESS HACK TOWN, when multiple source types generate the same output type.
				// if we're not a .nut, or we are and couldnt find a .goaml, show error.
				std::string extension = StringUtil::fGetExtension( fileInRes.fCStr( ) );
				if( extension != ".nut" || !FileSystem::fFileExists( ToolsPaths::fMakeResAbsolute( tFilePathPtr( StringUtil::fStripExtension( fileInRes.fCStr( ) ) + ".goaml" ) ) ) )
				{
					log_warning( 
						"The file [" << fileInRes << "] is no longer present - this is usually a result of renaming/moving files without updating all references to that file." );
					return false;
				}
			}
		}
#endif//target_tools

		return true;
	}

#ifdef target_tools
	void tBinaryFileBase::fSetSignature( tPlatformId pid, Rtti::tClassId cid, u32 version )
	{
		fCreateSignature( mSignature, pid );

		mFileType = cid;
		mVersion = version;
	}
#endif//target_tools


}
