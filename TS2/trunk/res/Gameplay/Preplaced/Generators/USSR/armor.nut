sigimport "Gameplay/Preplaced/generators/common/generator.nut"
sigimport "Anims/preplaced/red/gen_ussr_armor/gen_ussr_armor.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Armor_Generator( )
}

class USSR_Armor_Generator extends GeneratorBaseLogic
{
	function OnSpawn( )
	{
		::GeneratorBaseLogic.OnSpawn( )
	}

	function DebugTypeName( )
		return "USSR_Armor_Generator"
	
	function SetMotionMap( ) Animatable.MotionMap = USSRArmorGeneratorMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}

class USSRArmorGeneratorMoMap extends GeneratorMoMap
{
	constructor( )
	{
		GeneratorMoMap.constructor( )
		animPack = GetAnimPack( "Anims/preplaced/red/gen_ussr_armor/gen_ussr_armor.anipk" )
	}
}
