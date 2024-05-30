//------------------------------------------------------------------------------
// \file tProgressiveMeshTool.cpp - 18 Jul 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "ToolsPch.hpp"
#include "tProgressiveMeshTool.hpp"

#include <malloc.h> // Alloca

// for conversion from float->half for vertex attribute compression
#include "Dx360Util.hpp"

namespace Sig { namespace LOD 
{

	//------------------------------------------------------------------------------
	// Verbose checking defines for debugging
	//------------------------------------------------------------------------------
	//#define mesh_connectivity_build_validation
	#ifdef mesh_connectivity_build_validation
		#define mesh_connectivity_cascade_validation
	#endif //mesh_connectivity_build_validation

	//------------------------------------------------------------------------------
	// tQuadric
	//------------------------------------------------------------------------------
	u32 tQuadric::fCalculateDimension( const Gfx::tVertexFormat & vf )
	{
		u32 dimension = 0;

		// Calculate the dimension of the quadric based on the elements of the vertex format
		{
			const Gfx::tVertexElement * ptr = vf.fBegin( );
			const Gfx::tVertexElement * term = vf.fEnd( );
			for( ; ptr != term; ++ptr )
			{
				switch( ptr->mSemantic )
				{
				case Gfx::tVertexElement::cSemanticPosition:
					if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_3 )
						dimension += 3;
					else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
						dimension += 4;
					break;
				case Gfx::tVertexElement::cSemanticNormal:
					if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_3 ||
						ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
						dimension += 3;
					else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_2 )
						dimension += 2;
					break;
				case Gfx::tVertexElement::cSemanticBinormal:
					dimension += 3;
					break;
				case Gfx::tVertexElement::cSemanticTangent:
					dimension += 3;
					break;
				case Gfx::tVertexElement::cSemanticTexCoord:
					if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_2 ||
						ptr->mFormat == Gfx::tVertexElement::cFormat_f16_2 )
					{
						dimension += 2;
					}
					else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_4 || 
							 ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
					{
						// There will be 4, but we only count once
						if( !( ptr->mSemanticIndex & 1 ) )
							dimension += 4;
					}
					break;
				case Gfx::tVertexElement::cSemanticColor:
					dimension += 4;
					break;
				case Gfx::tVertexElement::cSemanticBoneWeights:
					//dimension += 4;
					break;
				case Gfx::tVertexElement::cSemanticBoneIndices:
					// Nothing
					break;
				}
			}
		}

		return dimension;
	}

	//------------------------------------------------------------------------------
	u32 tQuadric::fCalculateCoeffsNeeded( u32 dim )
	{
		sigassert( dim >= 1 && "Quadric expects to operate in R1 or higher" );

		// n multichoose k == (n + k - 1) choose k, here n == dim + 1( for the planar constant ) and k == 2
		return Math::fBinomial( dim + 2, 2 ); 
	}

	//------------------------------------------------------------------------------
	u32 tQuadric::fCalculateSize( u32 coeffsNeeded )
	{
		// The object already has one coeff so we subtract 1 in the size calculation
		return sizeof( tQuadric ) + ( ( coeffsNeeded - 1 ) * sizeof( f64 ) );
	}

	//------------------------------------------------------------------------------
	tQuadric * tQuadric::fInPlaceNew( void * mem, u32 dimension, u32 coeffsNeeded )
	{
		sigassert( fCalculateCoeffsNeeded( dimension ) == coeffsNeeded );
		return new( mem ) tQuadric( dimension, coeffsNeeded );
	}

	//------------------------------------------------------------------------------
	tQuadric * tQuadric::fNew( u32 dim )
	{
		u32 coeffsNeeded = fCalculateCoeffsNeeded( dim );
		const u32 size = fCalculateSize( coeffsNeeded );

		void * mem = new byte[ size ]; // Allocate
		return new( mem ) tQuadric( dim, coeffsNeeded ); // Construct
	}

	//------------------------------------------------------------------------------
	void tQuadric::fDelete( tQuadric * q )
	{
		//q->~tQuadric( ); // Destruct
		delete [] (byte*)q; // Free
	}

	//------------------------------------------------------------------------------
	void tQuadric::fConvertToRn( f64 * vRn, const void * v, const Gfx::tVertexFormat & vf )
	{
		f64 * vOut = vRn;
		const byte * vIn = (const byte *)v;
		const Gfx::tVertexElement * ptr = vf.fBegin( );
		const Gfx::tVertexElement * term = vf.fEnd( );
		for( ; ptr != term; ++ptr )
		{
			const byte * vSrc = ( vIn + ptr->mOffsetFromBase );
			switch( ptr->mSemantic )
			{
			case Gfx::tVertexElement::cSemanticPosition:
				if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_3 )
				{
					const Math::tVec3f & src = *(const Math::tVec3f *)vSrc;	
					*vOut++ = src.x;
					*vOut++ = src.y;
					*vOut++ = src.z;
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					const Math::tVec4f src = Dx360Util::fConvertHalfToVec4f( *(const Math::tVector4<u16>*)vSrc );
					*vOut++ = src.x;
					*vOut++ = src.y;
					*vOut++ = src.z;
					*vOut++ = src.w; // This is most likely a terrain mesh with src.w == normal.z
				} 
				else
				{
					sigassert( !"Invalid vertex format for cSemanticPosition!" );
				}
				break;
			case Gfx::tVertexElement::cSemanticNormal:
				if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_3 )
				{
					const Math::tVec3f & src = *(const Math::tVec3f *)vSrc;
					*vOut++ = src.x;
					*vOut++ = src.y;
					*vOut++ = src.z;
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					const Math::tVec4f src = Dx360Util::fConvertHalfToVec4f( *(const Math::tVector4<u16>*)vSrc );
					*vOut++ = src.x;
					*vOut++ = src.y;
					*vOut++ = src.z;
					// src.w always equals 0
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_2 )
				{
					// This is likely a terrain mesh with normal.z in pos.w
					const Math::tVec2f src = Dx360Util::fConvertHalfToVec2f( *(const Math::tVector2<u16>*)vSrc );
					*vOut++ = src.x;
					*vOut++ = src.y;
				}
				else
				{
					sigassert( !"Invalid vertex format for cSemanticNormal!" );
				}
				break;
			case Gfx::tVertexElement::cSemanticTangent:
				if(ptr->mFormat == Gfx::tVertexElement::cFormat_f32_4 )
				{
					const Math::tVec4f & src = *(const Math::tVec4f*)vSrc;
					*vOut++ = src.x;
					*vOut++ = src.y;
					*vOut++ = src.z;
					// sign is propagated
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					const Math::tVec4f src = Dx360Util::fConvertHalfToVec4f( *(const Math::tVector4<u16>*)vSrc );
					*vOut++ = src.x;
					*vOut++ = src.y;
					*vOut++ = src.z;
					// sign is propagated
				}
				else
				{
					sigassert( !"Invalid vertex format for cSemanticTangent!" );
				}
				break;
			case Gfx::tVertexElement::cSemanticBinormal:
				break;
			case Gfx::tVertexElement::cSemanticTexCoord:
				if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_2 )
				{
					const Math::tVec2f & src = *(const Math::tVec2f*)vSrc;
					*vOut++ = src.x;
					*vOut++ = src.y;
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_2 )
				{
					const Math::tVec2f src = Dx360Util::fConvertHalfToVec2f( *(const Math::tVector2<u16>*)vSrc );
					*vOut++ = src.x;
					*vOut++ = src.y;
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_4 )
				{
					const Math::tVec4f & src = *(const Math::tVec4f*)vSrc;
					
					if( ptr->mSemanticIndex & 1 )
					{
						*vOut++ = src.z;
						*vOut++ = src.w;
					}
					else
					{
						*vOut++ = src.x;
						*vOut++ = src.y;
					}
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					const Math::tVec4f src = Dx360Util::fConvertHalfToVec4f( *(const Math::tVector4<u16>*)vSrc );
					
					if( ptr->mSemanticIndex & 1 )
					{
						*vOut++ = src.z;
						*vOut++ = src.w;
					}
					else
					{
						*vOut++ = src.x;
						*vOut++ = src.y;
					}
				}
				else
				{
					sigassert( !"Invalid format for cSemanticTexCoord!" );
				}
				break;
			case Gfx::tVertexElement::cSemanticColor:
				{
					sigassert( ptr->mFormat == Gfx::tVertexElement::cFormat_u8_4_Color );

					Gfx::tVertexColor color( *(u32*)vSrc );

					*vOut++ = color.mA;
					*vOut++ = color.mR;
					*vOut++ = color.mG;
					*vOut++ = color.mB;

				}break;
			case Gfx::tVertexElement::cSemanticBoneWeights:
				break;
			case Gfx::tVertexElement::cSemanticBoneIndices:
				break;
			}
		}

		sigassert( vOut == vRn + fCalculateDimension( vf ) && "Sanity!" );
	}

	//------------------------------------------------------------------------------
	void tQuadric::fConvertFromRn( void * v, const f64 * vRn, const Gfx::tVertexFormat & vf )
	{
		const byte * vOut = (const byte *)v;
		const f64 * vIn = vRn;

		// We support normals compressed into w of pos if pos is 4x16
		const Gfx::tVertexElement * posAs4x16 = NULL; 

		const Gfx::tVertexElement * ptr = vf.fBegin( );
		const Gfx::tVertexElement * term = vf.fEnd( );
		for( ; ptr != term; ++ptr )
		{
			const byte * vDest = ( vOut + ptr->mOffsetFromBase );
			switch( ptr->mSemantic )
			{
			case Gfx::tVertexElement::cSemanticPosition:
				if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_3 )
				{
					Math::tVec3f & dest = *(Math::tVec3f *)vDest;
					dest.x = (f32)*vIn++;
					dest.y = (f32)*vIn++;
					dest.z = (f32)*vIn++;
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					posAs4x16 = ptr;

					Math::tVec4f src;
					src.x = (f32)*vIn++;
					src.y = (f32)*vIn++;
					src.z = (f32)*vIn++;

					// This may be normal.z, will cause normal issues if
					// it is not and normals are 2x16. Note: we lose fidelity
					// here because of the conversion to/from/to half
					src.w = (f32)*vIn++; 

					u16 * dest = (u16*)vDest;
					dest[0] = Dx360Util::fConvertFloatToHalf( src.x );
					dest[1] = Dx360Util::fConvertFloatToHalf( src.y );
					dest[2] = Dx360Util::fConvertFloatToHalf( src.z );
					dest[3] = Dx360Util::fConvertFloatToHalf( src.w );
				}
				break;
			case Gfx::tVertexElement::cSemanticNormal:
				if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_3 )
				{
					Math::tVec3f & dest = *(Math::tVec3f *)vDest;
					dest.x = (f32)*vIn++;
					dest.y = (f32)*vIn++;
					dest.z = (f32)*vIn++;

					dest.fNormalize( );
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					Math::tVec3f src;
					src.x = (f32)*vIn++;
					src.y = (f32)*vIn++;
					src.z = (f32)*vIn++;

					src.fNormalize( );

					u16 * dest = (u16*)vDest;
					dest[0] = Dx360Util::fConvertFloatToHalf( src.x );
					dest[1] = Dx360Util::fConvertFloatToHalf( src.y );
					dest[2] = Dx360Util::fConvertFloatToHalf( src.z );
					dest[3] = 0;
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_2 )
				{
					sigassert( posAs4x16 && "2x16 normals must succeed 4x16 pos!" );
					
					u16 * pos = (u16*)(vOut + posAs4x16->mOffsetFromBase);

					Math::tVec3f src;
					src.x = (f32)*vIn++;
					src.y = (f32)*vIn++;
					src.z = Dx360Util::fConvertHalfToFloat( pos[3] );

					src.fNormalize( );

					u16 * dest = (u16*)vDest;
					dest[0] = Dx360Util::fConvertFloatToHalf( src.x );
					dest[1] = Dx360Util::fConvertFloatToHalf( src.y );

					pos[3] = Dx360Util::fConvertFloatToHalf( src.z );
				}
				else
				{
					sigassert( !"Invalid vertex format for cSemanticNormal!" );
				}
				break;
			case Gfx::tVertexElement::cSemanticTangent:
				if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_4 )
				{
					Math::tVec3f & dest = *(Math::tVec3f *)vDest;
					dest.x = (f32)*vIn++;
					dest.y = (f32)*vIn++;
					dest.z = (f32)*vIn++;
					// sign is propagated

					dest.fNormalize( );
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					Math::tVec3f src;
					src.x = (f32)*vIn++;
					src.y = (f32)*vIn++;
					src.z = (f32)*vIn++;

					src.fNormalize( );

					u16 * dest = (u16*)vDest;
					dest[0] = Dx360Util::fConvertFloatToHalf( src.x );
					dest[1] = Dx360Util::fConvertFloatToHalf( src.y );
					dest[2] = Dx360Util::fConvertFloatToHalf( src.z );
					// sign is propagated
				}
				else
				{
					sigassert( !"Invalid vertex format for cSemanticTangent!" );
				}
				break;
			case Gfx::tVertexElement::cSemanticBinormal:
				break;
			case Gfx::tVertexElement::cSemanticTexCoord:
				if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_2 )
				{
					Math::tVec2f & dest = *(Math::tVec2f*)vDest;
					dest.x = (f32)*vIn++;
					dest.y = (f32)*vIn++;
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_2 )
				{
					u16 * dest = (u16*)vDest;
					dest[0] = Dx360Util::fConvertFloatToHalf( (f32)*vIn++ );
					dest[1] = Dx360Util::fConvertFloatToHalf( (f32)*vIn++ );
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f32_4 )
				{
					Math::tVec4f * dest = (Math::tVec4f*)vDest;
					if( ptr->mSemanticIndex & 1 )
					{
						dest->z = (f32)*vIn++;
						dest->w = (f32)*vIn++;
					}
					else
					{
						dest->x = (f32)*vIn++;
						dest->y = (f32)*vIn++;
					}
				}
				else if( ptr->mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					u16* dest = (u16*)vDest;
					if( ptr->mSemanticIndex & 1 )
					{
						dest[2] = Dx360Util::fConvertFloatToHalf( (f32)*vIn++ );
						dest[3] = Dx360Util::fConvertFloatToHalf( (f32)*vIn++ );
					}
					else
					{
						dest[0] = Dx360Util::fConvertFloatToHalf( (f32)*vIn++ );
						dest[1] = Dx360Util::fConvertFloatToHalf( (f32)*vIn++ );
					}
				}
				break;
			case Gfx::tVertexElement::cSemanticColor:
				{
					sigassert( ptr->mFormat == Gfx::tVertexElement::cFormat_u8_4_Color );

					u32 * dest = (u32*)vDest;

					int a = (int)*vIn++;
					int r = (int)*vIn++;
					int g = (int)*vIn++;
					int b = (int)*vIn++;
					Gfx::tVertexColor color( r, g, b, a );

					*dest = color.mAsU32;

				}break;
			case Gfx::tVertexElement::cSemanticBoneWeights:
				break;
			case Gfx::tVertexElement::cSemanticBoneIndices:
				break;
			}
		}

		sigassert( vIn == vRn + fCalculateDimension( vf ) && "Sanity!" );
	}

	//------------------------------------------------------------------------------
	void tQuadric::fBuild( 
		const void * v0, 
		const void * v1, 
		const void * v2,
		const Gfx::tVertexFormat & vf )
	{
		sigassert( mDimension == fCalculateDimension( vf ) );

		const u32 RnVecSize = sizeof( f64 ) * mDimension;
		f64 * p = (f64 *)alloca( RnVecSize );
		f64 * q = (f64 *)alloca( RnVecSize );
		f64 * r = (f64 *)alloca( RnVecSize );

		// Build the vectors from the incoming data
		fConvertToRn( p, v0, vf );
		fConvertToRn( q, v1, vf );
		fConvertToRn( r, v2, vf );

		// Now that we have our vectors in Rn space build the plane
		f64 * h = (f64*)alloca( RnVecSize );
		f64 * k = (f64*)alloca( RnVecSize );

		Math::fVectorSubtract( h, q, p, mDimension ); // h = q - p
		Math::fVectorSubtract( k, r, p, mDimension ); // k = r - p

		f64 * e1 = (f64*)alloca( RnVecSize );
		f64 * e2 = (f64*)alloca( RnVecSize );
		f64 base, height; // Used for area weighting of the quadric

		// e1 = normalize(h)
		Math::fVectorNormalizeSafe( e1, h, mDimension, base );
		
		// e2 = normalize( k - (e1 dot k) * e1 )
		const f64 e1DotK = Math::fVectorDot( e1, k, mDimension );
		Math::fVectorScale( e2, e1, e1DotK, mDimension ); // e2 = (e1 dot k) * e1
		Math::fVectorSubtract( e2, k, e2, mDimension ); // e2 = k - (e1 dot k) * e1
		Math::fVectorNormalizeSafe( e2, mDimension, height );
		
		// Q = (A, b, c), A = n x n, b = n x 1, c = scalar

		u32 coeffIndex = 0;
		 
		// Construct A == I - e1e1 - e2e2
		for( u32 r = 0; r < mDimension; ++r )
		{
			// Note: c = r as we're only storing the upper half of the matrix 
			// due to it's symmetry about the diagonal
			for( u32 c = r; c < mDimension; ++c )
			{
				const f64 identVal = ( r == c ? 1 : 0 );
				mCoeffs[ coeffIndex++ ] = identVal - e1[c] * e1[r] - e2[c] * e2[r];
			}
		}

		// Construct b == (p dot e1)e1 + (p dot e2)e2 - p
		const f64 pDotE1 = Math::fVectorDot( p, e1, mDimension );
		const f64 pDotE2 = Math::fVectorDot( p, e2, mDimension );
		for( u32 r = 0; r < mDimension; ++r )
			mCoeffs[ coeffIndex++ ] = pDotE1 * e1[r] + pDotE2 * e2[ r ] - p[ r ];

		// Construct c == (p dot p) - sq(p dot e1) - sq(p dot e2 )
		const f64 pDotP = Math::fVectorDot( p, p, mDimension );
		mCoeffs[ coeffIndex++ ] = pDotP - ( pDotE1 * pDotE1 ) - ( pDotE2 * pDotE2 );

		sigassert( coeffIndex == mCoeffCount && "Sanity" );

		// To avoid quadric dependency on tesselation we scale the quadric by an
		// equal proportion of the triangles surface area. This distribute a quadrics weight
		// evenly across all vertices in the triangle, thus the divide by 3.
		f64 weight = ( base * height * 0.5 ) / 3;
		Math::fVectorScale( mCoeffs, mCoeffs, weight, mCoeffCount );
	}

	//------------------------------------------------------------------------------
	b32 tQuadric::fCalculateOptimal( f64 * v ) const
	{
		f64 * A = (f64 *)alloca( sizeof( f64 ) * ( mDimension * mDimension ) );

		fTensor( A );

		if( !Math::fMatrixInvert( A, mDimension ) )
			return false;

		// v = -inv(A) * b
		const f64 * b = mCoeffs + fTensorCoeffCount( );
		Math::fMatrixTransformVector( v, A, b, mDimension );
		Math::fVectorScale( v, v, -1.0, mDimension );
		return true;
	}

	//------------------------------------------------------------------------------
	f64 tQuadric::fCalculateCost( const f64 * v ) const
	{
		f64 cost = 0; // Q(v) = vT * A * v + 2 * bT * v + c

		const u32 tensorCoeffCount = fTensorCoeffCount( );

		// vT * A * v
		for( u32 i = 0; i < mDimension; ++i )
		{	
			f64 val = 0;

			// A * v for row i
			for( u32 j = 0; j < mDimension; ++j )
			{
				// Note: See construction of A for note on c == r
				u32 r = i, c = j;
				if( r > c )
					fSwap( r, c );

				// Calculate the coeff index for r, c
				const u32 coeffIndex = c + ( r * mDimension ) - ( ( 1 + r ) * r ) / 2;
				sigassert( coeffIndex < tensorCoeffCount && "Sanity!" );

				val += mCoeffs[ coeffIndex ] * v[ j ];
			}

			// vT * (A * v) for column i of vT
			cost += v[ i ] * val;
		}


		// 2 * bT * v
		const f64 * b = mCoeffs + tensorCoeffCount;
		cost += 2 * Math::fVectorDot( b, v, mDimension );

		// c
		const f64 * c = b + mDimension;
		cost += *c;

		return cost;
	}

	//------------------------------------------------------------------------------
	tQuadric& tQuadric::operator+=( const tQuadric& other )
	{
		sigassert( mCoeffCount == other.mCoeffCount && "Quadrics must be of same dimension to accumulate" );

		Math::fVectorAdd( mCoeffs, mCoeffs, other.mCoeffs, mCoeffCount );
		return *this;
	}

	//------------------------------------------------------------------------------
	tQuadric& tQuadric::operator=( const tQuadric& Q )
	{
		sigassert( mDimension == Q.mDimension && "Quadrics must be of same dimension to assign" );

		Math::fVectorStore( mCoeffs, Q.mCoeffs, mCoeffCount );

		return *this;
	}

	//------------------------------------------------------------------------------
	tQuadric::tQuadric( u32 dimension, u32 coeffCount )
		: mDimension( (u8)dimension )
		, mCoeffCount( (u8)coeffCount )
	{
		sigassert( dimension < 255 );
		sigassert( coeffCount < 255 );

		Math::fVectorSet( mCoeffs, 0.0, mCoeffCount );
	}

	//------------------------------------------------------------------------------
	u32 tQuadric::fTensorCoeffCount( ) const
	{
		// The number of coeffs for A is equal to 1 + 2 + .... + dim == (1+dim)*(dim/2)
		return ( ( 1 + mDimension ) * mDimension ) / 2;
	}

	//------------------------------------------------------------------------------
	void tQuadric::fTensor( f64 * m ) const
	{
		u32 coeffIndex = 0;
		for( u32 r = 0; r < mDimension; ++r )
		{
			// NB: See construction of A above for c = r note.
			for( u32 c = r; c < mDimension; ++c )
			{
				const f64 & coeff = mCoeffs[ coeffIndex++ ];
				m[ r * mDimension + c ] = coeff;

				// Handle symmetric elements
				if( c != r )
					m[ c * mDimension + r ] = coeff;
			}
		}
	}

	//------------------------------------------------------------------------------
	// tFace
	//------------------------------------------------------------------------------
	u32 tFace::fOtherVert( u32 notVId0, u32 notVId1 ) const
	{
		if( mVertIds[ 0 ] != notVId0 && mVertIds[ 0 ] != notVId1 )
			return mVertIds[ 0 ];
		else if( mVertIds[ 1 ] != notVId0 && mVertIds[ 1 ] != notVId1 )
			return mVertIds[ 1 ];
		else
		{
			sigassert( mVertIds[ 2 ] != notVId0 && mVertIds[ 2 ] != notVId1 );
			return mVertIds[ 2 ];
		}
	}

	//------------------------------------------------------------------------------
	void tFace::fOtherVerts( u32 notVId, u32 & vIdOut0, u32 & vIdOut1 ) const
	{
		if( mVertIds[ 0 ] == notVId )
		{
			vIdOut0 =  mVertIds[ 1 ];
			vIdOut1 =  mVertIds[ 2 ];
		}
		else if( mVertIds[ 1 ] == notVId )
		{
			vIdOut0 =  mVertIds[ 0 ];
			vIdOut1 =  mVertIds[ 2 ];
		}
		else
		{
			sigassert( mVertIds[ 2 ] == notVId );
			vIdOut0 =  mVertIds[ 0 ];
			vIdOut1 =  mVertIds[ 1 ];
		}
	}

	//------------------------------------------------------------------------------
	u32 tFace::fReplaceVert( u32 vOld, u32 vNew )
	{
		u32 * v = mVertIds.fFind( vOld ); 
		sigassert( v && "Sanity!" );
		*v = vNew;

		return fPtrDiff( v, mVertIds.fBegin( ) );
	}

	//------------------------------------------------------------------------------
	// tEdge
	//------------------------------------------------------------------------------
	u32 tEdge::fOtherVert( u32 vId ) const
	{
		if( mVertIds[ 0 ] == vId )
			return mVertIds[ 1 ];
		
		sigassert( mVertIds[ 1 ] == vId && "Sanity!" );
		return mVertIds[ 0 ];
	}

	//------------------------------------------------------------------------------
	void tEdge::fReplaceVert( u32 vOld, u32 vNew )
	{
		if( mVertIds[ 0 ] == vOld )
			mVertIds[ 0 ] = vNew;
		else
		{
			sigassert( mVertIds[ 1 ] == vOld && "Sanity!" );
			mVertIds[ 1 ] = vNew;
		}
	}

	//------------------------------------------------------------------------------
	// tMeshConnectivity
	//------------------------------------------------------------------------------
	tMeshConnectivity::tMeshConnectivity( 
		const Gfx::tVertexFormat & vf, 
		const void * vb, u32 vbCount, 
		const u32 * ib, u32 ibCount,
		const tDecimationControls & controls )
		: mControls( controls )
		, mVertexFormat( vf )
		, mQuadricDimension( tQuadric::fCalculateDimension( vf ) )
		, mActiveVertCount( vbCount )
		, mActiveFaceCount( ibCount / 3 )
		, mCancelledEdgeCount( 0 )
		, mHighestCost( 0.f )
		, mAverageCost( 0.f )
		, mLowestCascadeCancelledCost( Math::cInfinity )
		, mHelperQuadric( tQuadric::fNew( mQuadricDimension ) )
		
	{
		sigassert( controls.mTargetPolicy >= 0 && controls.mTargetPolicy < tDecimationControls::cTargetPolicyCount );
		sigassert( ibCount % 3 == 0 && "Improper triangle list" );

		fBuild( vb, ib );
	}

	//------------------------------------------------------------------------------
	tMeshConnectivity::~tMeshConnectivity( )
	{
		tQuadric::fDelete( mHelperQuadric );
	}

	//------------------------------------------------------------------------------
	void tMeshConnectivity::fBuild( const void * vb, const u32 * ib)
	{
		const u32 quadricCoeffs = tQuadric::fCalculateCoeffsNeeded( mQuadricDimension );
		const u32 quadricSize = tQuadric::fCalculateSize( quadricCoeffs );
		const u32 vertStride = mVertexFormat.fVertexSize( );

		// Vert allocation
		const u32 vbCount = mActiveVertCount;
		mVerts.fSetCount( vbCount );
		mVertDataBuffer.fNewArray( ( vbCount * quadricSize ) + ( vbCount * vertStride ) );

		// Face allocation
		mFaces.fSetCapacity( mActiveFaceCount );

		// Initialize vert data and quadric object
		const byte * cV = (const byte *)vb;
		byte * vertData = mVertDataBuffer.fBegin( );
		for(u32 v = 0; v < vbCount; ++v, cV += vertStride )
		{
			tVertex & vert = mVerts[ v ];

			// Vert data
			vert.mData = vertData; 
			fMemCpy( vert.mData, cV, vertStride );
			vertData += vertStride;

			// Quadric
			vert.mQuadric = tQuadric::fInPlaceNew( vertData, mQuadricDimension, quadricCoeffs );
			vertData += quadricSize;
		}

		sigassert( vertData == mVertDataBuffer.fEnd( ) && "Sanity!" );
		sigassert( cV == (const byte*)vb + mActiveVertCount * vertStride && "Sanity!" );

		// Initialize faces and build vertex quardrics and face lists
		const u32 * cI = ib;
		for(u32 f = 0; f < mActiveFaceCount; ++f )
		{
			tFace face;

			face.mVertIds[ 0 ] = *cI++;
			face.mVertIds[ 1 ] = *cI++;
			face.mVertIds[ 2 ] = *cI++;

			// Skip degenerate triangles
			if( face.mVertIds[ 0 ] == face.mVertIds[ 1 ] ||
				face.mVertIds[ 0 ] == face.mVertIds[ 2 ] || 
				face.mVertIds[ 1 ] == face.mVertIds[ 2 ] )
			{
				continue;
			}

			sigassert( face.mVertIds[ 0 ] < vbCount && "Out of bounds index" );
			sigassert( face.mVertIds[ 1 ] < vbCount && "Out of bounds index" );
			sigassert( face.mVertIds[ 2 ] < vbCount && "Out of bounds index" );

			// Get the verts so we can update their info with the face info
			tVertex & v0 = mVerts[ face.mVertIds[ 0 ] ];
			tVertex & v1 = mVerts[ face.mVertIds[ 1 ] ];
			tVertex & v2 = mVerts[ face.mVertIds[ 2 ] ];

			// Update the quadrics
			mHelperQuadric->fBuild( v0.mData, v1.mData, v2.mData, mVertexFormat );
			( *v0.mQuadric ) += *mHelperQuadric;
			( *v1.mQuadric ) += *mHelperQuadric;
			( *v2.mQuadric ) += *mHelperQuadric;

			const u32 faceIndex = mFaces.fCount( );
			mFaces.fPushBack( face );

			// Update the face ids on the verts
			v0.mFaceIds.fPushBack( faceIndex );
			v1.mFaceIds.fPushBack( faceIndex );
			v2.mFaceIds.fPushBack( faceIndex );
		}
		sigassert( cI == ib + mActiveFaceCount * 3 && "Sanity!" );

		// Capture the actual face count after degenerates were skipped
		mActiveFaceCount = mFaces.fCount( );

		// Maps ( v0 << 32 | v1 ) where v0 < v1 to the edge index
		tHashTable<u64, u32> edgeMap;

		// Build the edge information
		for( u32 f = 0; f < mActiveFaceCount; ++f )
		{
			tFace & face = mFaces[ f ];
			for( u32 i = 0; i < 3; ++i )
			{
				u32 vi0 = face.mVertIds[ i ];
				u32 vi1 = face.mVertIds[ ( i + 1 ) % 3 ];

				// Make sure we consider vi0 as the lower index
				if( vi0 > vi1 )
					fSwap( vi0, vi1 );

				// Build the edge key
				u64 edgeKey = ( u64(vi0) << 32 ) | vi1; 

				// If the edge doesn't exist create it
				if( !edgeMap.fFind( edgeKey ) )
				{
					tEdge edge; 
					edge.mVertIds[ 0 ] = vi0;
					edge.mVertIds[ 1 ] = vi1;

					tVertex & v0 = mVerts[ vi0 ];
					tVertex & v1 = mVerts[ vi1 ];

					const u32 newEdgeId = mEdges.fCount( );

					// Add the edge id to the verts that compose it
					v0.mEdgeIds.fPushBack( newEdgeId );
					v1.mEdgeIds.fPushBack( newEdgeId );

					mEdges.fPushBack( edge );
					edgeMap.fInsert( edgeKey, newEdgeId );
				}
			}
		}

		// The edgemap is no longer needed so free it's memory
		edgeMap.fClear( );

		const u32 edgeCount = mEdges.fCount( );

		// Apply boundary restrictions by locking verts that are used in
		// boundary edges
		for( u32 e = 0; e < edgeCount; ++e )
		{
			const tEdge & edge = mEdges[ e ];
			const u32 v0Id = edge.mVertIds[ 0 ];
			const u32 v1Id = edge.mVertIds[ 1 ] ;

			tVertex & v0 = mVerts[ v0Id ];
			tVertex & v1 = mVerts[ v1Id ];

			// Find two faces that use both these verts, to do this we only need to run through
			// the faces of one of the verts
			u32 facesToFind = 2;
			const u32 faceCount = v0.mFaceIds.fCount( );
			for( u32 f = 0; f < faceCount && facesToFind > 0; ++f )
			{
				const tFace & face = mFaces[ v0.mFaceIds[ f ] ];
				if( face.mVertIds.fFind( v1Id ) )
					--facesToFind;
			}

			// This edge cannot be contracted, thus the verts it uses cannot 
			// be destroyed or moved
			if( facesToFind > 0 )
			{
				v0.mLockedReasons |= tVertex::cLockedReasonBoundary;
				v1.mLockedReasons |= tVertex::cLockedReasonBoundary;
			}
		}
		
		// We break the building of edge costs into two steps so that
		// we calculate a valid highest edge contraction cost before
		// using that cost to sort the edges that violate neighborhood
		// consistency checks

		// Initialize edge target buffers if needed
		if( fTargetPolicyRequiresData( ) )
		{
			mEdgeTargetBuffer.fNewArray( edgeCount * mQuadricDimension );
			f64 * targetData = mEdgeTargetBuffer.fBegin( );
			for( u32 e = 0; e < edgeCount; ++e, targetData += mQuadricDimension )
			{
				tEdge & edge = mEdges[ e ];
				edge.mTarget = targetData;
			}

			sigassert( targetData == mEdgeTargetBuffer.fEnd( ) && "Sanity!" );
		}

		// Initialize the edges with target and cost, we do this
		// separately from the consistency checks so that we can compute
		// the highest cost from target selection
		for( u32 e = 0; e < edgeCount; ++e )
			fSelectTarget( mEdges[ e ] );

		// Apply consistency checks to the edges and add to the priority queue
		for( u32 e = 0; e < edgeCount; ++e )
		{
			tEdge & edge = mEdges[ e ];

			// Cancelled edge contractions have double locked verts and thus cannot be applied
			if( !edge.mCancelledReasons )
			{
				// Apply checks and add
				fApplyConsistencyChecks( edge );
				mCandidates.fPut( &edge );
			}

			// Accumulate for average
			mAverageCost += edge.mContractionCost;
		}

		// Calculate average
		mAverageCost /= mEdges.fCount( );

		// Validate that we built a cohesive structure
		fValidateBuild( );
	}

	//------------------------------------------------------------------------------
	void tMeshConnectivity::fValidateBuild( )
	{
#ifdef mesh_connectivity_build_validation

		// Check that all the face to vert to face cross references
		// as well as the active face count
		u32 activeFaceCount = 0;
		const u32 faceCount = mFaces.fCount( );
		for( u32 f = 0; f < faceCount; ++f )
		{
			const tFace & face = mFaces[ f ];
			if( face.mIsDead )
				continue;

			++activeFaceCount;

			for( u32 v = 0; v < face.mVertIds.cDimension; ++v )
			{
				const tVertex & vert = mVerts[ face.mVertIds[ v ] ];
				sigassert( vert.mFaceIds.fFind( f ) );
			}
		}
		sigassert( activeFaceCount == mActiveFaceCount );

		// Check all the edge to vert to edge cross references
		// as well as the active edge count
		u32 activeEdgeCount = 0;
		const u32 edgeCount = mEdges.fCount( );
		for( u32 e = 0; e < edgeCount; ++e )
		{
			const tEdge & edge = mEdges[ e ];
			if( edge.mIsDead )
				continue;

			++activeEdgeCount;

			sigassert( edge.mVertIds[ 0 ] != edge.mVertIds[ 1 ] );
			for( u32 v = 0; v < edge.mVertIds.cDimension; ++v )
			{
				const tVertex & vert = mVerts[ edge.mVertIds[ v ] ];
				sigassert( vert.mEdgeIds.fFind( e ) );
			}
		}
		sigassert( activeEdgeCount == mCandidates.fCount( ) + mCancelledEdgeCount );

		// Check all the vert to * cross references as well as the active vert count
		u32 activeVertCount = 0;
		const u32 vertCount = mVerts.fCount( );
		for( u32 v = 0; v < vertCount; ++v )
		{
			const tVertex & vert = mVerts[ v ];
			if( vert.mIsDead )
				continue;

			++activeVertCount;
		
			// Vert to face to vert
			const u32 vertFaceCount = vert.mFaceIds.fCount( );
			for( u32 f = 0; f < vertFaceCount; ++f )
			{
				const tFace & face = mFaces[ vert.mFaceIds[ f ] ];
				sigassert( face.mVertIds.fFind( v ) );
			}

			// Vert to edge to vert
			const u32 vertEdgeCount = vert.mEdgeIds.fCount( );
			for( u32 e = 0; e < vertEdgeCount; ++e )
			{
				const tEdge & edge = mEdges[ vert.mEdgeIds[ e ] ];
				sigassert( edge.mVertIds.fFind( v ) );
			}
		}
		sigassert( activeVertCount == mActiveVertCount );

#ifdef mesh_connectivity_cascade_validation

		// Validate the contractions adhere to cascade restrictions
		if( mControls.fRestrictsCascades( ) )
		{
			tGrowableArray<u32> facesUsed;

			u32 startContraction = 0;
			const u32 cascadeMarkerCount = mCascadeResetMarkers.fCount( );
			for( u32 m = 0; m < cascadeMarkerCount; ++m )
			{
				facesUsed.fSetCount( 0 );

				const u32 endContraction = mCascadeResetMarkers[ m ];
				for( u32 c = startContraction; c < endContraction; ++c )
				{
					const tContractionRecordPtr & record = mContractions[ c ];

					const u32 changedFaceCount = record->mFaceChanges.fCount( );
					for( u32 f = 0; f < changedFaceCount; ++f )
					{
						const u32 faceIndex = record->mFaceChanges[ f ].mA ;
						sigassert( !facesUsed.fFind( faceIndex ) );
						facesUsed.fPushBack( faceIndex );
					}

					const u32 deadFaceCount = record->mFacesDestroyed.fCount( );
					for( u32 f = 0; f < deadFaceCount; ++f )
					{
						const u32 faceIndex = record->mFacesDestroyed[ f ];
						sigassert( !facesUsed.fFind( faceIndex ) );
						facesUsed.fPushBack( faceIndex );
					}
				}

				startContraction = endContraction;
			}
		}
#endif // mesh_connectivity_cascade_validation

#endif // mesh_connectivity_build_validation
	}

	//------------------------------------------------------------------------------
	b32 tMeshConnectivity::fTargetPolicyRequiresData( )
	{
		switch( mControls.mTargetPolicy )
		{
		case tDecimationControls::cTargetPolicyOptimal:
			return true;
		case tDecimationControls::cTargetPolicySubset:
			return false;
		default:
			sig_nodefault( );
			return false;
		}
	}

	//------------------------------------------------------------------------------
	f32 tMeshConnectivity::fNextDecimationCost( ) const 
	{ 
		if( mCandidates.fCount( ) )
			return mCandidates.fViewTop( )->mContractionCost;
		
		return Math::cInfinity; 
	}

	//------------------------------------------------------------------------------
	b32 tMeshConnectivity::fDecimate( )
	{
		if( !mCandidates.fCount( ) )
			return false;

		// Check for cascade tolerance
		if( mCancelledEdgeCount && mControls.fRestrictsCascades( ) )
		{
			// None are allowed
			if( mControls.mCascadeTolerance == 0 )
			{
				return false;
			}

			// If its infinity they're all allowed
			else if( mControls.mCascadeTolerance != Math::cInfinity )
			{
				// Test variance
				const f32 availableCost = mCandidates.fViewTop( )->mContractionCost;
				const f32 maxCost = mLowestCascadeCancelledCost + mControls.mCascadeTolerance * mAverageCost;
				if( availableCost > maxCost )
					return false;
			}
		}

		tContractionRecordPtr record( new tContractionRecord );

		// The edge to contract
		tEdge * edge = mCandidates.fGet( );

		const u32 v0Id = edge->mVertIds[ 0 ];
		const u32 v1Id = edge->mVertIds[ 1 ];
		record->mVertChanged.mId = v0Id;
		record->mVertDestroyed.mId = v1Id; 

		tVertex & v0 = mVerts[ v0Id ];
		tVertex & v1 = mVerts[ v1Id ];
		
		// Accumulate the error metric
		*v0.mQuadric += *v1.mQuadric;

		// Move the vertex v0 and capture data
		if( fTargetPolicyRequiresData( ) )
		{
			record->mVertChanged.mData.fInitialize( (const byte*)v0.mData, mVertexFormat.fVertexSize( ) );
			tQuadric::fConvertFromRn( v0.mData, edge->mTarget, mVertexFormat );
		}
		record->mVertDestroyed.mData.fInitialize( (const byte*)v1.mData, mVertexFormat.fVertexSize( ) );

		// Update edges that use v1 by killing the dead ones and pointing the
		// changed ones to v0
		tGrowableArray<u32> newV0EdgeIds; // To prevent running over edges that we've already updated
		const u32 v1EdgeCount = v1.mEdgeIds.fCount( );
		for( u32 e1 = 0; e1 < v1EdgeCount; ++e1 )
		{
			tEdge & edge1 = mEdges[ v1.mEdgeIds[ e1 ] ];
			sigassert( edge1.mVertIds[0] != edge1.mVertIds[1] );

			const u32 e1OtherVId = edge1.fOtherVert( v1Id );
			b32 e1IsDead = false;

			// If edge1's other vert shares an edge with v0 then
			// we know that edge1 is dead
			const u32 v0EdgeCount = v0.mEdgeIds.fCount( );
			for( u32 e0 = 0; e0 < v0EdgeCount; ++e0 )
			{
				tEdge & edge0 = mEdges[ v0.mEdgeIds[ e0 ] ];

				if( e1OtherVId == v0Id || edge0.fOtherVert( v0Id ) == e1OtherVId )
				{
					e1IsDead = true;
					mCandidates.fErase( &edge1 );
					edge1.mIsDead = true;

					// Remove from the average
					u32 ai = mCandidates.fCount( ) + mCancelledEdgeCount;
					mAverageCost = ( ( ai + 1 ) * mAverageCost - edge1.mContractionCost ) / ai;
					
					b32 found = mVerts[ e1OtherVId ].mEdgeIds.fFindAndErase( v1.mEdgeIds[ e1 ] );
					sigassert( found );
					break;
				}
			}
			

			// Otherwise, update the edge to refer to v0 instead of v1
			if( !e1IsDead )
			{
				edge1.fReplaceVert( v1Id, v0Id );
				sigassert( edge1.mVertIds[0] != edge1.mVertIds[1] );
				newV0EdgeIds.fPushBack( v1.mEdgeIds[ e1 ] );
			}
		}

		// Add the ids for edges that now share v0
		v0.mEdgeIds.fJoin( newV0EdgeIds );

		// Update faces that use v1 by killing the dead ones and pointing the
		// changed ones to v0
		const u32 v1FaceCount = v1.mFaceIds.fCount( );
		for( u32 f1 = 0; f1 < v1FaceCount; ++f1 )
		{
			tFace & face = mFaces[ v1.mFaceIds[ f1 ] ];

			// Is the face dead?
			if( face.mVertIds.fFind( v0Id ) )
			{
				record->mFacesDestroyed.fPushBack( v1.mFaceIds[ f1 ] );

				// Remove the dead face from v0
				v0.mFaceIds.fFindAndErase( v1.mFaceIds[ f1 ] );
				
				// Remove the dead face from the other vert it references
				mVerts[ face.fOtherVert( v0Id, v1Id ) ].mFaceIds.fFindAndErase( v1.mFaceIds[ f1 ] );

				face.mIsDead = true;
				--mActiveFaceCount;
			}

			// If it's not dead then we update the face to refer to the new vert
			else
			{
				u32 idxChanged = face.fReplaceVert( v1Id, v0Id );
				v0.mFaceIds.fPushBack( v1.mFaceIds[ f1 ] );

				record->mFaceChanges.fPushBack( tFaceChange( v1.mFaceIds[ f1 ], idxChanged ) );

				// If we're restricting cacades then all the verts that this face uses
				// need to be locked so that this face cannot be used in any further
				// decimations
				if( mControls.fRestrictsCascades( ) )
				{
					for( u32 v = 0; v < 3; ++v )
						mVerts[ face.mVertIds[ v ] ].mLockedReasons |= tVertex::cLockedReasonCascade;
				}
			}
		}

		// Update all the remaining edges affected by this change
		// NOTE: While updated edges may increase the highest edge contraction 
		// cost we do not break this into multiple steps, but rely 
		// instead on a sense of decimation coherency
		const u32 v0EdgeCount = v0.mEdgeIds.fCount( );
		for( u32 e = 0; e < v0EdgeCount; ++e )
		{
			tEdge & edge = mEdges[ v0.mEdgeIds[ e ] ];

			fUpdateCandidate( &edge );

			// Update the edges that share verts with edges that refer to v0
			const u32 otherVId = edge.fOtherVert( v0Id );
			const tVertex & otherVert = mVerts[ otherVId ];
			const u32 otherVEdgeCount = otherVert.mEdgeIds.fCount( );
			for( u32 otherE = 0; otherE < otherVEdgeCount; ++otherE )
			{
				tEdge & otherEdge = mEdges[ otherVert.mEdgeIds[ otherE ] ];

				// We're going to update this edge already because it uses v0
				if( otherEdge.fOtherVert( otherVId ) == v0Id )
					continue;

				fUpdateCandidate( &otherEdge );
			}
		}

		// edge is no longer a valid edge
		sigassert( edge->mIsDead );

		// v1 is no longer a valid vertex
		v1.mIsDead = true;
		--mActiveVertCount;

		mContractions.fPushBack( record );
		fValidateBuild( ); // Ensure we still have a valid connected mesh

		return true;
	}

	//------------------------------------------------------------------------------
	u32 tMeshConnectivity::fResetCascades( )
	{
		// We don't restrict
		if( !mControls.fRestrictsCascades( ) )
			return 0;

		// We haven't done any contractions either at all or since the last marker
		if( !mContractions.fCount( ) || 
			( mCascadeResetMarkers.fCount( ) && mCascadeResetMarkers.fBack( ) == mContractions.fCount( ) ) )
			return 0;

		mCascadeResetMarkers.fPushBack( mContractions.fCount( ) );

		// Remove cascade locks from the verts
		const u32 vertCount = mVerts.fCount( );
		for( u32 v = 0; v < vertCount; ++v )
			mVerts[ v ].mLockedReasons &= ~tVertex::cLockedReasonCascade;

		u32 startCancelledCount = mCancelledEdgeCount;

		// Update edges that were cancelled due to cascade restrictions
		const u32 edgeCount = mEdges.fCount( );
		for( u32 e = 0; e < edgeCount; ++e )
		{
			tEdge & edge = mEdges[ e ];
			if( edge.mIsDead )
				continue;
			
			// Only reset edges that were cancelled due to cascade restrictions
			if( edge.mCancelledReasons & tVertex::cLockedReasonCascade )
			{
				// Remove the cascade reason
				edge.mCancelledReasons &= ~tVertex::cLockedReasonCascade;

				// If the edge still has reasons for being cancelled see 
				// if they're still valid
				if( edge.mCancelledReasons )
				{
					const tVertex & v0 = mVerts[ edge.mVertIds[ 0 ] ];
					const tVertex & v1 = mVerts[ edge.mVertIds[ 1 ] ];

					// If either vert is not locked then this edge should be in
					// play and the persisting reasons are not valid
					if( !v0.mLockedReasons || !v1.mLockedReasons )
						edge.mCancelledReasons = 0;
				}

				// If the edge is no longer cancelled reinsert it in the candidates list
				// and update it
				if( !edge.mCancelledReasons )
				{
					--mCancelledEdgeCount; // Update the count of cancelled edges
					sigassert( !mCandidates.fContains( &edge ) && "Sanity!" );

					mCandidates.fPut( &edge );
					fUpdateCandidate( &edge );
				}
			}
		}

		// There are no longer any edges cancelled due to cascade restrictions
		mLowestCascadeCancelledCost = Math::cInfinity;
		
		fValidateBuild( ); 

		// Returns the number of edges reinstated
		return startCancelledCount - mCancelledEdgeCount;
	}

	//------------------------------------------------------------------------------
	u32 tMeshConnectivity::fCalculateNewFaceCount( u32 triOffset, u32 oldNumTris ) const
	{	
		sigassert( triOffset + oldNumTris <= mFaces.fCount( ) );

		u32 newNumTris = 0;
		const u32 triEnd = triOffset + oldNumTris;
		for( u32 f = triOffset; f < triEnd; ++f )
		{
			if( !mFaces[ f ].mIsDead )
				++newNumTris;
		}

		return newNumTris;
	}

	//------------------------------------------------------------------------------
	void tMeshConnectivity::fCapture( 
			Gfx::tGeometryBufferSysRam & vb, 
			Gfx::tIndexBufferSysRam &ib, 
			tDynamicArray<u32> * outIndexMap, 
			tDynamicArray<u32> * outFaceMap ) const
	{
		// Allocate the space
		vb.fAllocate( mVertexFormat, mActiveVertCount );

		const Gfx::tIndexFormat ibFormat = Gfx::tIndexFormat::fCreateAppropriateFormat( 
			Gfx::tIndexFormat::cPrimitiveTriangleList, mActiveVertCount );
		ib.fAllocate( ibFormat, mActiveFaceCount * 3 );

		// Copy the verts and build the index mapping
		tDynamicArray<u32> indexMapInternal;
		tDynamicArray<u32> & indexMap = ( outIndexMap ? *outIndexMap : indexMapInternal );
		indexMap.fNewArray( mVerts.fCount( ) );
		indexMap.fFill( ~0 );

		const u32 vertCount = mVerts.fCount( );
		const u32 vertStride = vb.fVertexFormat( ).fVertexSize( );
		byte * vDest = vb.fBegin( );
		for( u32 v = 0, vMapped = 0; v < vertCount; ++v )
		{
			const tVertex & vert = mVerts[ v ];
			if( !vert.mIsDead )
			{
				indexMap[ v ] = vMapped++;
				fMemCpy( vDest, vert.mData, vertStride );
				vDest += vertStride;
			}
		}
		sigassert( vDest == vb.fEnd( ) && "Sanity!" );

		// Copy the indices and build the optional face mapping
		u32 iDestIdx = 0;
		const u32 faceCount = mFaces.fCount( );
		if( outFaceMap )
		{
			outFaceMap->fNewArray( faceCount );
			outFaceMap->fFill( ~0 );
		}

		for( u32 f = 0, fMapped = 0; f < faceCount; ++f )
		{
			const tFace & face = mFaces[ f ];
			if( !face.mIsDead )
			{
				// Build the face map
				if( outFaceMap )
					(*outFaceMap)[f] = fMapped++;

				u32 mappedIndices[ 3 ] = {
					indexMap[ face.mVertIds[ 0 ] ],
					indexMap[ face.mVertIds[ 1 ] ],
					indexMap[ face.mVertIds[ 2 ] ]
				};

				ib.fSetIndices( iDestIdx, mappedIndices, 3 );
				iDestIdx += 3;
			}
		}
		sigassert( iDestIdx == ib.fIndexCount( ) && "Sanity!" );
			
	}

	//------------------------------------------------------------------------------
	void tMeshConnectivity::fUpdateCandidate( tEdge * edge )
	{
		// Nothing to do for cancelled edges
		if( edge->mCancelledReasons )
			return;
		
		// Remove from the average
		u32 ai = mCandidates.fCount( ) + mCancelledEdgeCount - 1;
		mAverageCost = ( ( ai + 1 ) * mAverageCost - edge->mContractionCost ) / ai;


		if( fSelectTarget( *edge ) )
		{
			fApplyConsistencyChecks( *edge );
			mCandidates.fUpdate( edge );
		}

		// Add back to the average
		mAverageCost = mAverageCost + ( edge->mContractionCost - mAverageCost ) / ( ai + 1 );
	}

	//------------------------------------------------------------------------------
	b32 tMeshConnectivity::fSelectTarget( tEdge & edge )
	{
		sigassert( !edge.mCancelledReasons && "Sanity!" );

		const tVertex * v0 = &mVerts[ edge.mVertIds[ 0 ] ];
		const tVertex * v1 = &mVerts[ edge.mVertIds[ 1 ] ];

		// If both verts on this edge are locked this edge must be cancelled
		if( v0->mLockedReasons && v1->mLockedReasons )
		{
			edge.mCancelledReasons = ( v0->mLockedReasons | v1->mLockedReasons );
			mCandidates.fErase( &edge );
			++mCancelledEdgeCount;

			// Track the lowest cost for a cascade cancelled edge. Boundary locked verts
			// are locked in the build phase and are thus never locked for cascade reasons.
			// It follows then that if either vert is locked due to a cascade restriction 
			// then we can consider the edge cancelled for that reason
			if( ( edge.mCancelledReasons & tVertex::cLockedReasonCascade ) && 
				edge.mContractionCost < mLowestCascadeCancelledCost )
				mLowestCascadeCancelledCost = edge.mContractionCost;

			return false;
		}

		// If v1 is locked then v0 must not be and we need a swap
		if( v1->mLockedReasons )
		{
			fSwap( v0, v1 );
			fSwap( edge.mVertIds[ 0 ], edge.mVertIds[ 1 ] );
		}

		// If v0 is locked then we must use it's data
		if( v0->mLockedReasons )
		{
			// Accumulate quadrics for measuring contraction error
			*mHelperQuadric = *v0->mQuadric;
			*mHelperQuadric += *v1->mQuadric;

			f64 * target = edge.mTarget;
			if( !target )
				target = (f64*)alloca( sizeof(f64) * mQuadricDimension );

			tQuadric::fConvertToRn( target, v0->mData, mVertexFormat );
			edge.mContractionCost = mHelperQuadric->fCalculateCost( target );
		}

		// Otherwise select the target by policy
		else
		{
			switch( mControls.mTargetPolicy )
			{
			case tDecimationControls::cTargetPolicyOptimal:
				fSelectOptimalTarget( edge );
				break;
			case tDecimationControls::cTargetPolicySubset:
				fSelectSubsetTarget( edge );
				break;
			default:
				sig_nodefault( );
			}
		}

		// Update the highest cost contraction
		if( edge.mContractionCost > mHighestCost )
			mHighestCost = edge.mContractionCost;

		return true;
	}

	//------------------------------------------------------------------------------
	void tMeshConnectivity::fSelectOptimalTarget( tEdge & edge )
	{
		const tVertex & v0 = mVerts[ edge.mVertIds[ 0 ] ];
		const tVertex & v1 = mVerts[ edge.mVertIds[ 1 ] ];

		// Accumulate quadrics for measuring contraction error
		*mHelperQuadric = *v0.mQuadric;
		*mHelperQuadric += *v1.mQuadric;

		// Attempt optimal solution
		if( mHelperQuadric->fCalculateOptimal( edge.mTarget ) )
		{
			edge.mContractionCost = mHelperQuadric->fCalculateCost( edge.mTarget );
		}

		// If A was singular the optimal cannot be found. Check v0, v1, and halfway
		else
		{
			const u32 RnVecSize = sizeof( f64 ) * mQuadricDimension;
			f64 * v1Target = (f64*)alloca( RnVecSize );
			f64 * halfWayTarget = (f64*)alloca( RnVecSize );

			// Calculate for v0, assume v0 will be cheaper
			tQuadric::fConvertToRn( edge.mTarget, v0.mData, mVertexFormat );
			edge.mContractionCost = mHelperQuadric->fCalculateCost( edge.mTarget );
			
			// Calculate for v1
			tQuadric::fConvertToRn( v1Target, v1.mData, mVertexFormat );
			const f64 v1Cost = mHelperQuadric->fCalculateCost( v1Target );

			// Calculate halfway it's cost
			Math::fVectorLerp( halfWayTarget, edge.mTarget, v1Target, 0.5, mQuadricDimension );
			const f64 halfWayCost = mHelperQuadric->fCalculateCost( halfWayTarget );

			// Assumption of v0 may have been wrong
			if( v1Cost < edge.mContractionCost && v1Cost < halfWayCost )
			{
				edge.mContractionCost = v1Cost;
				Math::fVectorStore( edge.mTarget, v1Target, mQuadricDimension );
			}
			else if( halfWayCost < edge.mContractionCost )
			{
				edge.mContractionCost = halfWayCost;
				Math::fVectorStore( edge.mTarget, halfWayTarget, mQuadricDimension );
			}
		}
	}

	//------------------------------------------------------------------------------
	void tMeshConnectivity::fSelectSubsetTarget( tEdge & edge )
	{
		const tVertex & v0 = mVerts[ edge.mVertIds[ 0 ] ];
		const tVertex & v1 = mVerts[ edge.mVertIds[ 1 ] ];

		// Accumulate quadrics for measuring contraction error
		*mHelperQuadric = *v0.mQuadric;
		*mHelperQuadric += *v1.mQuadric;

		// Determine which of v0, v1 introduces less error
		const u32 RnVecSize = sizeof( f64 ) * mQuadricDimension;
		f64 * v0Target = (f64*)alloca( RnVecSize );
		f64 * v1Target = (f64*)alloca( RnVecSize );

		// Calculate for v0
		tQuadric::fConvertToRn( v0Target, v0.mData, mVertexFormat );
		edge.mContractionCost = mHelperQuadric->fCalculateCost( v0Target );
		
		// Calculate for v1
		tQuadric::fConvertToRn( v1Target, v1.mData, mVertexFormat );
		const f64 v1Cost = mHelperQuadric->fCalculateCost( v1Target );

		// If v1 costs less than v0 then swap the edge indices as
		// all collapses happen in subset by converging to v0
		if( v1Cost < edge.mContractionCost)
		{
			edge.mContractionCost = v1Cost;
			fSwap( edge.mVertIds[ 0 ], edge.mVertIds[ 1 ] );
		}
	}

	//------------------------------------------------------------------------------
	void tMeshConnectivity::fApplyConsistencyChecks( tEdge & edge )
	{
		// No check, the edge already has been cancelled
		sigassert( !edge.mCancelledReasons );

		// If the policy requires no data then the verts do not move
		// and we do not need to apply consistency checks
		if( !fTargetPolicyRequiresData( ) )
			return;

		// Do our best to prevent fold over
		const u32 v0Id = edge.mVertIds[ 0 ];
		const u32 v1Id = edge.mVertIds[ 1 ];

		const tVertex & v0 = mVerts[ v0Id ];
		const tVertex & v1 = mVerts[ v1Id ];

		// Test the neighborhood around v0, v1, excluding shared faces
		// accumulating the cost of any transgressions
		f32 neighborHoodCost = fTestNeighborhoodConsistency( 
			edge.mTarget, v0Id, v1.mFaceIds.fBegin( ), v1.mFaceIds.fCount( ) );

		neighborHoodCost += fTestNeighborhoodConsistency( 
			edge.mTarget, v1Id, v0.mFaceIds.fBegin( ), v0.mFaceIds.fCount( ) );

		// All neighborhood cost incurring edge contractions must happen last
		// and are sorted by the standard contraction cost adjusted for the neighborhood
		// cost afterward
		if( neighborHoodCost > 0 )
			edge.mContractionCost += mHighestCost + neighborHoodCost; 
	}

	//------------------------------------------------------------------------------
	f32 tMeshConnectivity::fTestNeighborhoodConsistency( 
			const f64 * vRn, const u32 vId, 
			const u32 * faceIdsToIgnore, u32 ignoreCount )
	{
		const tArraySleeve<const u32> ignore( faceIdsToIgnore, ignoreCount );
		const tVertex & v = mVerts[ vId ];

		const u32 RnVecSize = sizeof( f64 ) * mQuadricDimension;
		f64 * p = (f64*)alloca( RnVecSize );
		f64 * q = (f64*)alloca( RnVecSize );
		f64 * r = (f64*)alloca( RnVecSize );

		tQuadric::fConvertToRn( r, v.mData, mVertexFormat ); // convert v0 to Rn in r

		// For building the plane
		f64 * h = (f64*)alloca( RnVecSize );
		f64 * k = (f64*)alloca( RnVecSize );
		f64 * e1 = (f64*)alloca( RnVecSize );
		f64 * e2 = (f64*)alloca( RnVecSize );

		f32 neighborhoodCost = 0;

		// Check the planes around v0 to ensure the target lies inside the neighborhood
		const u32 faceCount = v.mFaceIds.fCount( );
		for( u32 f = 0; f < faceCount; ++f )
		{
			// Faces shared between v0, v1 are defined as contained in the neighborhood
			if( ignore.fFind( v.mFaceIds[ f ] ) )
				continue;

			const tFace & face = mFaces[ v.mFaceIds[ f ] ];

			// Grab the other verts
			u32 pId, qId;
			face.fOtherVerts( vId, pId, qId );

			// Build the vectors into Rn space
			tQuadric::fConvertToRn( p, mVerts[ pId ].mData, mVertexFormat );
			tQuadric::fConvertToRn( q, mVerts[ qId ].mData, mVertexFormat );

			// Now that we have the vectors in Rn, build the plane
			Math::fVectorSubtract( h, q, p, mQuadricDimension );
			Math::fVectorSubtract( k, r, p, mQuadricDimension );

			// e1 = normalize(h)
			Math::fVectorNormalizeSafe( e1, h, mQuadricDimension );

			// e2 = normalize( k - (e1 dot k) * e1 )
			const f64 e1DotK = Math::fVectorDot( e1, k, mQuadricDimension );
			Math::fVectorScale( e2, e1, e1DotK, mQuadricDimension ); // e2 = (e1 dot k) * e1
			Math::fVectorSubtract( e2, k, e2, mQuadricDimension ); // e2 = k - (e1 dot k) * e1
			Math::fVectorNormalizeSafe( e2, mQuadricDimension );

			// Now we have a plane defined by point p and normal e2, calculate d of that plane
			const f64 d = -Math::fVectorDot( e2, p, mQuadricDimension );
			
			// Find the signed distance to this plane for the target point
			const f64 signedDist = Math::fVectorDot( vRn, e2, mQuadricDimension ) + d;

			// If the point falls outside this plane then it is outside the neighborhood
			if( signedDist < 0 )
				neighborhoodCost += Math::fSquare( signedDist );
		}

		return neighborhoodCost;
	}

	//------------------------------------------------------------------------------
	// tProgressiveMeshTool
	//------------------------------------------------------------------------------
	tProgressiveMeshTool::tProgressiveMeshTool( 
		const Gfx::tVertexFormat & vf,
		const void * vb, u32 vbCount,
		const u32 * ib, u32 ibCount,
		const tDecimationControls & controls)
		: mMesh( vf, vb, vbCount, ib, ibCount, controls )
	{

	}

	//------------------------------------------------------------------------------
	u32 tProgressiveMeshTool::fReduceToRatio( f32 percentage )
	{
		return fReduceToCount( mMesh.fActiveFaceCount( ) * percentage );
	}

	//------------------------------------------------------------------------------
	u32 tProgressiveMeshTool::fReduceToCount( u32 faceCount )
	{
		u32 count = 0;
		while( mMesh.fActiveFaceCount( ) > faceCount )
		{
			if( !fDoDecimate( ) )
				break;
			++count;
		}
		return count;
	}

	//------------------------------------------------------------------------------
	u32 tProgressiveMeshTool::fReduceToCost( f32 cost )
	{
		u32 count = 0;
		while( 1 )
		{
			// If we're over cost, but the mesh is restricting cascades attempt to reset the system
			// in an attempt to reinstate lower cost edge contractions
			if( mMesh.fNextDecimationCost( ) >= cost )
			{
				// If no edges are reinstated then we're done
				if( !mMesh.fResetCascades( ) )
					break;
			}

			// If we're over cost then we're done
			if( mMesh.fNextDecimationCost( ) >= cost )
				break;

			// If we can't do any decimations then we're done
			if( !fDoDecimate( ) )
				break;

			++count;
		}
		return count;
	}

	//------------------------------------------------------------------------------
	void tProgressiveMeshTool::fCapture( )
	{
		tDynamicArray<u32> indexMap, faceMap;

		// Capture the base and the index remapping for progressive construction
		mMesh.fCapture( mM0.mVerts, mM0.mIndices, &indexMap, &faceMap );

		// Grab the contractions
		const tContractionList & contractions = mMesh.fContractions( );
		const u32 contractionCount = contractions.fCount( );

		// Grab the cascade markers and convert them from contraction to expansion indices
		{
			const tGrowableArray<u32> & cascadeResets = mMesh.fCascadeResetMarkers( );
			const u32 cascadeResetCount = cascadeResets.fCount( );
			mCascadeMarkers.fNewArray( cascadeResetCount );
			for( u32 c = 0; c < cascadeResetCount; ++c )
				mCascadeMarkers[ cascadeResetCount - c - 1 ] = contractionCount - cascadeResets[ c ];

			// E.g. 5, 10, 25, 33 out of 50 => 17, 25, 40, 45 
		}

		// Grab the faces
		const tFaceList & faces = mMesh.fFaces( );
		const u32 faceCount = faces.fCount( );

		const b32 movingTargets = mMesh.fTargetPolicyRequiresData( );

		const u32 vertexSize = mMesh.fVertexFormat( ).fVertexSize( );
		mExpansionVertData.fNewArray( ( movingTargets ? 2 : 1 ) * contractionCount * vertexSize );
		byte * vertDest = mExpansionVertData.fBegin( );

		mExpansions.fNewArray( contractionCount );

		// Iterate over the contraction in reverse to build a forward array
		// of progressive mesh expansions
		u32 activeFaceCount = mMesh.fActiveFaceCount( );
		u32 activeVertCount = mMesh.fActiveVertCount( );
		for( s32 c = contractionCount - 1, e = 0; c >= 0; --c, ++e )
		{
			const tContractionRecordPtr & record = contractions[ c ];
			tExpansion & expansion = mExpansions[ e ];
			
			// Vert to update
			expansion.mUpdateVertIndex = indexMap[ record->mVertChanged.mId ];
			sigassert( expansion.mUpdateVertIndex < activeVertCount );

			// Some target selection policies do not create new data
			if( movingTargets )
			{
				fMemCpy( vertDest, record->mVertChanged.mData.fBegin( ), vertexSize );
				expansion.mUpdateVertDataOffset = fPtrDiff( vertDest, mExpansionVertData.fBegin( ) );
				vertDest += vertexSize;
			}

			// Vert to construct
			indexMap[ record->mVertDestroyed.mId ] = activeVertCount++;
			fMemCpy( vertDest, record->mVertDestroyed.mData.fBegin( ), vertexSize );
			expansion.mNewVertDataOffset = fPtrDiff( vertDest, mExpansionVertData.fBegin( ) );
			vertDest += vertexSize;

			// Change the update faces to refer to the mapped face index
			const u32 updateFacesCount = record->mFaceChanges.fCount( );
			expansion.mIndicesToChange.fNewArray( updateFacesCount ); 
			for( u32 uf = 0; uf < updateFacesCount; ++uf )
			{
				const tFaceChange & change = record->mFaceChanges[ uf ];

				expansion.mIndicesToChange[ uf ] = faceMap[ change.mA ] * 3 + change.mB;
				sigassert( faceMap[ change.mA ] < activeFaceCount );
			}

			// Change the new faces to refer to the last vert index for the destroyed
			const u32 newFaceCount = record->mFacesDestroyed.fCount( );
			expansion.mNewFaces.fNewArray( record->mFacesDestroyed.fCount( ) * 3 );
			for( u32 nf = 0, indexDest = 0; nf < newFaceCount; ++nf )
			{
				const u32 faceIndex = record->mFacesDestroyed[ nf ];
				const tFace & srcFace = faces[ faceIndex ];

				sigassert( indexMap[ srcFace.mVertIds[ 0 ] ] < activeVertCount );
				sigassert( indexMap[ srcFace.mVertIds[ 1 ] ] < activeVertCount );
				sigassert( indexMap[ srcFace.mVertIds[ 2 ] ] < activeVertCount );

				// These were the vert ids of the face when it was destroyed
				expansion.mNewFaces[ indexDest++ ] = indexMap[ srcFace.mVertIds[ 0 ] ];
				expansion.mNewFaces[ indexDest++ ] = indexMap[ srcFace.mVertIds[ 1 ] ];
				expansion.mNewFaces[ indexDest++ ] = indexMap[ srcFace.mVertIds[ 2 ] ];


				faceMap[ faceIndex ] = activeFaceCount++;
			}
		}

		sigassert( vertDest == mExpansionVertData.fEnd( ) && "Sanity!" );
	}

	//------------------------------------------------------------------------------
	void tProgressiveMeshTool::fCaptureM0( )
	{
		// Capture M0
		mMesh.fCapture( mM0.mVerts, mM0.mIndices, NULL, NULL );
	}

	//------------------------------------------------------------------------------
	b32 tProgressiveMeshTool::fDoDecimate( )
	{
		if( !mMesh.fDecimate( ) )
		{
			const tDecimationControls & controls = mMesh.fControls( );

			// If cascades have been restriced we may be able to reset the system
			// and then perform the decimation, but only if edges were actually reinstated
			if( mMesh.fResetCascades( ) )
				return mMesh.fDecimate( );
			else
				return false;
		}

		return true;
	}

}} // namespace ::Sig::LOD
