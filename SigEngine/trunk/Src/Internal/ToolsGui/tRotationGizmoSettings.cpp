#include "ToolsGuiPch.hpp"
#include "tRotationGizmoSettings.hpp"
#include "tWxNumericText.hpp"
#include "tWxSlapOnSpinner.hpp"

// wx controls
#include "tWxSlapOnGroup.hpp"
namespace Sig
{
	tRotationGizmoSettings::tRotationGizmoSettings( 
		wxWindow* parent, const std::string& regKeyName )
		: tWxSlapOnDialog( "Rotation Gizmo Settings", parent )
		, mRegKeyName( regKeyName )
	{
		// enforce width constraints on window
		const int width = 350;
		const int height = 100;
		SetMinSize( wxSize( width, height ) );
		SetMaxSize( wxSize( width, height ) );
		SetSize( wxSize( width, height ) );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		SetIcon( wxIcon( "appicon" ) );

		GetSizer( )->AddSpacer( 5 );
		tWxSlapOnGroup* angleSettingGroup = new tWxSlapOnGroup( this, "Angle Snap Settings", false );
		mAngleSnap = new tWxSlapOnSpinner( angleSettingGroup->fGetMainPanel( ), "Angle Snap", 1.f, 360, 1.f, 0);
		angleSettingGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 10 );

		GetSizer( )->AddSpacer( 5 );

		SetWindowStyle( wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxTAB_TRAVERSAL | wxWANTS_CHARS | wxFRAME_NO_TASKBAR | wxFRAME_FLOAT_ON_PARENT );
		Fit( );
		fLoad( );
	}
	tRotationGizmoSettings::~tRotationGizmoSettings( )
	{
		fSave( );
	}

	void tRotationGizmoSettings::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, mAngleSnap->fGetValue( ), "AngleSnap" );
	}
	void tRotationGizmoSettings::fLoadInternal( HKEY hKey )
	{
		f32 angleSnap;
		Win32Util::fGetRegistryKeyValue( hKey, angleSnap, "AngleSnap" );
		mAngleSnap->fSetValue( angleSnap );
	}

}

