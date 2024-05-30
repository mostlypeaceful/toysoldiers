#ifndef __Math__
#error Include Math.hpp instead
#endif//__Math__
#ifndef __tMatrix__
#define __tMatrix__

namespace Sig { namespace Math
{
	template<class t>
	class tPRSXform;

	template<class t>
	class tMatrix4;

	///
	/// \brief Represents a simplified 4x4 matrix with a tacit last row equaling 0,0,0,1;
	/// this matrix is capable of representing most transformations (i.e., rotation/scale/translation), 
	/// but cannot represent projections. Basically just a class that is both memory and performance
	/// optimized for the last row assumption.
	template<class t>
	class tMatrix3 : public tBaseVector<t, 12, tMatrix3<t> >
	{
		friend class tMatrix4<t>;
		declare_reflector( );
		define_class_pool_new_delete( tMatrix3, 256 );
		sig_make_loggable( tMatrix3, "(" << mRow0 << ", " << mRow1 << ", " << mRow2 << ")" );
	public:
		typedef tBaseVector<t, 12, tMatrix3<t> > tBase;

	public:

		tVector4<t> mRow0;
		tVector4<t> mRow1;
		tVector4<t> mRow2;

		inline tVector4<t>&			fRow( u32 i )		{ sigassert( i < 3 ); return (&mRow0)[i]; }
		inline const tVector4<t>&	fRow( u32 i ) const { sigassert( i < 3 ); return (&mRow0)[i]; }

	public:

		static const tMatrix3 cIdentity;

		inline tMatrix3( ) { sig_setvecinvalid( t, mRow0.x, tBase::cDimension ); }

		inline explicit tMatrix3( tNoOpTag ) 
			: mRow0( cNoOpTag )
			, mRow1( cNoOpTag )
			, mRow2( cNoOpTag )
		{ }

		inline explicit tMatrix3( MatrixUtil::tIdentityTag )
		{
			tMatrix3& m = *this;
			fMemSet( m, 0 );
			m(0,0)=m(1,1)=m(2,2)=1.f;
		}

		inline explicit tMatrix3( const t& scale )
		{
			tMatrix3& m = *this;
			fMemSet( m, 0 );
			m(0,0)=m(1,1)=m(2,2)=scale;
		}

		inline explicit tMatrix3( const tVector3<t>& scale )
		{
			tMatrix3& m = *this;
			fMemSet( m, 0 );
			m(0,0) = scale.x;
			m(1,1) = scale.y;
			m(2,2) = scale.z;
		}

		inline explicit tMatrix3( const tQuaternion<t>& rotation, const tVector3<t>& pos = tVector3<t>::cZeroVector )
		{
			rotation.fToMatrix( *this );
			fSetTranslation( pos );
		}

		inline explicit tMatrix3( const tPRSXform<t>& prsXform );

		inline tMatrix3( 
			t m00, t m01, t m02, t m03,
			t m10, t m11, t m12, t m13,
			t m20, t m21, t m22, t m23 ) 
		{
			tMatrix3& m = *this;
			m(0,0) = m00; m(0,1) = m01; m(0,2) = m02; m(0,3) = m03;
			m(1,0) = m10; m(1,1) = m11; m(1,2) = m12; m(1,3) = m13;
			m(2,0) = m20; m(2,1) = m21; m(2,2) = m22; m(2,3) = m23;
		}

		inline tMatrix3( 
			const tVector3<t>& x,
			const tVector3<t>& y,
			const tVector3<t>& z,
			const tVector3<t>& p )
		{
			fXAxis( x );
			fYAxis( y );
			fZAxis( z );
			fSetTranslation( p );
		}
		
		inline tVector4<t>&			operator[]( const u32 row )					{ return fRow( row ); }
		inline const tVector4<t>&	operator[]( const u32 row ) const			{ return fRow( row ); }

		inline t&			operator()( const u32 row, const u32 col )			{ return fRow( row ).fAxis( col ); }
		inline const t&		operator()( const u32 row, const u32 col ) const	{ return fRow( row ).fAxis( col ); }

		inline void			fSetRow( const u32 row, const tVector3<t>& v )		{ sig_assertvecvalid( v ); fRow( row ).x = v.x; fRow( row ).y = v.y; fRow( row ).z = v.z; }
		inline tVector3<t>	fGetRow( const u32 row ) const						{ return tVector3<t>( fRow( row ).x, fRow( row ).y, fRow( row ).z ); }
		inline void			fSetCol( const u32 col, const tVector3<t>& v )		{ sig_assertvecvalid( v ); mRow0.fAxis( col ) = v.x; mRow1.fAxis( col ) = v.y; mRow2.fAxis( col ) = v.z; }
		inline tVector3<t>	fGetCol( const u32 col ) const						{ return tVector3<t>( mRow0.fAxis( col ), mRow1.fAxis( col ), mRow2.fAxis( col ) ); }

