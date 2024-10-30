sigimport "Gameplay/Mobile/Airborne/Common/Airbornelogic.nut"
sigimport "Gameplay/Mobile/Airborne/usa/usaplanefighter02_momap.nut"
sigimport "gui/textures/waveicons/usa/vehicle_fighter_g.png"
sigimport "effects/fx/units/vehicles/jet_afterburner_02.fxml"
sigimport "effects/fx/units/vehicles/jet_boost_02.fxml"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Plane_Fighter_02( )
}

class USA_Plane_Fighter_02 extends AirborneVehicleLogic
{
	constructor( )
	{
		AirborneVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USA_Plane_Fighter_02"

	function OnSpawn( )
	{
// USA F14 Vulcan, USA F14 Napalm (LT)
		AirborneVehicleLogic.OnSpawn( )
		
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "USA_MUSTANG_NAPALM", "" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER	

		local gunBank = WeaponStation( 0 ).Bank( 1 )
		gunBank.AddWeapon( "USA_MUSTANG_VULCAN", "gun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		AirborneVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Fighter_Jet_Explosion" )
		SetRandomFlyTimeRemaining( 60 )
		
		InitializeJetEngineFx( "effects/fx/units/vehicles/jet_afterburner_02.fxml", "effects/fx/units/vehicles/jet_boost_02.fxml" )
	}
	
	function SetMotionMap( )
		Animatable.MotionMap = USAPlaneFighter02MoMap( this )

}
