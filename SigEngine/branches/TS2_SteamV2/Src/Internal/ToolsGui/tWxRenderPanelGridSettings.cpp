#include "ToolsGuiPch.hpp"
#include "tWxRenderPanelGridSettings.hpp"

// wx controls
#include "tWxSlapOnGroup.hpp"
#include "tWxSlapOnCheckBox.hpp"
#include "tWxSlapOnButton.hpp"
#include "tWxSlapOnSpinner.hpp"

namespace Sig
{

	class tWxRenderPanelGridSettingsUpdate : public tWxSlapOnButton
	{
		tWxRenderPanelGridSettings* mParent;
	public:
		tWxRenderPanelGridSettingsUpdate( tWxRenderPanelGridSettings* parent )
			: tWxSlapOnButton( parent, "Update Grid" ), mParent( parent ) { }
		virtual void fOnControlUpdated( ) { mParent->fUpdateGrid( ); }
	};

	tWxRenderPanelGridSettings::tWxRenderPanelGridSettings( 
		wxWindow* parent, const std::string& regKeyName, Gfx::tSolidColorGrid& grid, b32& showGrid, b32& snapToGrid )
		: tWxSlapOnDialog( "Grid Settings", parent )
		, mRegKeyName( regKeyName )
		, mGrid( grid )
		, mGridLineCount( 0 )
		, mMinorGridLines( 0 )
		, mMajorGridLines( 0 )
		, mCenterX( 0 )
		, mCenterY( 0 )
		, mCenterZ( 0 )
		, mGridXAxis( 1.f, 0.f, 0.f )
		, mGridZAxis( 0.f, 0.f, 1.f )
	{
		// enforce width constraints on window
		const int width = 350;
		const int height = 350;
		SetMinSize( wxSize( width, height ) );
		SetMaxSize( wxSize( width, height ) );
		SetSize( wxSize( width, height ) );

		SetSizer( new wxBoxSizer( wxVERTICAL ) );

		SetIcon( wxIcon( "appicon" ) );

		GetSizer( )->AddSpacer( 5 );
		mDisplayGrid = new tWxSlapOnCheckBoxDataSync( this, "Display Grid", reinterpret_cast<s32&>( showGrid ) );
		mSnapToGrid = new tWxSlapOnCheckBoxDataSync( this, "Snap To Grid", reinterpret_cast<s32&>( snapToGrid ) );

		tWxSlapOnGroup* linesGroup = new tWxSlapOnGroup( this, "Lines", false );
		mGridLineCount  = new tWxSlapOnSpinner( linesGroup->fGetMainPanel( ), "Grid Line Count", 1.f, 400.f, 1.f, 0 );
		mMinorGridLines = new tWxSlapOnSpinner( linesGroup->fGetMainPanel( ), "Minor Line Spacing", 1.f, 100.f, 1.f, 0 );
		mMajorGridLines = new tWxSlapOnSpinner( linesGroup->fGetMainPanel( ), "Major Line Spacing", 1.f, 100.f, 1.f, 0 );
		linesGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 10 );

		tWxSlapOnGroup* positionGroup = new tWxSlapOnGroup( this, "Position", false );
		mCenterX = new tWxSlapOnSpinner( positionGroup->fGetMainPanel( ), "Center X", -10000.f, +10000.f, 1.0f, 0 );
		mCenterY = new tWxSlapOnSpinner( positionGroup->fGetMainPanel( ), "Center Y", -10000.f, +10000.f, 1.0f, 0 );
		mCenterZ = new tWxSlapOnSpinner( positionGroup->fGetMainPanel( ), "Center Z", -10000.f, +10000.f, 1.0f, 0 );
		positionGroup->fGetMainPanel( )->GetSizer( )->AddSpacer( 10 );

		mGridLineCount->fSetValue( 50.f );
		mMinorGridLines->fSetValue( 1.f );
		mMajorGridLines->fSetValue( 10.f );
		mCenterX->fSetValue( 0.f );
		mCenterY->fSetValue( 0.f );
		mCenterZ->fSetValue( 0.f );

		new tWxRenderPanelGridSettingsUpdate( this );
		GetSizer( )->AddSpacer( 5 );

