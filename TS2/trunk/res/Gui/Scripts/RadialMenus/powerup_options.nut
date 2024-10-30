
sigimport "gui/scripts/controls/radialmenu.nut"

sigexport function CanvasCreateRadialMenu( radialMenu )
{
	return PowerupOptionsRadialMenu( )
}

class PowerupOptionsIcon extends RadialMenuIcon
{
	constructor( imagePath, selectCb )
	{
		RadialMenuIcon.constructor( imagePath, selectCb )
		SetActiveInactiveScale( Math.Vec2.Construct( 0.5, 0.5 ), Math.Vec2.Construct( 1.0, 1.0 ) )
	}
	function OnHighlight( active )
	{
		RadialMenuIcon.OnHighlight( active )
	}
	function OnSelect( )
	{
		if( RadialMenuIcon.OnSelect( ) )
		{
			return true
		}
		return false
	}
}

class PowerupOptionsRadialMenu extends RadialMenu
{
	unit = null
	player = null

	function DefaultSetup( params )
	{
		unit = params.Unit
		player = params.Player
		audioSource = player.AudioSource
		
		SetRadius( 66 )
		SetIconCount( 1 )

		icons[ 0 ] = TurretOptionsIcon( "gui/textures/radialmenus/use_unit_g.png", UsePowerupCallback.bindenv(this) )
		icons[ 0 ].displayStringCallback = function( unit, player )
		{
			return ::GameApp.LocString( "Use_Unit" )
		}

		FinalizeIconSetup( )
	}

	function TryHotKeys( gamepad )
	{
		local iconIndex = -1
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_UP ) )
			iconIndex = 0

		if( iconIndex >= 0 )
			return icons[ iconIndex ].onSelectCallback( )

		return false
	}

	function UsePowerupCallback( )
	{
		//OnSelectionMade( )
		return unit.TryToUse( player )
	}
	
	function HighlightByAngle( angle, magnitude )
	{
		local retValue = ::RadialMenu.HighlightByAngle( angle, magnitude )
		if( highlightIndex < 0 ) return retValue
		
		if( retValue )
		{
			local outString = icons[ highlightIndex ].DisplayString( unit, player )

			if( outString )
				displayText.BakeBoxLocString( 400, outString, TEXT_ALIGN_CENTER )
			else
				displayText.BakeCString( "", TEXT_ALIGN_CENTER )
		}
			
		return retValue
	}
}
