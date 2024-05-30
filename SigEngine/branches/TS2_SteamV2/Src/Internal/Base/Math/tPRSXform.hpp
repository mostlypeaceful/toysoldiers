#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tPRSXform__
#define __tPRSXform__

namespace Sig { namespace Math
{
	template<class t>
	class tPRSXform // position, rotation, scale xform
	{
		declare_reflector( );

	public:
		static const tPRSXform cIdentity;
		static const tPRSXform cZeroXform;

	public:
		tVector3<t>		mP; t pad0;	// position
		tQuaternion<t>	mR;			// rotation
		tVector3<t>		mS; t pad1;	// scale

	public:
		
		inline tPRSXform( ) { }
		
		explicit tPRSXform( const tMatrix3<t>& xform )
			: mP( xform.fGetTranslation( ) )
			, mS( xform.fGetScale( ) )
			, mR( xform )
		{
			mR.fNormalize( );
		}

		inline tPRSXform( const tVector3<t>& p, const tQuaternion<t>& r, const tVector3<t>& s )
			: mP( p ), mR( r ), mS( s )
		{
		}

		inline void fToMatrix( tMatrix3<t>& xform ) const
		{
			mR.fToMatrix( xform );
			xform.fScaleLocal( mS );
			xform.fSetTranslation( mP );
		}

		inline tPRSXform operator*( const t& scale ) const
		{
			tPRSXform o;
			o.mP = mP * scale;
			o.mR = mR * scale;
			o.mS = mS * scale;
			return o;
		}

		inline tPRSXform operator+( const tPRSXform& other ) const
		{
			tPRSXform o;
			o.mP = mP + other.mP;
			o.mR = mR + other.mR;
			o.mS = mS + other.mS;
			return o;
		}

		inline tPRSXform& operator+=( const tPRSXform& other )
		{
			mP += other.mP;
			mR += other.mR;
			mS += other.mS;
			return *this;
		}

		inline tPRSXform operator-( const tPRSXform& other ) const
		{
			tPRSXform o;
			o.mP = mP - other.mP;
			o.mR = mR - other.mR;
			o.mS = mS - other.mS;
			return o;
		}

		inline tPRSXform& operator-=( const tPRSXform& other )
		{
			mP -= other.mP;
			mR -= other.mR;
			mS -= other.mS;
			return *this;
		}

		inline b32 fEqual( const tPRSXform & other, f32 epsilon=0.00001f )
		{
			if( !mP.fEqual( other.mP, epsilon ) )
				return false;

			if( !mR.fEqual( other.mR, epsilon ) )
				return false;

			if( !mS.fEqual( other.mS, epsilon ) )
				return false;

			return true;
		}

		inline void fBlendLerpR( const tPRSXform& other, f32 x )
		{
			if( !mR.fAntiPodal( other.mR ) )	mR = fLerp( mR,  other.mR, x );
			else								mR = fLerp( mR, -other.mR, x );
		}

		inline void fBlendLerp( const tPRSXform& other, f32 x )
		{
			mP = fLerp( mP, other.mP, x );
			fBlendLerpR( other, x );
			mS = fLerp( mS, other.mS, x );
		}

		inline void fBlendNLerpR( const tPRSXform& other, f32 x )
		{
			mR = fNLerpRotation( mR, other.mR, x );
		}

		inline void fBlendNLerp( const tPRSXform& other, f32 x )
		{
			mP = fLerp( mP, other.mP, x );
			fBlendNLerpR( other, x );
			mS = fLerp( mS, other.mS, x );
		}

		inline void fBlendAdditive( const tPRSXform& db )
		{
			mP += db.mP;
			mR += db.mR;
			mS += db.mS;
			mR.fNormalize( );
		}

		inline b32 fApplyAsRefFrameDelta( tMatrix3<t>& xformInOut, t scale ) const
		{
			const b32 dpIsZero = mP.fIsZero( );
			const b32 drIsZero = mR.fIsZero( );
			const b32 dsIsZero = mS.fIsZero( );
			if( dpIsZero && drIsZero && dsIsZero )
				return false;

			tPRSXform refFrame( xformInOut );
			if( !dpIsZero ) refFrame.mP += xformInOut.fXformVector( scale * mP );
			if( !drIsZero ) refFrame.mR *= ( tQuaternion<t>::cIdentity + mR ).fNormalize( );
			if( !dsIsZero ) refFrame.mS += scale * mS;
			refFrame.fToMatrix( xformInOut );
			return true;
		}
	};

	template<class t>
	const tPRSXform<t> tPRSXform<t>::cIdentity( tVector3<t>::cZeroVector, tQuaternion<t>::cIdentity, tVector3<t>::cOnesVector );

	template<class t>
	const tPRSXform<t> tPRSXform<t>::cZeroXform( tVector3<t>::cZeroVector, tQuaternion<t>::cZeroQuat, tVector3<t>::cZeroVector );

	typedef tPRSXform<f32> tPRSXformf;
	typedef tPRSXform<f64> tPRSXformd;

	
	template<class t>
	inline tMatrix3<t>::tMatrix3( const tPRSXform<t>& prsXform )
	{
		prsXform.mR.fToMatrix( *this );
		fScaleLocal( prsXform.mS );
		fSetTranslation( prsXform.mP );
	}

}}

#endif//__tPRSXform__
