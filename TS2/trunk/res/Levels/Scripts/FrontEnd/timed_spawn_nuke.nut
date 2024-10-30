
sigexport function EntityOnCreate( entity )
{
	entity.Logic = TimedSpawnLogic( )
	entity.Logic.EffectID = "Nuke"
	
	// If the entity that this script is attached to has a name,
	//  that name will be converted to a float and override the time set here.
	entity.Logic.Time = ObjectiveRand.Float( 5.0, 10.0 )
	entity.Logic.RepeatTime = ObjectiveRand.Float( 10.0, 15.0 )
}