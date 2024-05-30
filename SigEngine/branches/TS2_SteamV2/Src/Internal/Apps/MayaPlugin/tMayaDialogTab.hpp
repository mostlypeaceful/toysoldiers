#ifndef __tMayaDialogTab__
#define __tMayaDialogTab__
#include "tWxSlapOnPanel.hpp"
#include "tMayaGuiBase.hpp"

namespace Sig
{
	class tMayaDialogTab : 
		public tWxSlapOnPanel, 
		public tMayaContainerBase
	{
		b32				mAlwaysVisible;
		tMayaEventPtr	mOnMayaSelChanged;

		b32				mNumOfMyTypeSelected;

	public:

		tMayaDialogTab( wxWindow* parent, const char* label );
		tMayaDialogTab( wxNotebook* parent, const char* label );

	private:

		void fCommonCtor( const char* label );
		void fOnMayaSelChanged( );
		b32  fVisitSelectedNode( MDagPath& path, MObject& component );
	};
}

#endif//__tMayaDialogTab__
