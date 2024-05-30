#ifndef __tEditorAppWindow__
#define __tEditorAppWindow__
#include "tStrongPtr.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "tWxSavedLayout.hpp"
#include "Editor/tEditorAction.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "tEditableObjectProperties.hpp"
#include "tEditablePropertyTypes.hpp"
#include "iSigEdPlugin.hpp"

namespace Sig
{
	class tEditorAppWindow;
	class tSaveLoadProgressDialog;
	class tManipulateObjectPanel;
	class tReferenceObjectPanel;
	class tLayerPanel;
	class tGroundCoverPanel;
	class tRemapReferencesDialog;
	class tSigEdExplicitDependenciesDialog;
	class tHeightFieldMaterialPaintPanel;
	class tEditorDialog;
	class tWxRenderPanelContainer;
	class tWxToolsPanelContainer;
	class tObjectBrowserDialog;
	class tSearchableOpenFilesDialog;
	class tMemoryViewerDialog;
	class tSigEdLightEdDialog;
	class tSigEdLenseFlareEdDialog;
	class tTilePaintPanel;


	///
	/// \brief Base class providing fairly generic editor/tools app functionality, storing
	/// render panels and tools panels containers. Automates quite a few common tasks related
	/// to general editor applications.
	class tEditorAppWindow : public tToolsGuiMainWindow
	{
	protected:

		// document stuff
		b32							mReadyToOpenDoc;
		tPlatformId					mPreviewPlatform;
		wxString					mOpenDocName;
		wxString					mOpenDocDirectory;

		// actions
		tEditorActionStack::tNotifyEvent::tObserver mOnDirty, mOnAddAction;

		// my hot keys
		tGrowableArray< tEditorHotKeyPtr > mHotKeyStorage;

		// selection stuff
		tEditorSelectionList::tOnSelectionChanged::tObserver mOnSelChanged;
		b32							mRefreshObjectProperties;

		// wx/window stuff
		tStrongPtr<tSaveLoadProgressDialog> mProgressBar;
		wxBoxSizer*					mMainSizer;
		tManipulateObjectPanel*		mManipTools;
		tReferenceObjectPanel*		mRefObjPanel;
		tLayerPanel*				mLayerPanel;
		tLayerPanel*				mVisibilityPanel;
		tHeightFieldMaterialPaintPanel*	mHeightFieldMaterials;
		tGroundCoverPanel *			mGCPanel;
		tSigEdLightEdDialog*		mLightEdDialog;
		tSigEdLenseFlareEdDialog*	mLenseFlareEdDialog;
		tTilePaintPanel*			mTilePaintPanel;

		tWxToolsPanelContainer*		mToolsPanelContainer;
		tGrowableArray<tEditorDialog*> mEditorDialogs;
		tObjectBrowserDialog*		mObjectBrowserDialog;
		tSearchableOpenFilesDialog*	mOpenDialog;
		tMemoryViewerDialog*		mMemoryViewerDialog;
		tGrowableArray< iSigEdPlugin* > mPlugins;

		// Lots of these dialogs contain the deserialized state of the file that is open
		tEditableObjectProperties*	mObjectProperties;
		tEditableObjectProperties*	mGlobalProperties;
		tRemapReferencesDialog*		mRemapReferencesDialog;
		tSigEdExplicitDependenciesDialog* mExplicitDependenciesDialog;
		tEditorPluginDataContainer mPluginData;

	public:

		enum tPreviewBuildConfig
		{
			cPreviewBuildConfigInternal,
			cPreviewBuildConfigPlaytest,
			cPreviewBuildConfigRelease,

			cPreviewBuildConfigCount
		};

		tEditorAppWindow( tToolsGuiApp& guiApp );
		~tEditorAppWindow( );
		virtual void fSetupRendering( );
		virtual void fOnTick( );
		virtual void fClearScene( b32 closing );
		virtual b32 fSerializeDoc( const tFilePathPtr& path );
		virtual std::string fEditableFileExt( ) const { return ".sigml"; }

