#ifndef __tMayaCheckOutButton__
#define __tMayaCheckOutButton__
#include "tWxSlapOnButton.hpp"

namespace Sig
{
	class tMayaCheckOutButton : public tWxSlapOnButton
	{
	public:

		tMayaCheckOutButton( wxWindow* parent, const char* label );
		virtual void fOnControlUpdated( );
	};
}

#endif//__tMayaCheckOutButton__
