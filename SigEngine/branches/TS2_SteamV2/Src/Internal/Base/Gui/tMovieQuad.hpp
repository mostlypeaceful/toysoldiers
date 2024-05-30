#ifndef __tMovieQuad__
#define __tMovieQuad__
#include "tRenderableCanvas.hpp"
#include "tMoviePlayer.hpp"

namespace Sig { namespace Gfx
{
	class tDevice;
}}

namespace Sig { namespace Gui
{

	class tMovieQuad : public tRenderableCanvas
	{
		define_dynamic_cast( tMovieQuad, tRenderableCanvas );
	public:
		tMovieQuad( );
		virtual ~tMovieQuad( );
		virtual void fOnRenderCanvas( Gfx::tScreen& screen ) const;
		virtual void fOnTickCanvas( f32 dt );

		void fSetMovie( const tFilePathPtr& moviePath, b32 looping );
		void fRefreshVideoRect( u32 l, u32 t, u32 r, u32 b );
		void fSetRect( f32 w, f32 h );
		void fSetRect( const Math::tVec2f& widthHeight );
		void fPause( b32 pause );
		void fStop( );
		b32 fFinishedPlaying( );

	protected:
		void fCommonCtor( );
		void fResetDeviceObjects( Gfx::tDevice& device );

		virtual void fOnMoved( );
		virtual void fOnParentMoved( );

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );
		void fSetPositionAndSize();
	protected:
		tMoviePlayerPtr		mPlayer;
		Sqrat::Function		mOnFinished;
		b32					mFinished;
	};

}}

#endif//__tMovieQuad__
