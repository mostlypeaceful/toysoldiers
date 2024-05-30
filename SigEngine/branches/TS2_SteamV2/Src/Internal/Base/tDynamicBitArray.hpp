#ifndef __tDynamicBitArray__
#define __tDynamicBitArray__

namespace Sig
{
	///
	/// Implements a dynamic size bit array, with the minimal number of storage units
	/// such that there are enough bits for the desired number. Both the number of
	/// bits required and the desired underlying storage unit are parameterized.
	template<class tStorage=u32>
	class tDynamicBitArray
	{
		static const u32 cNumBitsPerStorageUnit = sizeof(tStorage) * 8;
		tDynamicArray<tStorage>	mBits;

	public:
		tDynamicBitArray( ) { }

		inline void fNewArray( u32 numBits, b32 setAllBitsToThis=false )
		{
			const u32 actualNumBits = cNumBitsPerStorageUnit*(1+((numBits-1)/cNumBitsPerStorageUnit));
			const u32 numStorageUnits = actualNumBits / cNumBitsPerStorageUnit;
			mBits.fNewArray( numStorageUnits );
			fSetAll( setAllBitsToThis );
		}
		
		inline void fSetAll( b32 setAllBitsToThis )
		{
			const u32 totalBytes = mBits.fCount( ) * sizeof( tStorage );
			fMemSet( mBits.fBegin( ), setAllBitsToThis ? 0xff : 0x00, totalBytes );
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

		inline tDynamicBitArray operator~( ) const
		{
			tDynamicBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = ~mBits[i];
			return o;
		}

		inline tDynamicBitArray operator&( const tDynamicBitArray& other ) const
		{
			tDynamicBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = mBits[i] & other.mBits[i];
			return o;
		}

		inline tDynamicBitArray& operator&=( const tDynamicBitArray& other )
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				mBits[i] &= other.mBits[i];
			return *this;
		}

		inline tDynamicBitArray operator|( const tDynamicBitArray& other ) const
		{
			tDynamicBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = mBits[i] | other.mBits[i];
			return o;
		}

		inline tDynamicBitArray& operator|=( const tDynamicBitArray& other )
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				mBits[i] |= other.mBits[i];
			return *this;
		}

		inline tDynamicBitArray operator^( const tDynamicBitArray& other ) const
		{
			tDynamicBitArray o;
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				o.mBits[i] = mBits[i] ^ other.mBits[i];
			return o;
		}

		inline tDynamicBitArray& operator^=( const tDynamicBitArray& other )
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				mBits[i] ^= other.mBits[i];
			return *this;
		}

		inline b32 operator==( const tDynamicBitArray& other ) const
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				if( mBits[i] != other.mBits[i] )
					return false;
			return true;
		}

		inline b32 operator!=( const tDynamicBitArray& other ) const
		{
			for( u32 i = 0; i < mBits.fCount( ); ++i )
				if( mBits[i] != other.mBits[i] )
					return true;
			return false;
		}

	};

}//Sig


#endif//__tDynamicBitArray__
