sigimport "Effects/Entities/misc/red_smoke.sigml"
sigimport "Effects/Entities/misc/green_smoke.sigml"
sigimport "Gameplay/Preplaced/Breakables/generic_animated_goal.goaml"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = GeneratorBaseLogic( )
}

class GeneratorBaseLogic extends GeneratorLogic
{
	constructor( )
	{
		::GeneratorLogic.constructor( )
	}

	function OnSpawn( )
	{
		SetMotionMap( )
		SetMasterGoal( )
		
		::GeneratorLogic.OnSpawn( )

		GameApp.CurrentLevel.RegisterGenerator( OwnerEntity )
		TakesDamage = 0
		
		if( OwnerEntity.GetEnumValue( ENUM_TEAM ) == TEAM_RED )
			DropSmokeSigml = "Effects/Entities/misc/red_smoke.sigml"
		else
			DropSmokeSigml = "Effects/Entities/misc/green_smoke.sigml"
	}

	function DebugTypeName( )
		return "GeneratorBaseLogic"
	
	function SetMotionMap( ) { } //override this
	function SetMasterGoal( ) { }
}

class GeneratorMoMap extends Anim.MotionMap
{
	animPack = null
	
	constructor( )
	{
		Anim.MotionMap.constructor( )
		
		//add the following line to your constructor
		//animPack = GetAnimPack( "Anims/preplaced/red/gen_ussr_armor/gen_ussr_armor.anipk" )
	}

	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.0
		track.BlendOut = 0.0
		
		if( Logic.OpenDoors )
			track.Anim = animPack.Find( "launch_idle" )
		else
			track.Anim = animPack.Find( "idle" )
		
		track.Push( Stack )
	}

	function ReApply( params )
	{			
		if( Logic.Readying )
		{
			local track = Anim.KeyFrameTrack( )
			track.BlendIn = 1.5
			track.BlendOut = 0.0
			track.Anim = animPack.Find( "readying" )
			track.Push( Stack )
		}
	}

	function ReApplyOneShot( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.Flags = ANIM_TRACK_CLAMP_TIME
			
		if( Logic.OpenDoors )
			track.Anim = animPack.Find( "launch" )
		else
			track.Anim = animPack.Find( "shutdown" )
			
		track.Push( Stack )
		
		return track.Anim.OneShotLength - track.BlendOut
	}
}