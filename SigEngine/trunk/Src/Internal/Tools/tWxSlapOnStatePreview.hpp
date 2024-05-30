#ifndef __tWxSlapOnStatePreview__
#define __tWxSlapOnStatePreview__
#include "tWxSlapOnSpinner.hpp"

namespace Sig
{

	class tools_export tWxSlapOnStatePreview : public tWxSlapOnSpinner
	{
	protected:
		virtual void fOnControlUpdated( );

	public:
		tWxSlapOnStatePreview( wxWindow* parent, const char* label );

	};

}

#endif//__tWxSlapOnStatePreview__
