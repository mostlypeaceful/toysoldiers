#include "BasePch.hpp"
#include "Audio/tSoundBank.hpp"
#include "Audio/tSoundBankMapping.hpp"
#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include "Audio/tSystem.hpp"

namespace Sig { namespace Audio
{
	namespace Detail
	{
		class tSoundBankImpl;
		void fOnBankLoadComplete( tSoundBankImpl* bank );
		void fOnBankUnloadComplete( tSoundBankImpl* bank );

		//------------------------------------------------------------------------------
		// tSoundBankImpl
		//------------------------------------------------------------------------------
		class tSoundBankImpl : public tSoundBank
		{
			const tFilePathPtr mPath;
			AkBankID mId;

		public:

			//------------------------------------------------------------------------------
			tSoundBankImpl( const tFilePathPtr& path )
				: mPath( path )
				, mId( AK_INVALID_BANK_ID )
			{
			}

			//------------------------------------------------------------------------------
			virtual const tFilePathPtr& fPath( ) const
			{
				return mPath;
			}

			//------------------------------------------------------------------------------
			void fLoad( )
			{
				AKRESULT akr = AK::SoundEngine::LoadBank( mPath.fCStr( ), AK_DEFAULT_POOL_ID, mId );
				if( akr != AK_Success )
				{
					log_warning( "Failed triggering load for bank[" << mPath << "] AKRESULT[" << akr << "]" );
				}
			}

			//------------------------------------------------------------------------------
			void fLoadAsync( )
			{
				log_line( 0, "AUDIO - Beginning Async Load of [" << mPath << "]" );
				AKRESULT akr = AK::SoundEngine::LoadBank( mPath.fCStr( ), &fOnLoadCompleteCallback, this, AK_DEFAULT_POOL_ID, mId );
				if( akr != AK_Success )
				{
					log_warning( "Failed triggering async load for bank[" << mPath << "] AKRESULT[" << akr << "]" );
				}
			}

			//------------------------------------------------------------------------------
			void fUnload( )
			{
				log_line( 0, "AUDIO - Beginning Async UNLOAD of [" << mPath << "]" );
				AKRESULT akr = AK::SoundEngine::UnloadBank( mId, &fOnUnloadCompleteCallback, this );
				if( akr != AK_Success )
				{
					log_warning( "Failed triggering async unload for bank[" << mPath << "] AKRESULT[" << akr << "]" );
				}
			}

		private:

			//------------------------------------------------------------------------------
			static void fOnLoadCompleteCallback( 
				AkUInt32 in_bankID, 
				AKRESULT in_eLoadResult, 
				AkMemPoolId in_memPoolId, 
				void *in_pCookie )
			{
				tSoundBankImpl* obj = (tSoundBankImpl*)in_pCookie;
				if( in_eLoadResult != AK_Success )
				{
					log_warning( "Failed to load bank[" << obj->mPath << "]. AKRESULT[" << in_eLoadResult << "]" );
				}
				fOnBankLoadComplete( obj );
			}

			//------------------------------------------------------------------------------
			static void fOnUnloadCompleteCallback( 
				AkUInt32 in_bankID, 
				AKRESULT in_eLoadResult, 
				AkMemPoolId in_memPoolId, 
				void *in_pCookie )
			{
				tSoundBankImpl* obj = (tSoundBankImpl*)in_pCookie;
				if( in_eLoadResult != AK_Success )
				{
					log_warning( "Failed to load bank[" << obj->mPath << "]. AKRESULT[" << in_eLoadResult << "]" );
				}
				fOnBankUnloadComplete( obj );
			}
		};
		typedef tRefCounterPtr< tSoundBankImpl > tSoundBankImplPtr;

		//------------------------------------------------------------------------------
		// tSoundBankLoader
		//------------------------------------------------------------------------------
		class tSoundBankLoader : public tUncopyable
		{
			declare_singleton( tSoundBankLoader );
			Threads::tCriticalSection mCritSec;
			typedef tGrowableArray< tSoundBankImplPtr > tSoundBankList;
			tSoundBankList mSoundBanks;
			tSoundBankList mLoadingBanks;
			tSoundBankList mUnloadingBanks;
			tSoundBankList mUnloadedBanks;
		public:

			//------------------------------------------------------------------------------
			tSoundBankPtr fLoad( const tFilePathPtr& path )
			{
				Threads::tMutex lock( mCritSec );

				if( path.fLength( ) == 0 )
					return tSoundBankPtr( );

				tSoundBankImplPtr bank = fFindByPath_UnSafe( path, mSoundBanks );
				sigassert( !fFindByPath_UnSafe( path, mLoadingBanks ) && "Sync loading of a file already Async loading is not supported currently." );
				sigassert( !fFindByPath_UnSafe( path, mUnloadingBanks ) && "Sync loading of a file already Async unloading is not supported currently." );
				if( bank.fNull( ) )
				{
					bank.fReset( NEW_TYPED( tSoundBankImpl )( path ) );
					mSoundBanks.fPushBack( bank );
					bank->fLoad( );
				}

				return bank;
			}

			//------------------------------------------------------------------------------
			tSoundBankPtr fLoadAsync( const tFilePathPtr& path )
			{
				Threads::tMutex lock( mCritSec );

				if( path.fLength( ) == 0 )
					return tSoundBankPtr( );

				tSoundBankImplPtr bank = fFindByPath_UnSafe( path, mSoundBanks );

				if( bank.fNull( ) )
					bank = fFindByPath_UnSafe( path, mLoadingBanks );

				if( bank.fNull( ) )
				{
					bank = fFindByPath_UnSafe( path, mUnloadingBanks );
					if( !bank.fNull( ) )
					{
						fMove_UnSafe( bank.fGetRawPtr( ), mUnloadingBanks, mLoadingBanks );
						bank->fLoadAsync( );
					}
				}

				if( bank.fNull( ) )
				{
					bank.fReset( NEW_TYPED( tSoundBankImpl )( path ) );
					mLoadingBanks.fPushBack( bank );
					bank->fLoadAsync( );
				}			

				return bank;
			}

