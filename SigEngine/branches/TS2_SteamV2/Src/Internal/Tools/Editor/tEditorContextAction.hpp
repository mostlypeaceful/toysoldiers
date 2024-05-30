#ifndef __tEditorContextAction__
#define __tEditorContextAction__

class wxWindow;
class wxMenu;
class wxMouseEvent;

namespace Sig
{
	class tEditorContextAction;

	define_smart_ptr( tools_export, tRefCounterPtr, tEditorContextAction );

	typedef tGrowableArray< tEditorContextActionPtr > tEditorContextActionList;

	class tools_export tEditorContextAction : public tRefCounter
	{
		static u32 gUniqueActionId;
		static tEditorContextActionList gLastActionList;
	protected:
		wxPoint mRightClickPos;
	public:
		static void fDisplayContextMenuOnRightClick( wxWindow* window, wxMouseEvent& event, const tEditorContextActionList& contextActions );

		// N.B.!
		// This version does not automatically skip the event. If you need it skipped up, you will have to skip the event yourself.
		static void fDisplayContextMenuOnRightClick( wxWindow* window, wxMouseState& mouse, const tEditorContextActionList& contextActions );
		static void fHandleContextActionFromRightClick( wxWindow* window, wxCommandEvent& event, const tEditorContextActionList& contextActions );
		static tEditorContextActionList& fLastActionList( ) { return gLastActionList; }
	public:
		const wxPoint& fRightClickPos( ) const { return mRightClickPos; }
		virtual b32	fAddToContextMenu( wxMenu& menu ) { return false; } // return true if anything was added
		virtual b32 fHandleAction( u32 actionId ) { return false; } // return true if you handle the action

	protected:
		static u32 fNextUniqueActionId( ) { return ++gUniqueActionId; }	
	};

}

#endif//__tEditorContextAction__
