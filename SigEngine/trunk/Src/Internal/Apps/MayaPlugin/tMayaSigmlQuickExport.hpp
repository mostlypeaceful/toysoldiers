#ifndef __tMayaSigmlQuickExport__
#define __tMayaSigmlQuickExport__
#include "tWxSlapOnQuickExport.hpp"
#include "tMayaEvent.hpp"

namespace Sig
{
	class tMayaSigmlQuickExport : public tWxSlapOnQuickExport
	{
		tMayaEventPtr mSceneOpened;
		tMayaEventPtr mSceneImported;
	public:
		tMayaSigmlQuickExport( wxWindow* parent, const char* label );
	private:
		void fOnSceneOpened( );
		virtual wxString fGetFileWildcard( ) const;
		virtual void fOnControlUpdated( );
		virtual void fOnButtonPressed( );
	};
}

#endif//__tMayaSigmlQuickExport__
