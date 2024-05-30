#include "GameAppPch.hpp"
#include "tLightningLogic.hpp"
#include "tSceneGraph.hpp"
#include "tGameApp.hpp"
#include "tProfiler.hpp"
#include "tSync.hpp"

using namespace Sig::Math;

namespace Sig
{	

	devvar( bool, Debug_Lightning_Fracture, true );
	devvar( bool, Debug_Lightning_FracsOverride, false );
	devvar( u32, Debug_Lightning_Fracs, 3 );


	namespace 
	{ 
	}

	tLightningEntity::tLightningEntity( u32 tracerIndex ) 
		: tEntity( )
		, mTracerIndex( tracerIndex )
		, mFracs( 0 )
	{

	}

	void tLightningEntity::fOnSpawn( )
	{
		fOnPause( false );

		for( u32 i = 0; i < 3; ++i )
		{
			tLightningBolt* bolt = NEW tLightningBolt( tGameApp::fInstance( ).fTracerTrailDef( mTracerIndex ) );
			bolt->fSpawn( *this );
			mBolts.fPushBack( tRefCounterPtr<tLightningBolt>( bolt ) );
		}

		fSetFracs( mFracs );
	}

	void tLightningEntity::fOnDelete( )
	{
		mBolts.fSetCount( 0 );
		tEntity::fOnDelete( );
	}

