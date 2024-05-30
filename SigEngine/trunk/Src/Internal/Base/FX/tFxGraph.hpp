#ifndef __tFxGraph__
#define __tFxGraph__
#include "tFxKeyFrame.hpp"
#include "tMersenneGenerator.hpp"

namespace Sig { namespace FX
{
	//------------------------------------------------------------------------------
	// Editables
	//------------------------------------------------------------------------------
	class tGraph : public Rtti::tSerializableBaseClass, public tRefCounter
	{
		declare_null_reflector( );
	private:

		struct tSortKeyframe
		{
			inline b32 operator( )( const tKeyframePtr a, const tKeyframePtr b ) const
			{
				return a->fX( ) < b->fX( );
			}
		};

	public:

		tGraph( )
			: mUseLerp( true ), mUseRandom( true ), mMinRandomness( 0 ), mMaxRandomness( 0 ), mKeepLastKeyValue( false ), mDirty( true )
		{

		}

		tGraph( tGraph* rhs )
		{
			for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
				mKeyframes[ i ].fRelease( );
			mKeyframes.fSetCount( 0 );

			mMinRandomness = rhs->mMinRandomness;
			mMaxRandomness = rhs->mMaxRandomness;
			mKeepLastKeyValue = rhs->mKeepLastKeyValue;
			mUseLerp = rhs->mUseLerp;
			mUseRandom = rhs->mUseRandom;
			mDirty = rhs->mDirty;

			for( u32 i = 0; i < rhs->mKeyframes.fCount( ); ++i )
			{
				u32 id = rhs->mKeyframes[ i ]->fGetID( );
				tKeyframePtr newkey;

				if( id == Rtti::fGetClassId< f32 >( ) )
					newkey = tKeyframePtr( NEW tFxKeyframeF32( rhs->mKeyframes[ i ]->fX( ), rhs->mKeyframes[ i ]->fValue< f32 >( ) ) );
				else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
					newkey = tKeyframePtr( NEW tFxKeyframeV2f( rhs->mKeyframes[ i ]->fX( ), rhs->mKeyframes[ i ]->fValue< Math::tVec2f >( ) ) );
				else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
					newkey = tKeyframePtr( NEW tFxKeyframeV3f( rhs->mKeyframes[ i ]->fX( ), rhs->mKeyframes[ i ]->fValue< Math::tVec3f >( ) ) );
				else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
					newkey = tKeyframePtr( NEW tFxKeyframeV4f( rhs->mKeyframes[ i ]->fX( ), rhs->mKeyframes[ i ]->fValue< Math::tVec4f >( ) ) );
				else if( id == Rtti::fGetClassId< Math::tQuatf >( ) )
					newkey = tKeyframePtr( NEW tFxKeyframeQuatf( rhs->mKeyframes[ i ]->fX( ), rhs->mKeyframes[ i ]->fValue< Math::tQuatf >( ) ) );

				mKeyframes.fPushBack( newkey );
			}
		}

		virtual ~tGraph( )
		{
			fClear( );
		}

		void fClear( )
		{
			for( u32 i = 0; i < mKeyframes.fCount( ); ++i )
				mKeyframes[ i ].fRelease( );
			mKeyframes.fSetCount( 0 );
		}

		virtual void fBuildValues( ) = 0;
		virtual void fPrintValueArray( ) = 0;

		template<class t>
		t fSample( const tMersenneGenerator* random, f32 x ) const
		{
			t value;
			fSampleGraph( random, value, x );
			return value;
		}
		
		tKeyframePtr fNewKeyFromKey( tKeyframePtr key )
		{
			return fMakeCopyOfKey( key );
		}
		
