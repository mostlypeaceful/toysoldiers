#ifndef __tEditorCursorController__
#define __tEditorCursorController__
#include "tEntity.hpp"
#include "tEditorHotKey.hpp"
#include "Gui/tText.hpp"

namespace Sig
{
	class tToolsGuiApp;
	class tToolsGuiMainWindow;

	///
	/// \brief Base type for application cursors. Application cursors are
	/// (generally) more involved than just a modification of the windows cursor; 
	/// for example, a "cursor" could be a terrain painting tool, a model placement
	/// brush, etc.
	class toolsgui_export tEditorCursorController : public tRefCounter
	{
		tToolsGuiApp& mGuiApp;
	protected:
		Gui::tText							mToolTip;
		tEntityPtr							mCurrentHoverObject;
		Math::tVec3f						mLastHoverIntersection;
		tGrowableArray< tEditorHotKeyPtr >	mHotKeys;

	public:
		tEditorCursorController( tToolsGuiApp& guiApp );
		virtual ~tEditorCursorController( );
		virtual void fOnTick( ) { }
		virtual void fOnNextCursor( tEditorCursorController* nextController ) { } // just in case you need to do something

		///
		/// \brief Compute a picking ray from the eye to the unprojected cursor position,
		/// using the currently active render panel
		/// \return false if no render panels are active (i.e., the cursor is not over any of them).
		b32 fComputePickRay( Math::tRayf& rayOut );

		const tEntityPtr&	fLastHoverObject( ) const { return mCurrentHoverObject; }
		const Math::tVec3f& fLastHoverIntersection( ) const { return mLastHoverIntersection; }

		void fAddHotKey( const tEditorHotKeyPtr& hotKey ) { mHotKeys.fPushBack( hotKey ); }

		tToolsGuiApp& fGuiApp( ) { return mGuiApp; }
		tToolsGuiMainWindow& fMainWindow( );

	protected:

		///
		/// \brief Derived types can use this method to reject the (potential) hover object.
		virtual tEntityPtr fFilterHoverObject( const tEntityPtr& newHoverObject ) { return newHoverObject; }

		/// 
		/// \brief Derived types use this to do any unique picking.
		virtual tEntityPtr fPick( const Math::tRayf& ray, f32* bestTout = 0, tEntity* const* ignoreList = 0, u32 numToIgnore = 0 );

		///
		/// \brief Should return true if the derived object wants to show a tool tip.
		virtual b32 fDoToolTipOverHoverObject( );

		///
		/// \brief Derived cursors must call this method during their fOnTick method if they
		/// want "hover" logic; i.e., the hover object is the closest object intersected
		/// with the picking ray, or null if no intersection.
		void fHandleHover( );
	};

	typedef tRefCounterPtr<tEditorCursorController> tEditorCursorControllerPtr;

}

#endif//__tEditorCursorController__
