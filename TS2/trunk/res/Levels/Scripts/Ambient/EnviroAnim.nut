

function CreateEnviroAnimMoMap( entity )
{
	local moMapName = entity.GetName( )
	try
	{
		local moMapClass = rawget( moMapName )
		local moMapInstance = moMapClass.instance( )
		moMapInstance.constructor( )
		return moMapInstance
	}
	catch( e ) { }

	print( "Couldn't determine what sort of motion map to make for EnviroAnim: " + entity )
	return null
}

sigexport function EntityOnCreate( entity )
{
	entity.Logic = EnviroAnimLogic( CreateEnviroAnimMoMap( entity ) )
}


class EnviroAnimLogic extends AnimGoalLogic
{
	constructor( moMap )
	{
		assert( moMap )
		AnimGoalLogic.constructor( )
		Animatable.MotionMap = moMap
	}
	function DebugTypeName( )
		return "EnviroAnimLogic"
	function OnSpawn( )
	{
		GoalDriven.MasterGoal = MasterEnviroAnimGoal( this )
		AnimGoalLogic.OnSpawn( )
	}
}

class MasterEnviroAnimGoal extends AI.CompositeGoal
{
	constructor( logic )
	{
		AI.CompositeGoal.constructor( )
		AddPotentialGoal( EnviroAnimSpawnGoal( logic ), 1.0 )
	}
	function DebugTypeName( )
		return "MasterEnviroAnimGoal"
}
class EnviroAnimSpawnGoal extends AI.MotionGoal
{
	constructor( logic )
	{
		AI.MotionGoal.constructor( )
		SetMotionState( logic, "Spawn", {  } )
	}
	function DebugTypeName( )
		return "EnviroAnimSpawnGoal"
}


