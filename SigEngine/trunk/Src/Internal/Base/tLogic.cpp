#include "BasePch.hpp"
#include "tLogic.hpp"
#include "tSceneGraph.hpp"
#include "tSync.hpp"
#include "Threads/tThread.hpp"

// required for XCode/GCC typeid(...):
#include "Logic/tAnimatable.hpp"
#include "Logic/tGoalDriven.hpp"
#include "Logic/tPhysical.hpp"

namespace Sig
{
	//#define sig_tracklogics

	namespace
	{
		static s32 gGlobalLogicCount = 0;
		static s32 gGlobalHighWaterLogicCount = 0;

#ifdef sig_logging
		static tGrowableArray<tLogic*> gLogicTracker;
#endif//sig_logging

	}//unnamed namespace

	s32 tLogic::fGlobalLogicCount( )
	{
		return gGlobalLogicCount;
	}

	s32 tLogic::fGlobalHighWaterLogicCount( )
	{
		return gGlobalHighWaterLogicCount;
	}

#ifdef sig_logging
	const tGrowableArray<tLogic*>& tLogic::fGlobalLogicTracker( )
	{
		return gLogicTracker;
	}
#endif//sig_logging

	const char* tLogic::fRunListString( tRunListId id )
	{
		static const char* strs[]=
		{
			"cRunListActST",
			"cRunListAnimateMT",
			"cRunListCollideMT",
			"cRunListPhysicsMT",
			"cRunListMoveST",
			"cRunListEffectsMT",
			"cRunListThinkST",
			"cRunListCameraST",
			"cRunListPreRenderST",
			"cRunListCoRenderMT",
		};
		static_assert( array_length( strs ) == cRunListCount );
		sigassert( id < array_length( strs ) );
		return strs[ id ];
	}

	tLogic::tLogic( )
		: mSceneGraph( 0 )
		, mOwnerEntity( 0 )
		, mGuid( ~0 )
	{
		//sigassert_is_main_thread( );

		gGlobalHighWaterLogicCount = fMax( ++gGlobalLogicCount, gGlobalHighWaterLogicCount );

#if defined( sig_logging ) && defined( sig_tracklogics )
		gLogicTracker.fPushBack( this );
#endif//defined( sig_logging ) && defined( sig_tracklogics )

		fMemSet( mRunListIndices, ~0 );
	}
	tLogic::~tLogic( )
	{
		sigassert_is_main_thread( );

		sigassert( gGlobalLogicCount > 0 );
		--gGlobalLogicCount;

#if defined( sig_logging ) && defined( sig_tracklogics )
		gLogicTracker.fFindAndErase( this );
#endif//defined( sig_logging ) && defined( sig_tracklogics )

		fRemoveFromRunLists( );
	}
	void tLogic::fOnSpawn( )
	{
	}
	void tLogic::fOnDelete( )
	{
		fRemoveFromRunLists( );
	}
	void tLogic::fOnEmptyNest( )
	{
	}
	tEntity* tLogic::fRootEntity( ) const
	{
		return mSceneGraph ? &mSceneGraph->fRootEntity( ) : 0;
	}
	b32 tLogic::fIsOwnerOrParent( tEntity* ent ) const
	{
		if( !ent )
			return false;

		tEntity* i = mOwnerEntity;
		while( i )
		{
			if( i == ent )
				return true;
			i = i->fParent( );
		}
		return false;
	}
	void tLogic::fRemoveFromRunLists( )
	{
		if( fSceneGraph( ) )
		{
			for( u32 i = 0; i < cRunListCount; ++i )
				fRunListRemove( ( tRunListId )i );
		}
	}
	void tLogic::fInsertStandAloneToSceneGraph( tSceneGraph& sg )
	{
		fSetSceneGraph( &sg );
		sg.fInsertStandAloneLogic( *this );
		fOnSpawn( );
	}
	void tLogic::fRemoveStandAloneFromSceneGraph( )
	{
		if( fSceneGraph( ) )
		{
			fOnDelete( );
			fRemoveFromRunLists( );
			fSceneGraph( )->fRemoveStandAloneLogic( *this );
			fSetSceneGraph( 0 );
		}
	}
	void tLogic::fSetSceneGraph( tSceneGraph* sg ) 
	{ 
		mSceneGraph = sg;
		mGuid = mSceneGraph ? mSceneGraph->fNextLogicGuid( ) : ~0;
		sync_event_v_c( mGuid, tSync::cSCLogic );
	}
	void tLogic::fRunListInsert( tRunListId id )
	{
		// only insert if i'm not already in this runlist
		if( !fSceneGraph( ) || mRunListIndices[ id ] != cInvalidListIndex )
			return;

		sync_event_c( "(LogicGuid,ListId)", Math::tVec2u( mGuid, ( u32 )id ), tSync::cSCLogic );

		tRunList& runList = fSceneGraph( )->fRunLists( )[ id ];

		// store my index in the runlist
		mRunListIndices[ id ] = runList.fCount( );

		// add myself at the end
		runList.fPushBack( this );

	}
	void tLogic::fRunListRemove( tRunListId id )
	{
		// only remove if i'm already in this runlist
		if( !fSceneGraph( ) || mRunListIndices[ id ] == cInvalidListIndex )
			return;

		sync_event_c( "(LogicGuid,ListId)", Math::tVec2u( mGuid, ( u32 )id ), tSync::cSCLogic );

		tRunList& runList = fSceneGraph( )->fRunLists( )[ id ];
		sigassert( runList[ mRunListIndices[ id ] ] == this );

		// store current (what will become 'old') index
		const u16 oldIndex = mRunListIndices[ id ];

		// remove me from the runlist
		runList.fErase( oldIndex );

		// adjust new logic object's runlist index (as it's been moved to my old slot)
		if( oldIndex < runList.fCount( ) )
			runList[ oldIndex ]->mRunListIndices[ id ] = oldIndex;

		// invalidate my index
		mRunListIndices[ id ] = cInvalidListIndex;
	}
}

//--------------------------------------------------------------------------------------------------------------
//
//    Script-Specific Implementation
//
//--------------------------------------------------------------------------------------------------------------

namespace Sig
{

	namespace
	{
		static b32 fIsLogicValid( tLogic* logic )
		{
			return logic != 0;
		}
	}

	void tLogic::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class<tLogic, Sqrat::NoCopy<tLogic> > classDesc( vm.fSq( ) );
		classDesc
			.StaticFunc(_SC("IsValid"),		&fIsLogicValid)
			.Func(_SC("DebugTypeName"),		&tLogic::fDebugTypeName)
			.Func(_SC("OnSpawn"),			&tLogic::fOnSpawn)
			.Func(_SC("OnDelete"),			&tLogic::fOnDelete)
			.Func(_SC("OnEmptyNest"),		&tLogic::fOnEmptyNest)
			.Prop(_SC("Animatable"),		&tLogic::fQueryAnimatable)
			.Prop(_SC("Physical"),			&tLogic::fQueryPhysical)
			.Prop(_SC("SceneGraph"),		&tLogic::fSceneGraph)
			.Prop(_SC("OwnerEntity"),		&tLogic::fOwnerEntity)
			.Prop(_SC("RootEntity"),		&tLogic::fRootEntity)
			.Func(_SC("HandleLogicEvent"),	&tLogic::fHandleLogicEvent)
			.Func(_SC("IsOwnerOrParent"),	&tLogic::fIsOwnerOrParent)
			;

		vm.fRootTable( ).Bind( _SC("Logic"), classDesc );
	}

}