		tKeyframePtr fCopyPreviousKey( f32 x, f32 mindistancetocopy = 0.f )
		{
			tKeyframePtr key;

			for( u32 i = 0; i < mKeyframes.fCount( ) && !key; ++i )
			{
				f32 abs = fAbs( mKeyframes[ i ]->fX( ) - x );
				if( abs < mindistancetocopy )
					return mKeyframes[ i ];

				if( mKeyframes[ i ]->fX( ) == x )		//then delete this keyframe! now, do it now!
				{
					key = fNewKeyFromKey( mKeyframes[ i ] );
					mKeyframes.fErase( i );
				}
				else
				{
					if( mKeyframes.fCount( ) > 1 )
					{
						if( i == 0 && mKeyframes[ i ]->fX( ) > x )
							key = fNewKeyFromKey( mKeyframes[ i ] );
						else if( i > 1 && mKeyframes[ i ]->fX( ) > x )
							key = fNewKeyFromKey( mKeyframes[ i-1 ] );
					}
					else
					{
						key = fNewKeyFromKey( mKeyframes[ i ] );
					}
				}
			}

			if( mKeyframes.fCount( ) > 1 && !key )
				key = fNewKeyFromKey( mKeyframes[ mKeyframes.fCount( )-1 ] );

			if( key )
				key->fSetX( x );

			return key;
		}

		void fAddKeyframe( tKeyframe* key )
		{
			fAddKeyframe( tKeyframePtr( key ) );
		}
		void fAddKeyframeNoGraphUpdate( tKeyframe* key )
		{
			mKeyframes.fPushBack( tKeyframePtr( key ) );
		}

		void fAddKeyframe( tKeyframePtr key )
		{
			if( mKeyframes.fFindAndErase( key ) )
				log_warning( "Somehow a keyframe has been added twice..." );

			mKeyframes.fPushBack( key );

			fUpdate( );
		}

		void fRemoveKeyframe( tKeyframePtr key )
		{
			mKeyframes.fFindAndErase( key );
			fUpdate( );
		}

		u32 fNumKeyframes( ) const { return mKeyframes.fCount( ); }
		tKeyframePtr fKeyframe( u32 idx ) const { return mKeyframes[ idx ]; }

		virtual Rtti::tClassId fGetID( ) const { return 0; }

		void fUpdate( )
		{
			std::sort( mKeyframes.fBegin( ), mKeyframes.fEnd( ), tSortKeyframe( ) );
			fBuildValues( );
		}

		void fSetMinRandomness( const f32 val ) { mMinRandomness = val; fMarkDirty( ); }
		f32 fMinRandomness( ) const { return mMinRandomness; }
		
		void fSetMaxRandomness( const f32 val ) { mMaxRandomness = val; fMarkDirty( ); }
		f32 fMaxRandomness( ) const { return mMaxRandomness; }

		void fSetKeepLastKeyValue( b32 keep ) { mKeepLastKeyValue = keep; fMarkDirty( ); }
		b32 fKeepLastKeyValue( ) const { return mKeepLastKeyValue; }
		
		void fSetUseLerp( b32 lerp ) { mUseLerp = lerp; fMarkDirty( ); }
		b32 fUseLerp( ) const { return mUseLerp; }

		void fSetUseRandoms( b32 useRandoms ) { mUseRandom = useRandoms; fMarkDirty( ); }
		b32 fUseRandoms( ) const { return mUseRandom; }

		void fMarkDirty( ) { mDirty = true; }
		b32 fGetDirty( ) const { return mDirty; }
		void fClean( ) { mDirty = false; }

	protected:

		void fSampleGraph( const tMersenneGenerator* random, Math::tVec1f& value, f32 x ) const 
		{
			fSampleGraph( random, value.x, x );
		}

		virtual void fSampleGraph( const tMersenneGenerator* random, f32& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec2f& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec3f& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec4f& value, f32 x ) const { }

		virtual tKeyframePtr fMakeCopyOfKey( tKeyframePtr key ) = 0;

	//private:
	public:
		tGrowableArray< tKeyframePtr > mKeyframes;
		tGrowableArray< Math::tVec2f > mRandoms;
		//Math::tVec2f mRandoms[ N ];
		b16 mUseLerp;
		b16 mUseRandom;

	private:

		f32 mMinRandomness;
		f32 mMaxRandomness;
		b32 mKeepLastKeyValue;
		b32 mDirty;

	};

	typedef tRefCounterPtr< tGraph > tGraphPtr;

	template< class t >
	class tFxGraph : public tGraph
	{
	public:
		tFxGraph( )	{ }
		tFxGraph( tGraph* graph )
			: tGraph( graph )
		{
		}

		virtual ~tFxGraph( ) {	}

