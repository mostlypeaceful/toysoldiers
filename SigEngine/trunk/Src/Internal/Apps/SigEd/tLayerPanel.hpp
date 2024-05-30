#ifndef __tLayerPanel__
#define __tLayerPanel__
#include "tWxToolsPanel.hpp"

namespace Sig
{
	namespace Sigml { class tFile; }
	class tObjectLayerGui;
	class tEditableObjectContainer;
	class tEditableObject;

	class tLayerPanel : public tWxToolsPanelTool
	{
		tWxSlapOnGroup* mLayerGroup;
		u32 mBaseLayerSizerIndex;
		tGrowableArray<tObjectLayerGui*> mLayers;
		b32 mDialogInputActive;
		tObjectLayerGui* mDragging;
		tObjectLayerGui* mDragOver;


		// If this is true then this set of "layers" is actually a set of potential visibility sets.
		//  It's just so similar in manipulation and UI that the same class is used for both.
		b32 mForVisibility;

	public:
		tLayerPanel( tWxToolsPanel* parent, b32 forVisibility );
		virtual void fOnTick( );
		tEditableObjectContainer& fEditableObjects( );
		const tEditableObjectContainer& fEditableObjects( ) const;
		void fReset( b32 subReset = false );
		void fReset( const Sigml::tFile& file );
		void fMergeLayers( const Sigml::tFile& file );
		void fSaveLayers( Sigml::tFile& file, const tEditorSelectionList* selected ) const;
		void fDeleteLayer( tObjectLayerGui* layer, b32 subDelete = false );
		b32  fRenameLayer( tObjectLayerGui* layer, const std::string& name );
		void fClearFocus( );
		void fSetDialogInputActive( b32 active ) { mDialogInputActive = active; }
		void fMarkDirty( );
		void fBumpUp( tObjectLayerGui* layer );
		void fBumpDown( tObjectLayerGui* layer );
		u32  fFindLayerIndex( tObjectLayerGui* layer ) const;
		u32  fNumLayers( ) const { return mLayers.fCount( ); }
		const tGrowableArray<tObjectLayerGui*>& fLayers( ) const { return mLayers; }

		void fBeginBuildGui( );
		void fEndBuildGui( );

		b32 fDragging( ) const { return mDragging != NULL; }
		void fSetDragging( tObjectLayerGui* obj, b32 drop );
		void fSetDragOver( tObjectLayerGui* obj );

		void fRefreshLists( );

		// mostly private.
		void fAddToLayer( tEditableObject* eo, const std::string& name );
		void fGetObjects( tGrowableArray<tEditableObject*>& objOut, const std::string& name ) const;
		void fRemoveFromLayer( tEditableObject* eo, const std::string& name );

	private:
		void fAddDefaultLayer( );
		void fOnAddLayerPressed( wxCommandEvent& );
		void fAddLayer( );
		b32  fIsNameInUse( const char* name, u32 ignoreIndex = ~0 ) const;
	};
}

#endif//__tLayerPanel__
