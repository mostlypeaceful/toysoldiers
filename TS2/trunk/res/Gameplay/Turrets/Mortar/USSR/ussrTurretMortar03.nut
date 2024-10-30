
sigimport "Gameplay/Turrets/Common/TurretLogic.nut"
sigimport "gameplay/turrets/mortar/ussr/ussr_mortar_03_motionmap.nut"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Turret_Mortar_03( );
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.CreateDefaultArtillerySoldiers( entity )
}

class USSR_Turret_Mortar_03 extends BaseTurretLogic
{
	function DebugTypeName( )
		return "USSR_Turret_Mortar_03"

	function OnSpawn( )
	{
		WeaponStation( 0 ).Bank( 0 ).AddWeapon( "TURRET_2B11SANI_MORTAR", "" )
		
		BaseTurretLogic.OnSpawn( )
	}	
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = USSR_Mortar_03_MotionMap( this )
	}
}
