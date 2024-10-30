sigimport "Anims/Characters/Shared/Base_Soldier/artillery/artillery.anipk"

class ArtillerySoldierDefaultMoMap extends Anim.MotionMap
{
	sharedAnimPack = null
	constructor()
	{
		Anim.MotionMap.constructor()
		sharedAnimPack = GetAnimPack("Anims/Characters/Shared/Base_Soldier/artillery/artillery.anipk")
		
	}
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( "idle_stand" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}
	
	function Forward( params )
	{
	}
	
	function RandomAnim( params )
	{
		return 0.0
	}
	
	function Fall( params )
	{
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( "fall" )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )
	}
	
	function DeathFall( params )
	{
		Fall( params )
	}
	
	function Land( params )
	{
		return StandardDeath( params )
	}
	
	function StandardDeath( params )
	{
		local deathAnim = "death_toyspin1"
		
		switch( ObjectiveRand.Int( 0, 2 ) )
		{
		case 0:
			deathAnim = "death_toyspin1"
			break;
		case 1:
			deathAnim = "death_toyspin2"
			break;
		case 2:
			deathAnim = "death_toyspin3"
			break;
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = sharedAnimPack.Find( deathAnim )
		track.BlendIn = 0.2
		track.BlendOut = 0.0

		track.Push( Stack )

		return track.Anim.OneShotLength - 0.2
	}
	
	function PushPassengerTrack( params, animPack, frontLeft, frontRight, backLeft, backRight )
	{
		local track = Anim.VehiclePassengerTrack( )
		track.FrontLeftAnim = animPack.Find( frontLeft )
		track.FrontRightAnim = animPack.Find( frontRight )
		track.BackLeftAnim = animPack.Find( backLeft )
		track.BackRightAnim = animPack.Find( backRight )
		
		track.BlendIn = 0.2
		track.BlendOut = 0.1
		track.TimeScale = 1.0
		track.BlendScale = 1.0
		track.Acc = Logic.SprungMass( )
		track.Push( Stack )
	}
	
}

class FrontendSoldierMoMap extends ArtillerySoldierDefaultMoMap
{
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		local animName = ""
		
		local animRand = ObjectiveRand.Float( 0.0, 100.0 )
		
		if(animRand < 65)
			animName = "idle_seated_01"
		else if (animRand < 85 )
			animName = "idle_stand"
		else
			animName = "idle_scan"

		track.Anim = sharedAnimPack.Find( animName )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		track.Push( Stack )
	}
	
}

class FrontendRadiationGuyMoMap extends ArtillerySoldierDefaultMoMap
{
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
		local animName = ""
		
		switch( ObjectiveRand.Int( 0, 1 ) )
		{
		case 0:
			animName = "idle_stand_unarmed"
			break;
		case 1:
			animName = "idle_stand_unarmed"
			break;		
		}
		track.Anim = sharedAnimPack.Find( animName )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		track.Push( Stack )
	}
	
}

class AlliedVehiclePassengerMoMap extends ArtillerySoldierDefaultMoMap
{	
	function Idle( params )
	{
		PushPassengerTrack( params, sharedAnimPack, "veh_passenger_idle_forwardleft", "veh_passenger_idle_forwardright", "veh_passenger_idle_backleft", "veh_passenger_idle_backright" )
	}
}

class AlliedVehicleDriverMoMap extends ArtillerySoldierDefaultMoMap
{	
	function Idle( params )
	{
		PushPassengerTrack( params, sharedAnimPack, "veh_passenger_idle_forwardleft", "veh_passenger_idle_forwardright", "veh_passenger_idle_backleft", "veh_passenger_idle_backright" )
	}
}

// Creeper dude
class ATVRiderMoMap extends ArtillerySoldierDefaultMoMap
{
	function Idle( params )
	{
		PushPassengerTrack( params, sharedAnimPack, "atv_forwardleft", "atv_forwardright", "atv_backleft", "atv_backright" )
	}
}

