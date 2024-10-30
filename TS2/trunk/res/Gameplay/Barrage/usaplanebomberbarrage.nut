sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "gui/textures/waveicons/usa/vehicle_bomber_g.png"
sigimport "Gameplay/Mobile/Airborne/usa/usaplanebomber01_momap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Plane_Bomber_Barrage( )
}

class USA_Plane_Bomber_Barrage extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Plane_Bomber_Barrage"

	function OnSpawn( )
	{
		// Weapons: USA B52 Bomb

		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USA_B52_BOMB_BARRAGE", "" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		AirborneVehicleLogic.OnSpawn( )
		
		SetDestroyedEffect( "Bomber_Explosion" )
		SetRandomFlyTimeRemaining( 0 )
		DisableEvasion = 1
	}
	
		function SetMotionMap( )
		Animatable.MotionMap = USAPlaneBomber01MoMap( this )

}
