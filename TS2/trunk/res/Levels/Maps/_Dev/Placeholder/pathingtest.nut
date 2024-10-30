
sigimport "Levels/Scripts/Common/game_standard.nut"
sigimport "gameplay/characters/officers/usa/officer_01_in_box.sigml"
sigimport "Gui/Textures/WaveIcons/USA/infantry_lvl2_g.png"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, PathingTestLevelLogic( ) )
}


class PathingTestLevelLogic extends GameStandardLevelLogic
{
	lists = null
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )

		
		//TutOnlyPlaceThisUnit = UNIT_ID_TURRET_ARTY_01
		
		lists = { }
		AddList( "Infantry" )
		AddList( "Ropers" )
		AddList( "Tanks" )
		AddList( "Cars" )
		
		local parking = AddWaveList( "Parking" )
		parking.SetMakeSelectableUnits( 1 )
		
		AddList( "RandomFlying" )
		
		LevelIntro2P( "flyin", "flyin", null, false, true )
		
		/*
		
		The following is an example of finding named points and disabling them from the start.
		
		local pathP = GameApp.CurrentLevel.NamedPathPoint( "wp1" )
		if( Entity.IsValid( pathP ) )
			pathP.Logic.Accessible = 0*/
			
		local pathP = GameApp.CurrentLevel.NamedPathPoint( "tele_alt" )
		if( Entity.IsValid( pathP ) )
			pathP.Logic.Accessible = 0
	}
	
	function AddList( list )
	{
		local testList 
		testList = AddWaveList( list );
		if( !is_null( testList ) )
		{
			testList.SetLooping( true )	
			
			lists[ list ] <- testList
		}
	}
	
	function SpawnPreIntroWaves( )
	{
		////parking.Activate( )
		lists[ "Infantry" ].Activate( )
		//lists[ "Ropers" ].Activate( )
		//lists[ "Tanks" ].Activate( )
		////lists[ "Cars" ].Activate( )	
		////lists[ "RandomFlying" ].Activate( )	
	}

	function OnRegisterSpecialLevelObject( specialLevelObject )
	{
		if( specialLevelObject.GetName( ) == "PhoneBooth" )
			specialLevelObject.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnPhoneBoothDestroyed.bindenv( this ) )
			
		// be sure to always call the base
		GameStandardLevelLogic.OnRegisterSpecialLevelObject( specialLevelObject )
	}

	function OnPhoneBoothDestroyed( unitLogic )
	{
		waveList2.Activate( )
		waveList2.SetLooping( true )
	}
	
	function OnPathCameraFinished( pathName )
	{
		if( pathName == "flyin" )
			waveList1.Activate( )
	}
	
	function WaveFirst( )
	{
		print( "Wave first" )
	}
	
	function Wave1End( )
	{
		print( "Wave 1 end" )
		
		// Lock player in turret
		//local player = GameApp.FrontEndPlayer
		//player.LockInUnit( "turret1" )
	}
	
	function Wave2End( )
	{
		print( "Wave 2 end" )
		//local player = GameApp.FrontEndPlayer
		
		// Send player to atv1 but then unlock them so they can exit if they want
		//player.LockInUnit( "atv1" )
		//player.UnlockFromUnit( 0 ) //1 to kick them out
	}
}
