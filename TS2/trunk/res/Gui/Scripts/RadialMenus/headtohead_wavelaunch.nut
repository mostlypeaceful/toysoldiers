sigimport "gui/scripts/controls/radialmenu.nut"
sigimport "gui/textures/radialmenus/blank_g.png"
sigimport "gui/scripts/hud/wavelist_sp.nut"

sigexport function CanvasCreateRadialMenu( radialMenu ) 
{
	return WaveLaunchRadialMenu( )
}

class WaveLaunchIcon extends RadialMenuIcon
{
	unitIcon = null // Gui.TexturedQuad
	waveDesc = null // OffensiveWaveDesc
	count = null
	
	constructor( waveDesc_, unitID, count_, selectCb )
	{
		waveDesc = waveDesc_;
		count = count_
		
		local bgPath = "gui/textures/radialmenus/blank_g.png"
		if( waveDesc.Country == COUNTRY_USSR )
			bgPath = "gui/textures/radialmenus/blank_red_g.png"
			
		::RadialMenuIcon.constructor( bgPath, selectCb )
		SetActiveInactiveScale( ::Math.Vec2.Construct( 0.75, 0.75 ), ::Math.Vec2.Construct( 1.0, 1.0 ) )
		image.SetZPos( 0.05 )
		
		unitIcon = ::UnitIcon( unitID, waveDesc.Country )
		local size = unitIcon.WorldRect
		unitIcon.SetPosition( 0, 0, 0 )
		AddChild( unitIcon )
	}
	
	function OnHighlight( active )
	{
		::RadialMenuIcon.OnHighlight( active )
	}
}

class WaveLaunchRadialMenu extends RadialMenu
{
	player = null
	
	constructor( )
	{
		::RadialMenu.constructor( 80 )
		PlaySound( "Play_UI_Select_Forward" )
	}
	
	function DefaultSetup( player_ )
	{
		player = player_
		audioSource = player_.AudioSource

		displayText.BakeCString( "No Offensive Waves", TEXT_ALIGN_CENTER )
		errorSound = null
	}
	
	function HighlightByAngle( angle, magnitude )
	{
		local retValue = ::RadialMenu.HighlightByAngle( angle, magnitude )
		if( highlightIndex < 0 ) return
		
		if( retValue )
		{
			local waveDesc = icons[ highlightIndex ].waveDesc
			local count = icons[ highlightIndex ].count

			local outString = ::GameApp.LocString( "WaveLaunch_UnitNameFormat" ).Replace( "name", waveDesc.Name ).Replace( "count", count )
			outString = outString % "\n" % ::GameApp.LocString( "Cost" ).Replace( "money", ::LocString.ConstructMoneyString( waveDesc.Cost.tostring( ) ) )
			outString = outString % "\n\n" % ::GameApp.LocString( waveDesc.Desc )
			
			displayText.BakeBoxLocString( 400, outString, TEXT_ALIGN_CENTER )
		}
		
		return retValue
	}
	
	function AddLaunchableWave( waveDesc, unitID, count )
	{
		local menu = this
		icons.push( ::WaveLaunchIcon( waveDesc, unitID, count, function():(menu, waveDesc, player)
		{
			if( ::GameApp.CurrentLevel.LaunchOffensiveWave( waveDesc, player ) )
				PlaySound( "Play_Turret_Sell" )
			else
				PlaySound( "Play_HUD_WeaponMenu_Error" )
			return false
		} ) )
	}
}