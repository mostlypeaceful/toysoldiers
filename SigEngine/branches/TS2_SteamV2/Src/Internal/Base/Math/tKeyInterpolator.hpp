#ifndef __tKeyInterpolator__
#define __tKeyInterpolator__

namespace Sig { namespace Math
{
	template<class t>
	class tKeyInterpolator : public tDynamicArray< tPair<f32, t> >
	{
		typedef tDynamicArray< tPair<f32, t> > tBase;

	public:
		explicit tKeyInterpolator( u32 numKeys ) : tBase( numKeys ) { }

		t operator()( f32 timeInZeroToOne ) const
		{
			sigassert( fCount( ) >= 1 );

			for( u32 i = 0; i < fCount( ) - 1; ++i )
			{
				if( timeInZeroToOne < (*this)[ i + 1 ].mA )
				{
					const tPair<f32, t>& kPrev = ( i > 0 ? (*this)[ i - 1 ] : (*this)[ i ] );
					const tPair<f32, t>& k0 = (*this)[ i ];
					const tPair<f32, t>& k1 = (*this)[ i + 1 ];
					const tPair<f32, t>& kNext = ( i < fCount( ) - 2 ? (*this)[ i + 2 ] : (*this)[ i + 1 ] );
					const f32 amount = ( timeInZeroToOne - k0.mA ) / ( k1.mA - k0.mA );
					return Math::fHermite( kPrev.mB, k0.mB, k1.mB, kNext.mB, amount );
				}
			}

			return fBack( ).mB;
		}
	};

}}

#endif//__tKeyInterpolator__