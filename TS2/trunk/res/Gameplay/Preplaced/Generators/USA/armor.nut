sigimport "Gameplay/Preplaced/generators/common/generator.nut"
sigimport "Anims/preplaced/blue/gen_usa_armor/gen_usa_armor.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Armor_Generator( )
}

class USA_Armor_Generator extends GeneratorBaseLogic
{
	function OnSpawn( )
	{
		::GeneratorBaseLogic.OnSpawn( )
	}

	function DebugTypeName( )
		return "USA_Armor_Generator"
	
	function SetMotionMap( ) Animatable.MotionMap = USAArmorGeneratorMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}

class USAArmorGeneratorMoMap extends GeneratorMoMap
{
	constructor( )
	{
		GeneratorMoMap.constructor( )
		animPack = GetAnimPack( "Anims/preplaced/blue/gen_usa_armor/gen_usa_armor.anipk" )
	}
}
