
sigexport function EntityOnCreate( entity )
{
	entity.Logic = ProximityLogic( )
	
	local proximity = entity.Logic.Proximity
	proximity.SetRefreshFrequency( 1.0, 0.1 )
	proximity.LogicFilter.AddProperty( ENUM_UNIT_TYPE, UNIT_TYPE_VEHICLE )
	proximity.LogicFilter.AddProperty( ENUM_UNIT_TYPE, UNIT_TYPE_INFANTRY )
	proximity.LogicFilter.AddProperty( ENUM_UNIT_TYPE, UNIT_TYPE_AIR )
	
	GameApp.CurrentLevel.RegisterToyBoxAlarm(entity )
}

