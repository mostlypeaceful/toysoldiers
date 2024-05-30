#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tRect__
#define __tRect__

namespace Sig { namespace Math
{
	template<class t>
	struct tRectTemplate
	{
		static tRectTemplate<t> fConstruct( const Math::tVector2<t>& topLeft, const Math::tVector2<t>& widthHeight ) 
			{ return tRectTemplate<t>( topLeft, widthHeight ); }
		static tRectTemplate<t> fConstructEx( t x, t y, t width, t height ) 
			{ return tRectTemplate<t>( y, x, y + height, x + width ); }

		tRectTemplate( ) 
			{ fZeroOut( *this ); }
		tRectTemplate( t top, t left, t bottom, t right ) 
			: mT(top), mL(left), mB(bottom), mR(right) { }
		tRectTemplate( const Math::tVector2<t>& widthHeight ) 
			: mT(0), mL(0), mB(widthHeight.y), mR(widthHeight.x) { }
		tRectTemplate( const Math::tVector2<t>& topLeft, const Math::tVector2<t>& widthHeight ) 
			: mT(topLeft.y), mL(topLeft.x), mB(topLeft.y + widthHeight.y), mR(topLeft.x + widthHeight.x) { }
		void fInvalidate( )
			{ mB = mR = 0; mT = mL = 1; }
		void fInflate( t amount )
			{ mT -= amount; mB += amount; mL -= amount; mR += amount; }
		b32 fIsValid( ) const
			{ return mB >= mT && mR >= mL; }
		b32 fHasArea( ) const
			{ return ( fHeight( ) > 0.f && fWidth( ) > 0.f ); }
		Math::tVector2<t> fTopLeft( ) const 
			{ return Math::tVector2<t>( mL, mT ); }
		Math::tVector2<t> fTopRight( ) const 
			{ return Math::tVector2<t>( mR, mT ); }
		Math::tVector2<t> fBottomRight( ) const 
			{ return Math::tVector2<t>( mR, mB ); }
		Math::tVector2<t> fBottomLeft( ) const 
			{ return Math::tVector2<t>( mL, mB ); }
		Math::tVector2<t> fWidthHeight( ) const 
			{ return Math::tVector2<t>( fWidth( ), fHeight( ) ); }
		Math::tVec2f fBarycentric( f32 x, f32 y ) const
			{ return Math::tVec2f( mL + x * fWidth( ), mT + y * fHeight( ) ); }
		Math::tVec2f fCenter( ) const 
			{ return fBarycentric( 0.5f, 0.5f ); }
		Math::tVector2<t> fClampInside( const Math::tVector2<t>& point ) const
			{ return Math::tVector2<t>( fClamp( point.x, mL, mR ), fClamp( point.y, mT, mB ) ); }
		Math::tVec2f fClampToEdge( const Math::tVec2f& point ) const
			{
				Math::tVec2f halfSize = fWidthHeight( ) * 0.5f;
				Math::tVec2f center = fCenter( );
				Math::tVec2f direction = point - center;
				direction /= halfSize;

				f32 current;
				u32 max = direction.fMaxAxisMagnitudeIndex( current );
				sigassert( current != 0.f );
				f32 delta = 1.0f / current;

				direction *= delta * halfSize;
				return center + direction;
			}
		t fWidth( ) const
			{ return mR - mL; }
		t fHeight( ) const
			{ return mB - mT; }
		f32 fAspectRatio( ) const
			{ return fWidth( ) / (f32)fHeight( ); }
		b32 fContains( const tVector2<t>& pt ) const
			{ return fInBounds( pt.x, mL, mR ) && fInBounds( pt.y, mT, mB ); }
		inline tVector4<t> fConvertToLTRB( ) const { return tVector4<t>( mL, mT, mR, mB ); }

		inline tRectTemplate& operator |= ( const tVector2<t>& pt )
		{
			mT = fMin( pt.y, mT );
			mB = fMax( pt.y, mB );
			mL = fMin( pt.x, mL );
			mR = fMax( pt.x, mR );

			return *this;
		}

		inline void operator |= ( const tRectTemplate& o )
		{
			*this |= o.fTopLeft( );
			*this |= o.fBottomRight( );
		}

		tRectTemplate& operator *= ( const t& s )
		{
			mT *= s;
			mB *= s;
			mL *= s;
			mR *= s;
			return *this;
		}

		tRectTemplate& operator += ( const tVector2<t>& shift )
		{
			mT += shift.y;
			mB += shift.y;
			mL += shift.x;
			mR += shift.x;
			return *this;
		}

		inline b32 fIntersects( const tRectTemplate& other ) const
		{
			return !(other.mL > mR || other.mR < mL || other.mB < mT || other.mT > mB);
		}

		t mT, mL;
		t mB, mR;
	};

	typedef tRectTemplate<f32> tRect;
	typedef tRectTemplate<f32> tRectf;
	typedef tRectTemplate<u32> tRectu;
}}

#endif//__tRect__

