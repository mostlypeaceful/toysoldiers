sigimport "Gameplay/Mobile/Airborne/ussr/ussrplanebomber01_momap.nut"
sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "gui/textures/waveicons/ussr/vehicle_bomber_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Plane_Bomber_01( )
}

class USSR_Plane_Bomber_01 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Plane_Bomber_01"

	function OnSpawn( )
	{
// Weapons: USSR Bear Bomb

		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USSR_BEAR_BOMB", "" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		AirborneVehicleLogic.OnSpawn( )
		
		SetDestroyedEffect( "Bomber_Explosion" )
		SetRandomFlyTimeRemaining( 0 )
		DisableEvasion = 1
		DisableTimeScale = 1
	}
	
	function SetMotionMap( )
		Animatable.MotionMap = USSRPlaneBomber01MoMap( this )

}
