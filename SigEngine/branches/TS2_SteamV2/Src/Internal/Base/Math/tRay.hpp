#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tRay__
#define __tRay__

namespace Sig { namespace Math
{

	template<class t>
	class tRay
	{
	public:
		tVector3<t> mOrigin;
		tVector3<t> mExtent;

	public:
		inline tRay( ) { }
		inline tRay( const tVector3<t>& o, const tVector3<t>& e )
			: mOrigin( o ), mExtent( e ) { sig_assertvecvalid( o ); sig_assertvecvalid( e ); }

		inline tVector3<t> fEvaluate( f32 x ) const { return mOrigin + x * mExtent; }

		inline tRay fTransform( const Math::tMatrix3<t>& xform ) const
		{
			return tRay( xform.fXformPoint( mOrigin ), xform.fXformVector( mExtent ) );
		}

	};

	b32 fClosestPoint( const tRay<f32>& L, const tRay<f32>& R, f32& tOnLOut, f32& tOnROut );

	typedef tRay<f32> tRayf;
	typedef tRay<f64> tRayd;

	///
	/// \brief Output/result structure for a raycast query
	class base_export tRayCastHit
	{
	public:
		tVec3f				mN;
		f32					mT;

		inline tRayCastHit( ) : mT( Math::cInfinity ), mN( Math::tVec3f::cYAxis ) { }
		inline tRayCastHit( f32 t, const Math::tVec3f& n ) : mN( n ), mT( t ) { sig_assertvecvalid( mN ); }

		inline b32 fHit( ) const { return fInBounds( mT, 0.f, 1.f ); }
	};

}}


#endif//__tRay__
