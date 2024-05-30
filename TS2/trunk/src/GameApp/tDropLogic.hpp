#ifndef __tDropLogic__
#define __tDropLogic__
#include "tShellLogic.hpp"
#include "Audio/tSource.hpp"

namespace Sig
{
	//simple logic type to serve as a base for dropping things into the world

	class tDropLogic : public tProjectileLogic
	{
		define_dynamic_cast( tDropLogic, tProjectileLogic );
	public:
		tDropLogic( );
		virtual ~tDropLogic( );

		virtual void fOnSpawn( );
		virtual void fOnDelete( );
		virtual void fOnPause( b32 paused );

		virtual void fActST( f32 dt );
		virtual void fMoveST( f32 dt );
		virtual void fCoRenderMT( f32 dt );

		virtual void fComputeNewPosition( f32 dt );
		virtual void fHitSomething( const tEntityPtr& ent );

		virtual void fRayCast( );

		tStringPtr mSpawnAudioEvent;
		tStringPtr mDeleteAudioEvent;
		tStringPtr mLandAudioEvent;

	public:
		static void fSpawn( const Math::tVec3f& origin );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		tShellPhysics mPhysics;
		b32 mFalling;

		Audio::tSourcePtr mAudio;
	};

}

#endif//__tDropLogic__