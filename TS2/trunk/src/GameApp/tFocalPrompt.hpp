#ifndef __tFocalPrompt__
#define __tFocalPrompt__
#include "Gui/tScriptedControl.hpp"

namespace Sig { namespace Gui
{
	class tFocalPrompt : public tScriptedControl
	{
	public:
		explicit tFocalPrompt( const tResourcePtr& scriptResource, tUser& user );
		~tFocalPrompt( ) { }

		void fShow( const tStringPtr& locID );
		void fHide( b32 hide = true );
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	private:
	};

	typedef tRefCounterPtr< tFocalPrompt > tFocalPromptPtr;
}}

#endif //__tFocalPrompt__