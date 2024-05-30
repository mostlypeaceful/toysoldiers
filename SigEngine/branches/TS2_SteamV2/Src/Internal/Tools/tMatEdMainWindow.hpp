#ifndef __tMatEdMainWindow__
#define __tMatEdMainWindow__
#include "tWxSlapOnDialog.hpp"
#include "Editor/tEditableProperty.hpp"
#include "tMaterialPreviewPanel.hpp"

namespace Sig
{
	class tWxSlapOnGroup;
	class tDermlBrowser;
	class tEditorActionStack;
	namespace Derml { class tFile; class tMtlFile; }

	class tools_export tMatEdMainWindow : public tWxSlapOnDialog
	{
		tEditorActionStack& mActionStack;
		b32 mFullEditMode;
		wxStaticText* mHeaderText;
		tMaterialPreviewPanel* mPreviewPanel;
		tWxSlapOnGroup* mDermlBrowserPanel;
		tDermlBrowser* mDermlBrowser;
		wxScrolledWindow* mPropertyPanel;
		tGrowableArray<tEditablePropertyTable> mCommonProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnPropertyChanged;
		Gfx::tDevicePtr	mDevice;
	public:
		tMatEdMainWindow( wxWindow* parent, tEditorActionStack& actionStack, const std::string& regKeyName, b32 fullEditMode );
		~tMatEdMainWindow( );
		void fSetupPreviewWindow( const Gfx::tDevicePtr& device = Gfx::tDevicePtr( ) ); // if null device specified, MatEd window creates own device
		b32 fHasSameDevice( const Gfx::tDevicePtr& device ) const;
		const Gfx::tDevicePtr& fDevice( ) const { return mDevice; }
		Dx9Util::tTextureCachePtr fTextureCache( ) const;
		Gfx::tLightEntityPtr fDefaultLight( ) const;
		tMaterialPreviewBundlePtr fPreviewBundle( ) const;
		void fSetPreviewBundle( const tMaterialPreviewBundlePtr& bundle );
		void fClear( );
		void fEnableShaderBrowser( b32 enable );
		void fFromDermlFile( const Derml::tFile& derml, const tFilePathPtr& shaderPath = tFilePathPtr( ) );
		void fFromDermlMtlFile( const Derml::tMtlFile& mtlFile );
		b32 fSetDefaultBrowseDirectory( const tFilePathPtr& defaultBrowseDirectory = tFilePathPtr( ) );
		virtual b32 fOnTick( );
		virtual void fOnShaderSelected( const tFilePathPtr& shaderPath ) { }
	private:
		struct tPropertyChangeContext
		{
			u32 mLabelWidth, mControlWidth;
			u32 mNumPropsAdded;
			u32 mTotalHeight;
			tPropertyChangeContext( ) { fZeroOut( this ); }
		};
		tPropertyChangeContext fBeginPropertyChange( u32 numPotentialProps );
		void fEndPropertyChange( const tPropertyChangeContext& context );
		void fOnPropertyChanged( tEditableProperty& property );
	};
}

#endif//__tMatEdMainWindow__
