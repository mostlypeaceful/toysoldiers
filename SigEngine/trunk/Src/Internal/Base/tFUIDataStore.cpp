#include "basePch.hpp"
#include "tFuiDataStore.hpp"

#include "Fui.hpp"

namespace Sig
{
	namespace
	{
		void fFuiDataStoreGlobalInstance( Fui::tFuiFuncParams& params )
		{
			tFuiDataStore::tStringKey name;
			params.fGet( name );

			tFuiDataStore& store = tFuiDataStore::fInstance( );
			store.fResetResponse( tFuiDataStore::cInstance );

			tFuiDataStore::tGlobalInstance* inst = store.fFindGlobalInstance( name );
			store.fSetResponseInstance( inst ? inst->mInstance : FUIRat::tInstance( ) );

			store.fFlushResponse( params.fFui( ) );
		}

		FUIRat::tInstance fGetProviderParam( Fui::tFuiFuncParams& params )
		{
			sigassert( FUIRat::tMemberBase::cInstanceParamCount == 2 ); //one for instance, one for provider.

			u32 instance = 0;
			params.fGet( instance );
			void* realInstance = (void*)(s32)instance;

			u32 provider = 0;
			params.fGet( provider );
			void* realProvider = (void*)(s32)provider;

			return FUIRat::tInstance( realInstance, reinterpret_cast<FUIRat::tClassBindingData*>( realProvider ) );
		}

		void fDataMemberRequest( u32 type, Fui::tFuiFuncParams& params )
		{
			tFuiDataStore& store = tFuiDataStore::fInstance( );
			store.fResetResponse( type );

			FUIRat::tInstance inst = fGetProviderParam( params );
			if( inst.mPtr )
			{
				sigassert( inst.mProvider );

				tFuiDataStore::tStringKey memberKey;
				params.fGet( memberKey );

				FUIRat::tMemberBase* member = inst.mProvider->fMember( memberKey );
				if( member )
				{
					FUIRat::tMemberBase::tRequest request( inst.mPtr, &params );
					member->fFlush( request );
				}
				else
					log_warning( "Member: '" << memberKey << "' does not exist on type: '" << inst.mProvider->fTypeName( ) << "'" );
			}
			else
				log_warning( "Null ptr!" );

			store.fFlushResponse( params.fFui( ) );
		}

		void fFuiDataStoreInstanceRequestString( Fui::tFuiFuncParams& params )
		{
			fDataMemberRequest( tFuiDataStore::cString, params );
		}

		void fFuiDataStoreInstanceRequestInt( Fui::tFuiFuncParams& params )
		{
			fDataMemberRequest( tFuiDataStore::cS32, params );
		}

		void fFuiDataStoreInstanceRequestF32( Fui::tFuiFuncParams& params )
		{
			fDataMemberRequest( tFuiDataStore::cF32, params );
		}

		void fFuiDataStoreInstanceRequestInstance( Fui::tFuiFuncParams& params )
		{
			fDataMemberRequest( tFuiDataStore::cInstance, params );
		}
	}

	tFuiDataStore::tFuiDataStore( )
		: mResponseType( cRequestTypeCount )
		, mSatisifed( false )
	{ }

	tFuiDataStore::~tFuiDataStore( ) 
	{ }

	void tFuiDataStore::fClearGlobalInstance( const tStringPtr& name )
	{
		const b32 found = mGlobalInstances.fFindAndErase( name );
		sigassert( found );
	}

	tFuiDataStore::tGlobalInstance* tFuiDataStore::fFindGlobalInstance( const tStringPtr& name )
	{
		return mGlobalInstances.fFind( name );
	}

	b32 tFuiDataStore::fSetResponseString( const std::string& str )
	{
		if( mResponseType != cString )
		{
			log_warning( "Request was not a string." );
			return false;
		}

		mResponseString = str;
		mSatisifed = true;
		return true;
	}

	b32 tFuiDataStore::fSetResponseString( const std::wstring& wstr )
	{
		// By design, our Flash code calls fFuiDataStoreInstanceRequestString ignorant of whether
		// the response will be a string or wstring. Prior to this point, mResponseType == cString
		// by default but we can (and should) now correct it to cWString so fFlushResponse will send
		// the correct value--mResponseWString--back to Flash.
		if( mResponseType != cString ) //( mResponseType != cWString )
		{
			log_warning( "Request was not a string." );
			return false;
		}
		mResponseType = cWString;

		mResponseWString = wstr;
		mSatisifed = true;
		return true;
	}

	b32 tFuiDataStore::fSetResponseS32( s32 val )
	{
		if( mResponseType != cS32 )
		{
			log_warning( "Request was not an Int." );
			return false;
		}

		mResponseInt = val;
		mSatisifed = true;
		return true;
	}

	b32 tFuiDataStore::fSetResponseF32( f32 val )
	{
		if( mResponseType != cF32 )
		{
			log_warning( "Request was not a f32." );
			return false;
		}

		mResponseFloat = val;
		mSatisifed = true;
		return true;
	}

	b32 tFuiDataStore::fSetResponseInstance( const FUIRat::tInstance& instance )
	{
		if( mResponseType != cInstance )
		{
			log_warning( "Request was not an Instance." );
			return false;
		}

		mResponseInstance = instance;
		mSatisifed = true;
		return true;
	}

