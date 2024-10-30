sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "Gameplay/Mobile/Airborne/USSR/ussrplanefighter01_momap.nut"
sigimport "gui/textures/waveicons/ussr/vehicle_fighter_g.png"
sigimport "effects/fx/units/vehicles/jet_afterburner_01.fxml"
sigimport "effects/fx/units/vehicles/jet_boost_01.fxml"
sigimport "gameplay\projectiles\ussr\ussr_mig23_missile.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Plane_Fighter_01( )
}

class USSR_Plane_Fighter_01 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_Plane_Fighter_01"

	function OnSpawn( )
	{
// USA F14 Vulcan, USA F14 Missile
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USSR_MIG23_CANNON", "gun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "USSR_MIG23_MISSILE", "Rockets" )
		rocketBank.FireMode = FIRE_MODE_ALTERNATE
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local bombBank = WeaponStation( 0 ).Bank( 2 )
		bombBank.AddWeapon( "USSR_MIG23_BOMB", "Napalm" )
		bombBank.FireMode = FIRE_MODE_ALTERNATE
		bombBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER

		AddEnableBombBank( 2 )
		AddDisableBombBank( 0 )
		AddDisableBombBank( 1 )
		
		AirborneVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Fighter_Jet_Explosion" )
		SetRandomFlyTimeRemaining( 60 )
		InitializeJetEngineFx( "effects/fx/units/vehicles/jet_afterburner_01.fxml", "effects/fx/units/vehicles/jet_boost_01.fxml" )

		DisableTimeScale = 1
	
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USSRPlaneFighter01MoMap( this )
}
