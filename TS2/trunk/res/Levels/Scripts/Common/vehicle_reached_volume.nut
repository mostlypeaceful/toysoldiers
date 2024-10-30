

sigexport function EntityOnCreate( entity )
{
	entity.Logic = ProximityLogic( )
	
	local proximity = entity.Logic.Proximity
	proximity.SetRefreshFrequency( 0.25, 0.1 )
	proximity.LogicFilter.AddProperty( ENUM_UNIT_ID, UNIT_ID_INFANTRY_ATV_FULL_PHYSICS )
	
	entity.Logic.EntityCountChangedCallback = GameApp.CurrentLevel.VehicleReachedVolume.bindenv( GameApp.CurrentLevel )
}

