#ifndef __tSigAINodeCanvas__
#define __tSigAINodeCanvas__
#include "tDAGNodeCanvas.hpp"
#include "DerivedAINodes.hpp"
#include "Goaml.hpp"

namespace Sig
{
	class tEditEventHandlerContextAction;
	class tCreateNodeContextAction;
	class tSigAIMainWindow;

	class tSigAINodeCanvas : public tDAGNodeCanvas
	{
	private:
		tCreateNodeContextAction* mNodeCreator;
		Goaml::tFile mClipboard;

		tEditorContextActionList mActionList;
		tRefCounterPtr< tEditEventHandlerContextAction > mEventHandlerAction;
		tSigAIMainWindow* mMainWindow;
	public:
		explicit tSigAINodeCanvas( wxWindow* parent, tSigAIMainWindow* mainWindow );
		virtual void fAddNode( const tDAGNodePtr& shadeNode );
		virtual void fDeleteNode( const tDAGNodePtr& shadeNode );
		virtual void fReset( const tDAGNodeList& nodes, const tDAGNodeConnectionList& connections, b32 addTo = false );
		virtual void fClearCanvas( );
		void fEditHandler( tGoalEventHandlerPtr& handler, u32 row = 0, u32 col = 0 );

		void fAddDefaultNode( const wxPoint& p = wxPoint( -1, -1 ) );
		void fCopy( );
		void fPaste( );
		void fToGoamlFile( Goaml::tFile& file, b32 selectedOnly = false );
		void fFromGoamlFile( const Goaml::tFile& file, b32 addToScene = false );
		virtual void fOnMouseRightButtonUp( wxMouseEvent& event );
		virtual void fConnectionCreated( const tDAGNodeConnectionPtr& connection );
	};
	
	class tAddRemoveEventHandlerAction : public tEditorAction
	{
	protected:
		b32						mAdd;
		tGoalAINodePtr			mNode;
		tGoalEventHandlerPtr	mHandlerPtr;
		tDAGNodeCanvas&			mCanvas;
		u32						mIndex;
	public:
		tAddRemoveEventHandlerAction( tDAGNodeCanvas& canvas, tGoalAINodePtr& node, tGoalEventHandlerPtr& handler, b32 add )
			: mCanvas( canvas ), mNode( node ), mHandlerPtr( handler ), mAdd( add ), mIndex( ~0 )
		{
			if( !mAdd )
				mIndex = u32( mNode->fIndexOfEventHandler( mHandlerPtr ) );

			fRedo( );
		}

		virtual void fUndo( ) 
		{ 
			if( mAdd ) mNode->fRemoveEventHandler( mHandlerPtr );
			else mNode->fAddEventHandler( mHandlerPtr, mIndex );

			fCommon( ); 
		}
		virtual void fRedo( ) 
		{ 
			if( mAdd ) mNode->fAddEventHandler( mHandlerPtr, mIndex );
			else mNode->fRemoveEventHandler( mHandlerPtr );

			fCommon( );
		}
		void fCommon( )
		{
			mCanvas.Refresh( );
		}
	};


	class tEditPropertyAction : public tEditorAction
	{
		tGoalAINodePtr			mGoal;		
		std::string				mProp;	
		std::string				mPrev;		
		std::string				mNew;
		tDAGNodeCanvas&			mCanvas;
	public:
		tEditPropertyAction( tDAGNodeCanvas& canvas, tGoalAINodePtr& goal, const std::string& prop, const std::string& newData )
			: mCanvas( canvas ), mGoal( goal ), mProp( prop ), mNew( newData )
		{
			tEditablePropertyPtr* p = mGoal->fAIProps( ).fFind( mProp );
			sigassert( p );
			(*p)->fGetData( mPrev );

			fRedo( );
		}

		virtual void fUndo( ) 
		{ 
			(*mGoal->fAIProps( ).fFind( mProp ))->fSetData( mPrev );
			mGoal->fApplyPropertyValues( );
			mCanvas.Refresh( );
		}
		virtual void fRedo( ) 
		{ 
			(*mGoal->fAIProps( ).fFind( mProp ))->fSetData( mNew );
			mGoal->fApplyPropertyValues( );
			mCanvas.Refresh( );
		}
	};


	class tScriptChangeAction : public tEditorAction
	{
		tGoalEventHandlerPtr	mHandlerPtr;		
		std::string				mPrev;		
		std::string				mNew;
		tDAGNodeCanvas&			mCanvas;
	public:
		tScriptChangeAction( tDAGNodeCanvas& canvas, tGoalEventHandlerPtr& handler, const std::string& newData )
			: mCanvas( canvas ),mHandlerPtr( handler ), mPrev( handler->fScript( ) ), mNew( newData )
		{
			fRedo( );
		}

		virtual void fUndo( ) 
		{ 
			mHandlerPtr->fSetScript( mPrev );
			mCanvas.Refresh( );
		}
		virtual void fRedo( ) 
		{ 
			mHandlerPtr->fSetScript( mNew );
			mCanvas.Refresh( );
		}
	};
}

#endif//__tSigAINodeCanvas__
