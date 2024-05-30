#ifndef __tLogic__
#define __tLogic__
#include "Scripts/tScriptVm.hpp"

namespace Sig { namespace Logic
{
	class tAnimatable;
	class tGoalDriven;
	class tPhysical;
	class tDamageable;
	class tEvent;
}}

namespace Sig
{
	class tEntity;
	class tSceneGraph;

	class base_export tLogic : public tThreadSafeRefCounter
	{
		debug_watch( tLogic );
		declare_uncopyable( tLogic );
		define_dynamic_cast_base( tLogic );
		friend class tEntity;

	public:
		enum tRunListId
		{
			// ST = Single Threaded, MT = Multi Threaded. Run lists are executed in the order listed here.
			// Example: ThinkST comes after MoveST, and being single threaded, it should be safe to read the updated positions of other entities from that run list.
			cRunListActST,
			cRunListAnimateMT,
			cRunListCollideMT, // deprecated, use AnimateMT instead
			cRunListPhysicsMT, // deprecated, use AnimateMT instead
			cRunListMoveST,
			cRunListEffectsMT,
			cRunListThinkST,
			cRunListCameraST,
			cRunListPreRenderST,
			cRunListCoRenderMT, // lots of misc processing here: raycast, spatial query, pathfind, fx, etc. -- occurs while we're e.g. gathering draw calls
			cRunListCount
		};

		static s32 fGlobalLogicCount( );
		static s32 fGlobalHighWaterLogicCount( );
#ifdef sig_logging
		static const tGrowableArray<tLogic*>& fGlobalLogicTracker( );
#endif//sig_logging

		typedef tGrowableArray<tLogic*>				tRunList;
		typedef tFixedArray<tRunList,cRunListCount>	tRunListSet;
		static const char* fRunListString( tRunListId id );

	private:
		typedef tFixedArray<u16,cRunListCount> tRunListIndices;
		static const u16 cInvalidListIndex = ~0;
	private:
		tRunListIndices mRunListIndices;
		tSceneGraph*	mSceneGraph;
		tEntity*		mOwnerEntity;
		u32				mGuid;        
	public: // events
		tLogic( );
		virtual ~tLogic( );
		virtual void fSetupComponents( ) { }
		virtual void fOnSpawn( ); // add self to relevant run lists in fOnSpawn
		virtual void fOnDelete( ); // final deletion notification (i.e., getting removed from scene graph/parent)
		virtual void fOnPause( b32 paused ) { } // become notified of pause/unpause
		virtual void fOnEmptyNest( );
		virtual void fOnSkeletonPropagated( ) { } // become notified of skeleton propagation
		virtual b32  fReadyForDeletion( ) { return true; } // can override this to inform parent/scene graph that you'd like to be re-added to the root before the parent is deleted
		virtual b32  fHandleLogicEvent( const Logic::tEvent& e ) { return false; } // callback for all sorts of various events that need to inform game code
	public: // query specific components
		virtual Logic::tAnimatable*		fQueryAnimatable( ) { return 0; }
		virtual Logic::tPhysical*		fQueryPhysical( ) { return 0; }
		virtual Logic::tDamageable*		fQueryDamageable( ) { return 0; }
		template<class tDerived>
		inline tDerived* fQueryPhysicalDerived( );
		template<class tDerived>
		inline tDerived* fQueryDamageableDerived( );

	public: // kinda hackish maybe?
		virtual Math::tVec4f fQueryDynamicVec4Var( const tStringPtr& varName, u32 viewportIndex ) const { return Math::tVec4f::cZeroVector; }
		virtual u32 fQueryTransitionObjects( Math::tVec4f* o, u32 osize, const Math::tAabbf& box ) const { return 0; }
		virtual void fStateMaskEnable( u32 index, u32 mask ) { }

	public:
		u32 fGuid( ) const { return mGuid; }
		tSceneGraph* fSceneGraph( ) const { return mSceneGraph; }
		tEntity* fOwnerEntity( ) const { return mOwnerEntity; }
		tEntity* fRootEntity( ) const;
		b32 fIsOwnerOrParent( tEntity* ent ) const;
		b32 fInRunList( tRunListId id ) const { return mRunListIndices[ id ] != cInvalidListIndex; }
		void fRemoveFromRunLists( );

		void fInsertStandAloneToSceneGraph( tSceneGraph& sg );
		void fRemoveStandAloneFromSceneGraph( );

		inline void fOnTick( tRunListId id, f32 dt )
		{
			switch( id )
			{
			case cRunListActST:			fActST( dt ); break;
			case cRunListAnimateMT:		fAnimateMT( dt ); break;
			case cRunListCollideMT:		fCollideMT( dt ); break;
			case cRunListPhysicsMT:		fPhysicsMT( dt ); break;
			case cRunListMoveST:		fMoveST( dt ); break;
			case cRunListEffectsMT:		fEffectsMT( dt ); break;
			case cRunListThinkST:		fThinkST( dt ); break;
			case cRunListCameraST:		fCameraST( dt ); break;
			case cRunListPreRenderST:	fPreRenderST( dt ); break;
			case cRunListCoRenderMT:	fCoRenderMT( dt ); break;
			default: sigassert( !"Invalid run list" ); break;
			}
		}
		if_devmenu( virtual void fAddWorldDebugText( std::stringstream& ) const { } );
	
	public:

		void fRunListInsert( tRunListId id );
		void fRunListRemove( tRunListId id );
		
		virtual void fActST( f32 dt ) { }
		virtual void fAnimateMT( f32 dt ) { }
		virtual void fCollideMT( f32 dt ) { }
		virtual void fPhysicsMT( f32 dt ) { }
		virtual void fMoveST( f32 dt ) { }
		virtual void fEffectsMT( f32 dt ) { }
		virtual void fThinkST( f32 dt ) { }
		virtual void fCameraST( f32 dt ) { }
		virtual void fPreRenderST( f32 dt ) { }
		virtual void fCoRenderMT( f32 dt ) { }

		// ONLY MEANT TO BE CALLED BY SYSTEM FUNCTIONS, not users.
		void fSetSceneGraph( tSceneGraph* sg );
	private:
		void fSetOwnerEntity( tEntity* entity ) { mOwnerEntity = entity; }

	public: // script-specific
		static void	fExportScriptInterface( tScriptVm& vm );
	};
}

#endif//__tLogic__