		inline void			fXAxis( const tVector3<t>& x )						{ fSetCol( 0, x ); }
		inline tVector3<t>	fXAxis( ) const										{ return fGetCol( 0 ); }
		inline void			fYAxis( const tVector3<t>& y )						{ fSetCol( 1, y ); }
		inline tVector3<t>	fYAxis( ) const										{ return fGetCol( 1 ); }
		inline void			fZAxis( const tVector3<t>& z )						{ fSetCol( 2, z ); }
		inline tVector3<t>	fZAxis( ) const										{ return fGetCol( 2 ); }
		inline void			fSetTranslation( const tVector3<t>& p )				{ fSetCol( 3, p ); }
		inline tVector3<t>	fGetTranslation( ) const							{ return fGetCol( 3 ); }
		inline tVector3<t>	fGetScale( ) const									
			{ return tVector3<t>( fXAxis( ).fLength( ), fYAxis( ).fLength( ), fZAxis( ).fLength( ) ); }

		inline tMatrix3		operator*( const tMatrix3& mat ) const
		{
			tMatrix3 mo( cNoOpTag );

			mo.mRow0.x = mRow0.x * mat.mRow0.x + mRow0.y * mat.mRow1.x + mRow0.z * mat.mRow2.x;
			mo.mRow0.y = mRow0.x * mat.mRow0.y + mRow0.y * mat.mRow1.y + mRow0.z * mat.mRow2.y;
			mo.mRow0.z = mRow0.x * mat.mRow0.z + mRow0.y * mat.mRow1.z + mRow0.z * mat.mRow2.z;
			mo.mRow0.w = mRow0.x * mat.mRow0.w + mRow0.y * mat.mRow1.w + mRow0.z * mat.mRow2.w + mRow0.w;

			mo.mRow1.x = mRow1.x * mat.mRow0.x + mRow1.y * mat.mRow1.x + mRow1.z * mat.mRow2.x;
			mo.mRow1.y = mRow1.x * mat.mRow0.y + mRow1.y * mat.mRow1.y + mRow1.z * mat.mRow2.y;
			mo.mRow1.z = mRow1.x * mat.mRow0.z + mRow1.y * mat.mRow1.z + mRow1.z * mat.mRow2.z;
			mo.mRow1.w = mRow1.x * mat.mRow0.w + mRow1.y * mat.mRow1.w + mRow1.z * mat.mRow2.w + mRow1.w;

			mo.mRow2.x = mRow2.x * mat.mRow0.x + mRow2.y * mat.mRow1.x + mRow2.z * mat.mRow2.x;
			mo.mRow2.y = mRow2.x * mat.mRow0.y + mRow2.y * mat.mRow1.y + mRow2.z * mat.mRow2.y;
			mo.mRow2.z = mRow2.x * mat.mRow0.z + mRow2.y * mat.mRow1.z + mRow2.z * mat.mRow2.z;
			mo.mRow2.w = mRow2.x * mat.mRow0.w + mRow2.y * mat.mRow1.w + mRow2.z * mat.mRow2.w + mRow2.w;

			sig_assertvecvalid( mo );

			return mo;
		}

		inline tMatrix3		operator*=( const tMatrix3& mat ) { return ( *this = ( *this ) * mat ); }

		inline tVector3<t>	fXformVector( const tVector3<t>& v ) const
		{ 
			return tVector3<t>( 
				( mRow0.x * v.x + mRow0.y * v.y + mRow0.z * v.z ),
				( mRow1.x * v.x + mRow1.y * v.y + mRow1.z * v.z ),
				( mRow2.x * v.x + mRow2.y * v.y + mRow2.z * v.z ) );
		}

		inline tVector3<t>	fInverseXformVector( const tVector3<t>& v ) const
		{ 
			// This is the same as above, just uses the transposed rotation.
			return tVector3<t>( 
				( mRow0.x * v.x + mRow1.x * v.y + mRow2.x * v.z ),
				( mRow0.y * v.x + mRow1.y * v.y + mRow2.y * v.z ),
				( mRow0.z * v.x + mRow1.z * v.y + mRow2.z * v.z ) );
		}

		inline tVector3<t>	fXformPoint( const tVector3<t>& p ) const
		{ 
			return tVector3<t>( 
				( mRow0.x * p.x + mRow0.y * p.y + mRow0.z * p.z + mRow0.w ),
				( mRow1.x * p.x + mRow1.y * p.y + mRow1.z * p.z + mRow1.w ),
				( mRow2.x * p.x + mRow2.y * p.y + mRow2.z * p.z + mRow2.w ) );
		}

		inline tVector4<t>	fXform( const tVector4<t>& v ) const
		{
			return fXform( v.x, v.y, v.z, v.w );
		}

		inline tVector4<t>	fXform( t x, t y, t z, t w ) const
		{ 
			return tVector4<t>( 
				( mRow0.x * x + mRow0.y * y + mRow0.z * z + mRow0.w * w ),
				( mRow1.x * x + mRow1.y * y + mRow1.z * z + mRow1.w * w ),
				( mRow2.x * x + mRow2.y * y + mRow2.z * z + mRow2.w * w ), w );
		}

