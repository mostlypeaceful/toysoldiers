#include "GameAppPch.hpp"
#include "tTankPalette.hpp"
#include "tGameEffects.hpp"
#include "Physics/tGroundRayCastCallback.hpp"
#include "Gfx/tRenderableEntity.hpp"

using namespace Sig::Math;
using namespace Sig::Physics;

namespace Sig
{	
	namespace 
	{ 
		const tStringPtr cCollisionProbe( "collisionProbe" );
		const tStringPtr cCollisionEffect( "TankPaletteLand" );
	}

	tTankPalette::tTankPalette( )
		: mFired( false )
	{
	}
	void tTankPalette::fOnSpawn( )
	{
		mProbe.fReset( fOwnerEntity( )->fFirstDescendentWithName( cCollisionProbe ) );
		mInitialY = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ).y;
		tLogic::fOnSpawn( );
		fOnPause( false );
	}
	void tTankPalette::fOnDelete( )
	{
		mProbe.fRelease( );
		tLogic::fOnDelete( );
	}
	void tTankPalette::fOnPause( b32 paused )
	{
		if( paused )
		{
			fRunListRemove( cRunListThinkST );
		}
		else
		{
			fRunListInsert( cRunListThinkST );
		}
	}
	void tTankPalette::fThinkST( f32 dt )
	{
		if( mProbe )
		{
			f32 currentY = fOwnerEntity( )->fObjectToWorld( ).fGetTranslation( ).y;

			// probeLength expected to be negative
			const f32 probeLength = mProbe->fParentRelative( ).fGetTranslation( ).y;
			if( currentY <= mInitialY + probeLength - 1.f ) //one meter extra buffer
			{

				tRayf ray;
				ray.mExtent = tVec3f::cYAxis * probeLength;
				ray.mOrigin = mProbe->fObjectToWorld( ).fGetTranslation( ) - ray.mExtent;

				tGroundRayCastCallback cb( *fOwnerEntity( ), GameFlags::cFLAG_GROUND );
				fSceneGraph( )->fRayCastAgainstRenderable( ray, cb );

				if( cb.mHit.fHit( ) )
				{
					tGameEffects::fInstance( ).fPlayEffect( fOwnerEntity( ), cCollisionEffect );
					mProbe.fRelease( );
					mFired = true;
				}
			}
		}
	}
}


namespace Sig
{
	void tTankPalette::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::DerivedClass<tTankPalette, tLogic, Sqrat::NoCopy<tTankPalette> > classDesc( vm.fSq( ) );
			//classDesc
			//	;

			vm.fRootTable( ).Bind(_SC("TankPalette"), classDesc);
		}
	}
}