		virtual void fBuildValues( )
		{
			u32 N = 128;
			const u32 numKeyframes = fNumKeyframes( );
			if( numKeyframes == 1 )
				N = 2;

			mValueArray.fSetCount( N );
			mRandoms.fSetCount( N );

			const f32 delta = 1.f / ( f32 )N;
			const f32 epsilon = delta;
			
			for( u32 valueIdx = 0; valueIdx < N; ++valueIdx )
			{
				const f32 f = ( f32 ) valueIdx / ( f32 )( N - 1 );

				tKeyframePtr key1;
				tKeyframePtr key2;

				for( u32 i = 0; i < numKeyframes; ++i )
				{
					if( fKeyframe( i )->fX( ) <= (f+epsilon) )
						key1 = fKeyframe( i );

					if( i < fNumKeyframes( ) - 1 )
					{
						if( fKeyframe( i+1 )->fX( ) > (f-epsilon) )
						{
							key2 = fKeyframe( i+1 );
							break;
						}
					}
				}

				if( !key1 )
				{
					mValueArray[ valueIdx  ] = t( 0.f );
					mRandoms[ valueIdx ] = Math::tVec2f::cZeroVector;
				}
				else if( key1 && !key2 )		// then we're in a 1-key system
				{
					if( numKeyframes > 1 )
					{
						if( ( f - epsilon ) <= key1->fX( ) || fKeepLastKeyValue( ) )
						{
							mValueArray[ valueIdx ] = key1->fValue< t >( );
							mRandoms[ valueIdx ] = Math::tVec2f( fMinRandomness( ), fMaxRandomness( ) );
						}
						else
						{
							mValueArray[ valueIdx ] = t( 0.f );
							mRandoms[ valueIdx ] = Math::tVec2f::cZeroVector;
						}
					}
					else
					{
						if( ( f + epsilon ) >= key1->fX( ) || fKeepLastKeyValue( ) )
						{
							mValueArray[ valueIdx  ] = key1->fValue< t >( );
							mRandoms[ valueIdx ] = Math::tVec2f( fMinRandomness( ), fMaxRandomness( ) );
						}
						else
						{
							mValueArray[ valueIdx ] = t( 0.f );
							mRandoms[ valueIdx ] = Math::tVec2f::cZeroVector;
						}
					}
				}
				else
				{
					const f32 sX = f - key1->fX( );
					const f32 diff = key2->fX( ) - key1->fX( );
					const f32 x = (diff > 0.f) ? sX / diff : 0.0f;
					
					mValueArray[ valueIdx  ] = Math::fLerp( key1->fValue< t >( ), key2->fValue< t >( ), x );
					mRandoms[ valueIdx ] = Math::tVec2f( fMinRandomness( ), fMaxRandomness( ) );
				}
				
				// debug output
				//log_line( "key1=" << key1->fValue( ) << "key2=" << key2->fValue( ) );
				//log_line( "diff = " << diff );
				//log_line( "x = " << x );
				//log_line( "mValueArray[ " << valueIdx << " ] = " << mValueArray[ valueIdx ] );

				sigassert( mValueArray[ valueIdx ] == mValueArray[ valueIdx ] );
				sigassert( mRandoms[ valueIdx ] == mRandoms[ valueIdx ] );
			}

			mUseLerp = mUseRandom = true;
			if( numKeyframes == 1 )
			{
				tKeyframePtr key0 = fKeyframe( 0 );
				
				mUseLerp = false;
				mValueArray[ 0 ] = key0->fValue< t >( );
				mValueArray[ 1 ] = mValueArray[ 0 ];
				mRandoms[ 0 ] = Math::tVec2f( fMinRandomness( ), fMaxRandomness( ) );
				mRandoms[ 1 ] = fKeyframe( 0 )->fX( );
			}
			if( fMinRandomness( ) == fMaxRandomness( ) )
				mUseRandom = false;

			// debug output for testing the values...
			//for( u32 i = 0; i < N; ++i )
			//	log_line( "mValueArray[ " << i << " ] = " << mValueArray[ i ] );
			
			//for( f32 f = 0.f; f < 1.f; f += 1.f / 200.f )
			//	log_line( "x=" << f << "\t Sample=" << fSample( f ) );

			fMarkDirty( );
		}

		virtual void fPrintValueArray( )
		{
			for( u32 i = 0; i < mValueArray.fCount( ); ++i )
				log_line( 0, "mValueArray[ " << i << " ] = " << mValueArray[ i ] );
		}

		virtual Rtti::tClassId fGetID( ) const { return gCid; }

