//------------------------------------------------------------------------------
// \file tSaveUI.hpp - 10 Aug 2010
// \author Josh Wittner
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tSaveUI__
#define __tSaveUI__

#include "tScriptedControl.hpp"
#include "tSaveGameStorage.hpp"

namespace Sig { namespace Gui {

	//------------------------------------------------------------------------------
	// \class tSaveUI
	// \brief Manages async saves to disk along with scripted UI for the save
	//------------------------------------------------------------------------------
	class tSaveUI : public tScriptedControl 
	{
	public:
		class tSaveInstance : public tRefCounter
		{	
		public:
			b32 mStarted;
			b32 mErrored;

			tSaveInstance( ) : mStarted( false ), mErrored( false ) { }
			virtual ~tSaveInstance( ) { }

			// return true if you want the little saving icon to show up
			virtual b32 fWantsUI( ) const { return true; }

			virtual void fBegin( ) { mStarted = true; }
			virtual b32 fFinished( ) { return true; }
			virtual void fFinish( b32 wait )
			{
				if( wait )
				{
					while( !fFinished( ) ) 
						fSleep( 1 );
				}
			}

			virtual void fPostErrors( ) { }
		};

		typedef tRefCounterPtr<tSaveInstance> tSaveInstancePtr;

		class tGameStorageSaveInstance : public tSaveInstance
		{	
		private:
			tSaveGameStorageDesc mStorageDesc;
			tDynamicArray<byte> mData;
			tSaveGameStorageWriter mWriter;

		protected:
			tUserPtr mUser;

		public:
			tGameStorageSaveInstance( const tUserPtr& user, const tSaveGameStorageDesc& storageDesc, tGrowableArray<byte>& data )
				: mUser( user )
				, mStorageDesc( storageDesc )
			{
				data.fDisown( mData );
			}

			~tGameStorageSaveInstance( )
			{
				mData.fDeleteArray( );
			}

			virtual void fBegin( );
			virtual b32 fFinished( );
		};

		enum tState
		{
			cStateLoading,
			cStateSaving,
		};

	public:

		explicit tSaveUI( const tResourcePtr& scriptResource );
		virtual ~tSaveUI( );

		tState fState( ) const { return mState; }
		b32 fIsLoadComplete( ) const { return tScriptedControl::fScriptLoadCompleted( ); }
		b32 fIsFinishedSaving( ) const { return	!mSaveInstances.fNumItems( ) && mLifetime <= 0; }

		b32 fBeginSaving( );
		void fOnTick( float dt );
		void fWait( b32 processRemaining );

		void fAddSave( const tSaveInstancePtr& save );

		static void fExportScriptInterface( tScriptVm& vm );

	private:
		void fClear( b32 wait = false );
		void fShowUI( );
		
	private:
		float mLifetime;
		tState mState;
		tRingBuffer<tSaveInstancePtr> mSaveInstances;

		b32 mUIShown;
	};

	typedef tRefCounterPtr<tSaveUI> tSaveUIPtr;
}}

#endif //__tSaveUI__