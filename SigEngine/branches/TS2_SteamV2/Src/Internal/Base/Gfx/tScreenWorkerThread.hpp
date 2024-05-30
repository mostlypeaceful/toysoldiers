#ifndef __tScreenWorkerThread__
#define __tScreenWorkerThread__
#include "Threads/tWorkerThread.hpp"
#include "tScreen.hpp"

namespace Sig { namespace Gfx
{

	class tScreenWorkerThread : public Threads::tWorkerThread
	{
	public:
		enum tState
		{
			cStateIdle,
			cStateBuildDisplayLists,
			cStateProcessCanvas,
			cStateCount
		};
	public:
		tScreenWorkerThread( );
		void fBeginWork( 
			tSceneGraph* sceneGraph,
			tScreen::tViewportArray* viewports,
			tDynamicArray<tLightEntityList>* lights,
			tDynamicArray< tLightComboList >* lightCombos,
			tDynamicArray< tWorldSpaceDisplayList >* depthDisplayLists,
			b32 buildDepthList );
		void fBeginWork( tScreen* screen, Gui::tCanvas* rootCanvas );
		void fCompleteWork( );
		virtual void fOnThreadTick( );
	private:
		tState mState;
		Threads::tSemaphore mWorkComplete;

		tSceneGraph* mSceneGraph;
		tScreen::tViewportArray* mViewports;
		tDynamicArray<tLightEntityList>* mLights;
		tDynamicArray<tLightComboList>* mLightCombos;
		tDynamicArray<tWorldSpaceDisplayList>* mDepthDisplayLists;
		b32 mBuildDepthList;

		tScreen* mScreen;
		Gui::tCanvas* mRootCanvas;
	};

}}

#endif//__tScreenWorkerThread__
