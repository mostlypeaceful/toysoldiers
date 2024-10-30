
sigimport "gui/scripts/controls/radialmenu.nut"
sigimport "gui/textures/radialmenus/use_unit_nouse_g.png"

sigexport function CanvasCreateRadialMenu( radialMenu )
{
	return VehicleOptionsRadialMenu( )
}

class VehicleOptionsRadialMenu extends RadialMenu
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

		local menu = this
		if( ::GameApp.GetDifficulty( ) == DIFFICULTY_GENERAL )
		{
			icons[ 0 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/use_unit_nouse_g.png", function( ) { return false } )
			icons[ 0 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "General_NoUse" )
			}
		}
		else
		{
			icons[ 0 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/use_unit_g.png", function( ):(unit, player)
			{
				return unit.TryToUse( player )
			} )
			icons[ 0 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Use_Unit" )
			}
		}

		FinalizeIconSetup( )
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
	
	function TryHotKeys( gamepad )
	{
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_UP ) )
			return icons[ 0 ].onSelectCallback( )

		return false
	}
}
