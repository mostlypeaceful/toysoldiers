#include "AtlasPch.hpp"
#include "tAtlasWindow.hpp"


namespace Sig
{

	///
	/// \brief Derived wxApp, basically an entry point into the wx widgets application.
	class tAtlasApp : public wxApp
	{
	public:

		///
		/// this one is called on application startup and is a good place for the app
		/// initialization (doing it here and not in the ctor allows to have an error
		/// return: if OnInit() returns false, the application terminates)
		virtual bool OnInit()
		{
			wxFrame *frame = new tAtlasWindow(wxT("SigAtlas"));
			frame->Show( );
			SetTopWindow( frame );
			return true;
		}
	};

}

using namespace Sig;
IMPLEMENT_APP(tAtlasApp)

