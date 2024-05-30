#ifndef __tMersenneGenerator__
#define __tMersenneGenerator__
#include "tRandom.hpp"

namespace Sig
{
	// Based upon the Mersenne Twister algorithm derived from: http://www.qbrundage.com/michaelb/pubs/essays/random_number_generation.html

	class base_export tMersenneGenerator
	{
	public:
		explicit tMersenneGenerator( u32 seed );		
		tMersenneGenerator( );		

		static const f32 cLimit;

		inline u32 fIndex( ) const { return mIndex; }

		inline f32 fFloatZeroToOne( ) const
		{
			return ( f32 )fGenerateRandomMersenne( ) / cLimit;
		}

		inline f32 fFloatMinusOneToOne( ) const
		{
			return fFloatZeroToOne( ) * 2.f - 1.f;
		}

		inline Math::tVec3f fRandomVec3( ) const
		{
			return Math::tVec3f( fFloatMinusOneToOne( ), fFloatMinusOneToOne( ), fFloatMinusOneToOne( ) );
		}

		inline Math::tVec3f fNormalizedVec3( ) const
		{
			return fRandomVec3( ).fNormalize( );
		}

		inline f32 fFloatInRange( f32 min, f32 max ) const
		{
			return min + fFloatZeroToOne( ) * ( max - min );
		}

		inline void fAddRandomFloatsInRange( f32& value, f32 min, f32 max ) const
		{
			value += fFloatInRange( min, max );
		}
		inline void fAddRandomFloatsInRange( Math::tVec2f& value, f32 min, f32 max ) const
		{
			value.x += fFloatInRange( min, max );
			value.y += fFloatInRange( min, max );
		}
		inline void fAddRandomFloatsInRange( Math::tVec3f& value, f32 min, f32 max ) const
		{
			value.x += fFloatInRange( min, max );
			value.y += fFloatInRange( min, max );
			value.z += fFloatInRange( min, max );
		}
		inline void fAddRandomFloatsInRange( Math::tVec4f& value, f32 min, f32 max ) const
		{
			value.x += fFloatInRange( min, max );
			value.y += fFloatInRange( min, max );
			value.z += fFloatInRange( min, max );
			value.w += fFloatInRange( min, max );
		}

	private:

		u32 fGenerateRandomMersenne( ) const;
		mutable u32 mIndex;
	};

}


#endif	// __tMersenneGenerator__

