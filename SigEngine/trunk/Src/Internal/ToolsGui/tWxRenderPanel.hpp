#ifndef __tWxRenderPanel__
#define __tWxRenderPanel__
#include "tResourceDepot.hpp"

// input
#include "Input/tMouse.hpp"

// gfx stuff
#include "Gfx/tDevice.hpp"
#include "Gfx/tScreen.hpp"
#include "Gfx/tSolidColorGrid.hpp"
#include "Gui/tFpsText.hpp"

namespace Sig
{
	class tToolsGuiApp;
	class tToolsGuiMainWindow;
	class tWxRenderPanelGridSettings;
	class tWxRenderPanelContainer;
	class tToolsMouseAndKbCamera;
	class tEditableCameraEntity;

	///
	/// \brief Encapsulates a single panel that can be rendered to
	/// using the graphics engine (dx9), as well as work within a wxWidgets window chain.
	/// Specifically, this class manages a Gfx::tScreen object (a swap chain), and expects
	/// to be a child of a tWxRenderPanelContainer.
	class toolsgui_export tWxRenderPanel : public wxPanel
	{
	public:

		typedef tRefCounterPtr< tToolsMouseAndKbCamera > tEditorCameraPtr;

	protected:

		// for saving registry key values
		std::string						mRegKeyName;

		// wx stuff
		tWxRenderPanelContainer*		mContainer;
		tWxRenderPanelGridSettings*		mGridSettings;

		// owned by me
		Input::tMouse					mMouse;
		Gfx::tScreenPtr					mScreen;
		tEditorCameraPtr				mCamera;
		Gfx::tSolidColorGridPtr			mGrid;
		b32								mShowGrid;
		b32								mSnapToGrid;
		Gui::tFpsTextPtr				mFpsText;
		Gui::tTextPtr					mStatsText;
		u32								mTickFrame;
		u32								mLastSizeFrame;

		// Data for previewing an editable camera object
		tRefCounterPtr< tEditableCameraEntity > mPreviewingCamera;
		Gfx::tCamera					mPreviousCamera;

	public:

		tWxRenderPanel( tWxRenderPanelContainer* container, wxWindow* parentWindow, const std::string& regKeyName );
		~tWxRenderPanel( );

		HWND fGetRenderHwnd( );

		tToolsGuiMainWindow* fMainWindow( );
		b32 fIsVisible( ) const { return GetSize( ).x > 0 && GetSize( ).y > 0; }

		void fResetProjectionMatrices( u32 bbWidth, u32 bbHeight );
		void fSetupRendering( tToolsGuiApp& guiApp );
		void fOnTick( );
		void fRender( const Gfx::tDisplayStats* selectedDisplayStats );
		void fFrame( const Math::tAabbf& frameBox );

		void fSetProjectionType( const Math::tVec3f& viewAxis, const Math::tVec3f& up, u32 projType );
		void fSetOrthoAndLookPos( const Math::tVec3f& lookPos, const Math::tVec3f& viewAxis, const Math::tVec3f& up );
		void fDisableRotation( b32 disable = true );
		void fDisableOrthoToggle( b32 disable = true );

		// Pass null to disable.
		void fSetPreviewCamera( tEditableCameraEntity* cam );
		b32 fIsPreviewingCamera( ) const { return !mPreviewingCamera.fNull( ); }

		tWxRenderPanelContainer*		fGetContainer( ) const { return mContainer; }
		tWxRenderPanelGridSettings*		fGetGridSettings( ) const { return mGridSettings; }
		const tEditorCameraPtr&			fGetCamera( ) const { return mCamera; }
		const Input::tMouse&			fGetMouse( ) const { return mMouse; }
		const Gfx::tScreenPtr&			fGetScreen( ) const { return mScreen; }
		Math::tVec3f					fGetGridCenter( ) const { return mGrid->fObjectToWorld( ).fGetTranslation( ); }

		b32								fSnapToGrid( ) const { return mSnapToGrid; }
		void							fSnapVertex( Math::tVec3f& vert ) const;

	private:
		b32  fCheckForDialogInput( );
		void fSetupGrid( );
		void fOnAction( wxCommandEvent& event );
		void fOnSize( wxSizeEvent& event );
		void fOnRightClick( wxMouseEvent& event );
	};

}

#endif//__tWxRenderPanel__
