sigimport "Gameplay/Characters/Officers/common/officer_base.nut"
sigimport "Gameplay/Characters/officers/usa/officer_01_momap.nut"
sigimport "art/characters/red/russian_commando_debris.sigml"
sigimport "Gui/Textures/WaveIcons/USsr/infantry_officer_g.png"
sigimport "gameplay/mobile/helicopter/usa/jet_pack.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSROfficerSurvivalLogic( )
}

class USSROfficerSurvivalLogic extends BaseOfficerLogic
{
	constructor( )
	{
		BaseOfficerLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "art/characters/red/russian_commando_debris.sigml" )
		
		VehicleBasePath = "gameplay/mobile/helicopter/usa/jet_pack.sigml"
	}
	
	function DebugTypeName( )
		return "USSROfficerSurvivalLogic"
		
	function OnSpawn( )
	{		
		DamageModifier = 0.2
		AITargeting = 1
		
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "SUR_OFFICER_RIFLE", "gun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		local rocket = rocketBank.AddWeapon( "SUR_OFFICER_ROCKET", "rocket" )
		rocket.SetTurretEntity( OwnerEntity )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		BaseOfficerLogic.OnSpawn( )
	}
	
	function SetMotionMap( )
		Animatable.MotionMap = USA_Officer_01_Momap( this, true )
}
