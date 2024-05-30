#include "SigFxPch.hpp"
#include "tParticleSystemPropertiesPanel.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "wx/tooltip.h"
#include "tWxToolsPanelSlider.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{
	using namespace FX;

	class tRandomStartTimeCheckbox : public tWxSlapOnCheckBox
	{
		tSigFxMainWindow* mSigFxMainWindow;
	public:
		tRandomStartTimeCheckbox( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: tWxSlapOnCheckBox( parent, label )
			, mSigFxMainWindow( fxWindow )
		{ }

		virtual void fOnControlUpdated( )
		{
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}
	};

	tFxmlPropertiesPanel::tFxmlPropertiesPanel( tWxToolsPanel* parent, tSigFxMainWindow* window )
		: tWxToolsPanelTool( parent, "Fxml Properties", "Fxml Properties", "FxmlProps" )
		, mSigFxMainWindow( window )
	{				
		wxBoxSizer* container = new wxBoxSizer( wxVERTICAL );

		wxPanel* group = new wxPanel( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );

		// Add property controls
		mRandomStartTimeCheckbox = new tRandomStartTimeCheckbox( mSigFxMainWindow, group, "Random Start Time" );

		container->Add( group, 0, wxALIGN_LEFT , 4 );

		container->AddSpacer( 8 );
		fGetMainPanel( )->GetSizer( )->Add( container, 0, wxALIGN_CENTER, 0 );
	}

	void tFxmlPropertiesPanel::fUpdateSelectedList( tEditorSelectionList& list )
	{
	}

	void tFxmlPropertiesPanel::fSetRandomStartTime( b32 set )
	{
		mRandomStartTimeCheckbox->fSetValue( set ); 
	}

	b32 tFxmlPropertiesPanel::fRandomStartTime( ) const
	{
		return mRandomStartTimeCheckbox->fGetValueBool(); 
	}
}


