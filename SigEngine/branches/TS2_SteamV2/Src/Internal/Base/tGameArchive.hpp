#ifndef __tGameArchive__
#define __tGameArchive__

#include "tEncryption.hpp"

namespace Sig
{
	///
	/// \class tGameArchive
	/// \brief 
	class tGameArchive
	{
	public:
		enum tMode { cModeSave, cModeLoad };
		tGameArchive( tMode mode ) : mMode( mode ), mFailed( false ) { }
		tMode fMode( ) const { return mMode; }
		template<class t>
		void fSaveLoad( t& object );

		b32 fFailed( ) const { return mFailed; }
		void fFail( ) { mFailed = true; }

	private:
		tMode mMode;
		b32 mFailed;
	};

	///
	/// \class tGameArchiveSave
	/// \brief 
	class tGameArchiveSave : public tGameArchive
	{
	public:
		tGameArchiveSave( );

		template<class t>
		void fSaveLoad( t& object );
		void fPut( const void* data, u32 numBytes );

		tGrowableArray<Sig::byte>& fBuffer( ) { return mBuffer; }
		const tGrowableArray<Sig::byte>& fBuffer( ) const { return mBuffer; }

		void fEncrypt( )
		{
			// encrypt buffer
			tGrowableArray<byte> encrypted;
			tEncryption::fEncrypt( mBuffer, encrypted );
			mBuffer.fSwap( encrypted );
		}

	private:
		tGrowableArray<Sig::byte> mBuffer;
	};

	///
	/// \class tGameArchiveLoad
	/// \brief 
	class tGameArchiveLoad : public tGameArchive
	{
	public:
		// This object will not take ownership of this data. if decryption is necessary, it will copy the data.
		tGameArchiveLoad( const Sig::byte* data, u32 dataLen );

		template<class t>
		void fSaveLoad( t& object );
		void fGet( void* data, u32 numBytes );

		void fResetReadPos( ) { mReadPos = 0; }
		const ::Sig::byte* fCurrentPos( ) const { return mData + mReadPos; }
		void fAdvance( u32 numBytes );

		// returns true if decryption was successful
		b32 fDecrypt( )
		{
			tGrowableArray<byte> encrypted;
			encrypted.fInsert( 0, mData, mDataLen );

			tEncryption::fDecrypt( encrypted, mDecryptedBuffer );
			mData = mDecryptedBuffer.fBegin( );
			mDataLen = mDecryptedBuffer.fCount( );

			return (mDecryptedBuffer.fCount( ) != 0);
		}

	private:
		u32			mReadPos;
		const byte* mData;
		u32			mDataLen;
		tGrowableArray<Sig::byte> mDecryptedBuffer;
	};

	template<class t>
	void tGameArchive::fSaveLoad( t& object )
	{
		switch( mMode )
		{
		case cModeSave: static_cast<tGameArchiveSave*>(this)->fSaveLoad( object ); break;
		case cModeLoad: static_cast<tGameArchiveLoad*>(this)->fSaveLoad( object ); break;
		}
	}
}

#include "GameArchiveUtils.hpp"

namespace Sig
{
	template<class t>
	void tGameArchiveSave::fSaveLoad( t& object )
	{
		tGameArchiveSaveLoad< tIsBuiltInType< t >::cIs >::fSave( *this, object );
	}
	template<class t>
	void tGameArchiveLoad::fSaveLoad( t& object )
	{
		tGameArchiveSaveLoad< tIsBuiltInType< t >::cIs >::fLoad( *this, object );
	}
}

namespace Sig
{
	class tEntity;

	///
	/// \class tEntitySaveData
	/// \brief 
	struct tEntitySaveData : public Rtti::tSerializableBaseClass,  public tRefCounter
	{
		declare_null_reflector( );
		implement_rtti_serializable_base_class( tEntitySaveData, 0x237B67A );
	public:
		tEntitySaveData( )
			: mDeleted( false )
		{
		}
		virtual void fSpawnSavedEntity( ) { }
		virtual void fRestoreSavedEntity( tEntity* entity ) const { }
		virtual void fSaveLoadDerived( tGameArchive& archive ) { }
		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mDeleted );
			fSaveLoadDerived( archive );
		}

		b8 mDeleted;
	};
}

#endif//__tGameArchive__
