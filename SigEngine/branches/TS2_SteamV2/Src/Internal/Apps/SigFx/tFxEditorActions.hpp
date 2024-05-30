#ifndef __tFxEditorActions__
#define __tFxEditorActions__
#include "tSigFxKeyframe.hpp"
#include "tSigFxKeylineTrack.hpp"
#include "FxEditor/tSigFxAttractor.hpp"
#include "FxEditor/tSigFxMeshSystem.hpp"

namespace Sig
{

	class tGraphlineAction : public tEditorAction
	{
		tGraphPtr* mGraph;
		tGraphPtr mNewGraph;
		tGraphPtr mOldGraph;

	public:
		tGraphlineAction( tGraphPtr* graph, const tGraphPtr& old )
			: mGraph( graph ), mOldGraph( old )
		{
			u32 id = graph->fGetRawPtr( )->fGetID( );

			if( id == Rtti::fGetClassId< f32 >( ) )
				mNewGraph.fReset( new tFxGraphF32( graph->fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec2f >( ) )
				mNewGraph.fReset( new tFxGraphV2f( graph->fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec3f >( ) )
				mNewGraph.fReset( new tFxGraphV3f( graph->fGetRawPtr( ) ) );
			else if( id == Rtti::fGetClassId< Math::tVec4f >( ) )
				mNewGraph.fReset( new tFxGraphV4f( graph->fGetRawPtr( ) ) );
		}

		virtual void fUndo( )
		{
			mGraph->fReset( mOldGraph.fGetRawPtr( ) );
			( *mGraph )->fUpdate( );
		}
		virtual void fRedo( )
		{
			mGraph->fReset( mNewGraph.fGetRawPtr( ) );
			( *mGraph )->fUpdate( );
		}
	};


	class tSaveParticleSystemGraphsAction : public tEditorAction
	{
		const tEditorSelectionList& mSystems;
		tGrowableArray< tGraphlineAction* > mActions;
		tGrowableArray< tGraphPtr > mOldGraphs;
		b32 mDirtyingAction;

	public:
		tSaveParticleSystemGraphsAction( const tEditorSelectionList& selectionList )
			: mSystems( selectionList )
			, mDirtyingAction( true )
		{
			fSetIsLive( true );

			for( u32 idx = 0; idx < mSystems.fCount( ); ++idx )
			{
				tSigFxParticleSystem* fxps = mSystems[ idx ]->fDynamicCast< tSigFxParticleSystem >( );
				tSigFxAttractor* fxa = mSystems[ idx ]->fDynamicCast< tSigFxAttractor >( );
				tSigFxMeshSystem* fxms = mSystems[ idx ]->fDynamicCast< tSigFxMeshSystem >( );

				if( fxps )
				{
					tParticleSystemPtr ps = fxps->fGetParticleSystem( );
					for( u32 i = 0; i < cEmissionGraphCount; ++i )
					{
						tGraphPtr old;
						tSigFxGraph::fSetOldGraph( ps->fState( ).fToolState( ).mEmissionGraphs[ i ], &old );
						mOldGraphs.fPushBack( old );
					}
					for( u32 i = 0; i < cParticleGraphCount; ++i )
					{
						tGraphPtr old;
						tSigFxGraph::fSetOldGraph( ps->fState( ).fToolState( ).mPerParticleGraphs[ i ], &old );
						mOldGraphs.fPushBack( old );
					}
					for( u32 i = 0; i < cMeshGraphCount; ++i )
					{
						tGraphPtr old;
						tSigFxGraph::fSetOldGraph( ps->fState( ).fToolState( ).mMeshGraphs[ i ], &old );
						mOldGraphs.fPushBack( old );
					}
				}
				else if( fxa )
				{
					tParticleAttractorPtr attractor = fxa->fGetAttractor( );
					if( attractor )
					{
						Attractors::tAttractorDataPtr data = attractor->fGetAttractorData( );

						for( u32 i = 0; i < cAttractorGraphCount; ++i )
						{
							tGraphPtr old;
							tSigFxGraph::fSetOldGraph( data->fGraph( i ), &old );
							mOldGraphs.fPushBack( old );
						}
					}
				}
				else if( fxms )
				{
					tMeshSystemPtr meshSystem = fxms->fFxMeshSystem( );
					if( meshSystem )
					{
						FxMeshSystem::tMeshSystemDataPtr data = meshSystem->fFxMeshSystemData( );
						for( u32 i = 0; i < cFxMeshSystemGraphCount; ++i )
						{
							tGraphPtr old;
							tSigFxGraph::fSetOldGraph( data->fGraph( i ), &old );
							mOldGraphs.fPushBack( old );
						}
					}
				}
			}
		}

		void fSetDirtyingAction( b32 dirty ) { mDirtyingAction = dirty; }
		virtual b32 fDirtyingAction( ) const { return mDirtyingAction; }

		virtual void fUndo( )
		{
			for( u32 i = 0; i < mActions.fCount( ); ++i )
				mActions[ i ]->fUndo( );
		}

		virtual void fRedo( )
		{
			for( u32 i = 0; i < mActions.fCount( ); ++i )
				mActions[ i ]->fRedo( );
		}

		virtual void fEnd( )
		{
			u32 oldGraphIdx = 0;
			for( u32 idx = 0; idx < mSystems.fCount( ); ++idx )
			{
				tSigFxParticleSystem* fxps = mSystems[ idx ]->fDynamicCast< tSigFxParticleSystem >( );
				tSigFxAttractor* fxa = mSystems[ idx ]->fDynamicCast< tSigFxAttractor >( );
				tSigFxMeshSystem* fxms = mSystems[ idx ]->fDynamicCast< tSigFxMeshSystem >( );

				if( fxps )
				{
					tParticleSystemPtr ps = fxps->fGetParticleSystem( );
					for( u32 i = 0; i < cEmissionGraphCount; ++i )
						mActions.fPushBack( new tGraphlineAction( &ps->fState( ).fToolState( ).mEmissionGraphs[ i ], mOldGraphs[ oldGraphIdx++ ] ) );
					for( u32 i = 0; i < cParticleGraphCount; ++i )
						mActions.fPushBack( new tGraphlineAction( &ps->fState( ).fToolState( ).mPerParticleGraphs[ i ], mOldGraphs[ oldGraphIdx++ ] ) );
					for( u32 i = 0; i < cMeshGraphCount; ++i )
						mActions.fPushBack( new tGraphlineAction( &ps->fState( ).fToolState( ).mMeshGraphs[ i ], mOldGraphs[ oldGraphIdx++ ] ) );
				}
				else if( fxa )
				{
					tParticleAttractorPtr attractor = fxa->fGetAttractor( );
					if( attractor )
					{
						for( u32 i = 0; i < cAttractorGraphCount; ++i )
						{
							mActions.fPushBack( new tGraphlineAction( &attractor->fGetAttractorData( )->fGraph( i ), mOldGraphs[ oldGraphIdx++ ] ) );
						}
					}
				}
				else if( fxms )
				{
					tMeshSystemPtr meshSystem = fxms->fFxMeshSystem( );
					if( meshSystem )
					{
						for( u32 i = 0; i < cFxMeshSystemGraphCount; ++i )
						{
							mActions.fPushBack( new tGraphlineAction( &meshSystem->fFxMeshSystemData( )->fGraph( i ), mOldGraphs[ oldGraphIdx++ ] ) );
						}
					}
				}
			}

			tEditorAction::fEnd( );
		}
	};

}


#endif // __tFxEditorActions__

