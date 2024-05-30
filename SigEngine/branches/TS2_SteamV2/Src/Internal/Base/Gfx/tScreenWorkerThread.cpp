#include "BasePch.hpp"
#include "tScreenWorkerThread.hpp"
#include "tLightCombo.hpp"
#include "tGeometryBufferVRamSlice.hpp"
#include "Gui/tCanvas.hpp"
#include "tProfiler.hpp"

using namespace Sig::Threads;

namespace Sig { namespace Gfx
{
	tScreenWorkerThread::tScreenWorkerThread( )
		: Threads::tWorkerThread( "ScreenWorkerThread", 1 )
		, mState( cStateIdle )
		, mWorkComplete( true, true )
		, mSceneGraph( 0 )
		, mViewports( 0 )
		, mLights( 0 )
		, mLightCombos( 0 )
		, mDepthDisplayLists( 0 )
		, mBuildDepthList( false )
		, mScreen( 0 )
		, mRootCanvas( 0 )
	{
	}
	void tScreenWorkerThread::fBeginWork( 
		tSceneGraph* sceneGraph,
		tScreen::tViewportArray* viewports,
		tDynamicArray<tLightEntityList>* lights,
		tDynamicArray< tLightComboList >* lightCombos,
		tDynamicArray< tWorldSpaceDisplayList >* depthDisplayLists,
		b32 buildDepthList )
	{
		sigassert( mState == cStateIdle );

		mState = cStateBuildDisplayLists;
		mSceneGraph = sceneGraph;
		mViewports = viewports;
		mLights = lights;
		mLightCombos = lightCombos;
		mDepthDisplayLists = depthDisplayLists;
		mBuildDepthList = buildDepthList;

		mWorkComplete.fReset( );
		mWaitForInput.fSignal( );
	}
	void tScreenWorkerThread::fBeginWork( tScreen* screen, Gui::tCanvas* rootCanvas )
	{
		sigassert( mState == cStateIdle );

		mState = cStateProcessCanvas;
		mScreen = screen;
		mRootCanvas = rootCanvas;

		mWorkComplete.fReset( );
		mWaitForInput.fSignal( );
	}
	void tScreenWorkerThread::fCompleteWork( )
	{
		// FIRST wait until signaled so thread work finishes
		mWorkComplete.fWaitUntilSignaled( );

		// THEN reset state (otherwise race conditions)

		mState = cStateIdle;

		mSceneGraph = 0;
		mViewports = 0;
		mLights = 0;
		mLightCombos = 0;
		mDepthDisplayLists = 0;

		mScreen = 0;
		mRootCanvas = 0;
	}
	void tScreenWorkerThread::fOnThreadTick( )
	{
		switch( mState )
		{
		case cStateBuildDisplayLists:
			{
				profile( cProfilePerfBuildDisplayLists );
				for( u32 i = 0; i < mViewports->fCount( ); ++i )
				{
					tViewport& vp = *(*mViewports)[ i ];
					tLightEntityList& lights = (*mLights)[ i ];
					tLightComboList& combos = (*mLightCombos)[ i ];
					tWorldSpaceDisplayList& depthDisplayList = (*mDepthDisplayLists)[ i ];

					combos.fBuildLightCombosMT( *mSceneGraph, vp, lights, depthDisplayList, mBuildDepthList );
				}
			}
			break;
		case cStateProcessCanvas:
			{
				//if( !tGeometryBufferVRamAllocator::fBufferLockingEnabled( ) )
				//{
				//	profile( cProfilePerfOnTickCanvas );
				//	mRootCanvas->fOnTickCanvas( mScreen->fSceneGraph( )->fFrameDeltaTime( ) );
				//}
				profile( cProfilePerfOnRenderCanvas );
				mRootCanvas->fOnRenderCanvas( *mScreen );
			}
			break;
		}

		mWorkComplete.fSignal( );
	}

}}

