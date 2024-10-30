
sigexport function EntityOnCreate( entity )
{
	entity.Logic = ProximityLogic( )
	
	entity.Logic.Enabled = 0
	
	local proximity = entity.Logic.Proximity
	proximity.SetRefreshFrequency( 2.0, 0.5 )
	proximity.LogicFilter.AddProperty( ENUM_UNIT_TYPE, UNIT_TYPE_VEHICLE )
	proximity.LogicFilter.AddProperty( ENUM_UNIT_TYPE, UNIT_TYPE_INFANTRY )
	proximity.LogicFilter.AddMustHaveProperty( ENUM_TEAM, Player.EnemyTeam( entity.GetEnumValue( ENUM_TEAM ) ) )
	
	GameApp.CurrentLevel.AddArtilleryBarragePt(entity )
}

