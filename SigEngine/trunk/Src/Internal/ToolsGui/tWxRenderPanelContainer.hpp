#ifndef __tWxRenderPanelContainer__
#define __tWxRenderPanelContainer__

// wx stuff
#include "tWxAutoDelete.hpp"
#include "tWxRenderPanel.hpp"

// gfx stuff
#include "Gfx/tGeometryBufferVRamSlice.hpp"
#include "Gfx/tIndexBufferVRamSlice.hpp"

namespace Sig
{
	class tEditorAppWindow;

	///
	/// \brief Window container that manages individual tWxRenderPanels.
	class toolsgui_export tWxRenderPanelContainer : public wxPanel
	{
	protected:

		enum tPanelType
		{
			cPanelTopLeft,
			cPanelTopRight,
			cMainSinglePanel = cPanelTopRight,
			cPanelBotLeft,
			cPanelBotRight,
			cPanelCount
		};

		// registry key name
		std::string mRegKeyName;

		// wxWidgets stuff
		tToolsGuiMainWindow* mMainWindow;
		wxToolBar* mToolBar; // may be null
		wxFlexGridSizer* mMainGrid;

		// render panels
		tFixedArray<tWxRenderPanel*,cPanelCount>	mRenderPanels;
		tFixedArray<wxPanel*,cPanelCount>			mSelectionPanels;

		tResourcePtr								mDefaultFont;
		Gfx::tDynamicGeometry						mTranslateGizmoTemplate;

		b32											mSinglePaneView;
		u32											mFocusPanel;

	public:

		tWxRenderPanelContainer( tToolsGuiMainWindow* parent, const std::string& regKeyName, b32 createToolBar );
		~tWxRenderPanelContainer( );

		void fSetupRendering( tToolsGuiApp& guiApp );
		void fOnTick( );
		void fRender( const Gfx::tDisplayStats* selectedDisplayStats );

		///
		/// \brief Returns the render panel containing the cursor, or null if the cursor is not inside any
		tWxRenderPanel*	  fGetActiveRenderPanel( );

		///
		/// \brief Returns the screen from the active render panel, if any
		Gfx::tScreen*	  fGetActiveScreen( );

		///
		/// \brief Returns the 'selected' or 'focused' render panel, basically the last one clicked on
		tWxRenderPanel*	  fGetFocusRenderPanel( );

		///
		/// \brief How many render panels.
		u32 fRenderPanelCount( ) const { return mRenderPanels.fCount( ); }

		/// 
		/// \brief Tests if the panel is the main panel.
		b32 fPanelIsMain( tWxRenderPanel* panel );

		/// 
		/// \brief Switches between one and four viewports.
		void fShowFour( );
		void fShowOne( );
		void fToggleViewMode( );

		/// 
		/// \brief Frames all viewports.
		void fFrameAllViewports( const Math::tAabbf& frameBox );

		///
		/// \brief Access all render panels; note that some may be null.
		tWxRenderPanel**			fRenderPanelsBegin( ) { return mRenderPanels.fBegin( ); }
		tWxRenderPanel**			fRenderPanelsEnd( ) { return mRenderPanels.fEnd( ); }

		tToolsGuiMainWindow*		fMainWindow( ) { return mMainWindow; }
		wxPanel*					fGetContainerPanel( ) { return this; }
		wxToolBar*					fGetToolBar( ) { return mToolBar; }

		
		const Gfx::tGeometryBufferVRamAllocatorPtr&		fGetSolidColorGeometryAllocator( ) const { return Gfx::tDefaultAllocators::fInstance( ).mSolidColorGeomAllocator; }
		const Gfx::tGeometryBufferVRamAllocatorPtr&		fGetParticleGeometryAllocator( ) const { return Gfx::tDefaultAllocators::fInstance( ).mParticleGeomAllocator; }
		
		const Gfx::tIndexBufferVRamAllocatorPtr&		fGetSolidColorIndexAllocator( ) const { return Gfx::tDefaultAllocators::fInstance( ).mIndexAllocator; }
		const Gfx::tIndexBufferVRamAllocatorPtr&		fGetTextIndexAllocator( ) const { return Gfx::tDefaultAllocators::fInstance( ).mIndexAllocator; }
		const Gfx::tIndexBufferVRamAllocatorPtr&		fGetQuadIndexAllocator( ) const { return Gfx::tDefaultAllocators::fInstance( ).mIndexAllocator; }

		const Gfx::tGeometryBufferVRamAllocatorPtr&		fGetTextGeometryAllocator( ) const { return Gfx::tDefaultAllocators::fInstance( ).mTextGeometryAllocator; }
		const Gfx::tMaterialPtr&						fGetSolidColorMaterial( ) const { return Gfx::tDefaultAllocators::fInstance( ).mSolidColorMaterial; }
		const tResourcePtr&								fGetSolidColorMaterialFile( ) const { return Gfx::tDefaultAllocators::fInstance( ).mSolidColorMaterialFile; }
		const tResourcePtr&								fGetFullBrightMaterialFile( ) const { return Gfx::tDefaultAllocators::fInstance( ).mFullBrightMaterialFile; }

	private:

		void fSetPanelFocus( u32 i );

		void fGenerateSelectedBoxTemplate( Gfx::tSolidColorLines& lines );
	};

}


#endif//__tWxRenderPanelContainer__

