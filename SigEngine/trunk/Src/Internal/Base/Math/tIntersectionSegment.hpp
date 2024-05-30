//------------------------------------------------------------------------------
// \file tIntersectionSegment.hpp - 24 Jul 2013
// \author mrickert
//
// Copyright Signal Studios 2013, All Rights Reserved
//------------------------------------------------------------------------------

#ifndef __tIntersectionSegment__
#define __tIntersectionSegment__

namespace Sig { namespace Math
{
	/// \class tIntersectionSegment
	/// \brief Represents a potential intersection.
	template< class t >
	class tIntersectionSegment
	{
		t		mMinT;
		t		mMaxT;
		b32		mIntersects;
	public:
		tIntersectionSegment( )
			: mMinT( Math::cInvalidFloat )
			, mMaxT( Math::cInvalidFloat )
			, mIntersects( false )
		{
		}

		tIntersectionSegment( t min, t max )
			: mMinT( min )
			, mMaxT( max )
			, mIntersects( true )
		{
			sigassert( min <= max );
		}

		static const tIntersectionSegment	cEmpty;
		static const tIntersectionSegment	cInfinite;

		b32		fIntersects( )			const { return mIntersects; }
		t		fMinT( )				const { return mMinT; }
		t		fMaxT( )				const { return mMaxT; }
		b32		fContainsT( t value )	const { return mIntersects && fInBounds( value, mMinT, mMaxT ); }
	};

	template< class t > const tIntersectionSegment<t> tIntersectionSegment<t>::cEmpty;
	template< class t > const tIntersectionSegment<t> tIntersectionSegment<t>::cInfinite = tIntersectionSegment<t>( (t)-Math::cInfinity, (t)+Math::cInfinity );

	/// \brief Find the intersection covered by BOTH lhs AND rhs
	template< class t >
	tIntersectionSegment<t> operator&( const tIntersectionSegment<t>& lhs, const tIntersectionSegment<t>& rhs )
	{
		if( lhs.fIntersects( ) && rhs.fIntersects( ) )
		{
			const t min = fMax( lhs.fMinT( ), rhs.fMinT( ) );
			const t max = fMin( lhs.fMaxT( ), rhs.fMaxT( ) );
			if( min <= max )
				return tIntersectionSegment<t>( min, max );
		}

		return tIntersectionSegment<t>::cEmpty;
	}

	/// \brief Find the intersection covered by EITHER lhs OR rhs
	template< class t >
	tIntersectionSegment<t> operator|( const tIntersectionSegment<t>& lhs, const tIntersectionSegment<t>& rhs )
	{
		if( lhs.fIntersects( ) && rhs.fIntersects( ) )
		{
			log_assert( (lhs & rhs).fIntersects( ), "lhs | rhs resulted in a disjoint intersection!" );
			return tIntersectionSegment<t>( fMin( lhs.fMinT(), rhs.fMinT() ), fMax( lhs.fMaxT(), rhs.fMaxT() ) );
		}
		else if( lhs.fIntersects( ) )
			return lhs;
		else if( rhs.fIntersects( ) )
			return rhs;
		else
			return tIntersectionSegment<t>::cEmpty;
	}
}}

#endif // ndef __tIntersectionSegment__
