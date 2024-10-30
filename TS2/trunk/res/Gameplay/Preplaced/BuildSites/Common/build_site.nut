
sigexport function EntityOnCreate( entity )
{
	entity.Logic = BuildSiteBaseLogic( )
}

// Needs to be EntityOnChildrenCreate because it requires child shape objects to have already been created
sigexport function EntityOnChildrenCreate( entity )
{
	local size = entity.GetEnumValue( ENUM_BUILD_SITE )
	
	if( size == BUILD_SITE_SMALL )
		GameApp.CurrentLevel.RegisterBuildSiteSmall( entity )
	else if ( size == BUILD_SITE_LARGE )
		GameApp.CurrentLevel.RegisterBuildSiteLarge( entity )
	else
		print( "Trying to register buildsite ( " + entity.GetName( ) + " ) without a size!" )
}

class BuildSiteBaseLogic extends BuildSiteLogic
{
	constructor( )
	{
		::BuildSiteLogic.constructor( )
	}

	function OnSpawn( )
	{
		::BuildSiteLogic.OnSpawn( )
		TakesDamage = 0
	}

	function DebugTypeName( )
		return "BuildSiteBaseLogic"
}