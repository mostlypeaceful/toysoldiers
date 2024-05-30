#ifndef __tExporterToolbox__
#define __tExporterToolbox__
#include "tWxSlapOnDialog.hpp"
#include "tWxSavedLayout.hpp"
#include "tStrongPtr.hpp"

namespace Sig
{
	class tExporterGuiFactory;
	class tWxSlapOnPanel;

	class tExporterToolboxLayout : public tWxSavedLayout
	{
		virtual std::string fRegistryKeyName( ) const
		{
			return ToolsPaths::fGetSignalRegistryKeyName( ) + "\\MayaExporterDialog";
		}
	};

	class tools_export tExporterToolbox : public tWxSlapOnDialog
	{
		tExporterToolboxLayout mLayout;

	public:

		static const char* fNameIgnoreObject( ) { return "IgnoreObject"; }
		static const char* fNameGeomType( ) { return "GeomType"; }
		static const char* fNameGeomTypeNone( ) { return "None/Ignore"; }
		static const char* fNameGeomTypeRenderMesh( ) { return "RenderMesh"; }

		static const char* fNameStateType( ) { return "StateType"; }
		static const char* fNameStateState( ) { return "State"; }
		static const char* fNameStateTransition( ) { return "Transition"; }
		static const char* fNameStateIndex( ) { return "StateIndex"; }
		static const char* fNameStateMask( ) { return "StateMask"; }

		static const char* fNameGameTags( ) { return "GameTags"; }

		static const char* fNameLodSlot( ) { return "LodSlot"; }
		static const char* fNameReferenceFrame( ) { return "ReferenceFrame"; }
		static const char* fNameRootNodeName( ) { return "RootNodeName"; }
		static const char* fNameExcludeBone( ) { return "ExcludeBone"; }
		static const char* fNameAdditiveBone( ) { return "Additive"; }
		static const char* fNameAnimationSources( ) { return "AnimationSources"; }
		static void		   fNameAnimationSources( u32 i, std::string & out );
		static const char* fNameIKBonePriority( ) { return "IKPriority"; }
		static const char* fNameLightType( ) { return "LightType"; }
		static const char* fNameLightTypeNone( ) { return "None/Ignore"; }
		static const char* fNameLightTypeLocal( ) { return "Local"; }
		static const char* fNameLightTypeWorld( ) { return "World"; }
		static const char* fNameTwoSided( ) { return "TwoSided"; }
		static const char* fNameFlipBackFaceNormal( ) { return "FlipBackFaceNormal"; }
		static const char* fNameTransparency( ) { return "Transparency"; }
		static const char* fNameAlphaCutOut( ) { return "AlphaCutOut"; }
		static const char* fNameAdditive( ) { return "Additive"; }
		static const char* fNameZBufferTest( ) { return "ZBufferTest"; }
		static const char* fNameZBufferTestDefault( ) { return "Default"; }
		static const char* fNameZBufferTestForceOn( ) { return "ForceOn"; }
		static const char* fNameZBufferTestForceOff( ) { return "ForceOff"; }
		static const char* fNameZBufferWrite( ) { return "ZBufferWrite"; }
		static const char* fNameZBufferWriteDefault( ) { return "Default"; }
		static const char* fNameZBufferWriteForceOn( ) { return "ForceOn"; }
		static const char* fNameZBufferWriteForceOff( ) { return "ForceOff"; }
		static const char* fNameFaceX( ) { return "FaceX"; }
		static const char* fNameFaceY( ) { return "FaceY"; }
		static const char* fNameFaceZ( ) { return "FaceZ"; }
		static const char* fNameSortOffset( ) { return "SortOffset"; }

		tExporterToolbox( const char* windowTitle, const tStrongPtr<tExporterGuiFactory>& factory );

		void fSaveLayout( );
		void fLoadLayout( );

	private:
		void fSetupMainTab( tExporterGuiFactory* factory, tWxSlapOnPanel* tab );
		void fSetupGeometryTab( tExporterGuiFactory* factory, tWxSlapOnPanel* tab );
		void fSetupAnimationTab( tExporterGuiFactory* factory, tWxSlapOnPanel* tab );
		void fSetupMaterialTab( tExporterGuiFactory* factory, tWxSlapOnPanel* tab );
		void fOnClose( wxCloseEvent& event );
		DECLARE_EVENT_TABLE()
	};

}

#endif//__tExporterToolbox__
