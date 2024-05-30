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

	f32 tFpsText::fBuild( Gfx::tScreen& screen )
	{
		const wchar_t* buildText = 
#if defined( target_game )
	#if defined( build_debug )
				L"BUILD: DEBUG\n";
	#elif defined( build_internal )
				L"BUILD: INTERNAL\n";
	#elif defined( build_playtest )
				L"BUILD: PLAYTEST\n";
	#elif defined( build_profile )
				L"BUILD: PROFILE\n";
	#elif defined( build_release )
				L"BUILD: RELEASE\n";
	#endif//build_xxxx
#else//target_xxx
			L"";
#endif//target_xxx

		mCounter.fPreRender( );
		const f32 fps = mCounter.fFps( );
		const f32 logicMs = mCounter.fLogicMs( );
		const f32 renderCpuMs = screen.fPreSwapTimer( ).fGetElapsedMs( );
#ifdef sig_profile_xbox_gpu
		const f32 renderGpu = screen.fGetLastGpuTimeMs( );
#endif//sig_profile_xbox_gpu

		wchar_t fpsText[512]={0};
		snwprintf( fpsText, array_length( fpsText ), 
			L"%s" 
			L"fps %.1f\n"
#if defined( target_game )
			L"logic = %.2f ms\n"
			L"render cpu = %.2f ms\n"
#ifdef sig_profile_xbox_gpu
			L"render gpu = %.2f ms\n"
#endif//sig_profile_xbox_gpu
#endif//defined( target_game )
			, buildText
			, fps
#if defined( target_game )
			, logicMs
			, renderCpuMs
#ifdef sig_profile_xbox_gpu
			, renderGpu
#endif//sig_profile_xbox_gpu
#endif//defined( target_game )
			);

		return fBakeBox( 400, fpsText, 0, Gui::tText::cAlignLeft );
	}

	void tFpsText::fAddDrawCall( Gfx::tScreen& screen, f32 x, f32 y )
	{
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
