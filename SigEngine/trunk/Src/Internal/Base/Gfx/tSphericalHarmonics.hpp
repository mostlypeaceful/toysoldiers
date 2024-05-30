#ifndef __tSphericalHarmonics__
#define __tSphericalHarmonics__

namespace Sig { namespace Gfx
{
	/*
		Spherical harmonic ambient light mode.
	*/

	struct tShBasisWeights;

	struct base_export tSphericalHarmonics
	{
		declare_reflector( );

		// Factors are the colors that are uploaded to the GPU.
		//  stored as float 4's to optimize upload to gpu
		static const u32 cFactorCount = 9;
		tFixedArray< Math::tVec4f, cFactorCount > mFactors;

		tSphericalHarmonics( ) 
		{ 
			mFactors.fFill( Math::tVec4f::cZeroVector ); 
		}

		tSphericalHarmonics( tNoOpTag )
			: mFactors( cNoOpTag )
		{
		}

		// Use this for adding harmonic info
		void fAddSphericalHarmonic( const Math::tVec4f& color, const Math::tVec3f& direction, const tShBasisWeights& weights );

		// Serialization
		template< typename tSerializer >
		void fSerializeXml( tSerializer& s )
		{
			s( "f", mFactors );
		}

		b32 operator == ( const tSphericalHarmonics& other ) const
		{
			return mFactors.fEqual( other.mFactors );
		}
	};

	tSphericalHarmonics operator * ( const tSphericalHarmonics& lhs, float rhs );
	tSphericalHarmonics operator * ( float lhs, const tSphericalHarmonics& rhs );
	tSphericalHarmonics operator + ( const tSphericalHarmonics& lhs, const tSphericalHarmonics& rhs );
	tSphericalHarmonics operator - ( const tSphericalHarmonics& lhs, const tSphericalHarmonics& rhs );
	
	/*
		Basis weights are used when constructing harmonics.
	*/
	struct base_export tShBasisWeights
	{
		static const u32 cWeightCount = 6;
		static const u32 cEditableWeightCount = 3; // only the first 3 properties are user editable.
		
		tFixedArray< f32, cWeightCount > mWeights;

		tShBasisWeights( );
	};

}}

#endif//__tSphericalHarmonics__

