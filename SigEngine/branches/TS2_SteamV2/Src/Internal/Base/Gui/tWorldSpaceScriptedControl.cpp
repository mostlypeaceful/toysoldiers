#include "BasePch.hpp"
#include "tWorldSpaceScriptedControl.hpp"
#include "tProfiler.hpp"

namespace Sig { namespace Gui
{
	b32 tWorldSpaceScriptedControl::cUseFadeSettings = false;

	tWorldSpaceScriptedControl::tWorldSpaceScriptedControl( const tStringPtr& createControlFunc ) 
		: mCreateControlFunc( createControlFunc )
		, mObjectSpaceOffset( Math::tVec3f::cZeroVector )
		, mOldFadeAlpha( 0.f )
	{
		sigassert( mCreateControlFunc.fExists( ) );
	}
	void tWorldSpaceScriptedControl::fCreate( const tResourcePtr& guiScript, tUser& user, tCanvasFrame& parentCanvas )
	{
		mScreenSpaceControls.fNewArray( 1 );
		mScreenSpaceControls[ 0 ].mUser = tUserPtr( &user );
		mScreenSpaceControls[ 0 ].mControl = fCreateControl( guiScript, user );
		parentCanvas.fAddChild( mScreenSpaceControls[ 0 ].mControl->fCanvas( ) );
	}
	void tWorldSpaceScriptedControl::fCreate( const tResourcePtr& guiScript, const tUserArray& userArray, tCanvasFrame& parentCanvas )
	{
		mScreenSpaceControls.fNewArray( userArray.fCount( ) );
		for( u32 i = 0; i < mScreenSpaceControls.fCount( ); ++i )
		{
			mScreenSpaceControls[ i ].mUser = userArray[ i ];
			mScreenSpaceControls[ i ].mControl = fCreateControl( guiScript, *userArray[ i ] );
			parentCanvas.fAddChild( mScreenSpaceControls[ i ].mControl->fCanvas( ) );
		}
	}
	void tWorldSpaceScriptedControl::fOnSpawn( )
	{
		fRunListInsert( cRunListWorldSpaceUIST );
		tEntity::fOnSpawn( );
	}
	void tWorldSpaceScriptedControl::fOnPause( b32 paused )
	{
		if( paused )
			fRunListRemove( cRunListWorldSpaceUIST );
		else
			fRunListInsert( cRunListWorldSpaceUIST );
	}
	void tWorldSpaceScriptedControl::fOnDelete( )
	{
		for( u32 i = 0; i < mScreenSpaceControls.fCount( ); ++i )
			mScreenSpaceControls[ i ].mControl->fCanvas( ).fDeleteSelf( );
		mScreenSpaceControls.fDeleteArray( );
	}
	void tWorldSpaceScriptedControl::fWorldSpaceUIST( f32 dt )
	{
		profile( cProfilePerfWorldSpaceSCST );

		for( u32 i = 0; i < mScreenSpaceControls.fCount( ); ++i )
		{
			tCanvas* cppCanvas = mScreenSpaceControls[ i ].mControl->fCanvas( ).fCanvas( );
			if( !cppCanvas || cppCanvas->fDisabled( ) || mScreenSpaceControls[ i ].mUser->fIsViewportVirtual( ) )
				continue;

			const Math::tVec3f screenPos = mScreenSpaceControls[ i ].mUser->fProjectToScreen( fObjectToWorld( ).fXformPoint( mObjectSpaceOffset ), -1.0f );
			if( fInBounds( screenPos.z, 0.0f, 1.0f ) )
			{
				if( tWorldSpaceScriptedControl::cUseFadeSettings )
				{
					tEntity* logicEnt = fFirstAncestorWithLogic( );
					if( logicEnt )
					{
						Gfx::tRenderableEntity* renderableEnt = logicEnt->fFirstDescendentOfType<Gfx::tRenderableEntity>( );
						if( renderableEnt )
						{							
							const f32 fadeAlpha = renderableEnt->fComputeFadeAlpha( mScreenSpaceControls[ i ].mUser->fViewport( )->fRenderCamera( ) );
							const f32 curAlpha = cppCanvas->fAlpha( );
							cppCanvas->fSetAlpha( ( curAlpha ? curAlpha : 1.0f ) * fadeAlpha / ( mOldFadeAlpha ? mOldFadeAlpha : 1.0f) );
							mOldFadeAlpha = fadeAlpha;
						}
					}
				}

				cppCanvas->fSetInvisible( false );
				//cppCanvas->fSetPosition( Math::tVec3f( screenPos.fXY(), 0.0f ) );
				cppCanvas->fSetPosition( Math::tVec3f( screenPos.fXY(), cppCanvas->fZPos( ) ) );
			}
			else
			{
				cppCanvas->fSetZPos( 0.0f );
				cppCanvas->fSetInvisible( true );
			}
		}
	}
	tScriptedControlPtr tWorldSpaceScriptedControl::fCreateControl( const tResourcePtr& guiScript, const tUser& user )
	{
		sigassert( guiScript->fLoaded( ) );

		tScriptedControlPtr control( NEW tScriptedControl( guiScript ) );

		control->fCreateControlFromScript( mCreateControlFunc, this );
		sigassert( !control->fCanvas( ).fIsNull( ) );

		tCanvas* cppCanvas = control->fCanvas( ).fCanvas( );
		sigassert( cppCanvas );
		cppCanvas->fSetInvisible( true );

		const Math::tRect safeRect = user.fComputeViewportRect( );
		cppCanvas->fSetScissorRect( safeRect );

		return control;
	}

	void tWorldSpaceScriptedControl::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tWorldSpaceScriptedControl,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		vm.fNamespace(_SC("Gui")).Bind( _SC("WorldSpaceControl"), classDesc );
	}

}}

