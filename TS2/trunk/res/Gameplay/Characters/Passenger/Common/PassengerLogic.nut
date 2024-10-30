sigimport "Gameplay/Characters/Passenger/Common/PassengerMoMap.nut"
sigimport "Gameplay/Characters/Passenger/Common/PassengerGoal.goaml"
sigimport "Art/Characters/Red/russian_infantry_debris.sigml"

function CreateArtillerySoldierMoMap( entity )
{
	local moMapName = entity.GetName( )
	try
	{
		local moMapClass = rawget( moMapName )
		local moMapInstance = moMapClass.instance( )
		moMapInstance.constructor( )
		return moMapInstance
	}
	catch( e ) { }

	print( "Couldn't determine what sort of motion map to make for ArtillerySoldier: " + entity )
	return null
}

sigexport function EntityOnCreate( entity )
{
	entity.Logic = ArtillerySoldierLogic( CreateArtillerySoldierMoMap( entity ) )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class ArtillerySoldierLogic extends VehiclePassengerLogic
{
	moMapToSet = null
	
	constructor( moMap )
	{
		VehiclePassengerLogic.constructor( )
		
		SetAlternateDebrisMesh( "Art/Characters/Red/russian_infantry_debris.sigml" )
		
		if( !moMap )
			moMap = ArtillerySoldierDefaultMoMap( );
		moMapToSet = moMap
	}
	
	function DebugTypeName( )
		return "ArtillerySoldierLogic"
		
	function OnSpawn( )
	{
		VehiclePassengerLogic.OnSpawn( )
		
		Animatable.MotionMap = moMapToSet
		GoalDriven.MasterGoal = PassengerGoal( this, { } )

		PostGoalSet( )
	}

	// Override these two functions in the country's derived classes returning the proper head and helmet mesh
	function GetHead( ) return "art/_placeholder/cube.mshml"
	function GetHelmet( ) return "art/_placeholder/cube.mshml"
	function GetBackpack( ) return ""
	
	function Outfit( entity )
	{
		local attach_head0 = entity.FirstChildWithName( "attach_head0" )
		local attach_helm0 = entity.FirstChildWithName( "attach_helm0" )
		local attach_backpack0 = entity.FirstChildWithName( "attach_backpack0" )
		
		local turret_head
		
		local turretLogic = OwnerEntity.Parent.Logic
		if( "GetHead" in turretLogic )
			turret_head = turretLogic.GetHead( )
		else
			turret_head = GetHead( )

		SpawnProp( attach_head0, turret_head )
		SpawnProp( attach_backpack0, GetBackpack( ) )
		if( !HasHelmetProp )
			SpawnProp( attach_helm0, GetHelmet( ) )
	}
	
	function SpawnProp( attach, path )
	{
		local ent = null
		if( Entity.IsValid( attach ) ) ent = OwnerEntity.SpawnChildFromProxy( path, attach )
		if( Entity.IsValid( ent ) ) ent.AddGameTags( FLAG_SPAWN_AS_DEBRIS )
	}
	
	function RealDeath( )
	{
		ExplodeIntoParts( )
		OwnerEntity.Delete( )	
	}
}
