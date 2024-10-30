// Versus Options Menu Panel

// Requires

class VersusOptionsPanel extends VerticalMenu
{
	// Display
	highlight = null
	teamSelect = null
	vehicleSelect = null
	barrageSelect = null
	turretAiSelect = null
	toyboxHpSelect = null
	cashSelect = null
	
	// Data
	active = null
	selectedIndex = null
	hpValues = null
	cashValues = null
	
	constructor( audioSource_ )
	{
		::VerticalMenu.constructor( )
		audioSource = audioSource_
		active = false
		selectedIndex = 0
		
		local asianLanguage = ::GameApp.IsAsianLanguage( )
		local width = 323
		
		local label = ::Gui.Text( )
		label.SetFontById( FONT_FANCY_MED )
		label.SetUniformScale( 0.7 )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( ::GameApp.LocString( "CustomMatch_Options" ), TEXT_ALIGN_CENTER )
		label.SetPosition( width * 0.5, 1, 0 )
		if( asianLanguage )
			label.SetYPos( 2 )
		else
			label.SetUniformScale( 0.7 )
		AddChild( label )
		
		highlight = ::PanelHighlight( ::Math.Vec2.Construct( width, 347 ), 3 )
		highlight.SetPosition( Math.Vec3.Construct( 0, 0, -0.02 ) )
		AddChild( highlight )
		
		local isSpanish = ( ::GameApp.Language == LANGUAGE_SPANISH )
		
		local itemSpacing = ( isSpanish ? 223 : ( asianLanguage ? 200 : 213 ) )
		local arrowWidth = ( asianLanguage ? 160 : 120 )
		
		// Items
		teamSelect = ::SettingItem_Choice( "CustomMatch_Team", [ "CustomMatch_USA", "CustomMatch_USSR" ], 0, 0, null, itemSpacing, ( isSpanish ? 145 : arrowWidth ), audioSource )
		teamSelect.SetLooping( true )
		teamSelect.SetMultiline( asianLanguage )
		icons.push( teamSelect )
		AddChild( teamSelect )
		
		vehicleSelect = ::SettingItem_Choice( "CustomMatch_Vehicles", [ "CustomMatch_Yes", "CustomMatch_No" ], 0, 0, null, itemSpacing, arrowWidth, audioSource )
		vehicleSelect.SetLooping( true )
		vehicleSelect.SetMultiline( asianLanguage )
		icons.push( vehicleSelect )
		AddChild( vehicleSelect )
		
		barrageSelect = ::SettingItem_Choice( "CustomMatch_Barrages", [ "CustomMatch_Yes", "CustomMatch_No" ], 0, 0, null, itemSpacing, arrowWidth, audioSource )
		barrageSelect.SetLooping( true )
		barrageSelect.SetMultiline( asianLanguage )
		icons.push( barrageSelect )
		AddChild( barrageSelect )
		
		turretAiSelect = ::SettingItem_Choice( "CustomMatch_TurretAI", [ "CustomMatch_On", "CustomMatch_Off" ], 0, 0, null, itemSpacing, arrowWidth, audioSource )
		turretAiSelect.SetLooping( true )
		turretAiSelect.SetMultiline( asianLanguage )
		icons.push( turretAiSelect )
		AddChild( turretAiSelect )
		
		hpValues = [ 10, 20, 30, 50, 100 ]
		toyboxHpSelect = ::SettingItem_Choice( "CustomMatch_ToyboxHP", hpValues, 1, 0, null, itemSpacing, arrowWidth, audioSource )
		toyboxHpSelect.SetLooping( true )
		toyboxHpSelect.SetMultiline( asianLanguage )
		icons.push( toyboxHpSelect )
		AddChild( toyboxHpSelect )
		
		cashValues = [
			1000,
			2000,
			4000,
			6000,
			10000
		]
		
		local cashOptions = array( cashValues.len( ) )
		for( local i = 0; i < cashValues.len( ); ++i )
			cashOptions[ i ] = ::LocString.ConstructMoneyString( cashValues[ i ].tostring( ) )
		cashSelect = ::SettingItem_Choice( "CustomMatch_Cash", cashOptions, 1, 0, null, itemSpacing, arrowWidth, audioSource )
		cashSelect.SetLooping( true )
		cashSelect.SetMultiline( asianLanguage )
		icons.push( cashSelect )
		AddChild( cashSelect )
		
		// Position Items
		menuPositionOffset = ::Math.Vec3.Construct( ( isSpanish ? 10 : 20 ), 40, 0 )
		::VerticalMenu.FinalizeIconSetup( )
		SetAlpha( 1.0 )
		SetActive( false )
	}
	
	function SelectedIndex( )
	{
		return highlightIndex
	}
	
	function Count( )
	{
		return icons.len( )
	}

	function SetActive( active_ )
	{
		active = active_
		highlight.SetActive( active )
		icons[ highlightIndex ].OnHighlight( active )
	}
	
	function AutoLaunch( )
	{
		return false
	}
	
	function SetSelection( index )
	{
		HighlightByIndex( index )
	}
	
	function SetLevelInfo( scores, info, player )
	{
	}
	
	function SetExtra( index, extra )
	{
		icons[ index ].SetIndex( extra )
	}
	
	function GetExtra( index = null )
	{
		if( index == null )
			index = highlightIndex
		return icons[ index ].CurrentIndex( )
	}
	
	function FillLoadInfo( info )
	{
		info.Country = teamSelect.CurrentIndex( ) == 0 ? COUNTRY_USA : COUNTRY_USSR
		info.Tickets = hpValues[ toyboxHpSelect.CurrentIndex( ) ]
		info.Money = cashValues[ cashSelect.CurrentIndex( ) ]
		info.ChallengeVehicles = 1 - vehicleSelect.CurrentIndex( ) //this will be 1 if currentIndex is zero, or 0 otherwise
		info.ChallengeBarrages = 1 - barrageSelect.CurrentIndex( )
		info.ChallengeTurretAI = 1 - turretAiSelect.CurrentIndex( )
	}
	
	function GetDescriptor( )
	{
		local desc = [
			"CustomMatch_Team_Description",
			"CustomMatch_Vehicles_Description",
			"CustomMatch_Barrages_Description",
			"CustomMatch_TurretAI_Description",
			"CustomMatch_ToyboxHP_Description",
			"CustomMatch_Cash_Description",
		]
		return desc[ SelectedIndex( ) ]
	}
	
	function ModeLocked( mode = null )
	{
		return false
	}
}