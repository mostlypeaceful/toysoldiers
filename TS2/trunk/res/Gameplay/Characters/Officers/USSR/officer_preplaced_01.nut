sigimport "Gameplay/Characters/Officers/common/officer_base.nut"
sigimport "Gameplay/Characters/officers/usa/officer_01_momap.nut"
sigimport "art/characters/blue/american_commando_debris.sigml"
sigimport "Gui/Textures/WaveIcons/USsr/infantry_officer_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSROfficerPreplacedLogic( )
}

class USSROfficerPreplacedLogic extends BaseOfficerLogic
{
	constructor( )
	{
		BaseOfficerLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "art/characters/blue/american_commando_debris.sigml" )
	}
	
	function DebugTypeName( )
		return "USSROfficerPreplacedLogic"
		
	function OnSpawn( )
	{				
		local gunBank = WeaponStation( 0 ).Bank( 0 )
		gunBank.AddWeapon( "OFFICER_PREPLACED_RIFLE", "gun" )
		gunBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER
		
		local rocketBank = WeaponStation( 0 ).Bank( 1 )
		local rocket = rocketBank.AddWeapon( "OFFICER_PREPLACED_ROCKET", "rocket" )
		rocket.SetTurretEntity( OwnerEntity )
		rocketBank.TriggerButton = GAMEPAD_BUTTON_LTRIGGER
		
		BaseOfficerLogic.OnSpawn( )
		
		if( ::GameApp.OneManArmy )
		{
			DeletePackage( )
			DamageModifier = 0.0 //dont take damage
		}
		else if( !GameApp.SpawningCurrentLevel )
			ShowWaitTimer( )
	}
	
	function SetMotionMap( )
		Animatable.MotionMap = USA_Officer_01_Momap( this, false )
}
