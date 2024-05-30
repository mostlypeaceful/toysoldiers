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
		declare_reflector( );
	public:

		static const u32 cNumBits				= cNumBitsTemplate;

	private:

		static const u32 cNumBitsPerStorageUnit = sizeof(tStorage) * 8;
		static const u32 cActualNumBits			= cNumBitsPerStorageUnit*(1+((cNumBits-1)/cNumBitsPerStorageUnit));
		static const u32 cNumStorageUnits		= cActualNumBits / cNumBitsPerStorageUnit;
		static const tStorage cLastBitsMask		= ( tStorage( 1 ) << ( cNumBits % cNumBitsPerStorageUnit ) ) - 1;
		tFixedArray<tStorage,cNumStorageUnits>	mBits;

	public:
		inline explicit tFixedBitArray( b32 setAllBitsToThis=false ) { fSetAll( setAllBitsToThis ); }
		inline tFixedBitArray( tNoOpTag ) {}
		
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

		inline b32 fAny( ) const
		{
			for( s32 i = mBits.fCount( ) - 2; i >= 0; --i )
			{
				if( mBits[ i ] )
					return true;
			}

			if( cLastBitsMask )
				return ( mBits.fBack( ) & cLastBitsMask ) ? true : false;
			else
				return mBits.fBack( ) ? true : false;
		}

		inline b32 fAll( ) const
		{
			for( s32 i = mBits.fCount( ) - 2; i >= 0; --i )
			{
				if( mBits[ i ] != ~0 )
					return false;
			}

			if( cLastBitsMask )
				return ( ( mBits.fBack( ) & cLastBitsMask ) == cLastBitsMask ) ? true : false;
			else
				return ( mBits.fBack( ) == ~0 ) ? true : false;
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
			// For all, but the last bit container we can straight compare
			for( s32 i = mBits.fCount( ) - 2; i >= 0 ; --i )
			{
				if( mBits[i] != other.mBits[i] )
					return false;
			}

			if( cLastBitsMask )
				return ( mBits.fBack( ) & cLastBitsMask ) == ( other.mBits.fBack( ) * cLastBitsMask );
			else
				return mBits.fBack( ) == other.mBits.fBack( );
		}

		inline b32 operator!=( const tFixedBitArray& other ) const
		{
			// For all but the last bit container we can straight compare
			for( s32 i = mBits.fCount( ) - 2; i >= 0 ; --i )
			{
				if( mBits[i] != other.mBits[i] )
					return true;
			}

			if( cLastBitsMask )
				return ( mBits.fBack( ) & cLastBitsMask ) != ( other.mBits.fBack( ) * cLastBitsMask );
			else
				return mBits.fBack( ) != other.mBits.fBack( );
		}

	};


	typedef tFixedBitArray<8,u8>	tFlags8;
	typedef tFixedBitArray<16,u16>	tFlags16;
	typedef tFixedBitArray<32,u32>	tFlags32;
	typedef tFixedBitArray<64,u64>	tFlags64;


}


#endif//__tFixedBitArray__

