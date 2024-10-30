
sigexport function EntityOnCreate( entity )
{
	entity.Logic = GoalBoxBaseLogic( )
	GameApp.CurrentLevel.RegisterGoalBox( entity )
}

class GoalBoxBaseLogic extends GoalBoxLogic
{
	constructor( )
	{
		::GoalBoxLogic.constructor( )
	}

	function OnSpawn( )
	{
		TakesDamage = 0
		::GoalBoxLogic.OnSpawn( )
	}

	function DebugTypeName( )
		return "GoalBoxBaseLogic"
}

