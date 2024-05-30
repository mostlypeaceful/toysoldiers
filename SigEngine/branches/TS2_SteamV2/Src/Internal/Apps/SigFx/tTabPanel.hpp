#ifndef __tTabPanel__
#define __tTabPanel__

#include "Editor/tEditorSelectionList.hpp"
#include "wx/notebook.h"

namespace Sig
{

	class tTabPanel : public wxNotebookPage, public Win32Util::tRegistrySerializer
	{
	public:
		tTabPanel( wxNotebook* parent );
		virtual ~tTabPanel( );

		virtual void fOnTick( f32 dt ) = 0;
		virtual void fBuildPageFromEntities( tEditorSelectionList& selected ) = 0;
		virtual std::string fRegistryKeyName( ) const { return mRegKeyName; }
		virtual void fSaveInternal( HKEY hKey ) = 0;
		virtual void fLoadInternal( HKEY hKey ) = 0;

	private:

		std::string mRegKeyName;


	};



}


#endif // __tTabPanel__