		tPlatformId fPreviewPlatform( ) const { return mPreviewPlatform; }

		void fNewDoc( );
		void fLoadPlugins( );
		b32  fBuild( );
		void fPreview( tPreviewBuildConfig config = cPreviewBuildConfigInternal );
		void fOpenDoc( );
		void fBrowseDoc( );
		void fOpenDoc( const tFilePathPtr& file );
		void fImportDoc( );
		void fImportDoc( const tFilePathPtr& file );
		void fUndo( );
		void fRedo( );
		void fDuplicateSelected( );
		void fGroupSelected( );
		void fBreakSelected( );
		void fSelectAll( );
		void fCut( );
		void fCopy( );
		void fPaste( );
		void fToggleObjectProperties( );
		void fToggleGlobalProperties( );
		void fToggleObjectBrowser( );
		void fTogglePlugin( wxCommandEvent& );

		void fToggleViewMode( );

		void fHideSelected( );
		void fHideUnselected( );
		void fUnhideAll( );

		void fFreezeSelected( );
		void fFreezeUnselected( );
		void fUnfreezeAll( );

		void fSetSelectionCursor( );

		void fSnapToGround( b32 onlySnapIfSpecified );
		void fRefreshObjectProperties( );
		void fRefreshLayers( );

		const tResourceDepotPtr&			fGetResourceDepot( ) { return fGuiApp( ).fResourceDepot( ); }
		const tSceneGraphPtr&				fGetSceneGraph( ) const { return fGuiApp( ).fSceneGraph( ); }

		tEditableObjectProperties*			fObjectProperties( ) const { return mObjectProperties; }
		tEditableObjectProperties*			fGlobalProperties( ) const { return mGlobalProperties; }
		tManipulateObjectPanel*				fManipulatePanel( ) const { return mManipTools; }
		tReferenceObjectPanel*				fReferenceObjectPanel( ) const { return mRefObjPanel; }

		wxString							fOpenDocName( ) { return mOpenDocName; }
		wxString							fOpenDocDirectory( ) { return mOpenDocDirectory; }

		void fRefreshHeightFieldMaterialTileFactors( );
		void fAcquireHeightFieldMaterialTileFactors( tDynamicArray<f32>& tilingFactors );

		// cleans all textures for all selected height field objects. the first material
		// will be the visible one, other two set to zero weight.
		void fCleanHeightFieldMaterials( u32 firstMat, u32 secondMat, u32 thirdMat );

		void fAddEditorDialog( tEditorDialog * dlg );
		void fRemoveEditorDialog( tEditorDialog * dlg );

		// Callbacks for property windows OnFocus
		void fOnGlobalPropertiesFocus( );
		void fOnObjectPropertiesFocus( );

		tGrowableArray< iSigEdPlugin* >& fPlugins( ) { return mPlugins; }
		tGrowableArray< tEditorPluginDataPtr >& fPluginData( ) { return mPluginData; }

		template< typename t >
		t* fFirstPluginData( )
		{
			for( u32 i = 0; i < mPluginData.fCount( ); ++i )
			{
				t *test = dynamic_cast< t* >( mPluginData[ i ].fGetRawPtr( ) );
				if( test )
					return test;
			}
			return NULL;
		}

	protected:

		void fOpenRecent( u32 ithRecentFile );
		void fOnClose( );

	private:
		void fOnActionUndoOrRedo( tEditorActionStack& stack );
		void fOnDirty( tEditorActionStack& stack );
		void fOnSelChanged( tEditorSelectionList& selectedObjects );
		void fOnObjectSerialized( u32 nObjectsSerialized, u32 nObjectsLeft );
		b32 fSerializeSigmlFile( const tFilePathPtr& filePath, b32 selected );
		b32 fDeserializeSigmlFile( const tFilePathPtr& filePath );
		b32 fImportAndMerge( const tFilePathPtr& filePath );
	};
}

#endif//__tEditorAppWindow__

