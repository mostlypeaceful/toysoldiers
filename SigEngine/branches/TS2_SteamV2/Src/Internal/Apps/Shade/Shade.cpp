#include "ShadePch.hpp"
#include "tShadeMainWindow.hpp"
#include "tAssetPluginDll.hpp"


namespace Sig
{

	class tSigAIApp : public wxApp
	{
	public:

		virtual bool OnInit()
		{
			// load asset plugins right-off the bat
			tAssetPluginDllDepot::fInstance( ).fLoadPluginsBasedOnCurrentProjectFile( );

			wxFrame *frame = new tShadeMainWindow( );
			frame->Show( );
			SetTopWindow( frame );
			return true;
		}
	};

}

using namespace Sig;
IMPLEMENT_APP( tSigAIApp )

