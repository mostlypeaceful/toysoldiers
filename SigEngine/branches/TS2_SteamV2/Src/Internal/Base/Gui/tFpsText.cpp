#include "BasePch.hpp"
#include "tFpsText.hpp"
#include "Gfx/tScreen.hpp"

namespace Sig { namespace Gui
{
	tFpsText::tFpsText( )
	{
		// set defaults
		fSetRgbaTint( Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.75f ) );
		fSetPosition( Math::tVec3f( 10.f, 10.f, 0.f ) );
	}
	tFpsText::tFpsText( Gfx::tDefaultAllocators& allocators )
		: tText( allocators )
	{
		// set defaults
		fSetRgbaTint( Math::tVec4f( 1.0f, 1.0f, 1.0f, 0.75f ) );
		fSetPosition( Math::tVec3f( 10.f, 10.f, 0.f ) );
	}

	void tFpsText::fPreRender( Gfx::tScreen& screen, f32 x, f32 y )
	{
		const char* buildText = 
#if defined( target_game )
	#if defined( build_debug )
				"BUILD: DEBUG\n";
	#elif defined( build_internal )
				"BUILD: INTERNAL\n";
	#elif defined( build_playtest )
				"BUILD: PLAYTEST\n";
	#elif defined( build_profile )
				"BUILD: PROFILE\n";
	#elif defined( build_release )
				"BUILD: RELEASE\n";
	#endif//build_xxxx
#else//target_xxx
			"";
#endif//target_xxx

		mCounter.fPreRender( );
		const f32 fps = mCounter.fFps( );
		const f32 logicMs = mCounter.fLogicMs( );
		const f32 renderCpuMs = screen.fPreSwapTimer( ).fGetElapsedMs( );

		char fpsText[512]={0};
		snprintf( fpsText, array_length( fpsText ), 
			"%s" 
			"fps %.1f\n"
#if defined( target_game )
			"logic = %.2f ms\n"
			"render cpu = %.2f ms\n"
#endif//defined( target_game )
			, buildText
			, fps
#if defined( target_game )
			, logicMs
			, renderCpuMs 
#endif//defined( target_game )
			);

		fBakeBox( 400, fpsText, 0, Gui::tText::cAlignLeft );
		fSetPosition( Math::tVec3f( x, y, 0.0f ) );

		Gfx::tDrawCall drawCall = fDrawCall( );
		if( drawCall.fValid( ) )
			screen.fAddScreenSpaceDrawCall( drawCall );
	}

	void tFpsText::fPostRender( )
	{
		mCounter.fPostRender( );
	}

}}