		SetWindowStyle( wxDEFAULT_DIALOG_STYLE | wxMINIMIZE_BOX | wxSTAY_ON_TOP | wxTAB_TRAVERSAL | wxWANTS_CHARS );
		Fit( );
		fLoad( );
	}
	tWxRenderPanelGridSettings::~tWxRenderPanelGridSettings( )
	{
		fSave( );
	}
	void tWxRenderPanelGridSettings::fUpdateGrid( )
	{
		mGrid.fGenerate( ( u32 )mGridLineCount->fGetValue( ), ( u32 )mMinorGridLines->fGetValue( ), ( u32 )mMajorGridLines->fGetValue( ), mGridXAxis, mGridZAxis );
		mGrid.fMoveTo( Math::tVec3f( mCenterX->fGetValue( ), mCenterY->fGetValue( ), mCenterZ->fGetValue( ) ) );
	}
	void tWxRenderPanelGridSettings::fSnapVertex( Math::tVec3f& vert )
	{
		vert -= mGrid.fObjectToWorld( ).fGetTranslation( );

		const f32 gridLineWidth = mMinorGridLines->fGetValue( );
		vert.x = fRound<f32>( vert.x / gridLineWidth ) * gridLineWidth;
		vert.y = fRound<f32>( vert.y / gridLineWidth ) * gridLineWidth;
		vert.z = fRound<f32>( vert.z / gridLineWidth ) * gridLineWidth;

		vert += mGrid.fObjectToWorld( ).fGetTranslation( );
	}
	void tWxRenderPanelGridSettings::fSaveInternal( HKEY hKey )
	{
		Win32Util::fSetRegistryKeyValue( hKey, ( b32 )mDisplayGrid->fGetValue( ), "DisplayGrid" );
		Win32Util::fSetRegistryKeyValue( hKey, ( b32 )mSnapToGrid->fGetValue( ), "SnapToGrid" );
		Win32Util::fSetRegistryKeyValue( hKey, ( u32 )mGridLineCount->fGetValue( ), "GridLineCount" );
		Win32Util::fSetRegistryKeyValue( hKey, ( u32 )mMinorGridLines->fGetValue( ), "MinorSpacing" );
		Win32Util::fSetRegistryKeyValue( hKey, ( u32 )mMajorGridLines->fGetValue( ), "MajorSpacing" );
		Win32Util::fSetRegistryKeyValue( hKey, mCenterX->fGetValue( ), "CenterX" );
		Win32Util::fSetRegistryKeyValue( hKey, mCenterY->fGetValue( ), "CenterY" );
		Win32Util::fSetRegistryKeyValue( hKey, mCenterZ->fGetValue( ), "CenterZ" );
	}
	void tWxRenderPanelGridSettings::fLoadInternal( HKEY hKey )
	{
		b32 displayGrid = true, snapToGrid = false;
		u32 count = 50, minor = 1, major = 10;
		Win32Util::fGetRegistryKeyValue( hKey, displayGrid, "DisplayGrid" );
		Win32Util::fGetRegistryKeyValue( hKey, snapToGrid, "SnapToGrid" );
		Win32Util::fGetRegistryKeyValue( hKey, count, "GridLineCount" );
		Win32Util::fGetRegistryKeyValue( hKey, minor, "MinorSpacing" );
		Win32Util::fGetRegistryKeyValue( hKey, major, "MajorSpacing" );
		f32 x = 0.f, y = 0.f, z = 0.f;
		Win32Util::fGetRegistryKeyValue( hKey, x, "CenterX" );
		Win32Util::fGetRegistryKeyValue( hKey, y, "CenterY" );
		Win32Util::fGetRegistryKeyValue( hKey, z, "CenterZ" );

		mDisplayGrid->fSetValue( displayGrid );
		mSnapToGrid->fSetValue( snapToGrid );
		mGridLineCount->fSetValue( ( f32 )count );
		mMinorGridLines->fSetValue( ( f32 )minor );
		mMajorGridLines->fSetValue( ( f32 )major );
		mCenterX->fSetValue( x );
		mCenterY->fSetValue( y );
		mCenterZ->fSetValue( z );
	}

}

