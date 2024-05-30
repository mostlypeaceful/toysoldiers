#ifndef __tComboTimerUI__
#define __tComboTimerUI__
#include "Gui/tScriptedControl.hpp"
#include "tUser.hpp"

namespace Sig { 

	struct tComboStatGroup;
	
namespace Gui
{

	class tComboTimerUI : public tScriptedControl
	{
	public:
		explicit tComboTimerUI( const tResourcePtr& scriptResource, const tUserPtr& user, const tStringPtr& texturePath );
		~tComboTimerUI( );

		void fStep( tComboStatGroup& combo, f32 dt );
		void fShow( );
		void fHide( );
		void fSetColor( const Math::tVec4f& color );

	public:
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		tUserPtr	mUser;
		f32			mAlpha;
		Math::tVec2f mCurrentPos;
	};

	typedef tRefCounterPtr< tComboTimerUI > tComboTimerUIPtr;

}}

#endif//__tComboTimerUI__
