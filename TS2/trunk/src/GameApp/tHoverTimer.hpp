#ifndef __tHoverTimer__
#define __tHoverTimer__

#include "Gui\tWorldSpaceScriptedControl.hpp"


namespace Sig { namespace Gui
{
	class tHoverTimer : public tWorldSpaceScriptedControl
	{
		define_dynamic_cast( tHoverTimer, tWorldSpaceScriptedControl );
	public:
		tHoverTimer( const tStringPtr& prefix, f32 time, b32 iconMode, u32 team );
		virtual ~tHoverTimer( );

		virtual void fThinkST( f32 dt );
		b32 fFinished( ) const { return mTime <= 0.f; }
		b32 fFadeOut( );

		f32 fPercentage( ) const { return fClamp( 1.0f - (mTime / mDuration), 0.f, 1.f ); }
		f32 fDuration( ) const { return mDuration; }

		b32 fIconMode( ) const { return mIconMode; }

	private:
		void fSetText( const tLocalizedString& text );

		f32 mTime;
		f32 mDuration;
		const tLocalizedString& mPrefix;

		b32 mFadingOut;
		b32 mIconMode;

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tHoverTimer );
} }

#endif //__tHoverTimer__