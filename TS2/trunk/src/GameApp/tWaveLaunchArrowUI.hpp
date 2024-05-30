#ifndef __tWaveLaunchArrowUI__
#define __tWaveLaunchArrowUI__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { namespace Gui
{
	class tWaveLaunchArrowUI : public tEntity
	{
	public:
		explicit tWaveLaunchArrowUI( const tResourcePtr& scriptResource, const tUserPtr& user, f32 duration );
	
		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 pause );
		virtual void fWorldSpaceUIST( f32 dt );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr mUser;
		f32 mDuration;
		f32 mBounceTime;
		f32 mLifeLeft;

		f32 mCurrentAngle;
		f32 mAngleBlend;

		void fShow( );
		void fDestroy( );

		tScriptedControl mControl;
	};

	typedef tRefCounterPtr< tWaveLaunchArrowUI > tWaveLaunchArrowUIPtr;

}}

#endif//__tWaveLaunchArrowUI__
