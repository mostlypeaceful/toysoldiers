#ifndef __tViewport__
#define __tViewport__
#include "tCamera.hpp"

namespace Sig { namespace Gfx
{
	class tDevicePtr;
	class tScreen;
	class tScreenPtr;
	class tLightComboList;
	class tRenderableEntityList;
	class tCameraController;
	struct tDisplayStats;

	///
	/// \brief TODO document
	class base_export tViewport : public tRefCounter
	{
	private:

		tCamera						mLogicCamera;
		tCamera						mRenderCamera;
		tStringPtr					mPostEffectSequence;
		Math::tRect					m16_9ClipBox;
		Math::tRect					mClipBox;
		const u32					mViewportIndex;
		b32							mIsVirtual;

	public:
		explicit tViewport( u32 vpIndex );
		~tViewport( );

		// this clip box crops the ClipBox to always fit in a 16_9 area
		Math::tRect						f16_9AdjustedClipBox( );

		void							fSet16_9ClipBox( const Math::tRect& tlbr ) { m16_9ClipBox = tlbr; }
		const Math::tRect&				f16_9ClipBox( ) { return m16_9ClipBox; }

		f32								fAspectRatio( ) const 
		{
			return mClipBox.fAspectRatio( );
		}
		const Math::tRect&				fClipBox( ) const { return mClipBox; }
		void							fSetClipBox( const Math::tRect& tlbr ) { mClipBox = tlbr; }

		const tCamera&					fLogicCamera( ) const			{ return mLogicCamera; }
		const tCamera&					fRenderCamera( ) const			{ return mRenderCamera; }
		void							fSetCameras( const tCamera& camera );
		void							fSetLogicCamera( const tCamera& camera ) { mLogicCamera = camera; }
		void							fSetRenderCamera( const tCamera& camera ) { mRenderCamera = camera; }

		b32								fIsVirtual( ) const				{ return mIsVirtual; }
		void							fSetIsVirtual( b32 virt )	{ mIsVirtual = virt; }

		b32								fZeroSized( ) const { return !mClipBox.fHasArea( ); }
		b32								fIsIgnored( ) const { return fZeroSized( ); }

		u32								fViewportIndex( ) const			{ return mViewportIndex; }

		void							fSetPostEffectSequence( const tStringPtr& seq ) { mPostEffectSequence = seq; }
		const tStringPtr&				fPostEffectSequence( ) const	{ return mPostEffectSequence; }

		Math::tRect						fComputeRect( f32 screenWidth, f32 screenHeight );
		Math::tRect						fComputeSafeRect( f32 screenWidth, f32 screenHeight, const Math::tVec2u& safeEdge );
		Math::tRect						fComputeRect( const tScreenPtr& screen );
		Math::tRect						fComputeSafeRect( const tScreenPtr& screen );

		// you probably shouldn't be calling this; called inside tScreen's render loop
		void fRenderLitWorld( 
			tScreen& screen,
			const tLightComboList& lightCombos,
			const tRenderableEntityList& explicitWorldObjects, 
			const tRenderableEntityList& explicitWorldTopObjects, 
			tDisplayStats& worldDisplayStats ) const;
	};

	typedef tRefCounterPtr< tViewport > tViewportPtr;

}}

#endif//__tViewport__
