#ifndef __tLightEffectLogic__
#define __tLightEffectLogic__

#include "tLogic.hpp"
#include "Gfx/tLightEntity.hpp"

namespace Sig
{
	class tLightEffectLogic : public tLogic
	{
		define_dynamic_cast( tLightEffectLogic, tLogic );

	public:
		tLightEffectLogic( f32 radius = 1.0f, f32 expandTime = 0.0f, f32 collapseTime = 1.f, b32 persist = false );
		virtual void fOnDelete( );
		virtual void fOnSpawn( );
		virtual void fOnPause( b32 paused );

		void fSetParameters( f32 radius, f32 expandTime, f32 collapseTime );
		void fSetColor( const Math::tVec4f& color );
		void fSetPersist( b32 persist );
		void fRestart( );
		void fSetActive( b32 active );

		// This will take ownership of the logic ptr
		static Gfx::tLightEntity* fSpawnLightEffect( const Math::tMat3f& worldXForm, tLightEffectLogic *logic, tEntity& parent, const Math::tVec4f& color = Math::tVec4f::cOnesVector );

	protected:
		virtual void fMoveST( f32 dt );

		Math::tVec2f fGetRadii( ) const;

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		f32 mRadius;

		f32 mAge;
		f32 mExpandTime;
		f32 mCollapseTime;

		b8 mExpanding;
		b8 mPersist;
		b8 mAlive;
		b8 pad1;

		Gfx::tLightEntityPtr mLight;
	};

}

#endif//__tLightEffectLogic__