		inline void fTranslateLocal( const tVector3<t>& trans )
		{
			mRow0.w += mRow0.x * trans.x + mRow0.y * trans.y + mRow0.z * trans.z;
			mRow1.w += mRow1.x * trans.x + mRow1.y * trans.y + mRow1.z * trans.z;
			mRow2.w += mRow2.x * trans.x + mRow2.y * trans.y + mRow2.z * trans.z;
		}

		inline void fTranslateGlobal( const tVector3<t>& trans )
		{
			mRow0.w += trans.x;
			mRow1.w += trans.y;
			mRow2.w += trans.z;
		}

		inline void fSetDiagonal( const tVector3<t>& scale )
		{
			sig_assertvecvalid( scale );
			tMatrix3& m = *this;
			m(0,0) = scale.x;
			m(1,1) = scale.y;
			m(2,2) = scale.z;
		}

		inline void fScaleLocal( const tVector3<t>& scales )
		{
			sig_assertvecvalid( scales );
			fXAxis( fXAxis( ) * scales.x );
			fYAxis( fYAxis( ) * scales.y );
			fZAxis( fZAxis( ) * scales.z );
		}

		inline void fScaleGlobal( const tVector3<t>& scales )
		{
			sig_assertvecvalid( scales );
			tMatrix3& m = *this;
			m(0,0) *= scales.x;
			m(1,1) *= scales.y;
			m(2,2) *= scales.z;
		}

		inline void fNormalizeBasis( )
		{
			fScaleLocal( tVector3<t>( 1.f / fXAxis( ).fLength( ), 1.f / fYAxis( ).fLength( ), 1.f / fZAxis( ).fLength( ) ) );
			sig_assertvecvalid( *this );
		}

		t fDeterminant( ) const
		{
			const tMatrix3& m = *this;
			const t a = m(2,2);
			const t b = m(2,1);
			const t d = m(2,0);

			return (
				m(0,0)*( a*m(1,1) - b*m(1,2) ) -
				m(0,1)*( a*m(1,0) - d*m(1,2) ) +
				m(0,2)*( b*m(1,0) - d*m(1,1) )
					);
		}

		inline t fTrace( ) const
		{
			return mRow0.x + mRow1.y + mRow2.z + 1.f;
		}

		tMatrix3 fInverse( ) const;

		inline tMatrix3 fInverseNoScale( ) const
		{
			tVector3<t> trs = fGetTranslation( );
			tMatrix3<t> inv = fTransposeRotation( );
			inv.fSetTranslation( -inv.fXformVector( trs ) );
			return inv;
		}

		tMatrix3 fTransposeRotation( ) const
		{
			const tMatrix3& m = *this;
			return tMatrix3(
				m(0,0), m(1,0), m(2,0), m(0,3),
				m(0,1), m(1,1), m(2,1), m(1,3),
				m(0,2), m(1,2), m(2,2), m(2,3) );
		}

