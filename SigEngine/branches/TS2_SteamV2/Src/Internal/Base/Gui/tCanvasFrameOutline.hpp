#ifndef __tCanvasFrameOutline__
#define __tCanvasFrameOutline__
#include "tLineList.hpp"

namespace Sig { namespace Gui
{

	class tCanvasFrameOutline : public tLineList
	{
	public:
		static b32 fDebugDrawEnabled( );
	public:
		tCanvasFrameOutline( );
		virtual void fOnMoved( );
		virtual void fOnTintChanged( );
		virtual void fOnParentMoved( );
		virtual void fOnParentTintChanged( );
	private:
		void fUpdateVerts( );
	};

	typedef tRefCounterPtr<tCanvasFrameOutline> tCanvasFrameOutlinePtr;

}}//Sig::Gui

#endif//__tCanvasFrameOutline__
