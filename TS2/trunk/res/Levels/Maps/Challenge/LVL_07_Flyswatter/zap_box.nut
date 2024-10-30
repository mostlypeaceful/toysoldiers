

sigexport function EntityOnCreate( entity )
{
	entity.Logic = ProximityLogic( )
	
	local proximity = entity.Logic.Proximity
	proximity.SetRefreshFrequency( 0.25, 0.1 )
	proximity.LogicFilter.AddProperty( ENUM_UNIT_TYPE, UNIT_TYPE_AIR )
	proximity.TrackNewEnts = 1
	
	entity.Logic.NewEntCallback = BugZapAMofo
}

function BugZapAMofo( entity )
{
	GameEffects.PlayEffect( entity, "InsectZapped" )
	entity.Logic.Destroy( )
}