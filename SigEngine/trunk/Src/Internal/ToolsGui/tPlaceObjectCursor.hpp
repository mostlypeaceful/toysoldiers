#ifndef __tPlaceObjectCursor__
#define __tPlaceObjectCursor__
#include "tEditorCursorControllerButton.hpp"
#include "Editor/tEditorSelectionList.hpp"

namespace Sig
{
	class toolsgui_export tPlaceObjectsAction : public tEditorButtonManagedCursorAction
	{
		tGrowableArray<tEntityPtr> mEntities;
		tEditorSelectionList mSavedSelection;
		b32 mDeleteObjects;
	public:
		tPlaceObjectsAction( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entity, b32 deleteObjs = false );
		tPlaceObjectsAction( tToolsGuiMainWindow& mainWindow, const tGrowableArray<tEntityPtr>& entities, b32 deleteObjs = false );
		virtual void fUndo( );
		virtual void fRedo( );
	private:
		void fAddToWorld( );
		void fRemoveFromWorld( );
	};


	class toolsgui_export tPlaceObjectCursorBase
	{
	protected:
		tToolsGuiMainWindow& mMainWindow;
		tEntityPtr mEntityMaster;
		Math::tVec3f mOffset;
	public:
		tPlaceObjectCursorBase( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entityMaster, const Math::tVec3f& offset = Math::tVec3f::cZeroVector );
		virtual ~tPlaceObjectCursorBase( );
		void fOnTick( );
		virtual void fOnEntityPlaced( const tEntityPtr& placedEntity ) { }
	protected:
		virtual b32 fComputePickRay( Math::tRayf& pickRay ) = 0;
	};

	class toolsgui_export tPlaceObjectCursor : public tEditorButtonManagedCursorController, public tPlaceObjectCursorBase
	{
	public:
		tPlaceObjectCursor( tEditorCursorControllerButton* button, const tEntityPtr& entityMaster, const Math::tVec3f& offset = Math::tVec3f::cZeroVector );
		tPlaceObjectCursor( tToolsGuiMainWindow& mainWindow, const tEntityPtr& entityMaster, const Math::tVec3f& offset = Math::tVec3f::cZeroVector );
		virtual void fOnTick( );
		virtual b32 fComputePickRay( Math::tRayf& pickRay );		
		virtual void fOnNextCursor( tEditorCursorController* nextController );
	};

}

#endif//__tPlaceObjectCursor__
