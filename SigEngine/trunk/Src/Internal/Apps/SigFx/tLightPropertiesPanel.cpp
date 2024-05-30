#include "SigFxPch.hpp"
#include "tLightPropertiesPanel.hpp"
#include "tToolsGuiMainWindow.hpp"
#include "Editor/tEditorSelectionList.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tSigFxMainWindow.hpp"

namespace Sig
{
	using namespace FX;

	class tCastsShadowsCheckBox : public tWxSlapOnCheckBox
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxLight* >* mList;
	public:
		tCastsShadowsCheckBox( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnCheckBox( parent, label )
			, mList( 0 )	{	}

		virtual void fOnControlUpdated( )
		{
			if( mList)
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fSetCastsShadows( fGetValueBool( ) );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxLight* >* list ) { mList = list; }
	};

	class tShadowIntensitySpinner : public tWxSlapOnSpinner
	{
		tSigFxMainWindow* mSigFxMainWindow;
		tGrowableArray< tSigFxLight* >* mList;
	public:
		tShadowIntensitySpinner( tSigFxMainWindow* fxWindow, wxWindow* parent, const char* label )
			: mSigFxMainWindow( fxWindow )
			, tWxSlapOnSpinner( parent, label, 0.f, 1.f, 0.01f, 2 )
			, mList( 0 )	{	}

		virtual void fOnControlUpdated( )
		{
			if( mList)
			{
				for( u32 i = 0; i < mList->fCount( ); ++i )
				{
					( *mList )[ i ]->fSetShadowIntensity( fGetValue( ) );
				}
			}
			mSigFxMainWindow->fGuiApp( ).fActionStack( ).fForceSetDirty( true );
		}

		void fSetList( tGrowableArray< tSigFxLight* >* list ) { mList = list; }
	};


	tLightPropertiesPanel::tLightPropertiesPanel( tSigFxMainWindow* fxWindow, tWxToolsPanel* parent )
		: mSigFxMainWindow( fxWindow )
		, tWxToolsPanelTool( parent, "Light Properties", "Light Properties", "LightProps" )
	{
		wxBoxSizer* container = new wxBoxSizer( wxVERTICAL );
		fGetMainPanel( )->GetSizer( )->Add( container, 1, wxALIGN_CENTER, 1 );

		wxPanel* group = new wxPanel( fGetMainPanel( ), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0 );
		mCastsShadowsCheckBox = new tCastsShadowsCheckBox( mSigFxMainWindow, group, "Casts Shadows" );
		mShadowIntensitySpinner = new tShadowIntensitySpinner( mSigFxMainWindow, group, "Shadow Intensity" );

		container->Add( group, 1, wxALIGN_LEFT | wxALL, 4 );
		container->AddSpacer( 8 );
	}


	void tLightPropertiesPanel::fUpdateSelectedList( tEditorSelectionList& list )
	{
		mLightList.fSetCount( 0 );
		list.fCullByType< tSigFxLight >( mLightList );

		if( !mLightList.fCount( ) )
			fSetCollapsed( true );
		else
			fSetCollapsed( false );

		mCastsShadowsCheckBox->fSetList( &mLightList );
		mShadowIntensitySpinner->fSetList( &mLightList );

		if( mLightList.fCount( ) )
		{
			mCastsShadowsCheckBox->fSetValue( mLightList[ 0 ]->fCastsShadows( ) );
			mShadowIntensitySpinner->fSetValueNoEvent( mLightList[ 0 ]->fShadowIntensity( ) );
		}
	}


}