	protected:

		virtual tKeyframePtr fMakeCopyOfKey( tKeyframePtr key )
		{
			sigassert( key );

			u32 id = key->fGetID( );
			tKeyframePtr newkey;
			
			if( id == Rtti::fGetClassId< f32 >( ) )
				newkey = tKeyframePtr( NEW tFxKeyframeF32( key->fX( ), key->fValue< f32 >( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
				newkey = tKeyframePtr( NEW tFxKeyframeV2f( key->fX( ), key->fValue< Math::tVec2f >( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
				newkey = tKeyframePtr( NEW tFxKeyframeV3f( key->fX( ), key->fValue< Math::tVec3f >( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
				newkey = tKeyframePtr( NEW tFxKeyframeV4f( key->fX( ), key->fValue< Math::tVec4f >( ) ) );
			else if( id == Rtti::fGetClassId< Math::tQuatf >( ) )
				newkey = tKeyframePtr( NEW tFxKeyframeQuatf( key->fX( ), key->fValue< Math::tQuatf >( ) ) );
			else
				sigassert( !"fMakeCopyOfKey failing!" );

			//fAddKeyframe( newkey );
			return newkey;
		}

		t fSampleGraphInternal( const tMersenneGenerator* random, f32 x ) const
		{
			sigassert( x >= 0.f && x <= 1.f );
			t value( 0.f );

			if( !mUseLerp && x >= mRandoms[ 1 ].x )
			{
				value = mValueArray[ 1 ];
				if( mUseRandom && random )
					random->fAddRandomFloatsInRange( value, mRandoms[ 0 ].x, mRandoms[ 0 ].y );
			}
			else if( mUseLerp )
			{
				const u32 N = mValueArray.fCount( );

				u32 x1 = ( u32 ) ( x * ( N-1 ) );
				u32 x2 = x1 == ( N-1 ) ? x1 : ( x1 + 1 );
				const f32 inverseN = 1.f / (f32)N;
				const f32 delta = ( x - ( x1 * inverseN ) ) / inverseN;

				value = Math::fLerp( mValueArray[ x1 ], mValueArray[ x2 ], delta );
				if( mUseRandom && random )
					random->fAddRandomFloatsInRange( value, mRandoms[ x1 ].x, mRandoms[ x2 ].y );
			}

			return value;
		}

	private:
		static const Rtti::tClassId gCid;
		tGrowableArray< t > mValueArray;
	};

	template< class t >
	const Rtti::tClassId tFxGraph< t >::gCid = Rtti::fGetClassId< t >( );

	//------------------------------------------------------------------------------
	// Binaries
	//------------------------------------------------------------------------------
	class tBinaryGraph : public Rtti::tSerializableBaseClass
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryGraph, 0x683D6FCC );
	public:

		typedef Math::tVector2<u16> tRandomKey;

		tBinaryGraph( )
		{
		}

		tBinaryGraph( tNoOpTag )
			: mRandomKeys( cNoOpTag )
			, mRandomMin( cNoOpTag )
			, mRandomMax( cNoOpTag )
		{
		}
		
		virtual void fCopyFromGraph( tGraphPtr graph ) = 0;

		template<class t>
		t fSample( const tMersenneGenerator* random, const f32 x ) const
		{
			t value;
			fSampleGraph( random, value, x );
			return value;
		}
		
		tDynamicArray< tRandomKey > mRandomKeys;

		Math::tVec2f mRandomMin;
		Math::tVec2f mRandomMax;

		b16 mUseLerp;
		b16 mUseRandom;

	protected:
		virtual void fSampleGraph( const tMersenneGenerator* random, f32& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec2f& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec3f& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec4f& value, f32 x ) const { }

		static void fToRandomKey( tRandomKey& out, const Math::tVec2f& in, const Math::tVec2f& min, const Math::tVec2f& max )
		{
			out[ 0 ] = fRound<u16>( 65535.f * Math::fRemapZeroToOne( min[ 0 ], max[ 0 ], in[ 0 ] ) );
			out[ 1 ] = fRound<u16>( 65535.f * Math::fRemapZeroToOne( min[ 1 ], max[ 1 ], in[ 1 ] ) );
		}

		static void fFromRandomKey( Math::tVec2f& out, const tRandomKey& in, const Math::tVec2f& min, const Math::tVec2f& max )
		{
			const f32 delta0 = max[ 0 ] - min[ 0 ];
			const f32 delta1 = max[ 1 ] - min[ 1 ];

			out[ 0 ] = min[ 0 ] + ( in[ 0 ] / 65535.f ) * delta0;
			out[ 1 ] = min[ 1 ] + ( in[ 1 ] / 65535.f ) * delta1;
		}

	private:
	};

	template< int N >
	struct tQuantizedVector
	{
		declare_reflector( );
		tFixedArray< u16, N > mValues;
	};

	template< class tVectorType >
	class tBinaryVectorTemplateGraph : public tBinaryGraph
	{
		declare_reflector( );
		
	protected:

		typedef tQuantizedVector< tVectorType::cDimension > tKey;

		tBinaryVectorTemplateGraph( )
		{
		}

		tBinaryVectorTemplateGraph( tNoOpTag )
			: tBinaryGraph( cNoOpTag )
			, mKeyArray( cNoOpTag )
			, mMin( cNoOpTag )
			, mMax( cNoOpTag )
		{

		}

		static void fToKey( tKey& out, const tVectorType& in, const tVectorType& min, const tVectorType& max )
		{
			for( u32 i = 0; i < tVectorType::cDimension; ++i )
			{
				out.mValues[ i ] = fRound<u16>( 65535.f * Math::fRemapZeroToOne( min[ i ], max[ i ], in[ i ] ) );
			}
		}

		static void fFromKey( tVectorType& out, const tKey& in, const tVectorType& min, const tVectorType& max )
		{
			for( u32 i = 0; i < tVectorType::cDimension; ++i )
			{
				const f32 delta = max[ i ] - min[ i ];
				out[ i ] = min[ i ] + ( in.mValues[ i ] / 65535.f ) * delta;
			}
		}

	public:

		tVectorType fSampleGraphInternal( const tMersenneGenerator* random, f32 x ) const
		{
			sigassert( x >= 0.f && x <= 1.f );
			
			tVectorType value( 0.f );
			
			Math::tVec2f rr1;
			fFromRandomKey( rr1, mRandomKeys[ 1 ], mRandomMin, mRandomMax );

			if( !mUseLerp && x >= rr1.x )
			{
				fFromKey( value, mKeyArray[ 1 ], mMin, mMax );
				if( mUseRandom )
				{
					sigassert( random );

					Math::tVec2f rr0;
					fFromRandomKey( rr0, mRandomKeys[ 0 ], mRandomMin, mRandomMax );

					value += random->fFloatInRange( rr0.x, rr0.y );
				}
			}
			else if( mUseLerp )
			{
				const u32 N = mKeyArray.fCount( );
				const u32 x1 = ( u32 ) ( x * ( N-1 ) );
				const u32 x2 = x1 == ( N-1 ) ? x1 : ( x1 + 1 );

				tVectorType v1, v2;
				fFromKey( v1, mKeyArray[ x1 ], mMin, mMax );
				fFromKey( v2, mKeyArray[ x2 ], mMin, mMax );

				const f32 inverseN = 1.f / ( f32) N;
				const f32 delta = x - x1 * inverseN;
				value = Math::fLerp( v1, v2, delta );

				if( mUseRandom )
				{
					sigassert( random );

					Math::tVec2f rr;
					fFromRandomKey( rr, mRandomKeys[ x1 ], mRandomMin, mRandomMax );

					value += random->fFloatInRange( rr.x, rr.y );
				}
			}
			
			return value;
		}

		virtual void fCopyFromGraph( tGraphPtr graph )
		{
			u32 N = graph->mRandoms.fCount( );
			mRandomKeys.fNewArray( N );
			mKeyArray.fNewArray( N );

			tDynamicArray<tVectorType> fullValues( N );
			tDynamicArray<Math::tVec2f> fullRandoms( N );

			mMin = Math::cInfinity;
			mMax = -Math::cInfinity;

			mRandomMin = Math::cInfinity;
			mRandomMax = Math::cInfinity;

			for( u32 i = 0; i < N; ++i )
			{
				f32 x = ( f32 ) i / ( f32 ) ( N - 1 );
				fullRandoms[ i ] = graph->mRandoms[ i ];
				fullValues[ i ] = graph->fSample< tVectorType >( 0, x );
				sigassert( fullValues[ i ] == fullValues[ i ] );

				mRandomMin[ 0 ] = fMin( fullRandoms[ i ][ 0 ], mRandomMin[ 0 ] );
				mRandomMin[ 1 ] = fMin( fullRandoms[ i ][ 1 ], mRandomMin[ 1 ] );
				mRandomMax[ 0 ] = fMax( fullRandoms[ i ][ 0 ], mRandomMax[ 0 ] );
				mRandomMax[ 1 ] = fMax( fullRandoms[ i ][ 1 ], mRandomMax[ 1 ] );

				for( u32 j = 0; j < tVectorType::cDimension; ++j )
				{
					mMin[ j ] = fMin( fullValues[ i ][ j ], mMin[ j ] );
					mMax[ j ] = fMax( fullValues[ i ][ j ], mMax[ j ] );
				}
			}

			for( u32 i = 0; i < N; ++i )
			{
				fToKey( mKeyArray[ i ], fullValues[ i ], mMin, mMax );
				fToRandomKey( mRandomKeys[ i ], fullRandoms[ i ], mRandomMin, mRandomMax );
			}
			
			mUseLerp = graph->mUseLerp;
			mUseRandom = graph->mUseRandom;

			//graph->fPrintValueArray( );
		}

		tDynamicArray< tKey > mKeyArray;
		
		tVectorType mMin;
		tVectorType mMax;

		//t mValueArray[ N ];
		//static const f32 mInverseN;
	};

	//template< class t >
	//const f32 tBinaryTemplateGraph<t>::mInverseN = 1.f / (f32)tBinaryTemplateGraph<t>::N;

	class tBinaryF32Graph : public tBinaryVectorTemplateGraph< Math::tVec1f >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryF32Graph, 0x5B53F92A );
		tBinaryF32Graph( ) {	}
		tBinaryF32Graph( tNoOpTag ) : tBinaryVectorTemplateGraph( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, f32& value, f32 x ) const { value = fSampleGraphInternal( random, x ).x; }
		typedef f32 tType;
	};
	class tBinaryV2Graph : public tBinaryVectorTemplateGraph< Math::tVec2f >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryV2Graph, 0x8DBD85F6 );
		tBinaryV2Graph( ) {	}
		tBinaryV2Graph( tNoOpTag ) : tBinaryVectorTemplateGraph( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec2f& value, f32 x ) const { value = fSampleGraphInternal( random, x ); }
		typedef Math::tVec2f tType;
	};
	class tBinaryV3Graph : public tBinaryVectorTemplateGraph< Math::tVec3f >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryV3Graph, 0x3C4F4A0B );
		tBinaryV3Graph( ) {	}
		tBinaryV3Graph( tNoOpTag ) : tBinaryVectorTemplateGraph( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec3f& value, f32 x ) const { value = fSampleGraphInternal( random, x ); }
		typedef Math::tVec3f tType;
	};
	class tBinaryV4Graph : public tBinaryVectorTemplateGraph< Math::tVec4f >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryV4Graph, 0x6C5803A );
		tBinaryV4Graph( ) {	}
		tBinaryV4Graph( tNoOpTag ) : tBinaryVectorTemplateGraph( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec4f& value, f32 x ) const { value = fSampleGraphInternal( random, x ); }
		typedef Math::tVec4f tType;
	};

	base_export tBinaryGraph* fCreateNewGraph( tGraphPtr original );

#define define_derived_graph( className, t, guid ) \
	class className : public tFxGraph< t > \
	{ \
		declare_null_reflector( ); \
		implement_rtti_serializable_base_class( className, guid ); \
		className( tGraph* graph ) : tFxGraph< t >( graph ) { } \
		className( ) { } \
		virtual void fSampleGraph( const tMersenneGenerator* random, t& value, f32 x ) const { value = fSampleGraphInternal( random, x ); } \
	};

	define_derived_graph( tFxGraphF32, f32, 0x620461DF );
	define_derived_graph( tFxGraphV2f, Math::tVec2f, 0x84C4036F );
	define_derived_graph( tFxGraphV3f, Math::tVec3f, 0x3CEBB8DF );
	define_derived_graph( tFxGraphV4f, Math::tVec4f, 0x75ADADCD );

}
}


#endif // __tFxGraph__
