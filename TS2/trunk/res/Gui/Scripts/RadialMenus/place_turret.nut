
sigimport "gui/scripts/controls/radialmenu.nut"
sigimport "gui/textures/radialmenus/locked_unit_g.png"


sigexport function CanvasCreateRadialMenu( radialMenu )
{
	return PlaceTurretRadialMenu( )
}

class PlaceTurretIcon extends RadialMenuIcon
{
	constructor( imagePath, selectCb )
	{
		RadialMenuIcon.constructor( imagePath, selectCb )
		SetActiveInactiveScale( Math.Vec2.Construct( 0.5, 0.5 ), Math.Vec2.Construct( 1.0, 1.0 ) )
	}
	function OnHighlight( active )
	{
		::RadialMenuIcon.OnHighlight( active )
	}
	function OnSelect( )
	{
		return ::BaseMenuIcon.OnSelect( )
	}
}

class PlaceTurretRadialMenu extends RadialMenu
{
	rtsCursor = null
	function CanPlace( unitId, shortName )
	{
		local player = rtsCursor.Player
		local onlyPlace = GameApp.CurrentLevel.TutOnlyPlaceThisUnit
		local result = (onlyPlace == -1 || onlyPlace == unitId) && !player.UnitLocked( shortName )
		audioSource = player.AudioSource
		
		if( GameApp.CurrentLevel.TutOnlyPlaceSmallUnits )
		{
			if( unitId == UNIT_ID_TURRET_ARTY_01 || unitId == UNIT_ID_TURRET_AA_01 || unitId == UNIT_ID_TURRET_AA_01 )
			return false
		}
		
		if( result && onlyPlace != -1 )
			restrictSelection = icons.len( )
			
		return result
	}
	
	function DefaultSetup( _rtsCursor )
	{
		rtsCursor = _rtsCursor
		radius = 75 //66
		

		// Set up icons. If the unit is not available, use a lock icon
		// TODO: Possibly use a unit-specific lock icon
		if( CanPlace( UNIT_ID_TURRET_MG_01, "TURRET_MG_01" ) )		icons.push( PlaceTurretIcon( "gui/textures/radialmenus/turrets_mg_g.png", CreateGhostMachineGun.bindenv(this) ) )
		else 														icons.push( PlaceTurretIcon( "gui/textures/radialmenus/locked_unit_g.png", NoGhostTurret.bindenv( this ) ) )
		
		if( CanPlace( UNIT_ID_TURRET_MORTAR_01, "TURRET_MORTAR_01" ) )	icons.push( PlaceTurretIcon( "gui/textures/radialmenus/turrets_mortar_g.png", CreateGhostMortar.bindenv(this) ) )
		else 															icons.push( PlaceTurretIcon( "gui/textures/radialmenus/locked_unit_g.png", NoGhostTurret.bindenv( this ) ) )
		
		if( CanPlace( UNIT_ID_TURRET_AT_01, "TURRET_AT_01" ) )		icons.push( PlaceTurretIcon( "gui/textures/radialmenus/turrets_at_g.png", CreateGhostAntiTank.bindenv(this) ) )
		else 														icons.push( PlaceTurretIcon( "gui/textures/radialmenus/locked_unit_g.png", NoGhostTurret.bindenv( this ) ) )
		
		if( CanPlace( UNIT_ID_TURRET_ARTY_01, "TURRET_ARTY_01" ) )		icons.push( PlaceTurretIcon( "gui/textures/radialmenus/turrets_howitzer_g.png", CreateGhostHowitzer.bindenv(this) ) )
		else 															icons.push( PlaceTurretIcon( "gui/textures/radialmenus/locked_unit_g.png", NoGhostTurret.bindenv( this ) ) )
		
		if( CanPlace( UNIT_ID_TURRET_AA_01, "TURRET_AA_01" )  )		icons.push( PlaceTurretIcon( "gui/textures/radialmenus/turrets_aa_g.png", CreateGhostAntiAir.bindenv(this) ) )
		else 														icons.push( PlaceTurretIcon( "gui/textures/radialmenus/locked_unit_g.png", NoGhostTurret.bindenv( this ) ) )

		if( CanPlace( UNIT_ID_TURRET_FLAME_01, "TURRET_FLAME_01" )  )		icons.push( PlaceTurretIcon( "gui/textures/radialmenus/flame_g.png", CreateGhostFlame.bindenv(this) ) )
		else 																icons.push( PlaceTurretIcon( "gui/textures/radialmenus/locked_unit_g.png", NoGhostTurret.bindenv( this ) ) )		
		
//		if( !player.UnitLocked( "WALL_WIRE" )	 )			icons.push( PlaceTurretIcon( "gui/textures/radialmenus/wall_wire_g.png", CreateGhostWire.bindenv(this) ) )
//		else 												icons.push( PlaceTurretIcon( "gui/textures/radialmenus/locked_unit_g.png", NoGhostTurret.bindenv( this ) ) )
					
		FinalizeIconSetup( )
	}
	function CreateGhostMachineGun( )	{ /*OnSelectionMade( );*/ return rtsCursor.CreateGhostTurret( UNIT_ID_TURRET_MG_01 ) }
	function CreateGhostMortar( ) 		{ /*OnSelectionMade( );*/ return rtsCursor.CreateGhostTurret( UNIT_ID_TURRET_MORTAR_01 ) }
	function CreateGhostHowitzer( ) 	{ /*OnSelectionMade( );*/ return rtsCursor.CreateGhostTurret( UNIT_ID_TURRET_ARTY_01 ) }
	function CreateGhostAntiAir( ) 		{ /*OnSelectionMade( );*/ return rtsCursor.CreateGhostTurret( UNIT_ID_TURRET_AA_01 ) }
	function CreateGhostAntiTank( ) 	{ /*OnSelectionMade( );*/ return rtsCursor.CreateGhostTurret( UNIT_ID_TURRET_AT_01 ) }
	function CreateGhostFlame( ) 		{ /*OnSelectionMade( );*/ return rtsCursor.CreateGhostTurret( UNIT_ID_TURRET_FLAME_01 ) }
//	function CreateGhostWire( ) 		{ /*OnSelectionMade( );*/ return rtsCursor.CreateGhostTurret( UNIT_ID_WALL_WIRE ) }
	function NoGhostTurret( ) 			{ return rtsCursor.DestroyGhostTurret( ) }
}
