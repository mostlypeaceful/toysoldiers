#include "GameAppPch.hpp"
#include "tLightEffectLogic.hpp"
#include "tSceneGraph.hpp"
#include "tProfiler.hpp"

using namespace Sig::Math;

namespace Sig
{	

	tLightEffectLogic::tLightEffectLogic( f32 radius, f32 expandTime, f32 collapseTime, b32 persist )
		: mRadius( radius )
		, mAge( 0 )
		, mExpandTime( expandTime )
		, mCollapseTime( collapseTime )
		, mExpanding( false )
		, mPersist( persist )
		, mAlive( true )
	{
		fRestart( );
	}

	void tLightEffectLogic::fOnDelete( )
	{
		mLight.fRelease( );
		tLogic::fOnDelete( );
	}

	void tLightEffectLogic::fOnSpawn( )
	{
		fOnPause( false );
		fOwnerEntity( )->fAddGameTagsRecursive( GameFlags::cFLAG_DUMMY );

		sigassert( mLight );

		f32 initialSize = mExpanding ? 0.f : 1.f;
		mLight->mLight.fSetTypePoint( fGetRadii( ) * initialSize );
		mLight->fSetLightDesc( mLight->mLight );
	}

	void tLightEffectLogic::fOnPause( b32 paused )
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

	void tLightEffectLogic::fSetParameters( f32 radius, f32 expandTime, f32 collapseTime )
	{
		mRadius = radius;
		mExpandTime = expandTime;
		mCollapseTime = collapseTime;
	}

	void tLightEffectLogic::fSetColor( const Math::tVec4f& color )
	{
		mLight->fColor( Gfx::tLight::cColorTypeFront ) = color;
	}

	void tLightEffectLogic::fSetPersist( b32 persist )
	{
		mPersist = persist;
		if( !mPersist && !mAlive && mLight ) mLight->fDelete( );
	}

	void tLightEffectLogic::fRestart( )
	{
		mAlive = true;
		mAge = 0.f;
		mExpanding = ( mExpandTime > 0.f );
	}

	void tLightEffectLogic::fSetActive( b32 active )
	{
		mAlive = active;
		if( !mAlive ) mLight->fSetOn( false );
	}

	Math::tVec2f tLightEffectLogic::fGetRadii( ) const
	{
		return Math::tVec2f( mRadius, mRadius * 2.0f );
	}
	
	void tLightEffectLogic::fMoveST( f32 dt )
	{
		profile( cProfilePerfLightEffectsMoveST );

		if( mAlive )
		{
			mAge += dt;

			f32 size = 1.f;

			if( mExpanding )
			{
				size = mAge / mExpandTime;
				if( mAge > mExpandTime )
				{
					mExpanding = false;
					mAge = 0.f;
				}
			}
			else
			{
				size = mAge / mCollapseTime;
				size = 1.f - size;

				if( size < 0 )
				{
					if( mPersist ) 
						mAlive = false;
					else 
					{
						fOwnerEntity( )->fDelete( );
						return;
					}
				}
			}

			size *= size;

			mLight->mLight.fRadii( ) = fGetRadii( ) * size;
			mLight->fSetOn( true );
			mLight->fSetLightDesc( mLight->mLight );
		}
		else
			mLight->fSetOn( false );
	}

	/* static */ Gfx::tLightEntity* tLightEffectLogic::fSpawnLightEffect( const Math::tMat3f& worldXForm, tLightEffectLogic *logic, tEntity& parent, const Math::tVec4f& color )
	{
		sigassert( logic );

		logic->mLight.fReset( NEW Gfx::tLightEntity( worldXForm ) );
		logic->mLight->fSpawn( parent );
		logic->mLight->fColor( Gfx::tLight::cColorTypeFront ) = color;

		tLogicPtr *lpp = NEW tLogicPtr( logic );
		logic->mLight->fAcquireLogic( lpp );

		return logic->mLight.fGetRawPtr( );
	}
}


namespace Sig
{
	void tLightEffectLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tLightEffectLogic, tLogic, Sqrat::NoCopy<tLightEffectLogic> > classDesc( vm.fSq( ) );
		//classDesc;

		vm.fRootTable( ).Bind(_SC("LightEffectLogic"), classDesc);
	}
}

