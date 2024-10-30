
sigexport function EntityOnCreate( entity )
{
	GameApp.CurrentLevel.GroundHeight = entity.GetPosition( ).y
}
