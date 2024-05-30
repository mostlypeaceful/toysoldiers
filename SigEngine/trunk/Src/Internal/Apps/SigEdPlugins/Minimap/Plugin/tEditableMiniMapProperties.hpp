#ifndef __tEditableMiniMapProperties__
#define __tEditableMiniMapProperties__
#include "iSigEdPlugin.hpp"
#include "tEntityData.hpp"
#include "tAssetPluginDll.hpp"

namespace Sig
{

	class tEditableMiniMapProperties;

	class dll_export tMiniMapEditorPlugin : public iAssetPlugin, public iSigEdPlugin
	{
	public:
		// from iAssetPlugin	
		virtual iSigEdPlugin* fGetSigEdPluginInterface( ) { return this; }
	
	public:
		// from iSigEdPlugin	
		virtual void fConstruct( tEditorAppWindow* editor );
		virtual std::string fName( ) const { return "Mini Map"; }

		static const u32 cUniqueID = 0x3F6F762C;
		virtual u32 fUniqueID( ) const { return cUniqueID; }

		virtual void fToggle( );
		virtual void fFileOpened( );

		virtual tEntityData* fSerializeData( tLoadInPlaceFileBase* fileOut, tEditorPluginData* baseDataPtr );

	private:
		tEditableMiniMapProperties* mDialog;
	};

}

#endif//__tEditableMiniMapProperties__
