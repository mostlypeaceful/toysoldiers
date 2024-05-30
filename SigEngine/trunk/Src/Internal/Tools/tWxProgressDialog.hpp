#ifndef __tWxProgressDialog__
#define __tWxProgressDialog__
#include "wx/progdlg.h"

namespace Sig
{
	class tools_export tWxProgressDialog : public wxProgressDialog
	{
		f32 mProgress;
		b32 mPump;
	public:
		tWxProgressDialog( wxWindow* parent, const char* title, b32 pumpMessages = true  );
		virtual ~tWxProgressDialog( );
		virtual void fUpdate( f32 progressZeroToOne, const char* message="" );
		f32 fGetProgress( ) const { return mProgress; }

	private:

		void fPump( );
	};
}

#endif//__tWxProgressDialog__
