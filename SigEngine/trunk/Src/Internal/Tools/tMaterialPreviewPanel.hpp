#ifndef __tMaterialPreviewPanel__
#define __tMaterialPreviewPanel__
#include "tMaterialPreviewBundle.hpp"
#include "Derml.hpp"
#include "tSceneGraph.hpp"
#include "Gfx/tScreen.hpp"
#include "Gfx/tLightEntity.hpp"

namespace Sig
{
	class tools_export tMaterialPreviewPanel : public wxPanel
	{
		wxSize mOgSize;
		tSceneGraphPtr mSceneGraph;
		Gfx::tScreenPtr	mScreen;
		Gfx::tLightEntityPtr mDefaultLight;
		Math::tVec3f mNormalClearColor;
		Math::tVec3f mErrorClearColor;
		Math::tMat3f mInitialLightXform;
		Math::tMat3f mInitialCameraXform;
		Time::tStopWatch mTimer;
		f32 mLastElapsed;
		f32 mYaw;
		b32 mDoYaw;
		u32 mCurrentPreviewGeometry;
		tMaterialPreviewBundlePtr mPreviewBundle;
		Derml::tMtlFile mMaterialFile;
		Dx9Util::tTextureCachePtr mTextureCache;

		wxButton* mPlayButton;
		wxButton* mModeButton;
		wxButton* mResetButton;
		wxButton* mAmbButton;
		wxButton* mFrontButton;
		wxButton* mRimButton;
		wxButton* mBackButton;

	public:
		tMaterialPreviewPanel( wxWindow* parent, u32 width, u32 height,
			wxButton* playButton,
			wxButton* modeButton,
			wxButton* resetButton,
			wxButton* ambButton,
			wxButton* frontButton,
			wxButton* rimButton,
			wxButton* backButton );
		~tMaterialPreviewPanel( );
		void fSetup( const Gfx::tDevicePtr& device );
		const Dx9Util::tTextureCachePtr& fTextureCache( ) const { return mTextureCache; }
		const Gfx::tLightEntityPtr& fDefaultLight( ) const { return mDefaultLight; }
		const tMaterialPreviewBundlePtr& fPreviewBundle( ) const { return mPreviewBundle; }
		void fSetPreviewBundle( const tMaterialPreviewBundlePtr& bundle );
		void fRender( );
		void fSetTimeRunning( b32 run );
		void fClear( );
		void fFromDermlMtlFile( const Derml::tMtlFile& mtlFile );
		const Derml::tMtlFile& fMtlFile( ) const { return mMaterialFile; }
		Gfx::tScreen* fScreen( ) const { return mScreen.fGetRawPtr( ); }
		tSceneGraph* fSceneGraph( ) const { return mSceneGraph.fGetRawPtr( ); }
		void fSetCurrentPreviewGeometry( u32 geometryMode ) { mCurrentPreviewGeometry = geometryMode; }
	private:
		void fOnButtonPlay( wxCommandEvent& );
		void fOnButtonMode( wxCommandEvent& );
		void fOnButtonReset( wxCommandEvent& );
		void fOnButtonAmb( wxCommandEvent& );
		void fOnButtonFront( wxCommandEvent& );
		void fOnButtonRim( wxCommandEvent& );
		void fOnButtonBack( wxCommandEvent& );
	};
}

#endif//__tMaterialPreviewPanel__
