#ifndef __tMayaMatEdWindow__
#define __tMayaMatEdWindow__
#include "tMatEdMainWindow.hpp"
#include "tMayaEvent.hpp"
#include "Editor/tEditorAction.hpp"

namespace Sig
{
	class tMayaDermlMaterial;

	class tMayaMatEdWindow : public tMatEdMainWindow
	{
		tEditorActionStack mActionStack;
		tMayaEventPtr mOnMayaIdle;
		tMayaEventPtr mOnMayaSelChanged;
		tMayaDermlMaterial* mSelectedMaterial;
	public:
		static tMayaMatEdWindow* fInstance( );
	public:
		tMayaMatEdWindow( wxWindow* parent );
		~tMayaMatEdWindow( );
		void fSyncToMaterialNode( tMayaDermlMaterial* dermlMtl = 0 );
	private:
		virtual void fOnShaderSelected( const tFilePathPtr& shaderPath );
		void fUpdateDevice( );
		void fOnMayaIdle( );
		void fOnMayaSelChanged( );
	};

}

#endif//__tMayaMatEdWindow__

