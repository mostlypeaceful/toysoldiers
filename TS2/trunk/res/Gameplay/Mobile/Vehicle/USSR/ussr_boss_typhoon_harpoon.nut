sigimport "Art/units/vehicles/attachments/ropes_goals.goaml"
sigimport "Art/units/vehicles/attachments/rope.nut"
sigimport "Anims/Bosses/Red/sub_harpoon/sub_harpoon.anipk"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = SubHarpoonLogic( )
}

class SubHarpoonLogic extends RopeSlideLogic
{		
	constructor( )
	{
		RopeSlideLogic.constructor( )
	}

	function DebugTypeName( )
		return "SubHarpoonLogic"

	function OnSpawn( )
	{				
		SetMotionMap( )
		
		RopeSlideLogic.SetMasterGoal( )
		RopeSlideLogic.OnSpawn( )
	}	
	
	function SetMotionMap( ) Animatable.MotionMap = SubHarpoonMoMap( )	
}


class SubHarpoonMoMap extends RopeSlideMoMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		animPack = GetAnimPack( "Anims/Bosses/Red/sub_harpoon/sub_harpoon.anipk" )
	}

}