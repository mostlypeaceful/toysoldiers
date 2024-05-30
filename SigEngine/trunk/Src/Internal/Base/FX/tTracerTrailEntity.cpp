#include "BasePch.hpp"
#include "tTracerTrailEntity.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig { namespace FX
{

	tTracerTrailEntity::tTracerTrailEntity( tEntity& parent, const tTracerTrailDef& def )
		: tQuadTrailEntity( def.mTexture, def.mTint, &def.mRenderState, def.mBeefyQuads )
		, mDef( def )
		, mLastSpawn( 0.0f )
		, mSpinAngle( 0.0f )
		, mParent( &parent )
		, mPrevXform( parent.fObjectToWorld( ) )
		, mIntensity( 1.f )
	{
		fSetLockedToParent( false );

		mSpinVariation = sync_rand( fFloatInRange( 0.5f, 1.5f ) );

		if( sync_rand( fBool( ) ) )
			mSpinVariation *= -1.0f;
	}

	void tTracerTrailEntity::fOnSpawn( )
	{
		tQuadTrailEntity::fOnSpawn( );

		fOnPause( false );

		//spawn one element right away
		mLastSpawn = mDef.mSpawnRate;

		const f32 smallFakeDT = 0.0001f;
		fHeavyLiftingMTSafe( smallFakeDT, true );
		fThinkST( smallFakeDT );
	}
	
	void tTracerTrailEntity::fOnDelete( )
	{
		tQuadTrailEntity::fOnDelete( );

		mParent.fRelease( );
	}

	void tTracerTrailEntity::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListEffectsMT );
			fRunListRemove( cRunListThinkST );
			fRunListRemove( cRunListCoRenderMT );
		}
		else
		{
			fRunListInsert( cRunListEffectsMT );
			fRunListInsert( cRunListThinkST );
			fRunListInsert( cRunListCoRenderMT );
		}
	}

	void tTracerTrailEntity::fStopTrackingParent( )
	{
		mParent.fRelease( );
	}

	f32 tTracerTrailEntity::fComputeAlpha( f32 age ) const
	{
		return fClamp( 1.0f - age/mDef.mLifeSpan, 0.0f, 1.0f );
	}

	void tTracerTrailEntity::fHeavyLiftingMTSafe( f32 dt, b32 firstFrame )
	{
		const Math::tPRSXformf parentObjToWorld( mParent ? mParent->fObjectToWorld( ) : Math::tMat3f::cIdentity );
		const u32 numSteps = ( firstFrame || !mParent ) ? 1 : mDef.mStepCount;
		const f32 dtScale = 1.f/numSteps;
		for( u32 i = 0; i < numSteps; ++i )
		{
			const f32 parentLerp = ( i + 1.f ) / numSteps;
			fHeavyLiftingMTSafeInternal( dt * dtScale, parentLerp, parentObjToWorld );
		}
		mPrevXform = parentObjToWorld;
	}

	void tTracerTrailEntity::fHeavyLiftingMTSafeInternal( f32 dt, f32 parentLerp, const Math::tPRSXformf& parentObjToWorld )
	{
		// dynamic resizing, for debugging
		s32 elementsNeeded = s32(mDef.mLifeSpan / mDef.mSpawnRate) + 1;
		if(elementsNeeded > 0 && (s32)mElements.fCapacity( ) < elementsNeeded )
			mElements.fResize( elementsNeeded );

		// update trail
		if( mParent )
		{
			mSpinAngle += mDef.mSpinRate * mSpinVariation * dt;

			// always keep the head attached to parent
			tPRSXformf xform = mPrevXform;
			xform.fBlendNLerp( parentObjToWorld, parentLerp );

			const Math::tMat3f trans( xform);
			mHead = fCreateElement( trans );

			mLastSpawn += dt;

			if( mLastSpawn >= mDef.mSpawnRate )
			{
				// extend trail
				mLastSpawn -= mDef.mSpawnRate;

				if( mDef.mSmooth )
				{
					// delete any predicted elements before putting new ones
					while( mElements.fNumItems( ) >= 2 && mElements[ mElements.fNumItems( ) - 2 ].mPredicted )
					{
						tElement dummy, head;
						mElements.fTryPopLast( head );
						mElements.fTryPopLast( dummy );
						mElements.fPushLast( head );
					}

					for( u32 i = 0; i < mElements.fNumItems( ); ++i )
						sigassert( !mElements[ i ].mPredicted );

					// smooth out the existing path
					if( mElements.fNumItems( ) >= 3 )
					{
						const tElement fixup0 = fCatmullRom( 
							mElements[ mElements.fNumItems( ) - 3 ],
							mElements[ mElements.fNumItems( ) - 2 ],
							mElements[ mElements.fNumItems( ) - 1 ],
							mHead,
							0.333f );
						const tElement fixup1 = fCatmullRom( 
							mElements[ mElements.fNumItems( ) - 3 ],
							mElements[ mElements.fNumItems( ) - 2 ],
							mElements[ mElements.fNumItems( ) - 1 ],
							mHead,
							0.666f );

						tElement oldHead;
						mElements.fTryPopLast( oldHead );
						mElements.fPushLast( fixup0 );
						mElements.fPushLast( fixup1 );
						mElements.fPushLast( oldHead );

						const tElement tween0 = fCatmullRom( 
							mElements[ mElements.fNumItems( ) - 2 ],
							mElements[ mElements.fNumItems( ) - 1 ],
							mHead,
							0.333f );
						const tElement tween1 = fCatmullRom( 
							mElements[ mElements.fNumItems( ) - 2 ],
							mElements[ mElements.fNumItems( ) - 1 ],
							mHead,
							0.666f );
						mElements.fPushLast( tween0 );
						mElements.fPushLast( tween1 );
					}
				}

				mElements.fPushLast( mHead );
			}
		}
		else
		{
			//fade out head
			mHead.mAge += dt;
			mHead.mAgeAlpha = fComputeAlpha( mHead.mAge );
		}

		// tick and remove dead elements
		for( u32 i = 0; i < mElements.fNumItems( ); ++i )
		{
			tElement &e = mElements[ i ];
			e.mAge += dt;
			e.mAgeAlpha = fComputeAlpha( e.mAge );

			// dont remove element until next element has faded out also
			if( e.mAge >= mDef.mLifeSpan * 2.0f )
			{
				mElements.fTryPopFirst( );
				--i;
				continue;
			}
		}

		fFillGraphics( );
	}

	void tTracerTrailEntity::fThinkST( f32 dt )
	{
		if( !mParent && !fAlive( ) )
		{
			fDelete( );
		}
		else
		{
			tQuadTrailEntity::fThinkST( dt );
		}
	}

	void tTracerTrailEntity::fEffectsMT( f32 dt )
	{
		if( mDef.mRealTime )
			fHeavyLiftingMTSafe( dt );
	}

	void tTracerTrailEntity::fCoRenderMT( f32 dt )
	{
		if( !mDef.mRealTime )
			fHeavyLiftingMTSafe( dt );
	}

	tTracerTrailEntity::tElement tTracerTrailEntity::fCreateElement( const Math::tMat3f& transform ) const
	{
		tVec3f localAxis( Math::fCos( mSpinAngle ), Math::fSin( mSpinAngle ), 0 );

		tElement newElement;
		newElement.mPosition = transform.fGetTranslation( ) + transform.fZAxis( ) * mDef.mLeadDistance;
		newElement.mAxis = transform.fXformVector( localAxis ) * mDef.mWidth;
		newElement.mAge = 0.0f;
		newElement.mAgeAlpha = 1.f;
		newElement.mAlpha = mIntensity;
		newElement.mPredicted = false;
		return newElement;
	}


}}
