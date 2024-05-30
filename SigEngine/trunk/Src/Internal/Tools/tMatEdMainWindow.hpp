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
	protected:
		tEditorActionStack& mActionStack;
		b32 mEnableShaderBrowsing;
		b32 mEnableUserPropertyEditing;

		wxStaticText* mHeaderText;
		tMaterialPreviewPanel* mPreviewPanel;

		tWxSlapOnGroup* mDermlBrowserPanel;
		tDermlBrowser* mDermlBrowser;

		tWxSlapOnGroup* mMaterialPropertyGroup;
		tGrowableArray<tEditablePropertyTable> mMaterialCommonProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnMaterialPropertyChanged;

		tWxSlapOnGroup* mUserPropertyGroup;
		tEditablePropertyTable mUserProps;
		tEditablePropertyTable::tOnPropertyChanged::tObserver mOnUserPropertyChanged;

		Gfx::tDevicePtr	mDevice;
	public:
		tMatEdMainWindow( wxWindow* parent, tEditorActionStack& actionStack, const std::string& regKeyName, b32 enableShaderBrowsing, b32 enableLightProbeEditing, b32 enableMaterialEditing, u32 renderWidth = 280, u32 renderHeight = 180 );
		~tMatEdMainWindow( );
		
		void fSetupPreviewWindow( const Gfx::tDevicePtr& device = Gfx::tDevicePtr( ) ); // if null device specified, MatEd window creates own device
		
		b32 fHasSameDevice( const Gfx::tDevicePtr& device ) const;		
		const Gfx::tDevicePtr& fDevice( ) const { return mDevice; }
		Dx9Util::tTextureCachePtr fTextureCache( ) const;
		Gfx::tLightEntityPtr fDefaultLight( ) const;
		tMaterialPreviewPanel* fPreviewPanel( ) const { return mPreviewPanel; }
		
		tMaterialPreviewBundlePtr fPreviewBundle( ) const;
		void fSetPreviewBundle( const tMaterialPreviewBundlePtr& bundle );
		
		void fClear( );
		
		void fEnableShaderBrowser( b32 enable );
		b32 fSetDefaultBrowseDirectory( const tFilePathPtr& defaultBrowseDirectory = tFilePathPtr( ) );

		void fEnableMaterialEdit( b32 enable );

		void fFromDermlFile( const Derml::tFile& derml, const tFilePathPtr& shaderPath = tFilePathPtr( ) );
		void fFromDermlMtlFile( const Derml::tMtlFile& mtlFile );
		
		virtual b32 fOnTick( );
		virtual void fOnShaderSelected( const tFilePathPtr& shaderPath ) { }


		void fSetUserProperties( tEditablePropertyTable& table );

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
		void fOnMaterialPropertyChanged( tEditableProperty& property );

	protected:
		virtual void fOnUserPropertyChanged( tEditableProperty& property );
	};
}

#endif//__tMatEdMainWindow__
