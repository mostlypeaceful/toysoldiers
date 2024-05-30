#ifndef __tScreenSpaceFxSystem__
#define __tScreenSpaceFxSystem__
#include "tCanvas.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "Gfx/tDeviceResource.hpp"
#include "tFxFileRefEntity.hpp"

namespace Sig { namespace Gui
{
	class base_export tScreenSpaceFxSystem : public tCanvas, public Gfx::tRenderableEntity, public Gfx::tDeviceResource
	{
		define_dynamic_cast( tScreenSpaceFxSystem, tCanvas );
	public:
		explicit tScreenSpaceFxSystem( );
		virtual ~tScreenSpaceFxSystem( );

		virtual void fOnRenderCanvas( Gfx::tScreen& screen ) const;
		virtual void fOnDeviceLost( Gfx::tDevice* device )	{ }
		virtual void fOnDeviceReset( Gfx::tDevice* device ) { }
		virtual void fOnTickCanvas( f32 dt );

		void fSetPaused( b32 paused );
		void fSetSystem( const tFilePathPtr& path, s32 playcount, b32 local );
		void fSetPosition( f32 x, f32 y, f32 z );
		void fFadeOutSystems( f32 fadeOutTime );
		void fOverrideSystemAlphas( f32 newAlpha );
		void fSetDelay( f32 timeDelay ) { mDelay = timeDelay; }
		void fFastForward( f32 amount );
		void fSetEmissionRates( f32 rate );

	protected:
		void fCommonCtor( );
		void fResetDeviceObjects( Gfx::tDevice& device );
		void fAddDrawCalls( Gfx::tScreen& screen, tEntity* entity ) const;

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	private:
		FX::tFxFileRefEntityPtr mFxSystem;
		f32 mDelay;

	};
}}

#endif//__tScreenSpaceFxSystem__
