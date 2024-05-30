#include "GameAppPch.hpp"
#include "tHoverTimer.hpp"
#include "tGameApp.hpp"
#include "tUnitLogic.hpp"

namespace Sig { namespace Gui
{
	tHoverTimer::tHoverTimer( const tStringPtr& prefix, f32 time, b32 iconMode, u32 team )
		: mTime( time )
		, mDuration( time )
		, mPrefix( tGameApp::fInstance( ).fLocString( prefix ) )
		, mFadingOut( false )
		, mIconMode( iconMode )
	{
		tUserArray teamUsers;
		for( u32 i = 0; i < tGameApp::fInstance( ).fPlayerCount( ); ++i )
		{
			const tPlayer* player = tGameApp::fInstance( ).fGetPlayer( i );
			if( player->fTeam( ) == team )
				teamUsers.fPushBack( player->fUser( ) );
		}
		fCreate( tGameApp::fInstance( ).fGlobalScriptResource( tGameApp::cGlobalScriptHoverTimer ), teamUsers, tGameApp::fInstance( ).fRootHudCanvas( ).fToCanvasFrame( ) );

		const Gui::tWorldSpaceScriptedControl::tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
		{
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "Setup" ).Execute( iconMode );
			controls[ i ].mControl->fCanvas( ).fCanvas( )->fSetScissorRect( controls[ i ].mUser->fComputeViewportRect( ) );
			controls[ i ].mControl->fCanvas( ).fCanvas( )->fSetVirtualMode( controls[ i ].mUser->fIsViewportVirtual( ) );
		}
	}
	
	tHoverTimer::~tHoverTimer( )
	{
	}

	b32 tHoverTimer::fFadeOut( )
	{
		if( !mFadingOut )
		{
			mFadingOut = true;
			const Gui::tWorldSpaceScriptedControl::tControlArray& controls = fAccessControls( );
			for( u32 i = 0; i < controls.fCount( ); ++i )
				Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "FadeOut" ).Execute( );
		}
		else
		{
			const Gui::tWorldSpaceScriptedControl::tControlArray& controls = fAccessControls( );
			for( u32 i = 0; i < controls.fCount( ); ++i )
				if( Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "FadedOut" ).Evaluate<bool>( ) )
					return true;
		}

		return false;
	}

	void tHoverTimer::fThinkST( f32 dt )
	{
		if( mTime > 0 )
		{
			mTime -= dt;

			mTime = fMax( mTime, 0.f );
			
			if( !mIconMode )
			{
				tLocalizedString str = mPrefix;
				std::string timeStr = StringUtil::fToString( mTime, 0 ) + "s";
				str.fJoinWithCString( timeStr.c_str( ) );
				fSetText( str );
			}
		}

		if( mIconMode )
		{
			// Scale based on distance
			const Gui::tWorldSpaceScriptedControl::tControlArray& controls = fAccessControls( );
			for( u32 i = 0; i < controls.fCount( ); ++i )
			{
				f32 scale = 1.0;
				const Gfx::tViewportPtr& vp = controls[ i ].mUser->fViewport( );
				const Gfx::tCamera& camera = vp->fRenderCamera( );
				const Math::tVec3f distToCamera = fObjectToWorld( ).fGetTranslation( ) - camera.fGetTripod( ).mEye;
				const f32 minDist = 10.0f;
				const f32 maxDist = 250.0f;
				const f32 minScale = 0.25f;
				scale = Math::fLerp( 1.0f, minScale, fClamp( Math::fRemapZeroToOne( minDist, maxDist, distToCamera.fLength( ) ), 0.0f, 1.0f ) );

				Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "ScaleIcon" ).Execute( scale );
			}
		}
	}

	void tHoverTimer::fSetText( const tLocalizedString& text )
	{
		const Gui::tWorldSpaceScriptedControl::tControlArray& controls = fAccessControls( );
		for( u32 i = 0; i < controls.fCount( ); ++i )
		{
			Sqrat::Function( controls[ i ].mControl->fCanvas( ).fScriptObject( ), "SetText" ).Execute( text );
			controls[ i ].mControl->fCanvas( ).fCanvas( )->fSetScissorRect( controls[ i ].mUser->fComputeViewportRect( ) );
		}
	}

	void tHoverTimer::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tHoverTimer, Gui::tWorldSpaceScriptedControl, Sqrat::NoConstructor > classDesc( vm.fSq( ) );
		classDesc
			.Prop(_SC("IconMode"),	&tHoverTimer::fIconMode)
			;

		vm.fRootTable( ).Bind(_SC("HoverTimer"), classDesc);
	}
} }