			//------------------------------------------------------------------------------
			void fUpdate( )
			{
				Threads::tMutex lock( mCritSec );


				// Unload any banks that we hold the last reference to
				for( s32 i = mSoundBanks.fCount( ) - 1; i >= 0; --i )
				{
					if( mSoundBanks[ i ].fRefCount( ) == 1 )
					{
						mUnloadingBanks.fPushBack( mSoundBanks[ i ] );
						mSoundBanks[ i ]->fUnload( );
						mSoundBanks.fErase( i );
					}
				}

				mUnloadedBanks.fClear( );
			}

			//------------------------------------------------------------------------------
			b32 fAnyLoading( ) const
			{
				return mLoadingBanks.fCount( ) > 0;
			}

			//------------------------------------------------------------------------------
			void fOnLoadComplete( tSoundBankImpl* bank )
			{
				Threads::tMutex lock( mCritSec );

				log_line( 0, "AUDIO - Async Loading Completed. [" << bank->fPath( ) << "]" );
				fMove_UnSafe( bank, mLoadingBanks, mSoundBanks );
			}

			//------------------------------------------------------------------------------
			void fOnUnloadComplete( tSoundBankImpl* bank )
			{
				Threads::tMutex lock( mCritSec );

				log_line( 0, "AUDIO - Async UNLOADING Completed. [" << bank->fPath( ) << "]" );

				// If we destroy the tSoundBankImpl on this audio thread rather than the main thread, ~tSoundBankImpl
				// calling ~tFilePathPtr can do thread-unsafe removals from the global table on a non-main thread.  So
				// instead we defer destruction to the main thread by moving it to mUnloadedBanks, which the main
				// thread will clear.  Yes, I hit an assert for this.
				//		-- mrickert
				fMove_UnSafe( bank, mUnloadingBanks, mUnloadedBanks );
			}

		private:

			//------------------------------------------------------------------------------
			// NOT THREAD-SAFE, MUST LOCK BEFORE CALLING
			static void fMove_UnSafe( tSoundBankImpl* bank, tSoundBankList& from, tSoundBankList& to )
			{
				for( u32 i = 0; i < from.fCount( ); ++i )
				{
					if( from[ i ] == bank )
					{
						to.fPushBack( from[ i ] );
						from.fErase( i );
						break;
					}
				}
			}

			//------------------------------------------------------------------------------
			// NOT THREAD-SAFE, MUST LOCK BEFORE CALLING
			static tSoundBankImplPtr fFindByPath_UnSafe( const tFilePathPtr& path, const tSoundBankList& list )
			{
				for( u32 i = 0; i < list.fCount( ); ++i )
				{
					if( list[ i ]->fPath( ) == path )
						return list[ i ];
				}
				return tSoundBankImplPtr( );
			}
		};

		//------------------------------------------------------------------------------
		void fOnBankLoadComplete( tSoundBankImpl* bank )
		{
			tSoundBankLoader::fInstance( ).fOnLoadComplete( bank );
		}

		//------------------------------------------------------------------------------
		void fOnBankUnloadComplete( tSoundBankImpl* bank )
		{
			tSoundBankLoader::fInstance( ).fOnUnloadComplete( bank );
		}
	}

	//------------------------------------------------------------------------------
	// fLoadSoundBankById
	//------------------------------------------------------------------------------
	tSoundBankPtr fLoadSoundBankById( const tStringPtr& id )
	{
		const tFilePathPtr path = SoundBankMapping::fMap( id );
		return fLoadSoundBankByPath( path );
	}

	//------------------------------------------------------------------------------
	// fLoadSoundBankAsyncById
	//------------------------------------------------------------------------------
	tSoundBankPtr fLoadSoundBankAsyncById( const tStringPtr& id )
	{
		const tFilePathPtr path = SoundBankMapping::fMap( id );
		if( id.fLength( ) )
			log_line( Log::cFlagAudio, "Sound Bank Mapping [" << id << "] -> [" << path << "]" );
		return fLoadSoundBankAsyncByPath( path );
	}

	//------------------------------------------------------------------------------
	// fLoadSoundBankByPath
	//------------------------------------------------------------------------------
	tSoundBankPtr fLoadSoundBankByPath( const tFilePathPtr& path )
	{
		if( tSystem::fGlobalDisable( ) )
			return tSoundBankPtr( );

		return Detail::tSoundBankLoader::fInstance( ).fLoad( path );
	}

	//------------------------------------------------------------------------------
	// fLoadSoundBankAsyncByPath
	//------------------------------------------------------------------------------
	tSoundBankPtr fLoadSoundBankAsyncByPath( const tFilePathPtr& path )
	{
		if( tSystem::fGlobalDisable( ) )
			return tSoundBankPtr( );

		return Detail::tSoundBankLoader::fInstance( ).fLoadAsync( path );
	}

	//------------------------------------------------------------------------------
	// fTickSoundBanks
	//------------------------------------------------------------------------------
	void fUpdateBanks( )
	{
		profile_pix( "Audio::fUpdateBanks" );
		Detail::tSoundBankLoader::fInstance( ).fUpdate( );
	}

	//------------------------------------------------------------------------------
	// fAnyBanksLoading
	//------------------------------------------------------------------------------
	b32 fAnyBanksLoading( )
	{
		return Detail::tSoundBankLoader::fInstance( ).fAnyLoading( );
	}
}}
