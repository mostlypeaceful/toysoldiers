#ifndef __tMouseFollowCursor__
#define __tMouseFollowCursor__
#include "tSelectObjectsCursor.hpp"


namespace Sig
{
	class toolsgui_export tMouseFollowCursor : public tSelectObjectsCursor
	{

	public:
		tMouseFollowCursor( tEditorCursorControllerButton* button,
			const char* statusText,
			const tManipulationGizmoPtr& gizmo = tManipulationGizmoPtr( ) );
		virtual void fOnTick( );
		
	};
}

#endif //	__tMouseFollowCursor__
