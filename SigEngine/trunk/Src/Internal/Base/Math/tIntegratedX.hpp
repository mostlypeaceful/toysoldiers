#ifndef __tIntegratedX__
#define __tIntegratedX__

namespace Sig
{
	template<class t>
	class tIntegratedV
	{
	protected:

		t mA;
		t mV;

	public:

		inline explicit tIntegratedV( )
			: mA( 0.f )
			, mV( 0.f )
		{
		}

		inline const t& fV( ) const { return mV; }
		inline const t& fA( ) const { return mA; }

		inline void fAddA( const t& da ) { mA += da; }
		inline void fAddV( const t& dv ) { mV += dv; }

		inline void fStopMoving( )
		{
			mA = t( 0.f );
			mV = t( 0.f );
		}

		inline void fStep( f32 dt, const t& damping = t( 1.f ), f32 idealDt = 1.f/60.f )
		{
			// step in increments of 'idealDt'
			for( ; dt > idealDt; dt -= idealDt )
			{
				mV += mA * idealDt;
				mV *= t( 1.f ) - damping;
			}

			// step once more with any remaining time left (will be less than 'idealDt')
			mV += mA * dt;
			mV *= t( 1.f ) - ( damping * ( dt / idealDt ) );

			// clear acceleration
			mA = t( 0.f );
		}

	};

	template<class t>
	class tIntegratedX : public tIntegratedV< t >
	{
		t mX;

	public:

		inline explicit tIntegratedX( const t& startValue = 0.f )
			: mX( startValue )
		{
		}

		inline const t& fX( ) const { return mX; }

		inline void fAddX( const t& dx ) { mX += dx; }

		inline void fReset( const t& newX )
		{
			tIntegratedV< t >::fStopMoving( );
			mX = newX;
		}

		inline void fStep( f32 dt, const t& damping = t( 1.f ), f32 idealDt = 1.f/60.f )
		{
			// step in increments of 'idealDt'
			for( ; dt > idealDt; dt -= idealDt )
			{
				mX += tIntegratedV< t >::mV * idealDt;
				tIntegratedV< t >::mV += tIntegratedV< t >::mA * idealDt;
				tIntegratedV< t >::mV *= t( 1.f ) - damping;
			}

			// step once more with any remaining time left (will be less than 'idealDt')
			mX += tIntegratedV< t >::mV * dt;
			tIntegratedV< t >::mV += tIntegratedV< t >::mA * dt;
			tIntegratedV< t >::mV *= t( 1.f ) - ( damping * ( dt / idealDt ) );

			// clear acceleration
			tIntegratedV< t >::mA = t( 0.f );
		}

		inline void fClamp( const t& min, const t& max )
		{
			mX = Sig::fClamp( mX, min, max );
		}
	};
}

#endif//__tIntegratedX__
