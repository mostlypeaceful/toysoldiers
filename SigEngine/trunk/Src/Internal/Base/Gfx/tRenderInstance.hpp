#ifndef __tRenderInstance__
#define __tRenderInstance__
#include "tRenderBatch.hpp"
#include "tSphericalHarmonics.hpp"

namespace Sig
{
	class tSkinMap;
}

namespace Sig { namespace Gui
{
	class tRenderableCanvas;
}}

namespace Sig { namespace Gfx
{
	///
	/// \brief Encapsulates instance-specific renderable data.
	class base_export tRenderInstance
	{
		debug_watch( tRenderInstance );
	public:
		tRenderInstance( );
		virtual ~tRenderInstance( ) { }

		inline b32 fRequiresXparentSort( f32 fadeAlpha ) const				{ return ( fadeAlpha * mRgbaTint.w ) < ( 1.f - 1.f/255.f ); }
		inline b32 fBatchHasXparency( ) const								{ return mBatch && mBatch->fBatchData( ).mRenderState->fHasTransparency( ); }

		inline const Math::tVec4f& fRgbaTint( ) const						{ return mRgbaTint; }
		inline Math::tVec4f fRgbaTint( f32 fadeAlpha ) const				{ return Math::tVec4f( mRgbaTint.x, mRgbaTint.y, mRgbaTint.z, mRgbaTint.w * fadeAlpha ); }
		void fSetRgbaTint( const Math::tVec4f& rgbaTint )					{ mRgbaTint = rgbaTint; }

		inline const tRenderBatchPtr& fRenderBatch( ) const					{ return mBatch; }
		void fSetRenderBatch( const tRenderBatchPtr& batch )				{ mBatch = batch; }

		void							fRI_SetObjectToWorld( const Math::tMat3f* objectToWorld ) { mRI_ObjectToWorld = objectToWorld ? objectToWorld : &Math::tMat3f::cIdentity; }
		inline const Math::tMat3f&		fRI_ObjectToWorld( ) const			{ return *mRI_ObjectToWorld; }
		virtual const Math::tMat3f*		fRI_ObjectToLocal( ) const			{ return 0; }
		virtual const Math::tMat3f*		fRI_LocalToObject( ) const			{ return 0; }
		virtual const tSkinMap*			fRI_SkinMap( ) const				{ return 0; }
		virtual const Gui::tRenderableCanvas* fRI_RenderableCanvas( ) const { return 0; }
		virtual Math::tVec4f			fRI_DynamicVec4( const tStringPtr& varName, u32 viewportIndex ) const { return Math::tVec4f::cZeroVector; }
		virtual u32						fRI_TransitionObjects( Math::tVec4f* o, u32 osize ) const { return 0; }
		virtual f32						fRI_FarFadeOutDistance( ) const { return 0.f; }

	private:
		Math::tVec4f					mRgbaTint;
		tRenderBatchPtr					mBatch;
		const Math::tMat3f*				mRI_ObjectToWorld;
	};

}}

#endif//__tRenderInstance__
