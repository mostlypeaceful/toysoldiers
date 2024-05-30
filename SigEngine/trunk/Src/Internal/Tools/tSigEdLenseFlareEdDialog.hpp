#ifndef __tSigEdLenseFlareEdDialog__
#define __tSigEdLenseFlareEdDialog__
#include "tMatEdMainWindow.hpp"
#include "Editor/tEditableLightProbeEntity.hpp"

/*
	SigEd specific dialog for previewing lense flare parameters

	Most of the functionality should be in the base,
	 the intention of this class is merely to extend it to connect to the SigEd application.
*/

namespace Sig
{
	class tLenseFlareEditPanel;

	class tools_export tSigEdLenseFlareEdDialog : public tMatEdMainWindow
	{
		b32 mDefaultTextureBrowserPathSet;
		tMaterialPreviewBundlePtr mPreviewBundle;

		tEditablePropertyPtr mEditingProperty;
		tEditableLightProbeData mEditingData;
		tEntityPtr mEditingEnt;

		tLenseFlareEditPanel* mLenseFlarePanel;

	public:
		static tSigEdLenseFlareEdDialog* gDialog;

		tSigEdLenseFlareEdDialog( wxWindow* parent, Gfx::tDevicePtr device, tEditorActionStack& actionStack, const std::string& regKeyName );
		~tSigEdLenseFlareEdDialog( );

		virtual b32 fOnTick( );

		static void fShow( b32 show );

		//// This dialog is used by editable properties.
		//static void fAddProperty( tEditableProperty& prop, const tEditorSelectionList* selection );
		//static void fRefreshProperty( tEditableProperty& prop );
		//static void fRemoveProperty( tEditableProperty& prop );

	private:
		virtual void fOnPropertyChanged( tEditableProperty& property );

		void fSetDefaultShader( );
		virtual void fOnShaderSelected( const tFilePathPtr& shaderPath );

		virtual void fOnClose( wxCloseEvent& event );
	};

}


#endif//__tSigEdLenseFlareEdDialog__