		void fOrientXAxis( const tVector3<t>& newAxis, const tVector3<t>& y = tVector3<t>::cYAxis )
		{
			sigassert( ::Sig::fEqual( newAxis.fLengthSquared( ), 1.f ) );
			fXAxis( newAxis );
			fZAxis( newAxis.fCross( y ).fNormalizeSafe( tVector3<t>::cZAxis ) );
			fYAxis( fZAxis( ).fCross( fXAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientXAxisZAxis( const tVector3<t>& newAxis, const tVector3<t>& z = tVector3<t>::cZAxis )
		{
			sigassert( ::Sig::fEqual( newAxis.fLengthSquared( ), 1.f ) );
			fXAxis( newAxis );
			fYAxis( z.fCross( fXAxis( ) ).fNormalizeSafe( tVector3<t>::cYAxis ) );
			fZAxis( newAxis.fCross( fYAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientYAxis( const tVector3<t>& newAxis, const tVector3<t>& x = tVector3<t>::cXAxis )
		{
			sigassert( ::Sig::fEqual( newAxis.fLengthSquared( ), 1.f ) );
			fYAxis( newAxis );
			fZAxis( x.fCross( newAxis ).fNormalizeSafe( tVector3<t>::cZAxis ) );
			fXAxis( fYAxis( ).fCross( fZAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientYWithZAxis( const tVector3<t>& newAxis, const tVector3<t>& z = tVector3<t>::cZAxis )
		{
			sigassert( ::Sig::fEqual( newAxis.fLengthSquared( ), 1.f ) );
			fYAxis( newAxis );
			fXAxis( fYAxis( ).fCross( z ).fNormalizeSafe( tVector3<t>::cXAxis ) );
			fZAxis( fXAxis( ).fCross( newAxis ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientZAxis( const tVector3<t>& newAxis, const tVector3<t>& y = tVector3<t>::cYAxis )
		{
			sigassert( ::Sig::fEqual( newAxis.fLengthSquared( ), 1.f ) );
			fZAxis( newAxis );
			fXAxis( y.fCross( newAxis ).fNormalizeSafe( tVector3<t>::cXAxis ) );
			fYAxis( fZAxis( ).fCross( fXAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientZWithXAxis( const tVector3<t>& newAxis, const tVector3<t>& x = tVector3<t>::cXAxis )
		{
			sigassert( ::Sig::fEqual( newAxis.fLengthSquared( ), 1.f ) );
			fZAxis( newAxis );
			fYAxis( newAxis.fCross( x ).fNormalizeSafe( tVector3<t>::cYAxis ) );
			fXAxis( fYAxis( ).fCross( newAxis ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		b32 fTurnToFaceZ( const tVector3<t>& faceThisDirection, t lerpFactor, t dt )
		{
			const t idealDt = 1.f/60.f;
			lerpFactor *= dt / idealDt;
			lerpFactor = Sig::fMin<t>( lerpFactor, (t)1.f );

			const tVector3<t> facing = fZAxis( ).fNormalizeSafe( tVector3<t>::cZAxis );
			const tVector3<t> newFacing = fLerp( facing, faceThisDirection, lerpFactor ).fNormalizeSafe( tVector3<t>::cZAxis );
			if( !newFacing.fEqual( facing, (t)0.001f ) )
			{
				fOrientZAxis( newFacing );
				return true;
			}

			return false;
		}
	};

	template<class t>
	const tMatrix3<t> tMatrix3<t>::cIdentity( MatrixUtil::cIdentityTag );

	///
	/// \brief Represents a full 4x4 matrix, capable of representing
	/// all transformations including projections (unlike the tMatrix3).
	template<class t>
	class tMatrix4 : public tBaseVector<t, 16, tMatrix4<t> >
	{
		declare_reflector( );
		sig_make_loggable( tMatrix4, "(" << mRow0 << ", " << mRow1 << ", " << mRow2 << ", " << mRow3 << ")" );
	public:
		typedef tBaseVector<t, 16, tMatrix4<t> > tBase;
	public:

		tVector4<t> mRow0;
		tVector4<t> mRow1;
		tVector4<t> mRow2;
		tVector4<t> mRow3;

		inline tVector4<t>&			fRow( u32 i )		{ sigassert( i < 4 ); return (&mRow0)[i]; }
		inline const tVector4<t>&	fRow( u32 i ) const { sigassert( i < 4 ); return (&mRow0)[i]; }

	public:

		static const tMatrix4 cIdentity;

		inline tMatrix4( ) { sig_setvecinvalid( t, mRow0.x, tBase::cDimension ); }

		inline explicit tMatrix4( tNoOpTag ) 
			: mRow0( cNoOpTag )
			, mRow1( cNoOpTag )
			, mRow2( cNoOpTag )
			, mRow3( cNoOpTag ) 
		{ }

		inline explicit tMatrix4( MatrixUtil::tIdentityTag )
		{
			tMatrix4& m = *this;
			fMemSet( m, 0 );
			m(0,0)=m(1,1)=m(2,2)=m(3,3)=1.f;
		}

		inline explicit tMatrix4( const t& scale )
		{
			tMatrix4& m = *this;
			fMemSet( m, 0 );
			m(0,0)=m(1,1)=m(2,2)=scale;
			m(3,3)=1.f;
		}

		inline tMatrix4( 
			t m00, t m01, t m02, t m03,
			t m10, t m11, t m12, t m13,
			t m20, t m21, t m22, t m23,
			t m30=0, t m31=0, t m32=0, t m33=1 ) 
		{
			tMatrix4& m = *this;
			m(0,0) = m00; m(0,1) = m01; m(0,2) = m02; m(0,3) = m03;
			m(1,0) = m10; m(1,1) = m11; m(1,2) = m12; m(1,3) = m13;
			m(2,0) = m20; m(2,1) = m21; m(2,2) = m22; m(2,3) = m23;
			m(3,0) = m30; m(3,1) = m31; m(3,2) = m32; m(3,3) = m33;
		}

		inline explicit tMatrix4( const tMatrix3<t>& m3 )
			: mRow0( m3.mRow0 )
			, mRow1( m3.mRow1 )
			, mRow2( m3.mRow2 )
			, mRow3( 0.f, 0.f, 0.f, 1.f )
		{
		}

		inline tVector4<t>&			operator[]( const u32 row )					{ return fRow( row ); }
		inline const tVector4<t>&	operator[]( const u32 row ) const			{ return fRow( row ); }

		inline t&			operator()( const u32 row, const u32 col )			{ return fRow( row ).fAxis( col ); }
		inline const t&		operator()( const u32 row, const u32 col ) const	{ return fRow( row ).fAxis( col ); }

		inline void			fSetRow( const u32 row, const tVector3<t>& v )		{ fRow( row ).x = v.x; fRow( row ).y = v.y; fRow( row ).z = v.z; }
		inline tVector3<t>	fGetRow( const u32 row ) const						{ return tVector3<t>( fRow( row ).x, fRow( row ).y, fRow( row ).z ); }
		inline void			fSetCol( const u32 col, const tVector3<t>& v )		{ mRow0.fAxis( col ) = v.x; mRow1.fAxis( col ) = v.y; mRow2.fAxis( col ) = v.z; }
		inline tVector3<t>	fGetCol( const u32 col ) const						{ return tVector3<t>( mRow0.fAxis( col ), mRow1.fAxis( col ), mRow2.fAxis( col ) ); }

		inline void			fXAxis( const tVector3<t>& x )						{ fSetCol( 0, x ); }
		inline tVector3<t>	fXAxis( ) const										{ return fGetCol( 0 ); }
		inline void			fYAxis( const tVector3<t>& y )						{ fSetCol( 1, y ); }
		inline tVector3<t>	fYAxis( ) const										{ return fGetCol( 1 ); }
		inline void			fZAxis( const tVector3<t>& z )						{ fSetCol( 2, z ); }
		inline tVector3<t>	fZAxis( ) const										{ return fGetCol( 2 ); }
		inline void			fSetTranslation( const tVector3<t>& p )				{ fSetCol( 3, p ); }
		inline tVector3<t>	fGetTranslation( ) const							{ return fGetCol( 3 ); }
		inline tVector3<t>	fGetScale( ) const									
			{ return tVector3<t>( fXAxis( ).fLength( ), fYAxis( ).fLength( ), fZAxis( ).fLength( ) ); }

		inline tMatrix4		operator*( const tMatrix4& mat ) const
		{
			tMatrix4 mo( cNoOpTag );

			mo.mRow0.x = mRow0.x * mat.mRow0.x + mRow0.y * mat.mRow1.x + mRow0.z * mat.mRow2.x + mRow0.w * mat.mRow3.x;
			mo.mRow0.y = mRow0.x * mat.mRow0.y + mRow0.y * mat.mRow1.y + mRow0.z * mat.mRow2.y + mRow0.w * mat.mRow3.y;
			mo.mRow0.z = mRow0.x * mat.mRow0.z + mRow0.y * mat.mRow1.z + mRow0.z * mat.mRow2.z + mRow0.w * mat.mRow3.z;
			mo.mRow0.w = mRow0.x * mat.mRow0.w + mRow0.y * mat.mRow1.w + mRow0.z * mat.mRow2.w + mRow0.w * mat.mRow3.w;

			mo.mRow1.x = mRow1.x * mat.mRow0.x + mRow1.y * mat.mRow1.x + mRow1.z * mat.mRow2.x + mRow1.w * mat.mRow3.x;
			mo.mRow1.y = mRow1.x * mat.mRow0.y + mRow1.y * mat.mRow1.y + mRow1.z * mat.mRow2.y + mRow1.w * mat.mRow3.y;
			mo.mRow1.z = mRow1.x * mat.mRow0.z + mRow1.y * mat.mRow1.z + mRow1.z * mat.mRow2.z + mRow1.w * mat.mRow3.z;
			mo.mRow1.w = mRow1.x * mat.mRow0.w + mRow1.y * mat.mRow1.w + mRow1.z * mat.mRow2.w + mRow1.w * mat.mRow3.w;

			mo.mRow2.x = mRow2.x * mat.mRow0.x + mRow2.y * mat.mRow1.x + mRow2.z * mat.mRow2.x + mRow2.w * mat.mRow3.x;
			mo.mRow2.y = mRow2.x * mat.mRow0.y + mRow2.y * mat.mRow1.y + mRow2.z * mat.mRow2.y + mRow2.w * mat.mRow3.y;
			mo.mRow2.z = mRow2.x * mat.mRow0.z + mRow2.y * mat.mRow1.z + mRow2.z * mat.mRow2.z + mRow2.w * mat.mRow3.z;
			mo.mRow2.w = mRow2.x * mat.mRow0.w + mRow2.y * mat.mRow1.w + mRow2.z * mat.mRow2.w + mRow2.w * mat.mRow3.w;

			mo.mRow3.x = mRow3.x * mat.mRow0.x + mRow3.y * mat.mRow1.x + mRow3.z * mat.mRow2.x + mRow3.w * mat.mRow3.x;
			mo.mRow3.y = mRow3.x * mat.mRow0.y + mRow3.y * mat.mRow1.y + mRow3.z * mat.mRow2.y + mRow3.w * mat.mRow3.y;
			mo.mRow3.z = mRow3.x * mat.mRow0.z + mRow3.y * mat.mRow1.z + mRow3.z * mat.mRow2.z + mRow3.w * mat.mRow3.z;
			mo.mRow3.w = mRow3.x * mat.mRow0.w + mRow3.y * mat.mRow1.w + mRow3.z * mat.mRow2.w + mRow3.w * mat.mRow3.w;

			return mo;
		}

		inline tMatrix4		operator*=( const tMatrix4& mat ) { return ( *this = ( *this ) * mat ); }

		inline tVector3<t>	fXformVector( const tVector3<t>& v ) const
		{ 
			return tVector3<t>( 
				( mRow0.x * v.x + mRow0.y * v.y + mRow0.z * v.z ),
				( mRow1.x * v.x + mRow1.y * v.y + mRow1.z * v.z ),
				( mRow2.x * v.x + mRow2.y * v.y + mRow2.z * v.z ) );
		}

		inline tVector3<t>	fXformPoint( const tVector3<t>& p ) const
		{ 
			return tVector3<t>( 
				( mRow0.x * p.x + mRow0.y * p.y + mRow0.z * p.z + mRow0.w ),
				( mRow1.x * p.x + mRow1.y * p.y + mRow1.z * p.z + mRow1.w ),
				( mRow2.x * p.x + mRow2.y * p.y + mRow2.z * p.z + mRow2.w ) );
		}

		inline tVector4<t>	fXform( const tVector4<t>& v ) const
		{
			return fXform( v.x, v.y, v.z, v.w );
		}

		inline tVector4<t>	fXform( t x, t y, t z, t w ) const
		{ 
			return tVector4<t>( 
				( mRow0.x * x + mRow0.y * y + mRow0.z * z + mRow0.w * w ),
				( mRow1.x * x + mRow1.y * y + mRow1.z * z + mRow1.w * w ),
				( mRow2.x * x + mRow2.y * y + mRow2.z * z + mRow2.w * w ),
				( mRow3.x * x + mRow3.y * y + mRow3.z * z + mRow3.w * w ) );
		}

		inline void fTranslateLocal( const tVector3<t>& trans )
		{
			mRow0.w += mRow0.x * trans.x + mRow0.y * trans.y + mRow0.z * trans.z;
			mRow1.w += mRow1.x * trans.x + mRow1.y * trans.y + mRow1.z * trans.z;
			mRow2.w += mRow2.x * trans.x + mRow2.y * trans.y + mRow2.z * trans.z;
		}

		inline void fTranslateGlobal( const tVector3<t>& trans )
		{
			mRow0.w += trans.x;
			mRow1.w += trans.y;
			mRow2.w += trans.z;
		}

		inline void fSetDiagonal( const tVector3<t>& scale )
		{
			tMatrix4& m = *this;
			m(0,0) = scale.x;
			m(1,1) = scale.y;
			m(2,2) = scale.z;
		}

		inline void fScaleLocal( const tVector3<t>& scales )
		{
			fXAxis( fXAxis( ) * scales.x );
			fYAxis( fYAxis( ) * scales.y );
			fZAxis( fZAxis( ) * scales.z );
		}

		inline void fScaleGlobal( const tVector3<t>& scales )
		{
			tMatrix4& m = *this;
			m(0,0) *= scales.x;
			m(1,1) *= scales.y;
			m(2,2) *= scales.z;
		}

		t fDeterminant( ) const
		{
			const tMatrix4& m = *this;
			const t a = m(2,2) * m(3,3) - m(3,2) * m(2,3);
			const t b = m(2,1) * m(3,3) - m(3,1) * m(2,3);
			const t c = m(2,1) * m(3,2) - m(3,1) * m(2,2);
			const t d = m(2,0) * m(3,3) - m(3,0) * m(2,3);
			const t e = m(2,0) * m(3,2) - m(3,0) * m(2,2);
			const t f = m(2,0) * m(3,1) - m(3,0) * m(2,1);

			return (
				m(0,0)*(
						a*m(1,1) -
						b*m(1,2) +
						c*m(1,3)
						) -

				m(0,1)*(
						a*m(1,0) -
						d*m(1,2) +
						e*m(1,3)
						) +

				m(0,2)*(
						b*m(1,0) -
						d*m(1,1) +
						f*m(1,3)
						) -

				m(0,3)*(
						c*m(1,0) -
						e*m(1,1) +
						f*m(1,2)
					)
				);
		}

		inline t fTrace( ) const
		{
			return mRow0.x + mRow1.y + mRow2.z + mRow3.w;
		}

		tMatrix4 fInverse( ) const;

		inline tMatrix4 fInverseNoScale( ) const
		{
			tVector3<t> trs = fGetTranslation( );
			tMatrix4<t> inv = fTranspose( );
			inv.fSetTranslation( -inv.fXformVector( trs ) );
			return inv;
		}

		tMatrix4 fTranspose( ) const
		{
			const tMatrix4& m = *this;
			return tMatrix4(
				m(0,0), m(1,0), m(2,0), m(3,0),
				m(0,1), m(1,1), m(2,1), m(3,1),
				m(0,2), m(1,2), m(2,2), m(3,2),
				m(0,3), m(1,3), m(2,3), m(3,3) );
		}
	};


// don't worry, we undef these before the end of the file
#define m_rowcol(row,col) fsrc[ row*4+col ]
#define i_rowcol(row,col) fdst[ row*4+col ]

	template<class t>
	tMatrix4<t> tMatrix4<t>::fInverse( ) const
	{
		const t* fsrc = &mRow0.x;

		const t a0 = m_rowcol(0,0)*m_rowcol(1,1) - m_rowcol(0,1)*m_rowcol(1,0);
		const t a1 = m_rowcol(0,0)*m_rowcol(1,2) - m_rowcol(0,2)*m_rowcol(1,0);
		const t a2 = m_rowcol(0,0)*m_rowcol(1,3) - m_rowcol(0,3)*m_rowcol(1,0);
		const t a3 = m_rowcol(0,1)*m_rowcol(1,2) - m_rowcol(0,2)*m_rowcol(1,1);
		const t a4 = m_rowcol(0,1)*m_rowcol(1,3) - m_rowcol(0,3)*m_rowcol(1,1);
		const t a5 = m_rowcol(0,2)*m_rowcol(1,3) - m_rowcol(0,3)*m_rowcol(1,2);
		const t b0 = m_rowcol(2,0)*m_rowcol(3,1) - m_rowcol(2,1)*m_rowcol(3,0);
		const t b1 = m_rowcol(2,0)*m_rowcol(3,2) - m_rowcol(2,2)*m_rowcol(3,0);
		const t b2 = m_rowcol(2,0)*m_rowcol(3,3) - m_rowcol(2,3)*m_rowcol(3,0);
		const t b3 = m_rowcol(2,1)*m_rowcol(3,2) - m_rowcol(2,2)*m_rowcol(3,1);
		const t b4 = m_rowcol(2,1)*m_rowcol(3,3) - m_rowcol(2,3)*m_rowcol(3,1);
		const t b5 = m_rowcol(2,2)*m_rowcol(3,3) - m_rowcol(2,3)*m_rowcol(3,2);
		const t det = a0*b5 - a1*b4 + a2*b3 + a3*b2 - a4*b1 + a5*b0;

		const t epsilon = 0.0001f;
		if( fAbs( det ) <= epsilon )
			return tMatrix4<t>( 0.f );

		tMatrix4<t> dst;
		t* fdst = &dst(0,0);
		i_rowcol(0,0) = + m_rowcol(1,1)*b5 - m_rowcol(1,2)*b4 + m_rowcol(1,3)*b3;
		i_rowcol(1,0) = - m_rowcol(1,0)*b5 + m_rowcol(1,2)*b2 - m_rowcol(1,3)*b1;
		i_rowcol(2,0) = + m_rowcol(1,0)*b4 - m_rowcol(1,1)*b2 + m_rowcol(1,3)*b0;
		i_rowcol(3,0) = - m_rowcol(1,0)*b3 + m_rowcol(1,1)*b1 - m_rowcol(1,2)*b0;
		i_rowcol(0,1) = - m_rowcol(0,1)*b5 + m_rowcol(0,2)*b4 - m_rowcol(0,3)*b3;
		i_rowcol(1,1) = + m_rowcol(0,0)*b5 - m_rowcol(0,2)*b2 + m_rowcol(0,3)*b1;
		i_rowcol(2,1) = - m_rowcol(0,0)*b4 + m_rowcol(0,1)*b2 - m_rowcol(0,3)*b0;
		i_rowcol(3,1) = + m_rowcol(0,0)*b3 - m_rowcol(0,1)*b1 + m_rowcol(0,2)*b0;
		i_rowcol(0,2) = + m_rowcol(3,1)*a5 - m_rowcol(3,2)*a4 + m_rowcol(3,3)*a3;
		i_rowcol(1,2) = - m_rowcol(3,0)*a5 + m_rowcol(3,2)*a2 - m_rowcol(3,3)*a1;
		i_rowcol(2,2) = + m_rowcol(3,0)*a4 - m_rowcol(3,1)*a2 + m_rowcol(3,3)*a0;
		i_rowcol(3,2) = - m_rowcol(3,0)*a3 + m_rowcol(3,1)*a1 - m_rowcol(3,2)*a0;
		i_rowcol(0,3) = - m_rowcol(2,1)*a5 + m_rowcol(2,2)*a4 - m_rowcol(2,3)*a3;
		i_rowcol(1,3) = + m_rowcol(2,0)*a5 - m_rowcol(2,2)*a2 + m_rowcol(2,3)*a1;
		i_rowcol(2,3) = - m_rowcol(2,0)*a4 + m_rowcol(2,1)*a2 - m_rowcol(2,3)*a0;
		i_rowcol(3,3) = + m_rowcol(2,0)*a3 - m_rowcol(2,1)*a1 + m_rowcol(2,2)*a0;

		const t invDet = 1.f / det;
		for( u32 i = 0; i < dst.cDimension; ++i )
			fdst[ i ] *= invDet;

		return dst;
	}

	template<class t>
	tMatrix3<t> tMatrix3<t>::fInverse( ) const
	{
		const t* fsrc = &mRow0.x;

		// the commented out expressions in this function simply help to
		// illustrate the simplification that occurs between the 4x4 and 4x3 case,
		// i.e., we can assume the last row is 0,0,0,1

		//static const t m30 = 0.f;
		//static const t m31 = 0.f;
		//static const t m32 = 0.f;
		//static const t m33 = 1.f;

		const t a0 = m_rowcol(0,0)*m_rowcol(1,1) - m_rowcol(0,1)*m_rowcol(1,0);
		const t a1 = m_rowcol(0,0)*m_rowcol(1,2) - m_rowcol(0,2)*m_rowcol(1,0);
		const t a2 = m_rowcol(0,0)*m_rowcol(1,3) - m_rowcol(0,3)*m_rowcol(1,0);
		const t a3 = m_rowcol(0,1)*m_rowcol(1,2) - m_rowcol(0,2)*m_rowcol(1,1);
		const t a4 = m_rowcol(0,1)*m_rowcol(1,3) - m_rowcol(0,3)*m_rowcol(1,1);
		const t a5 = m_rowcol(0,2)*m_rowcol(1,3) - m_rowcol(0,3)*m_rowcol(1,2);
		//const t b0 = 0.f;//m_rowcol(2,0)*m31 - m_rowcol(2,1)*m30;
		//const t b1 = 0.f;//m_rowcol(2,0)*m32 - m_rowcol(2,2)*m30;
		const t b2 = m_rowcol(2,0);//m_rowcol(2,0)*m33 - m_rowcol(2,3)*m30;
		//const t b3 = 0.f;//m_rowcol(2,1)*m32 - m_rowcol(2,2)*m31;
		const t b4 = m_rowcol(2,1);//m_rowcol(2,1)*m33 - m_rowcol(2,3)*m31;
		const t b5 = m_rowcol(2,2);//m_rowcol(2,2)*m33 - m_rowcol(2,3)*m32;
		const t det = a0*b5 - a1*b4 /*+ a2*b3*/ + a3*b2 /*- a4*b1 + a5*b0*/;

		const t epsilon = 0.0001f;
		if( fAbs( det ) <= epsilon )
			return tMatrix3<t>( 0.f );

		tMatrix3<t> dst;
		t* fdst = &dst(0,0);

		i_rowcol(0,0) = + m_rowcol(1,1)*b5 - m_rowcol(1,2)*b4 /*+ m_rowcol(1,3)*b3*/;
		i_rowcol(1,0) = - m_rowcol(1,0)*b5 + m_rowcol(1,2)*b2 /*- m_rowcol(1,3)*b1*/;
		i_rowcol(2,0) = + m_rowcol(1,0)*b4 - m_rowcol(1,1)*b2 /*+ m_rowcol(1,3)*b0*/;
		i_rowcol(0,1) = - m_rowcol(0,1)*b5 + m_rowcol(0,2)*b4 /*- m_rowcol(0,3)*b3*/;
		i_rowcol(1,1) = + m_rowcol(0,0)*b5 - m_rowcol(0,2)*b2 /*+ m_rowcol(0,3)*b1*/;
		i_rowcol(2,1) = - m_rowcol(0,0)*b4 + m_rowcol(0,1)*b2 /*- m_rowcol(0,3)*b0*/;
		i_rowcol(0,2) = /*+ m31*a5 - m32*a4 + m33**/ a3;
		i_rowcol(1,2) = /*- m30*a5 + m32*a2 - m33**/-a1;
		i_rowcol(2,2) = /*+ m30*a4 - m31*a2 + m33**/ a0;
		i_rowcol(0,3) = - m_rowcol(2,1)*a5 + m_rowcol(2,2)*a4 - m_rowcol(2,3)*a3;
		i_rowcol(1,3) = + m_rowcol(2,0)*a5 - m_rowcol(2,2)*a2 + m_rowcol(2,3)*a1;
		i_rowcol(2,3) = - m_rowcol(2,0)*a4 + m_rowcol(2,1)*a2 - m_rowcol(2,3)*a0;

		const t invDet = 1.f / det;
		for( u32 i = 0; i < dst.cDimension; ++i )
			fdst[ i ] *= invDet;

		return dst;
	}

#undef i_rowcol
#undef m_rowcol


	template<class t>
	const tMatrix4<t> tMatrix4<t>::cIdentity( MatrixUtil::cIdentityTag );

	typedef tMatrix3<f32> tMat3f;
	typedef tMatrix3<f64> tMat3d;

	typedef tMatrix4<f32> tMat4f;
	typedef tMatrix4<f64> tMat4d;

}}

#endif//__tMatrix__
