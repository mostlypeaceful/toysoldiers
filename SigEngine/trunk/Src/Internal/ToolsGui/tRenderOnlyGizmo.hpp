//------------------------------------------------------------------------------
// \file tRenderOnlyGizmo.hpp - 07 Oct 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tRenderOnlyGizmo__
#define __tRenderOnlyGizmo__
#include "tGizmoGeometry.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "tToolsGuiMainWindow.hpp"

namespace Sig
{
	class tWxRenderPanel;

	///
	/// \class tRenderOnlyRenderable
	/// \brief A crude mimicry of tGizmoRenderable. In a perfect world, this would
	/// be a base type or something that the other gizmos would branch from.
	class toolsgui_export tRenderOnlyRenderable : public tRefCounter
	{
		tGizmoGeometryPtr				mGeometry;
		Gfx::tRenderableEntityPtr		mInstance;
	public:

		tRenderOnlyRenderable( const tGizmoGeometryPtr& geom );

		const Math::tMat3f& fLocalToWorld( ) const { return mInstance->fObjectToWorld( ); }
		void fSetLocalToWorld( const Math::tMat3f& localToWorld );
		Gfx::tDrawCall fCreateDrawCall( const Gfx::tCamera& camera ) const;
		const Gfx::tRenderableEntityPtr& fGizmoRenderableEntity( ) const { return mInstance; }
	};
	typedef tRefCounterPtr<tRenderOnlyRenderable> tRenderOnlyRenderablePtr;

	///
	/// \class tRenderOnlyGizmo
	/// \brief 
	class toolsgui_export tRenderOnlyGizmo : public tRefCounter
	{
	protected:
		typedef tGrowableArray< tRenderOnlyRenderablePtr > tRenderOnlyRenderableList;
		tRenderOnlyRenderableList mGizmoPerWindow;
		tGizmoGeometryPtr mGizmoGeometry;

		b32 mActive;

		f32 mScreenScale;
		f32 mWorldScale;

		b32 mIsScreenSpace;
		Math::tVec2f mGizmoRelativeScreenPos;
		f32 mPushForward;

	public:
		tRenderOnlyGizmo( const tGizmoGeometryPtr& geom );
		virtual ~tRenderOnlyGizmo( ) { }

		const tGizmoGeometryPtr&	fGetGizmoGeometry( ) const { return mGizmoGeometry; }

		void fSetActive( b32 turnOn = true ) { mActive = turnOn; }

		virtual void fTick( tToolsGuiMainWindow& editorWindow );
		virtual Math::tMat3f fComputeLocalToWorld( tToolsGuiMainWindow& editorWindow, const tWxRenderPanel* panel );
	};
	typedef tRefCounterPtr<tRenderOnlyGizmo> tRenderOnlyGizmoPtr;
}

#endif // __tRenderOnlyGizmo__