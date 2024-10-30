sigimport "Gameplay/Characters/Infantry/Common/InfantryLogic.nut"
sigimport "Art/Characters/Blue/usa_infantry_debris.sigml"
sigimport "Art/Characters/Blue/usa_head_1.mshml"
sigimport "Art/Characters/Blue/usa_head_2.mshml"
sigimport "Art/Characters/Blue/usa_head_3.mshml"
sigimport "Art/Characters/Blue/usa_head_4.mshml"
sigimport "Art/Characters/Blue/usa_head_5.mshml"
sigimport "Art/Characters/Blue/usa_head_6.mshml"
sigimport "Art/Characters/Blue/usa_helm_1.mshml"
sigimport "Art/Characters/Blue/american_cap.mshml"
sigimport "Art/Characters/Blue/american_sailorhat.mshml"
sigimport "Art/Characters/Blue/usa_backpack_1.mshml"
sigimport "Gui/Textures/WaveIcons/USA/infantry_lvl1_g.png"

sigexport function EntityOnCreate( entity )
{
	entity.Logic = USA_Infantry_Civilian_01( )
}

sigexport function EntityOnChildrenCreate( entity )
{
	entity.Logic.Outfit( entity )
}

class USA_Infantry_Civilian_01 extends InfantryLogic
{
	constructor( )
	{
		InfantryLogic.constructor( )
		SoldierLogic.SetAlternateDebrisMesh( "Art/Characters/Blue/usa_infantry_debris.sigml" )
	}
	
	function DebugTypeName( )
		return "USA_Infantry_Civilian_01"
		
	function OnSpawn( )
	{		
		InfantryLogic.OnSpawn( )
	}
	
	function GetHead( )
	 {
		 local heads = {}
		 heads[0] <- "Art/Characters/Blue/usa_head_1.mshml"
		 heads[1] <- "Art/Characters/Blue/usa_head_2.mshml"
		 heads[2] <- "Art/Characters/Blue/usa_head_3.mshml"
		 heads[3] <- "Art/Characters/Blue/usa_head_4.mshml"
		 heads[4] <- "Art/Characters/Blue/usa_head_5.mshml"
		 heads[5] <- "Art/Characters/Blue/usa_head_6.mshml"
		
		 local headRand = ObjectiveRand.Int(0,5)
		  		return heads[headRand]
	}
	
	function SetMotionMap( )
		Animatable.MotionMap = USACivilianMoMap( this )
}

class USACivilianMoMap extends SoldierMotionMap
{
	
	function Idle( params )
	{
		local track = Anim.KeyFrameTrack( )
	
		local animRand = ObjectiveRand.Int(0,1)
		if(animRand)
			track.Anim = animPack.Find( "russian_prisoner_help02" )
		else
			track.Anim = animPack.Find( "russian_prisoner_help01" )	
				
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )

		track.Push( Stack )
	}
	function RandomAnim ( params )
	{
		return 0.0
	}
	function MoveForward( params )
	{
		local runAnim = "run"
		local hurry = ObjectiveRand.Float( 1.1, 1.4 )	
		local persist = Logic.CurrentPersistentEffect
		
		switch( persist )
		{
			case PERSISTENT_EFFECT_BEHAVIOR_FIRE:
				runAnim = "burning_run"
				GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_OnFire" )
				break
			case PERSISTENT_EFFECT_BEHAVIOR_GAS:
				runAnim = "cough_stumble"
				GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Cough" )
				break
			case PERSISTENT_EFFECT_BEHAVIOR_STUN:
				runAnim = "flashbang_run"		
				break
			default:
			{
				if( Logic.FirstWaveLaunch )
				{
					Logic.FirstWaveLaunch = 0
					GameEffects.PlayEffect( Logic.OwnerEntity, "Voc_Charge" )
				}
				
				if( Logic.InAlarmZone )
				{
					switch( ObjectiveRand.Int( 1, 2 ) )
					{
						case 1: runAnim = "highstep_run"; break;
						case 2: runAnim = "taunt_run"; break;
					}
				}
				else
				{				
					// no swimming anims
					//switch( Logic.SurfaceTypeEnum( ) )
					//{
					////case SURFACE_TYPE_WATER :
					////	runAnim = "land"; break;
					//default:
						if( Logic.IsCaptain )
							runAnim = "flag_run";
						else
						{
							switch( ObjectiveRand.Int( 1, 2 ) )
							{
								case 1: runAnim = "russian_prisoner_run03"; break;
								case 2: runAnim = "russian_prisoner_run02"; break;
							}
						}
					//}
				}
			}
		}
		
		local track = Anim.KeyFrameTrack( )
		track.Anim = animPack.Find( runAnim )
		track.BlendIn = 0.2
		track.BlendOut = 0.0
		track.TimeScale = hurry
		track.StartTime = ObjectiveRand.Float( 0.0, track.Anim.OneShotLength )
		
		track.Push( Stack )
		
		FollowPath( params )
	}
}