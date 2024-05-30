#include "ToolsPch.hpp"
#include "tLocmlConverter.hpp"
#include "tFileWriter.hpp"
#include "tLoadInPlaceSerializer.hpp"

namespace Sig
{
	b32 tLocmlConverter::fLoadLocmlFile( const tFilePathPtr& path )
	{
		mLocmlPath = path;
		return mLocml.fLoadXml( path );
	}

	b32 tLocmlConverter::fConvertPlatformCommon( )
	{
		// Set up duplicates checking, process strings.
		tHashTable< std::string, int > viewedStrings( mLocml.mStringTable.fCount() );
		tGrowableArray< std::string > duplicateStrings;

		mRawStrings.fNewArray( mLocml.mStringTable.fCount( ) );
		for( u32 i = 0; i < mRawStrings.fCount( ); ++i )
		{
			// Test for duplicate keys.
			const std::string entryName = mLocml.mStringTable[ i ].mName;
			if( viewedStrings.fFind( entryName ) )
				duplicateStrings.fPushBack( entryName );
			else
				viewedStrings.fInsert( entryName, 0 );

			mRawStrings[ i ].mId = fAddLoadInPlaceStringPtr( entryName.c_str( ) );
			mRawStrings[ i ].mText = tLocalizedString( mLocml.mStringTable[ i ].mText );
		}


		// Reset viewed strings, process paths.
		viewedStrings.fClear( mLocml.mStringTable.fCount() );
		tGrowableArray< std::string > duplicatePaths;

		mRawPaths.fNewArray( mLocml.mPathTable.fCount( ) );
		for( u32 i = 0; i < mRawPaths.fCount( ); ++i )
		{
			// Test for duplicate keys.
			const std::string entryName = mLocml.mPathTable[ i ].mName;
			if( viewedStrings.fFind( entryName ) )
				duplicatePaths.fPushBack( entryName );
			else
				viewedStrings.fInsert( entryName, 0 );

			mRawPaths[ i ].mId = fAddLoadInPlaceStringPtr( mLocml.mPathTable[ i ].mName.c_str( ) );
			mRawPaths[ i ].mPath = fAddLoadInPlaceResourceId( tResourceId::fMake( 0, mLocml.mPathTable[ i ].mPath ) );
		}

		// Dump any duplicate keys discovered.
		if( duplicateStrings.fCount() > 0 || duplicatePaths.fCount() > 0 )
		{
			for( u32 i = 0; i < duplicateStrings.fCount(); ++i )
				log_warning( "Duplicate string key: [" + duplicateStrings[i] + "] in file: " + mLocmlPath.fCStr() );

			for( u32 i = 0; i < duplicatePaths.fCount(); ++i )
				log_warning( "Duplicate path key: [" + duplicatePaths[i] + "] in file: " + mLocmlPath.fCStr() );

			return false;
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
		this->fSetSignature<tLocalizationFile>( pid );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tLocalizationFile&>( *this ), ofile, pid );
	}

}

