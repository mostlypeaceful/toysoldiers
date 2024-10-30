
sigexport function EntityOnCreate( entity )
{
	entity.Logic = TimedSpawnLogic( )
	entity.Logic.EffectID = "BarrageArtillery"
	
	// If the entity that this script is attached to has a name,
	//  that name will be converted to a float and override the time set here.
	entity.Logic.Time = 5.0
	//entity.Logic.RepeatTime = 1.0 // set this value to be positive to spawn on the RepeatTime interval
	
	//alternatively you can spawn a sigml by specifying the filepath instead of an effectID
	//dont forget to sigimport this file
	//entity.Logic.Filepath = "blahblahblah.sigml"
}