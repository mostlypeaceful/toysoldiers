#ifndef __tMayaTexturePathFixerButton__
#define __tMayaTexturePathFixerButton__
#include "tWxSlapOnButton.hpp"

namespace Sig
{
	class tMayaTexturePathFixerButton : public tWxSlapOnButton
	{
	public:

		tMayaTexturePathFixerButton( wxWindow* parent, const char* label );
		virtual void fOnControlUpdated( );
	};
}

#endif//__tMayaTexturePathFixerButton__
