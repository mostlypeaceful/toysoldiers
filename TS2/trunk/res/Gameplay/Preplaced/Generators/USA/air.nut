sigimport "Gameplay/Preplaced/generators/common/generator.nut"
sigimport "Anims/preplaced/red/gen_ussr_air/gen_ussr_air.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Air_Generator( )
}

class USA_Air_Generator extends GeneratorBaseLogic
{
	function OnSpawn( )
	{
		::GeneratorBaseLogic.OnSpawn( )
	}

	function DebugTypeName( )
		return "USA_Air_Generator"
	
	function SetMotionMap( ) Animatable.MotionMap = USAAirGeneratorMoMap( )	
	function SetMasterGoal( ) GoalDriven.MasterGoal = GenericAnimatedBreakableGoal( this, { } )
}

class USAAirGeneratorMoMap extends GeneratorMoMap
{
	constructor( )
	{
		GeneratorMoMap.constructor( )
		animPack = GetAnimPack( "Anims/preplaced/red/gen_ussr_air/gen_ussr_air.anipk" )
	}
}
