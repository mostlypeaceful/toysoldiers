#include "SigAIPch.hpp"
#include "tSigAIMainWindow.hpp"
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

			wxFrame *frame = new tSigAIMainWindow( );
			frame->Show( );
			SetTopWindow( frame );
			return true;
		}
	};

}

using namespace Sig;
IMPLEMENT_APP( tSigAIApp )

