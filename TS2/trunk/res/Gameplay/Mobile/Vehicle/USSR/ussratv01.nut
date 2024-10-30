sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "gameplay/characters/infantry/ussr/infantry_basic_01.sigml"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Blue/tank_m60patton_lilgun/tank_m60patton_lilgun.anipk"
sigimport "gui/textures/waveicons/USSR/vehicle_atv_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_ATV_01( )
}


class USSR_ATV_01 extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_ATV_01"

	function OnSpawn( )
	{
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "ATV_Explode" )
	}
}
