#ifndef __tWorldSpaceFloatingText__
#define __tWorldSpaceFloatingText__

#include "Gui\tWorldSpaceText.hpp"

namespace Sig { namespace Gui
{
	class tWorldSpaceFloatingText : public tWorldSpaceText
	{
	public:
		tWorldSpaceFloatingText( const tUserPtr& user, u32 font, f32 zOffset = 0.0f );
		tWorldSpaceFloatingText( const tUserArray& user, u32 font, f32 zOffset = 0.0f );
		~tWorldSpaceFloatingText( );
	public:
		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );
		virtual void fWorldSpaceUIST( f32 dt );

		void fSetFloatSpeed( f32 speed ) { mFloatSpeed = speed; }
		f32 fFloatSpeed( ) const { return mFloatSpeed; }
		
		// Set to negative to never die
		void fSetTimeToLive( f32 ttl ) { mTimeToLive = ttl; mLimitedLife = (mTimeToLive > 0.f); }
		f32 fTimeToLive( ) const { return mTimeToLive; }

		void fSetScale( f32 scale ) { mScale = scale; }
		f32 fScale( ) const { return mScale; }

		void fSetTint( const Math::tVec4f& rgba );

		void fSetText( const char* );
		void fSetText( const tLocalizedString& locText );
		void fFadeIn( ) { mFadeDelta = 4.0f; }
		void fFadeOut( ) { mFadeDelta = -2.0f; }

		void fSetZOffset( f32 zOffset ) { mZOffset = zOffset; }

		void fMorphIntoFlyingText( );
	private:
		f32 mFloatSpeed;
		f32 mTimeToLive;
		f32 mCurrentLifetime;
		f32 mFadeDelta;
		f32 mScale;
		b32 mLimitedLife;
		f32 mFlyToScreenTimer;
		f32 mOverallScale;
		f32 mZOffset;
		tUserPtr mUser;
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	define_smart_ptr( base_export, tRefCounterPtr, tWorldSpaceFloatingText );
} }

#endif //__tWorldSpaceFloatingText__