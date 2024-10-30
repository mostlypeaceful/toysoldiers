sigimport "Gameplay/Characters/Infantry/Common/parachutelogic.nut"
sigimport "Anims/Vehicles/Attachments/tank_palette/tank_palette.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = TankPalette( )
}

class TankPalette extends ParachuteLogic
{
	ownerCharacter = null
	
	constructor( )
	{
		ParachuteLogic.constructor( )
	}
	
	function DebugTypeName( )
		return "TankPalette"
		
	function OnSpawn( )
	{
		ParachuteLogic.OnSpawn( )
	}
	
	function SetMotionMap( )
	{
		Animatable.MotionMap = TankPaletteMomap( )
	}
}

class TankPaletteMomap extends ParachuteMomap
{
	animPack = null
	
	
	constructor()
	{
		Anim.MotionMap.constructor()
		animPack = GetAnimPack("Anims/Vehicles/Attachments/tank_palette/tank_palette.anipk")		
	}

}