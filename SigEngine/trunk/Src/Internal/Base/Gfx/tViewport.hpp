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
		Math::tRect					mClipBox;
		const u32					mViewportIndex;
		b32							mIsVirtual;
		tScreen&					mScreen;

	public:
		explicit tViewport( u32 vpIndex, tScreen& screen );
		~tViewport( );

		tScreen&						fScreen( ) { return mScreen; }
		const tScreen&					fScreen( ) const { return mScreen; }

		f32								fAspectRatio( ) const { return mClipBox.fAspectRatio( ); }
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

		Math::tRect						fComputeRect( f32 screenWidth, f32 screenHeight ) const;
		Math::tRect						fComputeSafeRect( f32 screenWidth, f32 screenHeight, const Math::tVec2u& safeEdge ) const;
		Math::tRect						fComputeRect( ) const;
		Math::tRect						fComputeSafeRect( ) const;

		///
		/// \brief Projects a point in world space to a point in screen coords in the user's viewport.
		Math::tVec3f fProjectToScreen( const Math::tVec3f& worldPos ) const;
		Math::tVec3f fProjectToScreen( const Gfx::tCamera& camera, const Math::tVec3f& worldPos ) const;
		Math::tVec3f fProjectToScreen( const Gfx::tCamera& camera, const Math::tRect& safeRect, const Math::tVec3f& worldPos ) const;
		Math::tRectf fProjectToScreen( const Math::tAabbf& worldBounds, b32& entireOutsideFrustumOutput ) const;
		Math::tRectf fProjectToScreen( const Gfx::tCamera& camera, const Math::tRect& safeRect, const Math::tAabbf& worldBounds, b32& entireOutsideFrustumOutput ) const;
		b32 fProjectToScreenClamped( const Math::tVec3f& worldPos, Math::tVec3f& posOut ) const; //returns false if point is behind camera
		b32 fProjectToScreenClamped( const Gfx::tCamera& camera, const Math::tVec3f& worldPos, Math::tVec3f& posOut ) const; //returns false if point is behind camera

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
