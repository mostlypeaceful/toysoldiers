sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "gui/textures/waveicons/usa/vehicle_bomber_g.png"
sigimport "Gameplay/Mobile/Airborne/usa/usaplanebomber01_momap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Plane_Bomber_01( )
}

class USA_Plane_Bomber_01 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Plane_Bomber_01"

	function OnSpawn( )
	{
		// Weapons: USA B52 Bomb

		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USSR_BEAR_BOMB", "" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		AirborneVehicleLogic.OnSpawn( )
		
		SetDestroyedEffect( "Bomber_Explosion" )
		SetRandomFlyTimeRemaining( 0 )
		DisableEvasion = 1
	}
	
		function SetMotionMap( )
		Animatable.MotionMap = USAPlaneBomber01MoMap( this )

}
