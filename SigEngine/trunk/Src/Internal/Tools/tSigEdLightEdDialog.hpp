#ifndef __tSigEdLightEdDialog__
#define __tSigEdLightEdDialog__
#include "tMatEdMainWindow.hpp"
#include "Editor/tEditableLightProbeEntity.hpp"

/*
	SigEd specific dialog for previewing lighting parameters.
		By default it should have a very neutral white reference sphere.
		The user can override the preview material though.

	Most of the functionality should be in the base,
	 the intention of this class is merely to extend it to connect to the SigEd application.
*/

namespace Sig
{

	class tools_export tSigEdLightEdDialog : public tMatEdMainWindow
	{
		b32 mDefaultTextureBrowserPathSet;
		tMaterialPreviewBundlePtr mPreviewBundle;

		tEditablePropertyPtr mEditingProperty;
		tEditableLightProbeData mEditingData;
		tEntityPtr mEditingEnt;

		Dx9Util::tBaseTexturePtr mCurrentCubeMap;

	public:
		static tSigEdLightEdDialog* gDialog;

		tSigEdLightEdDialog( wxWindow* parent, Gfx::tDevicePtr device, tEditorActionStack& actionStack, const std::string& regKeyName );
		~tSigEdLightEdDialog( );

		virtual b32 fOnTick( );

		virtual void fOnShaderSelected( const tFilePathPtr& shaderPath );
		
		void fSetDefaultShader( );

		// This dialog is used by editable properties.
		static void fAddProperty( tEditableProperty& prop, const tEditorSelectionList* selection );
		static void fRefreshProperty( tEditableProperty& prop );
		static void fRemoveProperty( tEditableProperty& prop );

	private:
		void fSave( );
		void fLoad( );
		void fRender( );
		void fRefreshCubemapFile( );
		void fRefreshHarmonics( );

		virtual void fOnUserPropertyChanged( tEditableProperty& property );
	};

}


#endif//__tSigEdLightEdDialog__

