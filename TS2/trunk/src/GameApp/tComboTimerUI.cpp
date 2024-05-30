#include "GameAppPch.hpp"
#include "tComboTimerUI.hpp"
#include "tGameSessionStats.hpp"

namespace Sig { namespace Gui
{
	devvar( f32, Gameplay_Stats_Combos_AlphaLerp, 0.25f );
	devvar( f32, Gameplay_Stats_Combos_PositionLerp, 0.25f );

	namespace 
	{
		static const f32 cAlphaThreshold = 0.01f;
	}

	namespace { static const tStringPtr cCanvasCreateComboTimerUI( "CanvasCreateComboTimerUI" ); }

	tComboTimerUI::tComboTimerUI( const tResourcePtr& scriptResource, const tUserPtr& user, const tStringPtr& texturePath )
		: tScriptedControl( scriptResource )
		, mUser( user )
		, mCurrentPos( Math::tVec2f::cZeroVector )
		, mAlpha( 0.0f )
	{
		sigassert( mScriptResource->fLoaded( ) );
		fCreateControlFromScript( cCanvasCreateComboTimerUI, this );
		sigassert( !mCanvas.fIsNull( ) );

		Sqrat::Function( mCanvas.fScriptObject( ), "SetTexture" ).Execute( texturePath );
	}

	tComboTimerUI::~tComboTimerUI( )
	{
	}

	void tComboTimerUI::fStep( tComboStatGroup& combo, f32 dt )
	{
		b32 canSee = ( mAlpha > cAlphaThreshold );

		if( canSee )
			mCurrentPos = fLerp( mCurrentPos, combo.mScreenPos, (f32)Gameplay_Stats_Combos_PositionLerp );
		else if( combo.mRestart )
		{
			// user will not see the timer, snap it to where should be
			mCurrentPos = combo.mRestartPt;
		}

		// whether we handled it or not, clear it
		combo.mRestart = false;

		f32 targetAlpha = ( combo.mComboTimeRemaining > 0.0f ) ? 1.0f : 0.0f;
		mAlpha = Math::fLerp( mAlpha, targetAlpha, (f32)Gameplay_Stats_Combos_AlphaLerp );

		// Apply results
		fSetColor( Math::tVec4f( combo.mColor, 1.f ) );
		fCanvas( ).fCodeObject( )->fSetPosition( Math::tVec3f( mCurrentPos, 0.5f ) );

		Sqrat::Function( mCanvas.fScriptObject( ), "SetCombo" ).Execute( combo.mCurrentCombo );
		Sqrat::Function( mCanvas.fScriptObject( ), "SetTimerMeter" ).Execute( combo.mComboTimeRemaining / combo.mComboTimer );
		Sqrat::Function( mCanvas.fScriptObject( ), "SetAlpha" ).Execute( mAlpha );
	}

	void tComboTimerUI::fShow( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( true );
	}

	void tComboTimerUI::fHide( )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "Show" ).Execute( false );
	}

	void tComboTimerUI::fSetColor( const Math::tVec4f& color )
	{
		Sqrat::Function( mCanvas.fScriptObject( ), "SetColor" ).Execute( color );
	}
}}


namespace Sig { namespace Gui
{
	void tComboTimerUI::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tComboTimerUI,Sqrat::NoConstructor> classDesc( vm.fSq( ) );
		//classDesc
		//	;
		vm.fNamespace(_SC("Gui")).Bind( _SC("ComboTimerUI"), classDesc );
	}
}}

