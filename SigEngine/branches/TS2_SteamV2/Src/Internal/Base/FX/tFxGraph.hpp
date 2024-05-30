#ifndef __tFxGraph__
#define __tFxGraph__
#include "tFxKeyFrame.hpp"
#include "tMersenneGenerator.hpp"

namespace Sig { namespace FX
{
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
			: mUseLerp( true ), mUseRandom( true ), mMinRandomness( 0 ), mMaxRandomness( 0 ), mKeepLastKeyValue( false )
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
				log_warning( 0, "Somehow a keyframe has been added twice..." );

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

		void fSetMinRandomness( const f32 val ) { mMinRandomness = val; }
		f32 fMinRandomness( ) const { return mMinRandomness; }
		
		void fSetMaxRandomness( const f32 val ) { mMaxRandomness = val; }
		f32 fMaxRandomness( ) const { return mMaxRandomness; }

		void fSetKeepLastKeyValue( b32 keep ) { mKeepLastKeyValue = keep; }
		b32 fKeepLastKeyValue( ) const { return mKeepLastKeyValue; }
		
		void fSetUseLerp( b32 lerp ) { mUseLerp = lerp; }
		b32 fUseLerp( ) const { return mUseLerp; }

		void fSetUseRandoms( b32 useRandoms ) { mUseRandom = useRandoms; }
		b32 fUseRandoms( ) const { return mUseRandom; }

	protected:

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
					const f32 x = sX / diff;
					
					mValueArray[ valueIdx  ] = Math::fLerp( key1->fValue< t >( ), key2->fValue< t >( ), x );
					mRandoms[ valueIdx ] = Math::tVec2f( fMinRandomness( ), fMaxRandomness( ) );
				}
				
				// debug output
				//log_line( "key1=" << key1->fValue( ) << "key2=" << key2->fValue( ) );
				//log_line( "diff = " << diff );
				//log_line( "x = " << x );
				//log_line( "mValueArray[ " << valueIdx << " ] = " << mValueArray[ valueIdx ] );
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
		}

		virtual void fPrintValueArray( )
		{
			//for( u32 i = 0; i < N; ++i )
			//	log_line( "mValueArray[ " << i << " ] = " << mValueArray[ i ] );
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

	class tBinaryGraph : public Rtti::tSerializableBaseClass
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryGraph, 0x683D6FCC );
	public:

		tBinaryGraph( )
		{
		}

		tBinaryGraph( tNoOpTag )
			: mRandoms( cNoOpTag )
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


		tDynamicArray< Math::tVec2f > mRandoms;
		b16 mUseLerp;
		b16 mUseRandom;

	protected:
		virtual void fSampleGraph( const tMersenneGenerator* random, f32& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec2f& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec3f& value, f32 x ) const { }
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec4f& value, f32 x ) const { }

	private:
	};



	template< class t >
	class tBinaryTemplateGraph : public tBinaryGraph
	{
		declare_reflector( );
		
	protected:

		tBinaryTemplateGraph( )
		{
		}

		tBinaryTemplateGraph( tNoOpTag )
			: tBinaryGraph( cNoOpTag )
			, mValueArray( cNoOpTag )
		{

		}

		t fSampleGraphInternal( const tMersenneGenerator* random, f32 x ) const
		{
			sigassert( x >= 0.f && x <= 1.f );
			
			t value( 0.f );
			
			if( !mUseLerp && x >= mRandoms[ 1 ].x )
			{
				value = mValueArray[ 1 ];
				if( mUseRandom )
					value += random->fFloatInRange( mRandoms[ 0 ].x, mRandoms[ 0 ].y );
			}
			else if( mUseLerp )
			{
				const u32 N = mValueArray.fCount( );
				const u32 x1 = ( u32 ) ( x * ( N-1 ) );
				const u32 x2 = x1 == ( N-1 ) ? x1 : ( x1 + 1 );

				const f32 inverseN = 1.f / ( f32) N;
				const f32 delta = x - x1 * inverseN;
				value = Math::fLerp( mValueArray[ x1 ], mValueArray[ x2 ], delta );

				if( mUseRandom && random )
					value += random->fFloatInRange( mRandoms[ x1 ].x, mRandoms[ x1 ].y );
			}
			
			return value;
		}

	public:

		virtual void fCopyFromGraph( tGraphPtr graph )
		{
			u32 N = graph->mRandoms.fCount( );
			mRandoms.fNewArray( N );
			mValueArray.fNewArray( N );

			for( u32 i = 0; i < N; ++i )
			{
				f32 x = ( f32 ) i / ( f32 ) ( N - 1 );
				mRandoms[ i ] = graph->mRandoms[ i ];
				mValueArray[ i ] = graph->fSample< t >( 0, x );
			}
			
			mUseLerp = graph->mUseLerp;
			mUseRandom = graph->mUseRandom;
		}

		tDynamicArray< t > mValueArray;
		//t mValueArray[ N ];
		//static const f32 mInverseN;
	};

	//template< class t >
	//const f32 tBinaryTemplateGraph<t>::mInverseN = 1.f / (f32)tBinaryTemplateGraph<t>::N;

	class tBinaryF32Graph : public tBinaryTemplateGraph< f32 >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryF32Graph, 0x5B53F92A );
		tBinaryF32Graph( ) {	}
		tBinaryF32Graph( tNoOpTag ) : tBinaryTemplateGraph< f32 >( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, f32& value, f32 x ) const { value = fSampleGraphInternal( random, x ); }
	};
	class tBinaryV2Graph : public tBinaryTemplateGraph< Math::tVec2f >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryV2Graph, 0x8DBD85F6 );
		tBinaryV2Graph( ) {	}
		tBinaryV2Graph( tNoOpTag ) : tBinaryTemplateGraph< Math::tVec2f >( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec2f& value, f32 x ) const { value = fSampleGraphInternal( random, x ); }
	};
	class tBinaryV3Graph : public tBinaryTemplateGraph< Math::tVec3f >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryV3Graph, 0x3C4F4A0B );
		tBinaryV3Graph( ) {	}
		tBinaryV3Graph( tNoOpTag ) : tBinaryTemplateGraph< Math::tVec3f >( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec3f& value, f32 x ) const { value = fSampleGraphInternal( random, x ); }
	};
	class tBinaryV4Graph : public tBinaryTemplateGraph< Math::tVec4f >
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tBinaryV4Graph, 0x6C5803A );
		tBinaryV4Graph( ) {	}
		tBinaryV4Graph( tNoOpTag ) : tBinaryTemplateGraph< Math::tVec4f >( cNoOpTag )	{	}
		virtual void fSampleGraph( const tMersenneGenerator* random, Math::tVec4f& value, f32 x ) const { value = fSampleGraphInternal( random, x ); }
	};

	tBinaryGraph* fCreateNewGraph( tGraphPtr original );

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
