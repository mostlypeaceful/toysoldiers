sigimport "Gameplay/Characters/Common/SoldierLogic.nut"
sigimport "art/_placeholder/cube.mshml"
sigimport "art/_placeholder/Gameplay/australian_debris.sigml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = BasicInfantryLogic( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class InfantryLogic extends SoldierLogic
{
	didSpecialEntrance = false
	
	constructor( )
	{
		SoldierLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "art/_placeholder/Gameplay/australian_debris.sigml" )
		didSpecialEntrance = false
	}

	function DebugTypeName( )
		return "InfantryLogic"

	function OnSpawn( )
	{
		SetPersonalityType( ObjectiveRand.Int( 0, PERSONALITY_TYPE_COMMANDO - 1 ) )
		SoldierLogic.OnSpawn( )
	}

	function OnDelete( )
	{
		SoldierLogic.OnDelete( )
	}
	
	// called from a few places in goaml
	function RealDeath( )
	{
		ExplodeIntoParts( )
		OwnerEntity.Delete( )	
	}

	// Override these functions in the country's derived classes returning the proper head and helmet mesh
	function GetHead( ) return "art/_placeholder/cube.mshml"
	function GetHelmet( ) return "art/_placeholder/cube.mshml"
	function GetBackpack( ) return "art/_placeholder/cube.mshml"
		
	function Outfit( entity )
	{
		local attach_head0 = entity.FirstChildWithName( "attach_head0" )
		local attach_helm0 = entity.FirstChildWithName( "attach_helm0" )
		local attach_backpack0 = entity.FirstChildWithName( "attach_backpack0" )

		SpawnProp( attach_head0, GetHead( ) )
		SpawnProp( attach_backpack0, GetBackpack( ) )
//		if( !HasHelmetProp )
		SpawnProp( attach_helm0, GetHelmet( ) )
	}
	
	function SpawnProp( attach, path )
	{
		local ent = null
		if( Entity.IsValid( attach ) ) ent = OwnerEntity.SpawnChildFromProxy( path, attach )
		if( Entity.IsValid( ent ) ) ent.AddGameTags( FLAG_SPAWN_AS_DEBRIS )
	}
}
