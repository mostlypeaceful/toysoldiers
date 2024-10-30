sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "gui/textures/waveicons/USA/vehicle_atv_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_ATV_01( )
}


class USA_ATV_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_ATV_01"

	function OnSpawn( )
	{
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "ATV_Explode" )
	}
}
