sigexport function EntityOnCreate( entity )
{
	GameApp.CurrentLevel.RegisterNamedObject( entity )
	entity.Logic = Effects.PausedFx( )
}
