#include "ToolsPch.hpp"
#include "tLocmlConverter.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{
	b32 tLocmlConverter::fLoadLocmlFile( const tFilePathPtr& path )
	{
		return mLocml.fLoadXml( path );
	}

	b32 tLocmlConverter::fConvertPlatformCommon( )
	{
		mRawStrings.fNewArray( mLocml.mStringTable.fCount( ) );
		for( u32 i = 0; i < mRawStrings.fCount( ); ++i )
		{
			mRawStrings[ i ].mId = fAddLoadInPlaceStringPtr( mLocml.mStringTable[ i ].mName.c_str( ) );
			mRawStrings[ i ].mText = tLocalizedString( mLocml.mStringTable[ i ].mText );
		}

		mRawPaths.fNewArray( mLocml.mPathTable.fCount( ) );
		for( u32 i = 0; i < mRawPaths.fCount( ); ++i )
		{
			mRawPaths[ i ].mId = fAddLoadInPlaceStringPtr( mLocml.mPathTable[ i ].mName.c_str( ) );
			mRawPaths[ i ].mPath = fAddLoadInPlaceResourceId( tResourceId::fMake( 0, mLocml.mPathTable[ i ].mPath, false ) );
		}

		return true;
	}

	b32 tLocmlConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	void tLocmlConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		tBinaryFileBase::fSetSignature( pid, Rtti::fGetClassId<tLocalizationFile>( ), tLocalizationFile::cVersion );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tLocalizationFile&>( *this ), ofile, pid );
	}

}

