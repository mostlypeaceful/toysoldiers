#include "BasePch.hpp"
#include "tRenderableCanvas.hpp"
#include "Gfx/tScreen.hpp"

#ifdef sig_logging 
	#include "tTexturedQuad.hpp" //only for debugging out of Z range
#endif

namespace Sig { namespace Gui
{
	tRenderableCanvas::tRenderableCanvas( )
		: mRenderObjectToWorld( Math::tMat3f::cIdentity )
		, mDropShadowEnabled( false )
		, mScissorRectEnabled( false )
		, mDropShadowX( 2 )
		, mDropShadowY( 2 )
	{
		fRI_SetObjectToWorld( &mRenderObjectToWorld );
	}
	void tRenderableCanvas::fOnRenderCanvas( Gfx::tScreen& screen ) const
	{
		if_logging( 
		{
			f32 z = mRenderObjectToWorld.fGetTranslation( ).z;
			if( !fInBounds( z, 0.0f, 1.0f ) )
			{
				tTexturedQuad* tq = fDynamicCast<tTexturedQuad>( );
				if( tq )
				{
					log_warning( 0, "Canvas out of Z range! Texture: " << tq->fColorMapResource( )->fGetPath( ) );
				}
				else
					log_warning( 0, "Canvas out of Z range! Type: " << fDebugTypeName( ) );
			}
		} );

		// TODO REFACTOR take user/vp into account
		const b32 vpIsVirtual = fIsVirtualMode( );
		const b32 disabled = fDisabled( ) || fInvisible( );
		if( !disabled && !vpIsVirtual && fRenderBatch( ) && !fEqual( tRenderInstance::fRgbaTint( ).w, 0.f ) )
			screen.fAddScreenSpaceDrawCall( fDrawCall( ) );
	}
	void tRenderableCanvas::fOnMoved( )
	{
		tCanvas::fOnMoved( );
		fUpdateRenderObjectToWorld( );
	}
	void tRenderableCanvas::fOnTintChanged( )
	{
		tCanvas::fOnTintChanged( );
		fUpdateRenderTint( );
	}
	void tRenderableCanvas::fOnParentMoved( )
	{
		tCanvas::fOnParentMoved( );
		fUpdateRenderObjectToWorld( );
	}
	void tRenderableCanvas::fOnParentTintChanged( )
	{
		tCanvas::fOnParentTintChanged( );
		fUpdateRenderTint( );
	}
	void tRenderableCanvas::fUpdateRenderObjectToWorld( )
	{
		fUpdateRenderTint( );

		if( fEqual( fWorldXform( ).mAngle, 0.f ) && fWorldXform( ).mScale.fEqual( Math::tVec2f::cOnesVector ) )
		{
			// the object is not scaled, nor rotated, so make sure that it's position falls on integer boundaries to make it look nicer
			mRenderObjectToWorld.fSetTranslation( Math::tVec3f( fRound<f32>( fWorldXform( ).mPosition.x ), fRound<f32>( fWorldXform( ).mPosition.y ), fWorldXform( ).mPosition.z ) );
		}
		else
			mRenderObjectToWorld.fSetTranslation( fWorldXform( ).mPosition );

		const f32 sint = Math::fSin( fWorldXform( ).mAngle );
		const f32 cost = Math::fCos( fWorldXform( ).mAngle );

		mRenderObjectToWorld(0,0) = fWorldXform( ).mScale.x * cost; mRenderObjectToWorld(0,1) = -fWorldXform( ).mScale.y * sint;
		mRenderObjectToWorld(1,0) = fWorldXform( ).mScale.x * sint; mRenderObjectToWorld(1,1) =  fWorldXform( ).mScale.y * cost;
	}
	void tRenderableCanvas::fUpdateRenderTint( )
	{
		Gfx::tRenderInstance::fSetRgbaTint( fWorldXform( ).mRgbaTint );
	}

	void tRenderableCanvas::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::DerivedClass<tRenderableCanvas, tCanvas, Sqrat::NoCopy<tRenderableCanvas> > classDesc( vm.fSq( ) );

		vm.fNamespace(_SC("Gui")).Bind(_SC("RenderableCanvas"), classDesc);
	}

}}
