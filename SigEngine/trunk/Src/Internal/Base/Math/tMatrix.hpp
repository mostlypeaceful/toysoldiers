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

#define SIG_XNA_MATH 1

	template< typename tArchive, typename tMatrix >
	void fSaveLoadMatrix( tArchive& a, tMatrix& m )
	{
		sigassert( sizeof( m ) == m.cDimension * sizeof( tMatrix::tType ) );

		typename tMatrix::tType* f = &m(0,0);
		for( u32 i = 0; i < m.cDimension; ++i )
			a.fSaveLoad( f[ i ] );
	}

	template< typename tMatrix >
	inline b32 fIsMatrixNan( const tMatrix& m )
	{
		sigassert( sizeof( m ) == m.cDimension * sizeof( typename tMatrix::tType ) );

		const typename tMatrix::tType* f = &m(0,0);
		for( u32 i = 0; i < m.cDimension; ++i )
			if( f[ i ] != f[ i ] ) return true;

		return false;
	}

	static const f32 cMatrixEqualEpsilon = 0.00001f;

	template< typename tMatrix, typename tEpsilon >
	inline b32 fIsMatrixEqual( const tMatrix& m1, const tMatrix& m2, const tEpsilon& eps )
	{
		sigassert( sizeof( m1 ) == m1.cDimension * sizeof( typename tMatrix::tType ) );

		const typename tMatrix::tType* f1 = &m1(0,0);
		const typename tMatrix::tType* f2 = &m2(0,0);
		for( u32 i = 0; i < m1.cDimension; ++i )
			if( !fEqual( f1[ i ], f2[ i ], eps ) )
				return false;

		return true;
	}

	///
	/// \brief Represents a simplified 4x4 matrix with a tacit last row equaling 0,0,0,1;
	/// this matrix is capable of representing most transformations (i.e., rotation/scale/translation), 
	/// but cannot represent projections. Basically just a class that is both memory and performance
	/// optimized for the last row assumption.
	template<class t>
	class tMatrix3
	{
		friend class tMatrix4<t>;
		declare_reflector( );
		define_class_pool_new_delete( tMatrix3, 256 );
		sig_make_stringstreamable( tMatrix3, "(" << mRow0 << ", " << mRow1 << ", " << mRow2 << ")" );
	public:
		static const u32 cDimension = 12;
		typedef t tType;

	private:
		tVector4<t> mRow0;
		tVector4<t> mRow1;
		tVector4<t> mRow2;

		inline tVector4<t>&			fRow( u32 i )		{ sigassert( i < 3 ); return (&mRow0)[i]; }
		inline const tVector4<t>&	fRow( u32 i ) const { sigassert( i < 3 ); return (&mRow0)[i]; }

	public:
		static const tMatrix3 cIdentity;

		inline tMatrix3( ) { sig_setvecinvalid( t, fM( 0, 0 ), cDimension ); }

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

		inline t&			fM( const u32 row, const u32 col )					{ return fRow( row ).fAxis( col ); }
		inline const t&		fM( const u32 row, const u32 col ) const			{ return fRow( row ).fAxis( col ); }

		inline void			fSetRow( const u32 row, const tVector3<t>& v )		{ sig_assertvecvalid( v ); fRow( row ).x = v.x; fRow( row ).y = v.y; fRow( row ).z = v.z; }
		inline tVector3<t>	fGetRow( const u32 row ) const						{ return tVector3<t>( fRow( row ).x, fRow( row ).y, fRow( row ).z ); }
		inline void			fSetCol( const u32 col, const tVector3<t>& v )		{ sig_assertvecvalid( v ); fRow( 0 ).fAxis( col ) = v.x; fRow( 1 ).fAxis( col ) = v.y; fRow( 2 ).fAxis( col ) = v.z; }
		inline tVector3<t>	fGetCol( const u32 col ) const						{ return tVector3<t>( fRow( 0 ).fAxis( col ), fRow( 1 ).fAxis( col ), fRow( 2 ).fAxis( col ) ); }

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

			mo( 0, 0 ) = fM( 0, 0 ) * mat( 0, 0 ) + fM( 0, 1 ) * mat( 1, 0 ) + fM( 0, 2 ) * mat( 2, 0 );
			mo( 0, 1 ) = fM( 0, 0 ) * mat( 0, 1 ) + fM( 0, 1 ) * mat( 1, 1 ) + fM( 0, 2 ) * mat( 2, 1 );
			mo( 0, 2 ) = fM( 0, 0 ) * mat( 0, 2 ) + fM( 0, 1 ) * mat( 1, 2 ) + fM( 0, 2 ) * mat( 2, 2 );
			mo( 0, 3 ) = fM( 0, 0 ) * mat( 0, 3 ) + fM( 0, 1 ) * mat( 1, 3 ) + fM( 0, 2 ) * mat( 2, 3 ) + fM( 0, 3 );

			mo( 1, 0 ) = fM( 1, 0 ) * mat( 0, 0 ) + fM( 1, 1 ) * mat( 1, 0 ) + fM( 1, 2 ) * mat( 2, 0 );
			mo( 1, 1 ) = fM( 1, 0 ) * mat( 0, 1 ) + fM( 1, 1 ) * mat( 1, 1 ) + fM( 1, 2 ) * mat( 2, 1 );
			mo( 1, 2 ) = fM( 1, 0 ) * mat( 0, 2 ) + fM( 1, 1 ) * mat( 1, 2 ) + fM( 1, 2 ) * mat( 2, 2 );
			mo( 1, 3 ) = fM( 1, 0 ) * mat( 0, 3 ) + fM( 1, 1 ) * mat( 1, 3 ) + fM( 1, 2 ) * mat( 2, 3 ) + fM( 1, 3 );

			mo( 2, 0 ) = fM( 2, 0 ) * mat( 0, 0 ) + fM( 2, 1 ) * mat( 1, 0 ) + fM( 2, 2 ) * mat( 2, 0 );
			mo( 2, 1 ) = fM( 2, 0 ) * mat( 0, 1 ) + fM( 2, 1 ) * mat( 1, 1 ) + fM( 2, 2 ) * mat( 2, 1 );
			mo( 2, 2 ) = fM( 2, 0 ) * mat( 0, 2 ) + fM( 2, 1 ) * mat( 1, 2 ) + fM( 2, 2 ) * mat( 2, 2 );
			mo( 2, 3 ) = fM( 2, 0 ) * mat( 0, 3 ) + fM( 2, 1 ) * mat( 1, 3 ) + fM( 2, 2 ) * mat( 2, 3 ) + fM( 2, 3 );

			sig_assertvecvalid( mo );

			return mo;
		}

		inline tMatrix3		operator*=( const tMatrix3& mat ) { return ( *this = ( *this ) * mat ); }

		inline tVector3<t>	fXformVector( const tVector3<t>& v ) const
		{ 
			return tVector3<t>( 
				( fM( 0, 0 ) * v.x + fM( 0, 1 ) * v.y + fM( 0, 2 ) * v.z ),
				( fM( 1, 0 ) * v.x + fM( 1, 1 ) * v.y + fM( 1, 2 ) * v.z ),
				( fM( 2, 0 ) * v.x + fM( 2, 1 ) * v.y + fM( 2, 2 ) * v.z ) );
		}

		inline tVector3<t>	fInverseXformVector( const tVector3<t>& v ) const
		{ 
			// This is the same as above, just uses the transposed rotation.
			return tVector3<t>( 
				( fM( 0, 0 ) * v.x + fM( 1, 0 ) * v.y + fM( 2, 0 ) * v.z ),
				( fM( 0, 1 ) * v.x + fM( 1, 1 ) * v.y + fM( 2, 1 ) * v.z ),
				( fM( 0, 2 ) * v.x + fM( 1, 2 ) * v.y + fM( 2, 2 ) * v.z ) );
		}

		inline tVector3<t>	fXformPoint( const tVector3<t>& p ) const
		{ 
			return tVector3<t>( 
				( fM( 0, 0 ) * p.x + fM( 0, 1 ) * p.y + fM( 0, 2 ) * p.z + fM( 0, 3 ) ),
				( fM( 1, 0 ) * p.x + fM( 1, 1 ) * p.y + fM( 1, 2 ) * p.z + fM( 1, 3 ) ),
				( fM( 2, 0 ) * p.x + fM( 2, 1 ) * p.y + fM( 2, 2 ) * p.z + fM( 2, 3 ) ) );
		}

		inline tVector4<t>	fXform( const tVector4<t>& v ) const
		{
			return fXform( v.x, v.y, v.z, v.w );
		}

		inline tVector4<t>	fXform( t x, t y, t z, t w ) const
		{ 
			return tVector4<t>( 
				( fM( 0, 0 ) * x + fM( 0, 1 ) * y + fM( 0, 2 ) * z + fM( 0, 3 ) * w ),
				( fM( 1, 0 ) * x + fM( 1, 1 ) * y + fM( 1, 2 ) * z + fM( 1, 3 ) * w ),
				( fM( 2, 0 ) * x + fM( 2, 1 ) * y + fM( 2, 2 ) * z + fM( 2, 3 ) * w ), w );
		}

		inline void fTranslateLocal( const tVector3<t>& trans )
		{
			fM( 0, 3 ) += fM( 0, 0 ) * trans.x + fM( 0, 1 ) * trans.y + fM( 0, 2 ) * trans.z;
			fM( 1, 3 ) += fM( 1, 0 ) * trans.x + fM( 1, 1 ) * trans.y + fM( 1, 2 ) * trans.z;
			fM( 2, 3 ) += fM( 2, 0 ) * trans.x + fM( 2, 1 ) * trans.y + fM( 2, 2 ) * trans.z;
		}

		inline void fTranslateGlobal( const tVector3<t>& trans )
		{
			fM( 0, 3 ) += trans.x;
			fM( 1, 3 ) += trans.y;
			fM( 2, 3 ) += trans.z;
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
			fM(0,0) *= scales.x;
			fM(1,1) *= scales.y;
			fM(2,2) *= scales.z;
		}

		inline void fNormalizeBasis( )
		{
			fScaleLocal( tVector3<t>( 1.f / fXAxis( ).fLength( ), 1.f / fYAxis( ).fLength( ), 1.f / fZAxis( ).fLength( ) ) );
			sig_assertvecvalid( *this );
		}

		inline void fSafeNormalizeBasis( const f32 epsilon = cVectorEqualElementEpsilon )
		{
			sig_assertvecvalid( *this );

			const tVector3<t> x = fXAxis( );
			const tVector3<t> y = fYAxis( );
			const tVector3<t> z = fZAxis( );

			const tVector3<t> scale = fGetScale( );

			const u32 xlenZero = Sig::fEqual<t,t>( scale.x, t(0), epsilon ) ? 1 : 0;
			const u32 ylenZero = Sig::fEqual<t,t>( scale.y, t(0), epsilon ) ? 1 : 0;
			const u32 zlenZero = Sig::fEqual<t,t>( scale.z, t(0), epsilon ) ? 1 : 0;

			switch( xlenZero + ylenZero + zlenZero )
			{
			case 0: // no axises zero
				fXAxis( x * ( 1.f / scale.x ) );
				fYAxis( y * ( 1.f / scale.y ) );
				fZAxis( z * ( 1.f / scale.z ) );
				break;
			case 1: // 1 axis zero, infer missing axis from other two axises
				fXAxis( !xlenZero ? x * ( 1.f / scale.x ) : y.fCross( z ) * ( 1.f / ( scale.y * scale.z ) ) );
				fYAxis( !ylenZero ? y * ( 1.f / scale.y ) : z.fCross( x ) * ( 1.f / ( scale.z * scale.x ) ) );
				fZAxis( !zlenZero ? z * ( 1.f / scale.z ) : x.fCross( y ) * ( 1.f / ( scale.x * scale.y ) ) );
				break;
			case 2: // 2 axises zero, pick arbitrary right angle for other two axises
				{
					const u32 firstZeroCol = xlenZero ? 0 : 1;
					const u32 lastZeroCol = zlenZero ? 2 : 1;
					const u32 nonzeroCol = 3 - firstZeroCol - lastZeroCol;

					const tVector3<t> nonzero = fGetCol( nonzeroCol ).fNormalize( );
					// create right angles
					const tVector3<t> ra1 = nonzero.fBuildPerp( ).fNormalize( );
					const tVector3<t> ra2 = nonzero.fCross( ra1 ); // should result in a normal already

					fSetCol( nonzeroCol,	nonzero );
					fSetCol( firstZeroCol,	ra1 );
					fSetCol( nonzeroCol,	ra2 );
				}
				break;
			case 3: // all 3 axises zero, pick arbitrary x/y/z
				fXAxis( tVector3<t>( 1, 0, 0 ) );
				fYAxis( tVector3<t>( 0, 1, 0 ) );
				fZAxis( tVector3<t>( 0, 0, 1 ) );
				break;
			}
		}

		t fDeterminant( ) const
		{
			const t a = fM(2,2);
			const t b = fM(2,1);
			const t d = fM(2,0);

			return (
				fM(0,0)*( a*fM(1,1) - b*fM(1,2) ) -
				fM(0,1)*( a*fM(1,0) - d*fM(1,2) ) +
				fM(0,2)*( b*fM(1,0) - d*fM(1,1) )
					);
		}

		inline t fTrace( ) const
		{
			return fM( 0, 0 ) + fM( 1, 1 ) + fM( 2, 2 ) + 1.f;
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
			return tMatrix3(
				fM(0,0), fM(1,0), fM(2,0), fM(0,3),
				fM(0,1), fM(1,1), fM(2,1), fM(1,3),
				fM(0,2), fM(1,2), fM(2,2), fM(2,3) );
		}

		void fOrientXAxis( const tVector3<t>& newAxis, const tVector3<t>& y = tVector3<t>::cYAxis )
		{
			sig_assert_unit_vector( newAxis );
			fXAxis( newAxis );
			fZAxis( newAxis.fCross( y ).fNormalizeSafe( tVector3<t>::cZAxis ) );
			fYAxis( fZAxis( ).fCross( fXAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientXAxisZAxis( const tVector3<t>& newAxis, const tVector3<t>& z = tVector3<t>::cZAxis )
		{
			sig_assert_unit_vector( newAxis );
			fXAxis( newAxis );
			fYAxis( z.fCross( fXAxis( ) ).fNormalizeSafe( tVector3<t>::cYAxis ) );
			fZAxis( newAxis.fCross( fYAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientYAxis( const tVector3<t>& newAxis, const tVector3<t>& x = tVector3<t>::cXAxis )
		{
			sig_assert_unit_vector( newAxis );
			fYAxis( newAxis );
			fZAxis( x.fCross( newAxis ).fNormalizeSafe( tVector3<t>::cZAxis ) );
			fXAxis( fYAxis( ).fCross( fZAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientYWithZAxis( const tVector3<t>& newAxis, const tVector3<t>& z = tVector3<t>::cZAxis )
		{
			sig_assert_unit_vector( newAxis );
			fYAxis( newAxis );
			fXAxis( fYAxis( ).fCross( z ).fNormalizeSafe( tVector3<t>::cXAxis ) );
			fZAxis( fXAxis( ).fCross( newAxis ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientZAxis( const tVector3<t>& newAxis, const tVector3<t>& y = tVector3<t>::cYAxis )
		{
			sig_assert_unit_vector( newAxis );
			fZAxis( newAxis );
			fXAxis( y.fCross( newAxis ).fNormalizeSafe( tVector3<t>::cXAxis ) );
			fYAxis( fZAxis( ).fCross( fXAxis( ) ) );
			sigassert( fXAxis( ) == fXAxis( ) && fYAxis( ) == fYAxis( ) && fZAxis( ) == fZAxis( ) );
		}

		void fOrientZWithXAxis( const tVector3<t>& newAxis, const tVector3<t>& x = tVector3<t>::cXAxis )
		{
			sig_assert_unit_vector( newAxis );
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
			tAxisAngle<t> aa( facing, faceThisDirection, fYAxis( ).fNormalizeSafe( tVector3<t>::cYAxis ) );
			aa.mAngle *= lerpFactor;

			if( Sig::fEqual( aa.mAngle, t(0), (t)0.001f ) )
				return false;

			const tVector3<t> newFacing = tQuaternion<t>( aa ).fRotate( facing );
			fOrientZAxis( newFacing );
			return true;
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			fSaveLoadMatrix( archive, *this );
		}

		inline b32 fIsNan( ) const
		{
			return fIsMatrixNan( *this );
		}

		inline b32 fEqual( const tMatrix3& rhs, f32 epsilon = cMatrixEqualEpsilon ) const
		{
			return fIsMatrixEqual( *this, rhs, epsilon );
		}
	};

	template<class t>
	const tMatrix3<t> tMatrix3<t>::cIdentity( MatrixUtil::cIdentityTag );

	///
	/// \brief Represents a full 4x4 matrix, capable of representing
	/// all transformations including projections (unlike the tMatrix3).
	template<class t>
	class tMatrix4
	{
		declare_reflector( );
		sig_make_stringstreamable( tMatrix4, "(" << mRow0 << ", " << mRow1 << ", " << mRow2 << ", " << mRow3 << ")" );
	public:
		static const u32 cDimension = 16;
		typedef t tType;

		inline tVector4<t>&			fRow( u32 i )		{ sigassert( i < 4 ); return (&mRow0)[i]; }
		inline const tVector4<t>&	fRow( u32 i ) const { sigassert( i < 4 ); return (&mRow0)[i]; }

		inline t&			fM( const u32 row, const u32 col )					{ return fRow( row ).fAxis( col ); }
		inline const t&		fM( const u32 row, const u32 col ) const			{ return fRow( row ).fAxis( col ); }

	private:
		tVector4<t> mRow0;
		tVector4<t> mRow1;
		tVector4<t> mRow2;
		tVector4<t> mRow3;

	public:
		static const tMatrix4 cIdentity;

		inline tMatrix4( ) { sig_setvecinvalid( t, fM( 0, 0 ), cDimension ); }

		inline explicit tMatrix4( tNoOpTag ) 
			: mRow0( cNoOpTag )
			, mRow1( cNoOpTag )
			, mRow2( cNoOpTag )
			, mRow3( cNoOpTag ) 
		{ }

		inline explicit tMatrix4( MatrixUtil::tIdentityTag )
		{
			fMemSet( *this, 0 );
			fM(0,0) = fM(1,1) = fM(2,2) = fM(3,3) = 1.f;
		}

		inline explicit tMatrix4( const t& scale )
		{
			fMemSet( *this, 0 );
			fM(0,0) = fM(1,1) = fM(2,2) = scale;
			fM(3,3) = 1.f;
		}

		inline tMatrix4( 
			t m00, t m01, t m02, t m03,
			t m10, t m11, t m12, t m13,
			t m20, t m21, t m22, t m23,
			t m30=0, t m31=0, t m32=0, t m33=1 ) 
		{
			fM(0,0) = m00; fM(0,1) = m01; fM(0,2) = m02; fM(0,3) = m03;
			fM(1,0) = m10; fM(1,1) = m11; fM(1,2) = m12; fM(1,3) = m13;
			fM(2,0) = m20; fM(2,1) = m21; fM(2,2) = m22; fM(2,3) = m23;
			fM(3,0) = m30; fM(3,1) = m31; fM(3,2) = m32; fM(3,3) = m33;
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

			mo( 0, 0 ) = fM( 0, 0 ) * mat( 0, 0 ) + fM( 0, 1 ) * mat( 1, 0 ) + fM( 0, 2 ) * mat( 2, 0 ) + fM( 0, 3 ) * mat( 3, 0 );
			mo( 0, 1 ) = fM( 0, 0 ) * mat( 0, 1 ) + fM( 0, 1 ) * mat( 1, 1 ) + fM( 0, 2 ) * mat( 2, 1 ) + fM( 0, 3 ) * mat( 3, 1 );
			mo( 0, 2 ) = fM( 0, 0 ) * mat( 0, 2 ) + fM( 0, 1 ) * mat( 1, 2 ) + fM( 0, 2 ) * mat( 2, 2 ) + fM( 0, 3 ) * mat( 3, 2 );
			mo( 0, 3 ) = fM( 0, 0 ) * mat( 0, 3 ) + fM( 0, 1 ) * mat( 1, 3 ) + fM( 0, 2 ) * mat( 2, 3 ) + fM( 0, 3 ) * mat( 3, 3 );

			mo( 1, 0 ) = fM( 1, 0 ) * mat( 0, 0 ) + fM( 1, 1 ) * mat( 1, 0 ) + fM( 1, 2 ) * mat( 2, 0 ) + fM( 1, 3 ) * mat( 3, 0 );
			mo( 1, 1 ) = fM( 1, 0 ) * mat( 0, 1 ) + fM( 1, 1 ) * mat( 1, 1 ) + fM( 1, 2 ) * mat( 2, 1 ) + fM( 1, 3 ) * mat( 3, 1 );
			mo( 1, 2 ) = fM( 1, 0 ) * mat( 0, 2 ) + fM( 1, 1 ) * mat( 1, 2 ) + fM( 1, 2 ) * mat( 2, 2 ) + fM( 1, 3 ) * mat( 3, 2 );
			mo( 1, 3 ) = fM( 1, 0 ) * mat( 0, 3 ) + fM( 1, 1 ) * mat( 1, 3 ) + fM( 1, 2 ) * mat( 2, 3 ) + fM( 1, 3 ) * mat( 3, 3 );

			mo( 2, 0 ) = fM( 2, 0 ) * mat( 0, 0 ) + fM( 2, 1 ) * mat( 1, 0 ) + fM( 2, 2 ) * mat( 2, 0 ) + fM( 2, 3 ) * mat( 3, 0 );
			mo( 2, 1 ) = fM( 2, 0 ) * mat( 0, 1 ) + fM( 2, 1 ) * mat( 1, 1 ) + fM( 2, 2 ) * mat( 2, 1 ) + fM( 2, 3 ) * mat( 3, 1 );
			mo( 2, 2 ) = fM( 2, 0 ) * mat( 0, 2 ) + fM( 2, 1 ) * mat( 1, 2 ) + fM( 2, 2 ) * mat( 2, 2 ) + fM( 2, 3 ) * mat( 3, 2 );
			mo( 2, 3 ) = fM( 2, 0 ) * mat( 0, 3 ) + fM( 2, 1 ) * mat( 1, 3 ) + fM( 2, 2 ) * mat( 2, 3 ) + fM( 2, 3 ) * mat( 3, 3 );

			mo( 3, 0 ) = fM( 3, 0 ) * mat( 0, 0 ) + fM( 3, 1 ) * mat( 1, 0 ) + fM( 3, 2 ) * mat( 2, 0 ) + fM( 3, 3 ) * mat( 3, 0 );
			mo( 3, 1 ) = fM( 3, 0 ) * mat( 0, 1 ) + fM( 3, 1 ) * mat( 1, 1 ) + fM( 3, 2 ) * mat( 2, 1 ) + fM( 3, 3 ) * mat( 3, 1 );
			mo( 3, 2 ) = fM( 3, 0 ) * mat( 0, 2 ) + fM( 3, 1 ) * mat( 1, 2 ) + fM( 3, 2 ) * mat( 2, 2 ) + fM( 3, 3 ) * mat( 3, 2 );
			mo( 3, 3 ) = fM( 3, 0 ) * mat( 0, 3 ) + fM( 3, 1 ) * mat( 1, 3 ) + fM( 3, 2 ) * mat( 2, 3 ) + fM( 3, 3 ) * mat( 3, 3 );

			return mo;
		}

		inline tMatrix4		operator*=( const tMatrix4& mat ) { return ( *this = ( *this ) * mat ); }

		inline tVector3<t>	fXformVector( const tVector3<t>& v ) const
		{ 
			return tVector3<t>( 
				( fM( 0, 0 ) * v.x + fM( 0, 1 ) * v.y + fM( 0, 2 ) * v.z ),
				( fM( 1, 0 ) * v.x + fM( 1, 1 ) * v.y + fM( 1, 2 ) * v.z ),
				( fM( 2, 0 ) * v.x + fM( 2, 1 ) * v.y + fM( 2, 2 ) * v.z ) );
		}

		inline tVector3<t>	fXformPoint( const tVector3<t>& p ) const
		{ 
			return tVector3<t>( 
				( fM( 0, 0 ) * p.x + fM( 0, 1 ) * p.y + fM( 0, 2 ) * p.z + fM( 0, 3 ) ),
				( fM( 1, 0 ) * p.x + fM( 1, 1 ) * p.y + fM( 1, 2 ) * p.z + fM( 1, 3 ) ),
				( fM( 2, 0 ) * p.x + fM( 2, 1 ) * p.y + fM( 2, 2 ) * p.z + fM( 2, 3 ) ) );
		}

		inline tVector4<t>	fXform( const tVector4<t>& v ) const
		{
			return fXform( v.x, v.y, v.z, v.w );
		}

		inline tVector4<t>	fXform( t x, t y, t z, t w ) const
		{ 
			return tVector4<t>( 
				( fM( 0, 0 ) * x + fM( 0, 1 ) * y + fM( 0, 2 ) * z + fM( 0, 3 ) * w ),
				( fM( 1, 0 ) * x + fM( 1, 1 ) * y + fM( 1, 2 ) * z + fM( 1, 3 ) * w ),
				( fM( 2, 0 ) * x + fM( 2, 1 ) * y + fM( 2, 2 ) * z + fM( 2, 3 ) * w ),
				( fM( 3, 0 ) * x + fM( 3, 1 ) * y + fM( 3, 2 ) * z + fM( 3, 3 ) * w ) );
		}

		inline void fTranslateLocal( const tVector3<t>& trans )
		{
			fM( 0, 3 ) += fM( 0, 0 ) * trans.x + fM( 0, 1 ) * trans.y + fM( 0, 2 ) * trans.z;
			fM( 1, 3 ) += fM( 1, 0 ) * trans.x + fM( 1, 1 ) * trans.y + fM( 1, 2 ) * trans.z;
			fM( 2, 3 ) += fM( 2, 0 ) * trans.x + fM( 2, 1 ) * trans.y + fM( 2, 2 ) * trans.z;
		}

		inline void fTranslateGlobal( const tVector3<t>& trans )
		{
			fM( 0, 3 ) += trans.x;
			fM( 1, 3 ) += trans.y;
			fM( 2, 3 ) += trans.z;
		}

		inline void fSetDiagonal( const tVector3<t>& scale )
		{
			fM(0,0) = scale.x;
			fM(1,1) = scale.y;
			fM(2,2) = scale.z;
		}

		inline void fScaleLocal( const tVector3<t>& scales )
		{
			fXAxis( fXAxis( ) * scales.x );
			fYAxis( fYAxis( ) * scales.y );
			fZAxis( fZAxis( ) * scales.z );
		}

		inline void fScaleGlobal( const tVector3<t>& scales )
		{
			fM(0,0) *= scales.x;
			fM(1,1) *= scales.y;
			fM(2,2) *= scales.z;
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
			return fM( 0, 0 ) + fM( 1, 1 ) + fM( 2, 2 ) + fM( 3, 3 );
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
			return tMatrix4(
				fM(0,0), fM(1,0), fM(2,0), fM(3,0),
				fM(0,1), fM(1,1), fM(2,1), fM(3,1),
				fM(0,2), fM(1,2), fM(2,2), fM(3,2),
				fM(0,3), fM(1,3), fM(2,3), fM(3,3) );
		}

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			fSaveLoadMatrix( archive, *this );
		}

		inline b32 fIsNan( ) const
		{
			return fIsMatrixNan( *this );
		}

		inline b32 fEqual( const tMatrix4& rhs, f32 epsilon = cMatrixEqualEpsilon ) const
		{
			return fIsMatrixEqual( *this, rhs, epsilon );
		}
	};


// don't worry, we undef these before the end of the file
#define m_rowcol(row,col) fsrc[ row*4+col ]
#define i_rowcol(row,col) fdst[ row*4+col ]

	template<class t>
	tMatrix4<t> tMatrix4<t>::fInverse( ) const
	{
		const t* fsrc = &fM( 0, 0 );

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
		const t* fsrc = &fM( 0, 0 );

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

	// Matrix Math Functions

	// result = m0 * m1, i.e. the columns of m1 x the rows of m0
	template<class t>
	void fMatrixMultiply( t * result, const t * m0, const t * m1, u32 dim )
	{
		sigassert( result != m0 && result != m1 );

		for( u32 i = 0, ithRow = 0; i < dim; ++i, ithRow += dim )
		{
			for( u32 j = 0; j < dim; ++j )
			{
				result[ ithRow + j ] = 0;
				for( u32 k = 0, kthRow = 0; k < dim; ++k, kthRow += dim )
					result[ ithRow + j ] += m0[ ithRow + k ] * m1[ kthRow + j ]; 
			}
		}
	}

	// result = m * v, where v is dim vector and m is a dim x dim matrix
	template<class t>
	void fMatrixTransformVector( t * result, const t * m, const t * v, u32 dim )
	{
		sigassert( result != v && "Result cannot be src!" );
		for( u32 i = 0, ithRow = 0; i < dim; ++i, ithRow += dim )
		{
			result[ i ] = 0;

			for( u32 j = 0; j < dim; ++j )
				result[ i ] += m[ ithRow + j ] * v[ j ];
		}
	}

	// m = inverse( m ), where m is a dim x dim matrix
	template<class t>
	b32 fMatrixInvert( t * m, u32 dim )
	{
		// Nothing to invert
		if( dim == 0 )
			return false;

		// Single dimension is trivial
		if( dim == 1 )
		{
			if( fEqual( *m, 0 ) )
				return false;

			*m = 1 / *m;
			return true;
		}

		// LU factorization with partial row pivoting, i.e. LUP

		// Initialize pivot table
		u32 * pivot = NEW u32[ dim ]; // An unfortunate necessity
		for( u32 i = 0; i < dim; ++i )
			pivot[ i ] = i;
		
		for( u32 k = 0, kthRow = 0; k < dim; ++k, kthRow += dim )
		{
			// Perform partial pivot so that m[k][k] has largest value
			t p = 0; u32 kI;
			for( u32 i = k, ithRow = i * dim; i < dim; ++i, ithRow += dim )
			{
				t testP = fAbs( m[ ithRow + k ] );
				if( testP > p )
				{
					p = testP;
					kI = i;
				}
			}

			// The matrix is singular and thus not invertible
			if( fEqual( p, 0 ) )
			{
				delete [] pivot;
				return false;
			}

			// Perform the pivot if necessary
			if( k != kI )
			{
				fSwap( pivot[ k ], pivot[ kI ] );
				const u32 kIthRow = kI * dim;
				for( u32 i = 0; i < dim; ++i )
					fSwap( m[ kthRow + i ], m[ kIthRow + i ] );
			}

			for( u32 i = k + 1, ithRow = i * dim; i < dim; ++i, ithRow += dim )
			{
				m[ ithRow + k ] /= m[ kthRow + k ];

				for( u32 j = k + 1; j < dim; ++j )
					m[ ithRow + j ] -= m[ ithRow + k ] * m[ kthRow + j ];
			}
		}


		// A == LUP, so inv(A) == inv(LUP), which equals inv(P) * inv(U) * inv(L)
		
		// Foward substitution to solve inv(L). L being a lower triangluar matrix
		// will have inv(L) as a lower triangular matrix. Also L being unitriangular
		// will have inv(L) as unitriangular, thus we only need to calculate the 
		// lower off-diagonal elements.
		for( u32 i = 1; i < dim; ++i )
		{
			const u32 ithRow = i * dim;
			for( u32 j = 0; j < i; ++j )
			{
				t val = 0;
				val += m[ ithRow + j ]; //simulate 1's in the diagonal
				for( u32 k = j + 1, kthRow = k * dim; k < i; ++k, kthRow += dim )
					val += m[ ithRow + k ] * m[ kthRow + j ]; 

				m[ ithRow + j ] = -val;
			}
		}

		// Backward substitution to solve inv(U). U being an upper triangular matrix
		// will have inv(U) as an upper triangular matrix. However, unlike L, U is
		// not unitriangular and thus we must solve for the diagonal as well.
		for( s32 i = dim - 1, ithRow = i * dim; i >= 0; --i, ithRow -= dim )
		{
			for( s32 j = dim - 1; j >= i; --j )
			{
				t val = ( i == j ? t(1) : t(0) );
				for( s32 k = j, kthRow = k * dim; k > i; --k, kthRow -= dim )
					val -= m[ kthRow + j ] * m[ ithRow + k ];

				m[ ithRow + j ] = val / m[ ithRow + i ];
			}
		}

		// Now multiply inv(U) * inv(L)
		for( u32 i = 0, ithRow = 0; i < dim; ++i, ithRow += dim )
		{
			for( u32 j = 0; j < dim; ++j )
			{
				t val = 0;
				for( u32 k = fMax( i, j ), kthRow = k * dim; k < dim; ++k, kthRow += dim )
					val += ( k == j ? 1 : m[ kthRow + j ] ) *  m[ ithRow + k ];

				m[ ithRow + j ] = val;
			}
		}

		// Now multiply by inv(P), which we can achieve by column swapping
		for( u32 i = 0; i < dim; ++i )
		{
			// Iterate on i till the row that supposed to be in i is in i
			while( pivot[ i ] != i )
			{
				for( u32 j = 0; j < dim; ++j )
					fSwap( m[ j * dim + i ], m[ j * dim + pivot[ i ] ] );

				fSwap( pivot[ i ] , pivot[ pivot[ i ] ] );
			}
		}

		delete [] pivot;
		return true;

		// m == inv(m)
	}

	
}}

#endif//__tMatrix__
