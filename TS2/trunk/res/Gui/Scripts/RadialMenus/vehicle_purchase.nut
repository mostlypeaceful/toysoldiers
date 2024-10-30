
sigimport "gui/scripts/controls/radialmenu.nut"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_tank01_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_tank02_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_heligunner_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_heligunner_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_fighter_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_fighter_g.png"
sigimport "Gui/Textures/WaveIcons/USSR/vehicle_apc05_g.png"
sigimport "Gui/Textures/WaveIcons/USA/vehicle_apc05_g.png"



sigimport "gui/textures/radialmenus/use_unit_nouse_g.png"
sigimport "gameplay/mobile/vehicle/usa/tank_medium_01.sigml"
sigimport "Gameplay/Mobile/airborne/USA/plane_fighter_01.sigml"
sigimport "Gameplay/Mobile/Vehicle/USA/apc_ifv_01.sigml"
sigimport "Gameplay/Mobile/Helicopter/USA/helo_attack_01.sigml"
sigimport "Gameplay/Mobile/vehicle/USA/tank_heavy_01.sigml"


sigexport function CanvasCreateRadialMenu( radialMenu )
{
	return VehiclePurchaseRadialMenu( )
}


class VehiclePurchaseIcon extends RadialMenuIcon
{
	unitIcon = null // Gui.TexturedQuad
	
	constructor( country, unitID, selectCb )
	{		
		local bgPath = "gui/textures/radialmenus/blank_g.png"
			
		RadialMenuIcon.constructor( bgPath, selectCb )
		SetActiveInactiveScale( Math.Vec2.Construct( 0.75, 0.75 ), Math.Vec2.Construct( 1.0, 1.0 ) )
		local imgPos = image.GetPosition( )
		image.SetPosition( Math.Vec3.Construct( imgPos.x, imgPos.y, 0.05 ) )
		
		unitIcon = ::UnitIcon( unitID, country )
		unitIcon.SetPosition( Math.Vec3.Construct( 0, 0, 0.01 ) )
		AddChild( unitIcon )
	}
	
	function OnHighlight( active )
	{
		RadialMenuIcon.OnHighlight( active )
	}
}

class VehiclePurchaseRadialMenu extends RadialMenu
{
	unit = null
	player = null	
	data = [ ]

	function DefaultSetup( params )
	{
		unit = params.Unit
		player = params.Player
		audioSource = player.AudioSource
		data = [ ]
		
		SetRadius( 66 )
		
		AddUnit( player, unit, UNIT_ID_TANK_MEDIUM_01, 1000 )
		AddUnit( player, unit, UNIT_ID_TANK_HEAVY_01, 3000 )
		AddUnit( player, unit, UNIT_ID_APC_IFV_01, 2000 )
		AddUnit( player, unit, UNIT_ID_HELO_ATTACK_01, 4000 )
		AddUnit( player, unit, UNIT_ID_PLANE_FIGHTER_01, 5000 )


		FinalizeIconSetup( )
	}
	
	function HighlightByAngle( angle, magnitude )
	{
		local retValue = ::RadialMenu.HighlightByAngle( angle, magnitude )
		if( highlightIndex < 0 ) return retValue
		
		if( retValue )
		{
			local outString = ::UnitLogic.UnitLocClass( player.Country, data[ highlightIndex ].ID ).Clone( ) % "\n"
								% ::GameApp.LocString( "Cost" ).Replace( "money", ::LocString.ConstructMoneyString( data[ highlightIndex ].Price.tostring( ) ) )

			if( outString )
				displayText.BakeBoxLocString( 400, outString, TEXT_ALIGN_CENTER )
			else
				displayText.BakeCString( "", TEXT_ALIGN_CENTER )
		}
			
		return retValue
	}
	
	function AddUnit( player, unit, unitID, price )
	{
		icons.push( ::VehiclePurchaseIcon( player.Country, unitID, function( ):(unit, unitID, price, player)
		{
			return unit.PurchaseVehicle( player, unitID, price )
		} ) )
		
		
		data.push( { ID = unitID, Price = price } )
	}
}
