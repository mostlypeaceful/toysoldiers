sigimport "Gameplay/Turrets/Common/TurretMoMap.nut"
sigimport "gameplay/turrets/common/turretai.goaml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = BaseTurretLogic( )
}

// Base turret logic.

class BaseTurretLogic extends TurretLogic
{	
	constructor( )
	{
		TurretLogic.constructor( )
	}
	function DebugTypeName( )
		return "BaseTurretLogic"
	function OnSpawn( )
	{
		SetMotionMap( )
		SetMasterGoal( )
		SetDestroyedEffect( "Large_Turret_Explosion" )
		TurretLogic.OnSpawn( )
		
		FindHitpointLinkedChildren( )
		
		if( CreationType == CREATIONTYPE_FROMBUILDSITE )
		{
			switch( AttributeSize )
			{
			 case 0:
			 case -1:
				GameEffects.PlayEffect( OwnerEntity, "turret_place_small" )
				break;
			 case 1:
				GameEffects.PlayEffect( OwnerEntity, "turret_place_large" )
				break;
			}
		}
	}
	function CreateDefaultArtillerySoldiers( entity )
	{
		entity.ForEachChild( CreateDefaultArtillerySoldierOnEntity.bindenv( this ) )
	}
	function CreateDefaultArtillerySoldierOnEntity( entity )
	{
		if( entity.GetEnumValue( ENUM_ARTILLERY_SOLDIER ) == -1 )
			return
		
		local myTeam = GameApp.DefaultTeamFromCountry( OwnerEntity.GetEnumValue( ENUM_COUNTRY ) )
		local sigmlPath = null
		switch( myTeam )
		{
		case TEAM_BLUE: sigmlPath = gDefaultBlueTeamPassengerSigml; break;
		case TEAM_RED: 	sigmlPath = gDefaultRedTeamPassengerSigml; break;
		default:
			print("WARNING! team could not be determined for turret in BaseTurretLogic.CreateDefaultArtillerySoldierOnEntity")
			break
		}
		
		if( sigmlPath && sigmlPath.len( ) > 0 )
			OwnerEntity.SpawnChildFromProxy( sigmlPath, entity )
		else
			print( "WARNING! no sigml path specified in BaseTurretLogic.CreateDefaultArtillerySoldierOnEntity" )
	}

	// overrideable methods (i.e., these are defined as separate functions so they can be overridden by derived types)

	function SetMotionMap( )
	{
		Animatable.MotionMap = TurretMotionMap( this )
	}

	function SetMasterGoal( )
	{
		GoalDriven.MasterGoal = TurretGoal( this, { } )
	}
}


