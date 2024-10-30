sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "Gameplay/Mobile/Airborne/USA/usaplanefighter01_momap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_fighter_g.png"
sigimport "effects/fx/units/vehicles/jet_afterburner_02.fxml"
sigimport "effects/fx/units/vehicles/jet_boost_02.fxml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Plane_Fighter_01( )
}

class USA_Plane_Fighter_01 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Plane_Fighter_01"

	function OnSpawn( )
	{
// USA F14 Vulcan, USA F14 Missile
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USA_F14_VULCAN", "gun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		rocketBank.AddWeapon( "USA_F14_MISSILE", "Rockets" )
		rocketBank.FireMode = FIRE_MODE_ALTERNATE
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		local bombBank = WeaponStation( 0 ).Bank( 2 )
		bombBank.AddWeapon( "USA_F14_NAPALM", "Napalm" )
		bombBank.FireMode = FIRE_MODE_ALTERNATE
		bombBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		AddEnableBombBank( 2 )
		AddDisableBombBank( 0 )
		AddDisableBombBank( 1 )
		
		AirborneVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Fighter_Jet_Explosion" )
		SetRandomFlyTimeRemaining( 60 )

		InitializeJetEngineFx( "effects/fx/units/vehicles/jet_afterburner_02.fxml", "effects/fx/units/vehicles/jet_boost_02.fxml" )
		
		DisableTimeScale = 1
	}
	
	function SetMotionMap( ) 
		Animatable.MotionMap = USAPlaneFighter01MoMap( this )
}
