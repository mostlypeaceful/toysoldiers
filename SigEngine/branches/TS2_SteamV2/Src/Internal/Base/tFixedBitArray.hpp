#ifndef __tFixedBitArray__
#define __tFixedBitArray__

namespace Sig
{
	///
	/// Implements a fixed size bit array, with the minimal number of storage units
	/// such that there are enough bits for the desired number. Both the number of
	/// bits required and the desired underlying storage unit are parameterized.
	template<u32 cNumBitsTemplate, class tStorage=u32>
	class tFixedBitArray
	{
	public:

		static const u32 cNumBits				= cNumBitsTemplate;

	private:

		static const u32 cNumBitsPerStorageUnit = sizeof(tStorage) * 8;
		static const u32 cActualNumBits			= cNumBitsPerStorageUnit*(1+((cNumBits-1)/cNumBitsPerStorageUnit));
		static const u32 cNumStorageUnits		= cActualNumBits / cNumBitsPerStorageUnit;
		tFixedArray<tStorage,cNumStorageUnits>	mBits;

	public:

		inline explicit tFixedBitArray( b32 setAllBitsToThis=false ) { fSetAll( setAllBitsToThis ); }
		
		inline void fSetAll( b32 setAllBitsToThis )
		{
			fMemSet( mBits, setAllBitsToThis ? 0xff : 0x00 );
		}

		inline void fTurnBitOn( u32 bitNum )
		{
			mBits[ bitNum / cNumBitsPerStorageUnit ] |=  ( 1 << ( bitNum & (cNumBitsPerStorageUnit-1) ) );
		}

		inline void fTurnBitOff( u32 bitNum )
		{
			mBits[ bitNum / cNumBitsPerStorageUnit ] &= ~( 1 << ( bitNum & (cNumBitsPerStorageUnit-1) ) );
		}

		inline void fSetBit( u32 bitNum, b32 onOff )
		{
			onOff ? fTurnBitOn( bitNum ) : fTurnBitOff( bitNum );
		}

		inline b32 fGetBit( u32 bitNum ) const
		{
			return ( mBits[ bitNum / cNumBitsPerStorageUnit ] & ( 1 << ( bitNum & (cNumBitsPerStorageUnit-1) ) ) ) ? true : false;
		}

		inline tFixedBitArray operator~( ) const
		{
			tFixedBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = ~mBits[i];
			return o;
		}

		inline tFixedBitArray operator&( const tFixedBitArray& other ) const
		{
			tFixedBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = mBits[i] & other.mBits[i];
			return o;
		}

		inline tFixedBitArray& operator&=( const tFixedBitArray& other )
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				mBits[i] &= other.mBits[i];
			return *this;
		}

		inline tFixedBitArray operator|( const tFixedBitArray& other ) const
		{
			tFixedBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = mBits[i] | other.mBits[i];
			return o;
		}

		inline tFixedBitArray& operator|=( const tFixedBitArray& other )
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				mBits[i] |= other.mBits[i];
			return *this;
		}

		inline tFixedBitArray operator^( const tFixedBitArray& other ) const
		{
			tFixedBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = mBits[i] ^ other.mBits[i];
			return o;
		}

		inline tFixedBitArray& operator^=( const tFixedBitArray& other )
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				mBits[i] ^= other.mBits[i];
			return *this;
		}

		inline b32 operator==( const tFixedBitArray& other ) const
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				if( mBits[i] != other.mBits[i] )
					return false;
			return true;
		}

		inline b32 operator!=( const tFixedBitArray& other ) const
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				if( mBits[i] != other.mBits[i] )
					return true;
			return false;
		}

		inline u32 fCount( ) const
		{
			return cNumBitsTemplate;
		}

		inline b32 fAnyBitOn( ) const
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
			{
				if( mBits[ i ] )
					return true;
			}
			return false;
		}

		template<class tArchive>
		void fSaveLoad( tArchive & archive )
		{
			archive.fSaveLoad( mBits );
		}
	};


	typedef tFixedBitArray<8,u8>	tFlags8;
	typedef tFixedBitArray<16,u16>	tFlags16;
	typedef tFixedBitArray<32,u32>	tFlags32;
	typedef tFixedBitArray<64,u64>	tFlags64;


}


#endif//__tFixedBitArray__

