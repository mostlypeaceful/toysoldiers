#include "BasePch.hpp"
#include "tSphericalHarmonics.hpp"

namespace Sig { namespace Gfx
{

	tShBasisWeights::tShBasisWeights( )
	{
		//http://www.cs.columbia.edu/~cs4162/slides/spherical-harmonic-lighting.pdf
		mWeights[ 0 ] = 1.0f; //total
		mWeights[ 1 ] = 0.282f;
		mWeights[ 2 ] = 0.4886f;
		mWeights[ 3 ] = 1.0925f;
		mWeights[ 4 ] = 0.31539f;
		mWeights[ 5 ] = 1.0925f;
	}

	void tSphericalHarmonics::fAddSphericalHarmonic( const Math::tVec4f& color, const Math::tVec3f& direction, const tShBasisWeights& weights )
	{
		mFactors[ 0 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 1 ];
		mFactors[ 1 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 2 ] * direction.x;
		mFactors[ 2 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 2 ] * direction.y;
		mFactors[ 3 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 2 ] * direction.z;
		mFactors[ 4 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 3 ] * (direction.x * direction.z);
		mFactors[ 5 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 3 ] * (direction.z * direction.y);
		mFactors[ 6 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 3 ] * (direction.y * direction.x);
		mFactors[ 7 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 4 ] * (2.0f * direction.z * direction.z - (direction.x * direction.x) - (direction.y * direction.y));
		mFactors[ 8 ] += color * weights.mWeights[ 0 ] * weights.mWeights[ 5 ] * (direction.x * direction.x - direction.y * direction.y);
	}

	tSphericalHarmonics operator * ( const tSphericalHarmonics& lhs, float rhs )
	{
		tSphericalHarmonics harm = lhs;
		for( u32 i = 0; i < harm.mFactors.fCount( ); ++i )
			harm.mFactors[ i ] *= rhs;
		return harm;
	}

	tSphericalHarmonics operator * ( float lhs, const tSphericalHarmonics& rhs )
	{
		return rhs * lhs;
	}

	tSphericalHarmonics operator + ( const tSphericalHarmonics& lhs, const tSphericalHarmonics& rhs )
	{
		tSphericalHarmonics harm = lhs;
		for( u32 i = 0; i < harm.mFactors.fCount( ); ++i )
			harm.mFactors[ i ] += rhs.mFactors[ i ];
		return harm;
	}

	tSphericalHarmonics operator - ( const tSphericalHarmonics& lhs, const tSphericalHarmonics& rhs )
	{
		tSphericalHarmonics harm = lhs;
		for( u32 i = 0; i < harm.mFactors.fCount( ); ++i )
			harm.mFactors[ i ] -= rhs.mFactors[ i ];
		return harm;
	}

}}

