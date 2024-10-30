sigimport "Gameplay/Characters/Passenger/Common/passengerlogic.nut"


sigexport function EntityOnCreate( entity )
{
	entity.Logic = VietnamArtillerySoldierLogic( CreateArtillerySoldierMoMap( entity ) )
}
sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class VietnamArtillerySoldierLogic extends ArtillerySoldierLogic
{
	function GetHead( )
	 {
		 return ""
	 }
	function GetHelmet( ) 
	{
		return ""
	}
}
