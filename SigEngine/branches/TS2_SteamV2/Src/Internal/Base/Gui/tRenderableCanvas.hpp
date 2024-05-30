#ifndef __tRenderableCanvas__
#define __tRenderableCanvas__
#include "tCanvas.hpp"
#include "Gfx/tRenderableEntity.hpp"
#include "Gfx/tDeviceResource.hpp"

namespace Sig { namespace Gui
{
	class base_export tRenderableCanvas : public tCanvas, public Gfx::tRenderInstance, public Gfx::tDeviceResource
	{
		define_dynamic_cast( tRenderableCanvas, tCanvas );
	public:
		tRenderableCanvas( );
	public:
		virtual void fOnDeviceLost( Gfx::tDevice* device )	{ }
		virtual void fOnDeviceReset( Gfx::tDevice* device ) { }
		virtual void fOnRenderCanvas( Gfx::tScreen& screen ) const;

		// tRenderInstance
		virtual const Gui::tRenderableCanvas* fRI_RenderableCanvas( ) const { return this; }

		b32						fValid( ) const { return fRenderBatch( ) && fRenderBatch( )->fBatchData( ).fValid( ); }

		Gfx::tDrawCall			fDrawCall( ) const { return Gfx::tDrawCall( *this, fWorldXform( ).mPosition.z ); }
		const Math::tMat3f&		fRenderObjectToWorld( ) const { return mRenderObjectToWorld; }
		void					fSetRgbaTint( const Math::tVec4f& tint ) { tCanvas::fSetRgbaTint( tint ); }

		const Math::tRect&		fScissorRect( ) const { return mScissorRect; }
		b32						fScissorRectEnabled( ) const { return mScissorRectEnabled; }
		virtual void			fSetScissorRect( const Math::tRect& rect ) { mScissorRectEnabled = true; mScissorRect = rect; }
		void					fDisableScissorRect( ) { mScissorRectEnabled = false; mScissorRect = Math::tRect( ); }

		u32						fDropShadowEnabled( ) const { return mDropShadowEnabled; }
		void					fSetDropShadowEnabled( b32 enable ) { mDropShadowEnabled = enable; }
		u32						fDropShadowX( ) const { return mDropShadowX; }
		void					fSetDropShadowX( u32 xOffset ) { mDropShadowX = xOffset; }
		u32						fDropShadowY( ) const { return mDropShadowY; }
		void					fSetDropShadowY( u32 yOffset ) { mDropShadowY = yOffset; }

	public: // script-specific
		static void fExportScriptInterface( tScriptVm& vm );

	protected:
		virtual void fOnMoved( );
		virtual void fOnTintChanged( );
		virtual void fOnParentMoved( );
		virtual void fOnParentTintChanged( );
		void fUpdateRenderObjectToWorld( );
		void fUpdateRenderTint( );

	private:
		Math::tMat3f mRenderObjectToWorld;
		Math::tRect mScissorRect;
		u8 mDropShadowEnabled, mScissorRectEnabled, mDropShadowX, mDropShadowY;
	};

}}

#endif//__tRenderableCanvas__
