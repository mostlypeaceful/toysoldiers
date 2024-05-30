#include "BasePch.hpp"
#include "tSoundBankMapping.hpp"
#include "tApplication.hpp"
#include "tDataTableFile.hpp"
#include "tResourceLoadList2.hpp"

namespace Sig { namespace Audio { namespace SoundBankMapping
{
	devvar( bool, Audio_AssertOnMapErrors, false );
	devvar( bool, Audio_IgnoreStartScreenMapErrors, false );

	namespace Detail
	{
		const tFilePathPtr cDataTable( "DataTables/SoundBankMapping.tab" );
		const tStringPtr cTableName( "SoundBankMapping" );
		const tStringPtr cTableColumnPath( "Path" );

		const tStringPtr cStartScreenBank( "StartScreen" );

		tResourcePtr mBankMappingTableResource;
		const tDataTable* mMappingTable = NULL;
	}

	//------------------------------------------------------------------------------
	void fSetup( tApplication& app, tResourceLoadList2* preLoad )
	{
		const tResourceId cMappingTableRid = tResourceId::fMake< tDataTableFile >( Detail::cDataTable );
		if( preLoad )
		{
			preLoad->fAdd( cMappingTableRid );
		}
		else
		{
			Detail::mBankMappingTableResource = app.fResourceDepot( )->fQuery( cMappingTableRid );
			Detail::mMappingTable = Detail::mBankMappingTableResource->fCast<tDataTableFile>( )->fFindTable( Detail::cTableName );
			sigassert( Detail::mMappingTable );
		}
	}

	//------------------------------------------------------------------------------
	void fReset( )
	{
		Detail::mMappingTable = NULL;
		Detail::mBankMappingTableResource.fRelease( );
	}

	//------------------------------------------------------------------------------
	tFilePathPtr fMap( const tStringPtr& id )
	{
		if( !Detail::mMappingTable )
		{
			if( Audio_IgnoreStartScreenMapErrors && id == Detail::cStartScreenBank )
				return tFilePathPtr( );

			if( Audio_AssertOnMapErrors )
				log_assert( 0, "Unable to map [" << id << "] to a sound bank: Soundbank mapping table not yet available!" );
			else
				log_warning( "Unable to map [" << id << "] to a sound bank: Soundbank mapping table not yet available!" );

			return tFilePathPtr( );
		}

		const tFilePathPtr path = Detail::mMappingTable->fFindByRowCol<tFilePathPtr>( id, Detail::cTableColumnPath );
		return path;
	}

}}}
