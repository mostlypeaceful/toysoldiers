sigimport "Gameplay/Preplaced/generators/common/generator.nut"
sigimport "Anims/preplaced/red/gen_ussr_air/gen_ussr_air.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USSR_Air_Generator( )
}

class USSR_Air_Generator extends GeneratorBaseLogic
{
	function OnSpawn( )
	{
		::GeneratorBaseLogic.OnSpawn( )
	}

	function DebugTypeName( )
		return "USSR_Air_Generator"
	
	function SetMotionMap( ) Animatable.MotionMap = USSRAirGeneratorMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}

class USSRAirGeneratorMoMap extends GeneratorMoMap
{
	constructor( )
	{
		GeneratorMoMap.constructor( )
		animPack = GetAnimPack( "Anims/preplaced/red/gen_ussr_air/gen_ussr_air.anipk" )
	}
}
