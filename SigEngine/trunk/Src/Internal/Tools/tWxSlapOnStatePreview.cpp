#include "ToolsPch.hpp"
#include "tWxSlapOnStatePreview.hpp"
#include "WxUtil.hpp"

namespace Sig
{

	tWxSlapOnStatePreview::tWxSlapOnStatePreview( wxWindow* parent, const char* label )
		: tWxSlapOnSpinner( parent, label, -1, 256, 1, 0 )
	{
	}

	void tWxSlapOnStatePreview::fOnControlUpdated( )
	{
		log_line( 0, "SPIN!" );
	}

}