	void tLightningEntity::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListMoveST );
		}
		else
		{
			fRunListInsert( cRunListMoveST );
		}
	}

	void tLightningEntity::fMoveST( f32 dt )
	{
		profile( cProfilePerfBreakableMoveST );
		tEntity::fMoveST( dt );
	}

	void tLightningEntity::fSetTarget( const Math::tVec3f& target )
	{
		for( u32 i = 0; i < mBolts.fCount( ); ++i )
			mBolts[ i ]->fSetTarget( target );
	}

	void tLightningEntity::fSetAlpha( f32 alpha )
	{
		for( u32 i = 0; i < mBolts.fCount( ); ++i )
			mBolts[ i ]->fSetRgbaTint( tVec4f( 1, 1, 1, alpha ) );
	}

	void tLightningEntity::fSetFracs( u32 fracs )
	{
		mFracs = fracs;
		for( u32 i = 0; i < mBolts.fCount( ); ++i )
			mBolts[ i ]->fSetFracs( fracs );
	}

	void tLightningEntity::fSetAlphaTarget( f32 alpha )
	{
		for( u32 i = 0; i < mBolts.fCount( ); ++i )
			mBolts[ i ]->fSetAlphaTarget( alpha );
	}




	tLightningBolt::tLightningBolt( const FX::tTracerTrailDef& def )
		: FX::tQuadTrailEntity( def.mTexture, def.mTint, &def.mRenderState, def.mBeefyQuads )
		, mDef( def )
		, mSegments( 0 )
		, mFracs( 0 )
		, mRandom( sync_rand( fUInt( ) ) )
		, mTarget( 0, 0, 10.f )
		, mTargetAlpha( 0.f )
		, mCurrentAlpha( 0.f )
	{
	}

	void tLightningBolt::fOnSpawn( )
	{
		FX::tQuadTrailEntity::fOnSpawn( );

		fOnPause( false );

		const f32 smallFakeDT = 0.0001f;
		fHeavyLiftingMTSafe( smallFakeDT, true );
		fThinkST( smallFakeDT );

		fFracture( );
	}

	void tLightningBolt::fOnPause( b32 paused )
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

	void tLightningBolt::fSetTarget( const Math::tVec3f& target )
	{
		mTarget = target;
	}

	void tLightningBolt::fSetAlphaTarget( f32 alpha )
	{
		mTargetAlpha = alpha;
	}

	f32 tLightningBolt::fComputeAlpha( f32 age ) const
	{
		return fClamp( 1.0f - age/mDef.mLifeSpan, 0.0f, 1.0f );
	}

	void tLightningBolt::fHeavyLiftingMTSafe( f32 dt, b32 firstFrame )
	{
		fHeavyLiftingMTSafeInternal( dt );
	}

	void tLightningBolt::fHeavyLiftingMTSafeInternal( f32 dt )
	{
		if( Debug_Lightning_Fracture )
		{
			fFracture( );
		}

		fFillGraphics( );
	}

	void tLightningBolt::fSetFracs( u32 fracs )
	{
		mFracs = fracs;
		mSegments = (u32)fPow( 2.f, (f32)mFracs );

		if( mElements.fNumItems( ) != mSegments)
		{
			mElements.fResize( mSegments );
			mElements.fFill( tElement( ) );
		}
	}

	void tLightningBolt::fFrac( const tVec3f& p1, const tVec3f& p2, const tVec3f& axis, const tVec3f& otherAxis, u32 frac, f32 displacement, u32 beginSeg, u32 endSeg )
	{
		if( frac == 0 )
			mElements[ endSeg - 1 ] = fCreateElement( p1, axis );
		else
		{
			tVec3f midPoint = p1 + p2;
			midPoint *= 0.5f;

			// random offset
			midPoint += axis *      mRandom.fFloatInRange( -displacement, displacement );
			midPoint += otherAxis * mRandom.fFloatInRange( -displacement, displacement );

			u32 midSeg = endSeg - beginSeg;
			midSeg /= 2;
			midSeg += beginSeg;

			--frac;
			displacement *= 0.5f;

			fFrac( p1, midPoint, axis, otherAxis, frac, displacement, beginSeg, midSeg );
			fFrac( midPoint, p2, axis, otherAxis, frac, displacement, midSeg, endSeg );
		}
	}

	void tLightningBolt::fFracture( )
	{
		if( Debug_Lightning_FracsOverride )
			fSetFracs( Debug_Lightning_Fracs );

		f32 displacement = 10.f;

		tVec3f axis = fObjectToWorld( ).fYAxis( );
		mHead = fCreateElement( mTarget, axis ); // HEAD IS ACTUALLY TAIL LAWL

		const tMat3f& xForm = fParent( )->fObjectToWorld( );
		const tVec3f startPt = xForm.fGetTranslation( );
		fFrac( startPt, mHead.mPosition, axis, xForm.fXAxis( ), mFracs, displacement, 0, mSegments );


		//Failsafe
		//mHead = fCreateElement( tVec3f( 0,0,0 ), axis );

		//f32 step = 1.f / mSegments;
		//for( u32 i = 0; i < mSegments; ++i )
		//	mElements.fPut( fCreateElement( tVec3f( 0,0, length * step * (i+1) ), axis ) );
	}

	void tLightningBolt::fThinkST( f32 dt )
	{
		mCurrentAlpha = fLerp( mCurrentAlpha, mTargetAlpha, 0.2f );
		fSetRgbaTint( tVec4f( 1,1,1, mCurrentAlpha ) );
		FX::tQuadTrailEntity::fThinkST( dt );
	}

	void tLightningBolt::fEffectsMT( f32 dt )
	{
		if( mDef.mRealTime )
			fHeavyLiftingMTSafe( dt );
	}

	void tLightningBolt::fCoRenderMT( f32 dt )
	{
		if( !mDef.mRealTime )
			fHeavyLiftingMTSafe( dt );
	}

	tLightningBolt::tElement tLightningBolt::fCreateElement( const Math::tVec3f& pos, const Math::tVec3f& axis ) const
	{
		tElement newElement;
		newElement.mPosition = pos;
		newElement.mAxis = axis * mDef.mWidth;
		newElement.mAge = 0.0f;
		newElement.mAgeAlpha = 1.f;
		newElement.mAlpha = 1.f;
		newElement.mPredicted = false;
		return newElement;
	}
}


namespace Sig
{
	void tLightningEntity::fExportScriptInterface( tScriptVm& vm )
	{
		{
			//Sqrat::DerivedClass<tLightningEntity, tLogic, Sqrat::NoCopy<tLightningEntity> > classDesc( vm.fSq( ) );
			////classDesc
			////	;

			//vm.fRootTable( ).Bind(_SC("LightningLogic"), classDesc);
		}
	}
}