	void tFuiDataStore::fExportFuiInterface( )
	{
		Fui::tFuiSystem::fInstance( ).fRegisterFunc( "tFuiDataStore.fGlobalInstance",		fFuiDataStoreGlobalInstance );
		Fui::tFuiSystem::fInstance( ).fRegisterFunc( "tFuiDataStore.fInstRequestString",	fFuiDataStoreInstanceRequestString );
		Fui::tFuiSystem::fInstance( ).fRegisterFunc( "tFuiDataStore.fInstRequestInt",		fFuiDataStoreInstanceRequestInt );
		Fui::tFuiSystem::fInstance( ).fRegisterFunc( "tFuiDataStore.fInstRequestFloat",		fFuiDataStoreInstanceRequestF32 );
		Fui::tFuiSystem::fInstance( ).fRegisterFunc( "tFuiDataStore.fInstRequestInstance",	fFuiDataStoreInstanceRequestInstance );
	}

	void tFuiDataStore::fRegisterProvider( const FUIRat::tClassBindingDataPtr& prov )
	{
		sigassert( !mProviders.fFind( prov->fTypeName( ) ) && "Provider with this name already registered!" );
		mProviders.fInsert( prov->fTypeName( ), FUIRat::tClassBindingDataPtr( prov ) );
	}

	void tFuiDataStore::fFlushResponse( const Fui::tFuiPtr& fui )
	{
		if( !fui->mFUIDataStorePaths.fCount( ) )
		{
			fui->mFUIDataStorePaths.fNewArray( cResponseVariablesCount );

			b32 found = fui->fGetPath( "dataStorePtr.stringResult", fui->mFUIDataStorePaths[ cResponseString ] );
			sigassert( found && "Couldn't find value in swf: dataStorePtr.stringResult" );

			found = fui->fGetPath( "dataStorePtr.stringResult", fui->mFUIDataStorePaths[ cResponseWString ] );
			sigassert( found && "Couldn't find value in swf: dataStorePtr.stringResult" );

			found = fui->fGetPath( "dataStorePtr.intResult", fui->mFUIDataStorePaths[ cResponseS32 ] );
			sigassert( found && "Couldn't find value in swf: dataStorePtr.intResult" );

			found = fui->fGetPath( "dataStorePtr.floatResult", fui->mFUIDataStorePaths[ cResponseF32 ] );
			sigassert( found && "Couldn't find value in swf: dataStorePtr.floatResult" );

			found = fui->fGetPath( "dataStorePtr.instanceResultPtr", fui->mFUIDataStorePaths[ cResponseInstancePtr ] );
			sigassert( found && "Couldn't find value in swf: dataStorePtr.instanceResultPtr" );

			found = fui->fGetPath( "dataStorePtr.instanceResultProvider", fui->mFUIDataStorePaths[ cResponseInstanceProvider ] );
			sigassert( found && "Couldn't find value in swf: dataStorePtr.instanceResultProvider" );		
		}

		if( mSatisifed )
		{
			switch( mResponseType )
			{
			case cString:
				fui->fSetString( fui->mFUIDataStorePaths[ cResponseString ], mResponseString );
				break;
			case cWString:
				fui->fSetWString( fui->mFUIDataStorePaths[ cResponseWString ], mResponseWString );
				break;
			case cS32:
				fui->fSetS32( fui->mFUIDataStorePaths[ cResponseS32 ], mResponseInt );
				break;
			case cF32:
				fui->fSetFloat( fui->mFUIDataStorePaths[ cResponseF32 ], mResponseFloat );
				break;
			case cInstance:
				fui->fSetS32( fui->mFUIDataStorePaths[ cResponseInstancePtr ], (s32)mResponseInstance.mPtr );
				fui->fSetS32( fui->mFUIDataStorePaths[ cResponseInstanceProvider ], (s32)(void*)mResponseInstance.mProvider );
				break;
			}
		}
	}

	void tFuiDataStore::fDumpRegisteredInstancesAndProviders( )
	{
		// Global Instances
		log_line( 0, " ~~~Fui Data Store~~~ " );
		log_line( 0, " ~~~Global Instances~~~ " );
		log_line( 0, "" );
		for( u32 i = 0; i < mGlobalInstances.fCount( ); ++i )
		{
			log_line( 0, "   - " << mGlobalInstances[ i ].mName << " provider = " <<  mGlobalInstances[ i ].mInstance.mProvider->fTypeName( ) );
		}
		log_line( 0, "" );
		log_line( 0, " ~~~Providers~~~ " );


		tHashTable< tStringPtr, FUIRat::tClassBindingDataPtr >::tConstIterator itr = mProviders.fBegin( );
		tHashTable< tStringPtr, FUIRat::tClassBindingDataPtr >::tConstIterator end = mProviders.fEnd( );
		for( ; itr != end; ++itr )
		{
			if( itr->fNullOrRemoved( ) ) continue;

			log_line( 0, "" );
			log_line( 0, "   - " << itr->mKey );
			const FUIRat::tClassBindingDataPtr& provider = itr->mValue;

			tHashTable< tStringPtr, FUIRat::tMemberBasePtr >::tConstIterator itr2 = provider->fMembers( ).fBegin( );
			tHashTable< tStringPtr, FUIRat::tMemberBasePtr >::tConstIterator end2 = provider->fMembers( ).fEnd( );
			for( ; itr2 != end2; ++itr2 )
			{
				if( itr2->fNullOrRemoved( ) ) continue;

				//if( itr2->mKey.fLength( ) )
					log_line( 0, "   --- " << itr2->mKey );
			}
		}
	}

}