#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tPRSXform__
#define __tPRSXform__

namespace Sig { namespace Math
{
	/// \brief Position, Rotation, & Scale Transform
	template<class t>
	class tPRSXform
	{
		declare_reflector( );

	public:
		static const tPRSXform cIdentity;
		static const tPRSXform cZeroXform;

	public:
		tVector3<t>		mPosition;
		t				pad0;	///< Pointless waste of space? We're dealing with floats and doubles here, why pad to SSE boundaries when we don't have SSE alignment anyways?  Sadly, we serialize this type...
		tQuaternion<t>	mRotation;
		tVector3<t>		mScale;
		t				pad1;	///< Pointless waste of space? We're dealing with floats and doubles here, why pad to SSE boundaries when we don't have SSE alignment anyways?  Sadly, we serialize this type...

	public:
		
		inline tPRSXform( ) { }
		
		explicit tPRSXform( const tMatrix3<t>& xform )
		{
			fFromMatrix( xform );
		}

		inline tPRSXform( const tVector3<t>& p, const tQuaternion<t>& r, const tVector3<t>& s )
			: mPosition( p ), mRotation( r ), mScale( s )
		{
		}

		inline void fToMatrix( tMatrix3<t>& xform ) const
		{
			mRotation.fToMatrix( xform );
			xform.fScaleLocal( mScale );
			xform.fSetTranslation( mPosition );
		}

		inline void fFromMatrix( const tMatrix3<t> & xform )
		{
			mPosition = xform.fGetTranslation( );
			mScale = xform.fGetScale( );
			mRotation.fFromMatrix( xform );
			mRotation.fNormalize( );
		}

		inline tPRSXform& operator*=( t scale )
		{
			mPosition *= scale;
			mRotation *= scale;
			mScale *= scale;
			return *this;
		}

		inline tPRSXform& operator+=( const tPRSXform& other )
		{
			mPosition += other.mPosition;
			mRotation += other.mRotation;
			mScale += other.mScale;
			return *this;
		}

		inline tPRSXform& operator-=( const tPRSXform& other )
		{
			mPosition -= other.mPosition;
			mRotation -= other.mRotation;
			mScale -= other.mScale;
			return *this;
		}

		inline b32 fEqual( const tPRSXform & other, f32 epsilon=0.00001f )
		{
			if( !mPosition.fEqual( other.mPosition, epsilon ) )
				return false;

			if( !mRotation.fEqual( other.mRotation, epsilon ) )
				return false;

			if( !mScale.fEqual( other.mScale, epsilon ) )
				return false;

			return true;
		}

		inline void fBlendLerpR( const tPRSXform& other, f32 x )
		{
			if( !mRotation.fAntiPodal( other.mRotation ) )	mRotation = fLerp( mRotation,  other.mRotation, x );
			else								mRotation = fLerp( mRotation, -other.mRotation, x );
		}

		inline void fBlendLerp( const tPRSXform& other, f32 x )
		{
			mPosition = fLerp( mPosition, other.mPosition, x );
			fBlendLerpR( other, x );
			mScale = fLerp( mScale, other.mScale, x );
		}

		inline void fBlendNLerpR( const tPRSXform& other, f32 x )
		{
			//mR = fNLerpRotation( mR, other.mR, x );
			mRotation = fSlerp( mRotation, other.mRotation, x );
		}

		inline void fBlendNLerp( const tPRSXform& other, f32 x )
		{
			mPosition = fLerp( mPosition, other.mPosition, x );
			fBlendNLerpR( other, x );
			mScale = fLerp( mScale, other.mScale, x );
		}

		inline void fBlendAdditive( const tPRSXform& db )
		{
			mPosition += db.mPosition;
			mRotation += db.mRotation;
			mScale += db.mScale;
			mRotation.fNormalize( );
		}

		inline b32 fApplyAsRefFrameDelta( tMatrix3<t>& xformInOut, t scale ) const
		{
			const b32 dpIsZero = mPosition.fIsZero( );
			const b32 drIsZero = mRotation.fIsZero( );
			const b32 dsIsZero = mScale.fIsZero( );
			if( dpIsZero && drIsZero && dsIsZero )
				return false;

			tPRSXform refFrame( xformInOut );
			if( !dpIsZero ) refFrame.mPosition += xformInOut.fXformVector( scale * mPosition );
			if( !drIsZero ) refFrame.mRotation *= ( tQuaternion<t>::cIdentity + mRotation ).fNormalize( );
			if( !dsIsZero ) refFrame.mScale += scale * mScale;
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
		prsXform.fToMatrix( *this );
	}

	template<class t>
	inline tPRSXform<t> operator*( const tPRSXform<t>& xform, t scale )
	{
		tPRSXform<t> copy = xform;
		copy *= scale;
		return copy;
	}

	template<class t>
	inline tPRSXform<t> operator*( t scale, const tPRSXform<t>& xform )
	{
		tPRSXform<t> copy = xform;
		copy *= scale;
		return copy;
	}
	
	template<class t>
	inline tPRSXform<t> operator+( const tPRSXform<t>& lhs, const tPRSXform<t>& rhs )
	{
		tPRSXform<t> copy = lhs;
		copy += rhs;
		return copy;
	}

	template<class t>
	inline tPRSXform<t> operator-( const tPRSXform<t>& lhs, const tPRSXform<t>& rhs )
	{
		tPRSXform<t> copy = lhs;
		copy -= rhs;
		return copy;
	}

}}

#endif//__tPRSXform__
