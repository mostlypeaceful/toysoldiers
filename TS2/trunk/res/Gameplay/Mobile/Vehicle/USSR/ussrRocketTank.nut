sigimport "Gameplay/mobile/common/WheeledMobileLogic.nut"
sigimport "Gameplay/mobile/common/MobileTurretLogic.nut"
sigimport "Anims/Vehicles/Red/pechora2m_aa/pechora2m_aa.anipk"
sigimport "gui/textures/waveicons/ussr/vehicle_pechora2m_g.png"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_RocketTank( )
}


class USSR_RocketTank extends ScriptWheeledVehicleLogic
{
	constructor( )
	{
		ScriptWheeledVehicleLogic.constructor( )
	}

	function DebugTypeName( )
		return "USSR_RocketTank"

	function OnSpawn( )
	{
		local cannonBank = WeaponStation( 0 ).Bank( 0 )
		cannonBank.TriggerButton = GAMEPAD_BUTTON_RTRIGGER

		local cannon = cannonBank.AddWeapon( "USSR_ROCKETTANK_MISSILES", "cannon" )
		local cannonE = cannon.SetTurretEntityNamed( "cannon" )
		cannonE.Logic.Animatable.MotionMap = USSR_Tank_RocketTank_GunMoMap( )
		cannonE.Logic.Animatable.ExecuteMotionState( "Idle", { Weapon = cannon } )
		
		ScriptWheeledVehicleLogic.OnSpawn( )
		SetDestroyedEffect( "Medium_Vehicle_Explosion" )
	}
}


class USSR_Tank_RocketTank_GunMoMap extends MobileTurretMoMap
{
	function SetAnimPack( )
	{
		animPack = GetAnimPack( "Anims/Vehicles/Red/pechora2m_aa/pechora2m_aa.anipk" )
	}
}
