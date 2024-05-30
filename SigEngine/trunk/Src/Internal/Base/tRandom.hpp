#ifndef __tRandom__
#define __tRandom__

namespace Sig
{
	class base_export tRandom
	{
	private:
		static tRandom gSubjectiveRand;
		static tRandom gObjectiveRand;
	public:

		/// \brief For e.g. metro where our main thread isn't our initial thread.
		static void fSetMainThread( u32 threadId );
		static inline tRandom& fSubjectiveRand( ) { return gSubjectiveRand; }
		static tRandom& fObjectiveRand( );

	private:

		u32 mState;

		static f32 fRandStateToFloatMinusOneToOne( u32& randState );
		static f32 fRandStateToFloatZeroToOne( u32& randState );
		static void fRandStateAdvance( u32& randState );

	public:

		inline explicit tRandom( tNoOpTag ) { }
		inline explicit tRandom( ) : mState( 1 ) { }
		inline explicit tRandom( u32 seed ) : mState( seed ) { }

		inline u32 fState( ) const { return mState; }
		inline void fAdvanceState( ) { fRandStateAdvance( mState ); }

		///
		/// \brief Returns a float in the range [-1,+1]
		inline f32 fFloatMinusOneToOne( )
		{
			return fRandStateToFloatMinusOneToOne( mState );
		}

		///
		/// \brief Returns a float in the range [0,+1]
		inline f32 fFloatZeroToOne( )
		{
			return fRandStateToFloatZeroToOne( mState );
		}

		///
		/// \brief Returns a float in range [0, value]
		inline f32 fFloatZeroToValue( f32 value )
		{
			return fFloatZeroToOne( ) * value;
		}

		///
		/// \brief Returns a float in the range [min, max]
		inline f32 fFloatInRange( f32 min, f32 max )
		{
			return min + fFloatZeroToOne( ) * ( max - min );
		}

		///
		/// \brief Returns an integer in the range [min, max]
		inline s32 fIntInRange( s32 min, s32 max )
		{
			return fRound<s32>( fFloatInRange( min - 0.49f, max + 0.49f ) );
		}

		///
		/// \brief N-D vector
		template<class tVecNf>
		inline tVecNf fVec( )
		{
			tVecNf o;
			for( u32 i = 0; i < o.cDimension; ++i )
				o.fAxis( i ) = fFloatMinusOneToOne( );
			return o;
		}

		///
		/// \brief Normalized N-D vector
		template<class tVecNf>
		inline tVecNf fVecNorm( )
		{
			tVecNf o;

			do
			{
				o = fVec<tVecNf>( );
			} while( o.fIsZero( ) );

			o.fNormalize( );
			return o;
		}

		///
		/// \brief Rgba 4D Color value with components in [0,1]; you supply the alpha.
		inline Math::tVec4f fColor( f32 alpha = 1.f )
		{
			return Math::tVec4f( 
				fFloatZeroToOne( ),
				fFloatZeroToOne( ),
				fFloatZeroToOne( ),
				alpha );
		}

		///
		/// \brief Returns a signed integer in the range [-std::numeric_limits<s32>::max( ), +std::numeric_limits<s32>::max( )]
		inline s32 fInt( )
		{
			const f32 r = fFloatMinusOneToOne( );
			return fRound<s32>( r * std::numeric_limits<s32>::max( ) );
		}

		///
		/// \brief Returns an unsigned integer in the range [0, std::numeric_limits<u32>::max( )]
		inline u32 fUInt( )
		{
			const f32 r = fFloatZeroToOne( );
			return fRound<u32>( r * std::numeric_limits<u32>::max( ) );
		}

		inline u32 fIndex( u32 count )
		{
			const f32 r = fFloatZeroToOne( );
			return fRound<u32>( r * ( count - 1 ) );
		}

		///
		/// \brief Returns a random true or false value.
		inline b32 fBool( )
		{
			const f32 r = fFloatZeroToOne( );
			return r >= 0.5f;
		}

		inline b32 fChance( f32 zeroToOne )
		{
			if( zeroToOne == 0.f )
				return false;

			const f32 r = fFloatZeroToOne( );
			return r <= zeroToOne;
		}

		///
		/// \brief See if this instance is objective
		inline b32 fIsObjective( ) const { return this == &gObjectiveRand; }

		///
		/// \brief Synchronizes the objective random seed state (i.e., for lock step networking).
		static void fSyncObjectiveSeed( u32 seed );


	public: // script-specific
		static void					fExportScriptInterface( tScriptVm& vm );
	};

}

// To use these the caller must include tSync.hpp
#define sync_rand( methodCall ) ::Sig::tSync::fInstance( ).fLogWithReturn( tRandom::fObjectiveRand( ).methodCall, "Rand", __FILE__, __FUNCTION__, __LINE__, ::Sig::tSync::cSCRandom )
#define sync_randc( methodCall, context ) ::Sig::tSync::fInstance( ).fLogWithReturn( tRandom::fObjectiveRand( ).methodCall, ( context ), __FILE__, __FUNCTION__, __LINE__, ::Sig::tSync::cSCRandom )
//#define sync_rand( methodCall ) ::Sig::tRandom::fObjectiveRand( ).methodCall
//#define sync_randc( methodCall, context ) ::Sig::tRandom::fObjectiveRand( ).methodCall

#endif//__tRandom__
