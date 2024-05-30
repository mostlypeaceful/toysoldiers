#include "SigScriptPch.hpp"
#include "tLocmlConvertDictionary.hpp"

namespace Sig
{
	void tLocmlConvertDictionary::fExtractFromLocml( std::string lang, const Locml::tFile& loc )
	{
		if( mRecords.fGetItemCount() == 0 )
			mRecords.fSetCapacity( loc.mStringTable.fCount() );

		for( u32 i = 0; i < loc.mStringTable.fCount(); ++i )
		{
			const Locml::tStringEntry& entry = loc.mStringTable[i];
			fExtractRecord( lang, entry );
		}
	}

	void tLocmlConvertDictionary::fClear()
	{
		mRecords.fClear( 1 );
	}

	std::string tLocmlConvertDictionary::fGetReplacementText( std::string tag, std::string lang ) const
	{
		std::string ret( "" );

		tRecord* record = mRecords.fFind( tag );
		if( !record )
		{
			log_warning( "Missing record for: " << tag );
			return ret;
		}

		std::string* text = record->mTexts.fFind( lang );
		if( text )
			ret = *text;

		return ret;
	}

	b32 tLocmlConvertDictionary::fBuilt() const
	{
		return mRecords.fGetItemCount() > 0;
	}

	void tLocmlConvertDictionary::fPrint( )
	{
		tHashTable< std::string, tRecord >::tConstIteratorNoNullOrRemoved it( mRecords.fBegin( ), mRecords.fEnd( ) );
		for( ; it.fNotDone( ); ++it )
		{
			const tRecord& record = it->mValue;

			log_line( 0, "Tag: " << record.mTag );

			tHashTable< std::string, std::string >::tConstIteratorNoNullOrRemoved oth( record.mTexts.fBegin( ), record.mTexts.fEnd( ) );
			for( ; oth.fNotDone( ); ++oth )
			{
				wxString txt( oth->mValue.c_str() );
				log_line( 0, " Lang: " << oth->mKey << " - Text: " << txt );
			}
		}
	}

	void tLocmlConvertDictionary::fExtractRecord( std::string lang, const Locml::tStringEntry& entry )
	{
		tRecord* found = mRecords.fFind( entry.mName );
		if( found )
		{
			sigassert( !found->mTexts.fFind( lang ) );
			found->mTexts.fInsert( lang, Win32Util::fWStringToMultiByte( entry.mText ) );
		}
		else
		{
			tRecord newRec;
			newRec.mTag = entry.mName;
			newRec.mTexts.fInsert( lang, Win32Util::fWStringToMultiByte( entry.mText ) );
			mRecords.fInsert( entry.mName, newRec );
		}
	}
}
