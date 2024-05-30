#ifndef __tSaveGameStorage__
#define __tSaveGameStorage__

#include "tUser.hpp"
#include "tLocalizationFile.hpp"
#include "Threads/tThread.hpp"

namespace Sig
{
	struct base_export tSaveGameStorageDesc
	{
		tSaveGameStorageDesc( );
		tSaveGameStorageDesc( u32 deviceId, const tStringPtr& fileName, const tLocalizedString& displayName = tLocalizedString( ) );

		u32					mDeviceId;
		tStringPtr			mFileName;
		tLocalizedString	mDisplayName;
	};

	class base_export tSaveGameStorageEnumerator
	{
	public:

		enum tState
		{
			cStateIdle,
			cStateEnumerating
		};

		tSaveGameStorageEnumerator( );
		void fBeginEnumeration( const tUserPtr& user, u32 deviceId, u32 numSaveFiles );
		b32 fIsFinished( ) const { return mState == cStateIdle; }

		u32 fSaveFileCount( ) const { return mSaveFiles.fCount( ); }
		const tSaveGameStorageDesc& fSaveFileDesc( u32 index );
		void fAddSaveFile( const tSaveGameStorageDesc& desc ) { mSaveFiles.fPushBack( desc ); }

		const tUserPtr&	fUser( ) const { return mUser; }
		u32				fDeviceId( ) const { return mDeviceId; }
		u32				fMaxSaveFiles( ) const { return mMaxSaveFiles; }

	private:
		void fSetFinished( ) { mState = cStateIdle; }
		static Threads::tThreadReturn thread_call fThreadMain( void* threadParam );

		tState	mState;
		Threads::tThread mThread;
		tGrowableArray<tSaveGameStorageDesc> mSaveFiles;		
		tUserPtr mUser;
		u32	mDeviceId;
		u32 mMaxSaveFiles;
	};

	class base_export tSaveGameStorage
	{
	public:
		enum tMode
		{
			cModeRead,
			cModeWrite
		};

		enum tState
		{
			cStateIdle,
			cStateReadingWriting,
			cStateError
		};

		enum tErrors
		{
			cNoError,
			cGeneralError,
			cMissingData,
			cCorruptData,
		};

	public:
		virtual ~tSaveGameStorage( );

		tMode fMode( ) const { return mMode; }
		
		b32 fIsReadingOrWriting( ) const;
		b32 fIsFinished( ) const;
		b32 fIsErrored( ) const { return mState == cStateError; }
		u32 fErrorCode( ) const { return mErrorCode; }

		const tUserPtr&			fUser( ) const { return mUser; }
		const tSaveGameStorageDesc& fSaveGameDesc( ) const { return mSaveDesc; }

	protected:

		tSaveGameStorage( tMode mode );
		void fSetup( const tUserPtr& user, const tSaveGameStorageDesc& desc );
		static Threads::tThreadReturn thread_call fThreadMain( void* threadParam );

		void fSetFinished( ) { mState = cStateIdle; }
		void fSetErrored( tErrors code ) { mState = cStateError; mErrorCode = code; }

		virtual void fWriteFromBuffer( ) { }
		virtual void fReadToBuffer( ) { }

		tFilePathPtr fSaveGameFilePath( ) const;

	protected:

		tMode	mMode;
		tState	mState;
		tErrors mErrorCode;
		Threads::tThread mThread;
		tDynamicArray<byte> mBuffer;

		tUserPtr mUser;
		tSaveGameStorageDesc mSaveDesc;
	};

	class base_export tSaveGameStorageWriter : public tSaveGameStorage
	{
	public:
		tSaveGameStorageWriter( );
		void fGiveBuffer( tDynamicArray<byte> & buffer );
		void fBeginWriting( const tUserPtr& user, const tSaveGameStorageDesc& desc );
		
	protected:
		virtual void fWriteFromBuffer( );
	};

	class base_export tSaveGameStorageReader : public tSaveGameStorage
	{
	public:
		tSaveGameStorageReader( );

		void fGetBuffer( tDynamicArray<byte> & buffer );
		void fBeginReading( const tUserPtr& user, const tSaveGameStorageDesc& desc );

	protected:
		virtual void fReadToBuffer( );
	};
}

#endif//__tSaveGameStorage__
