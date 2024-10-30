
// Requires
sigimport "Gui/Scripts/RootMenus/singleplayer.nut"
sigimport "Gui/Scripts/RootMenus/headtohead.nut"
sigimport "Gui/Scripts/RootMenus/coop.nut"
sigimport "Levels/Scripts/Common/level_event.nut"
sigimport "gui/scripts/endgamescreens/allendgamescreens.nut"
sigimport "gui/scripts/hud/levelintrotext.nut"
sigimport "gui/scripts/hud/impacttext.nut"
sigimport "gui/scripts/utility/rationticketscripts.nut"

// Resources
sigimport "DataTables/UnitUpgradeProgression.tab"
sigimport "gameplay/characters/passenger/usa/passenger_01.sigml"
sigimport "gameplay/characters/passenger/ussr/passenger_01.sigml"
sigimport "gameplay/mobile/airborne/usa/plane_fighter_01.sigml"
sigimport "gameplay/mobile/airborne/ussr/plane_fighter_01.sigml"

// barrage imports
sigimport "Gui/Textures/misc/loading_g.png"
// Put usa and ussr resources in their barrage_imports.nut files. those get imported programaticlly

sigimport "gui/textures/barrage/barrage_artillery_g.png"
sigimport "gui/textures/barrage/barrage_rambo_g.png"
sigimport "gui/textures/barrage/barrage_ivan_g.png"
sigimport "gui/textures/barrage/barrage_b52_g.png"
sigimport "gui/textures/barrage/barrage_nuke_g.png"
sigimport "gui/textures/barrage/barrage_ac130_g.png"
sigimport "gui/textures/barrage/barrage_lazer_g.png"
sigimport "gui/textures/barrage/barrage_napalm_g.png"

// these things are used in the trial and in minigames
sigimport "Levels/Scripts/MiniGames/ShootingGalleryGoals.goaml"
sigimport "gui/scripts/hud/scoretimer.nut"
sigimport "gui/scripts/hud/readysetgocounter.nut"
sigimport "gui/scripts/hud/trialresultsdialog.nut"
sigimport "gui/scripts/hud/tutorialpresenter.nut"	
sigimport "gui/scripts/endgamescreens/trialbuygamescreen.nut"		
sigimport "gui/scripts/hud/currentplayerui.nut"
sigimport "gui/scripts/hud/minigamecomboui.nut"
sigimport "gui/scripts/hud/trialrestrictionui.nut"

// oneman army stuff imported in dlc_imports.nut

gDefaultBlueTeamPassengerSigml <- null
gDefaultRedTeamPassengerSigml <- null

// Barrages
class ScriptRamboBarrage extends RamboBarrage
{
	function Reset( player )
	{
		if( player.Team == TEAM_BLUE )
			::StandardImpactText( "Commando_Down", player )
		else
			::StandardImpactText( "Ivan_Down", player )
		RamboBarrage.Reset( player )
	}
}
class ScriptGunShipBarrage extends UsableWaveLaunchBarrage
{
	function Reset( player )
	{
		::StandardImpactText( "AC130_Over", player )
		UsableWaveLaunchBarrage.Reset( player )
	}
}

// Level logics
sigexport function EntityOnCreate( entity ) 
{
	RegisterLevelLogic( entity, GameStandardLevelLogic( ) )
}

function RegisterLevelLogic( entity, levelLogic )
{
	entity.Logic = levelLogic
	GameApp.CurrentLevel = levelLogic 
}

class GameStandardLevelLogic extends LevelLogic
{
	rootMenu = null
	initialWaves = null // array: all these waves will be spawned after the intro
	introCameraPath = null
	introText = null
	endGameStack = null
	sellDialogs = null
	onLevelIntroComplete = null // function, callback to fire on end of level intro
		
	// Ui for MiniGame
	scoreUI = null
	countDownTimer = null
	miniGameDuration = 60
	miniGameLeaderBoard = LEADERBOARD_TRIAL_MINIGAME_1
	miniGameBeginLocKey = "beginKillMiniGame"
	miniGameKillFunction = null
	miniGameStartStopFunction = null
	miniGameScoreIndex = -1
	miniGameActive = false
	miniGamePlays = 0
	currentPlayerUi = null
	UnslowDown = null
	
	// surival
	survivalMainWave = null
	survivalSpecialWave = null
	inSpecialWave = false
	showTraumaTut = false
	
	// Trial
	restrictionUI = null
	
	// Decorations / Challenge
	decorationData = null
	challengeCondition = null
	
	// Golden Babushkas
	goldenObjectsDestroyed = null
		
	constructor( )
	{
		::LevelLogic.constructor( )
		
		GameApp.SetUnitUpgradeProgressionTable( "DataTables/UnitUpgradeProgression.tab" )

		// set default passenger sigmls
		gDefaultBlueTeamPassengerSigml = "gameplay/characters/passenger/usa/passenger_01.sigml"
		gDefaultRedTeamPassengerSigml = "gameplay/characters/passenger/ussr/passenger_01.sigml"
		
		rootMenu = null
		initialWaves = null
		introCameraPath = null
		introText = null
		endGameStack = false
		sellDialogs = { }
		
		scoreUI = null
		countDownTimer = null
		miniGameDuration = 60
		miniGameLeaderBoard = LEADERBOARD_TRIAL_MINIGAME_1
		miniGameKillFunction = null
		miniGameStartStopFunction = null
		miniGameScoreIndex = -1
		miniGameActive = false
		miniGamePlays = 0
		UnslowDown = 0
		
		survivalMainWave = null
		survivalSpecialWave = null
		inSpecialWave = false
		showTraumaTut = false
		
		PlatformPrice = 200

		decorationData = array( 2, null )
		
		goldenObjectsDestroyed = 0
	}
	
	function OnSpawn( )
	{
		::LevelLogic.OnSpawn( )
		
		local gameApp = ::GameApp
		local hudRoot = gameApp.HudRoot
		local gameMode = gameApp.GameMode
			
		hudRoot.Invisible = false
		hudRoot.SetAlpha( 1 ) //super fail safe incase no ones calls LevelIntro2P

		if( gameMode.IsSinglePlayer )
			rootMenu = ::SinglePlayerStandardRootMenu( )
		else if( gameMode.IsVersus )
			rootMenu = ::HeadToHeadStandardRootMenu( )
		else if( gameMode.IsCoOp )
			rootMenu = ::CoOpStandardRootMenu( )
		
		if( rootMenu )
			SetRootMenu( rootMenu )
		PushStandardCameras( )
		
		SetupBarrages( )
		
		// Extra stuff
		if( !gameMode.IsFrontEnd && ExtraMode )
		{
			if( gameMode.IsSplitScreen )
				gameApp.SecondaryPlayer.SpawnCharacter( "Extra/Gameplay/Characters/max.sigml", "p2" )
			
			gameApp.FrontEndPlayer.SpawnCharacter( "Extra/Gameplay/Characters/max.sigml", "p1" )
		}
		
		if( MapType == MAP_TYPE_MINIGAME && gameApp.SingleScreenCoop )
		{
			currentPlayerUi = ::CurrentPlayerUI( ControllingPlayer( ) )
			::ResetPreviousPlayerScores( )
		}
		
		if( !gameApp.IsFullVersion && ( MapType == MAP_TYPE_MINIGAME || MapType == MAP_TYPE_SURVIVAL ) )
			restrictionUI = ::TrialRestrictionUI( ControllingPlayer( ) )
			
		/*if( !gameMode.IsFrontEnd && gameMode.IsMultiPlayer )
		{
			if( !::GameAppSession.IsHost && ::GameApp.FrontEndPlayer.IsNotAllowedToSaveStats )
				::CreateSimpleGlobalModalDialog( ::GameApp.FrontEndPlayer.User.IsLocal ? "Lobby_Player1Higher" : "Lobby_HostHigher", ::GameApp.FrontEndPlayer.User )
			else if( ::GameAppSession.IsHost && ::GameApp.SecondaryPlayer.IsNotAllowedToSaveStats && ::GameApp.SecondaryPlayer.User.IsLocal )
				::CreateSimpleGlobalModalDialog( "Lobby_Player1Higher", ::GameApp.SecondaryPlayer.User )
		}*/
	}
	
	function AddSellDialog( user, dialog )
	{
		local vpIndex = user.ViewportIndex
		if( vpIndex in sellDialogs )
			sellDialogs[ vpIndex ].Close( )
		sellDialogs[ vpIndex ] <- dialog
	}
	
	function ClearAllSellDialogs( )
	{
		foreach( i, dialog in sellDialogs )
		{
			if( dialog && "Close" in dialog )
				dialog.Close( )
		}
	}
	
	function OnDelete( )
	{
		// this is called to get KillMenuStack called on any existing menu
		MoveToGuiScreen( null )
		LevelLogic.OnDelete( )
		
		if( currentPlayerUi )
			currentPlayerUi.FadeOutAndDie( )
		
		if( restrictionUI )
			restrictionUI.FadeOutAndDie( )
	}

	function LevelIntro2P( usaPath, ussrPath, waves = null, showText = true, skipable = true )
	{
		if( ::Player.RandyMode( ) )
			skipable = true
			
		if( GameApp.FromSaveGame )
		{			
			SetCurrentLevelProgression( LEVEL_PROGRESSION_PLAYING )
			if( MapType == MAP_TYPE_HEADTOHEAD )
			{
				PreSpawnPreIntroWaves( )
				SpawnWaves( )
			}
		}
		else
		{
			introCameraPath = usaPath
			initialWaves = waves
			
			if( usaPath == null || ussrPath == null )
			{
				LogWarning( 0, "You need to specify two Level intro paths!", false )
				
				// If you don't run the fly-in camera, activiate the initial wave (if set) and show the HUD
				PreSpawnPreIntroWaves( )
				SpawnWaves( )
				::GameApp.HudRoot.SetAlpha( 1 )
			}
			else
			{
				// Show the intro text for the level
				if( showText ) 
				{
					ShowIntroText( skipable )
					// Hide the HUD
					::GameApp.HudRoot.SetAlpha( 0 )
				}
				
				// Start the fly-in camera
				PreSpawnPreIntroWaves( )
				SetCurrentLevelProgression( LEVEL_PROGRESSION_INTRO )
				
				if( !::GameApp.GameMode.IsCoOp )
				{
					local playerCount = ::GameApp.PlayerCount;
					for( local p = 0; p < playerCount; ++p )
					{
						local player = ::GameApp.GetPlayer( p )
						{
							local country = player.Country
							if( country == COUNTRY_USA || country == COUNTRY_DEFAULT )
								player.BeginCameraPath( usaPath, skipable )
							else if( country == COUNTRY_USSR )
								player.BeginCameraPath( ussrPath, skipable )
						}
					}
				}
				else
				{
					::GameApp.GetPlayer( 0 ).BeginCameraPath( usaPath, skipable )
					::GameApp.GetPlayer( 1 ).BeginCameraPath( ussrPath, skipable )
				}
			}
		}
	}
	
	function PreSpawnPreIntroWaves( )
	{
		SpawnPreIntroWaves( )
		
		if( ::GameApp.OneManArmy )
			SetupOneManArmy( )
	}
	
	function SpawnPreIntroWaves( )
	{
	}
	
	function SetupOneManArmy( )
	{
		local player1 = ::GameAppSession.HostPlayer
		local player2 = ::GameAppSession.ClientPlayer
		
		// TODO: Use special OneManArmy units
		SpawnOneManArmyCharacter( player1, "gameplay/characters/officers/usa/officer_01_survival.sigml", "OneManArmy" )
		if( ::GameApp.GameMode.IsCoOp )
			SpawnOneManArmyCharacter( player2, "gameplay/characters/officers/ussr/officer_01_survival.sigml", "OneManArmyCoop" )
	}
	
	function CommandoUnitName( player )
	{
		return "OneManArmy_" + player.PlayerIndex.tostring( )
	}
	
	function SpawnOneManArmyCharacter( player, path, point )
	{
		local character = OwnerEntity.SpawnChild( path )
		if( is_null( character ) || !::Entity.IsValid( character ) )
		{
			::print( "Warning: error creating character for one man army" )
			return
		}
		
		local unitName = CommandoUnitName( player )
		character.SetName( unitName )
		character.AddGameTags( FLAG_SELECTABLE )
		
		local country = ::GameApp.CurrentLevelLoadInfo.Country
		character.SetEnumValue( ENUM_COUNTRY, country )
		
		local spawnPoint = OwnerEntity.FirstChildWithName( point )
		if( !is_null( spawnPoint ) && ::Entity.IsValid( spawnPoint ) )
		{
			character.MoveToXForm( spawnPoint.GetMatrix( ) )
		}
		else
		{
			::print( "Warning: One Man Army Setup: couldn't find spawn point: " + ( point ? point.tostring( ) : "null" ) )
			return
		}
		
		if( character.Logic )
			character.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OneManArmyDestroyed.bindenv( this ) )
	}
	
	function OneManArmyDestroyed( logic )
	{
		if( !GameApp.CurrentLevel.VictoryOrDefeat ) 
		{
			GameApp.CurrentLevel.DefeatedPlayer = GameApp.FrontEndPlayer
			GameApp.AddTimedCallback( 0.0, GameEnded.bindenv( this ) ) //delay defeat, in seconds
		}	
	}
	
	function LockPlayersInCommandoUnits( )
	{
		local players = [ ::GameAppSession.HostPlayer ]
		if( ::GameApp.GameMode.IsCoOp )
			players.push( ::GameAppSession.ClientPlayer )
			
		foreach( player in players )
		{
			local unit = OwnerEntity.FirstChildWithName( CommandoUnitName( player ) )
			if( !is_null( unit ) && ::Entity.IsValid( unit ) )
				player.LockInUnitDirect( unit.Logic )
			else
				::print( "Warning: One Man Army Setup: couldn't find character: " + CommandoUnitName( player ) )
		}
	}
	
	function SpawnWaves( )
	{		
		if( initialWaves )
		{
			foreach( wave in initialWaves )
			{
				wave.Activate()
			}
		}
	}
	
	function ShowIntroText( skipable )
	{
		introText = LevelIntroText( LevelNumber, MapType, skipable )
		rootMenu.AddChild( introText )
	}
	
	function OnPathCameraFinished( pathName )
	{
		if( pathName == introCameraPath )
		{
			// Remove the intro text
			if( introText )
				introText.FadeThroughBlackAndDelete( )
			
			// Show the HUD
			::GameApp.HudRoot.SetAlpha( 1 )
			
			// Activate the initial wave, if set
			SpawnWaves( )
			
			// If survival and Commando mode, lock players in commando units
			if( MapType == MAP_TYPE_SURVIVAL && ::GameApp.CurrentLevelLoadInfo.ChallengeMode == CHALLENGE_MODE_COMMANDO )
				LockPlayersInCommandoUnits( )
			
			// Fire event
			if( onLevelIntroComplete )
				onLevelIntroComplete( )
		}
	}
	
	function OnBossDestroyed( )
	{
		if( !GameApp.CurrentLevel.VictoryOrDefeat )
		{
			//::DisplayImpactText( "BossDestroyed" ) // TODO
			GameApp.CurrentLevel.VictoriousPlayer = GameApp.FrontEndPlayer
			GameApp.AddTimedCallback( 0.0, GameEnded.bindenv( this ) ) //delay victory, in seconds
		}
	}	
	
	function OnBossReachedGoal( )
	{
		if( !GameApp.CurrentLevel.VictoryOrDefeat ) 
		{
			GameApp.CurrentLevel.DefeatedPlayer = GameApp.FrontEndPlayer
			GameApp.AddTimedCallback( 0.0, GameEnded.bindenv( this ) ) //delay defeat, in seconds
		}
	}
	
	function MoveToGuiScreen( screen )
	{
		if( "KillMenuStack" in rootMenu )
			rootMenu.KillMenuStack( )
		
		ClearAllSellDialogs( )
		
		rootMenu = screen
		SetRootMenu( rootMenu )
	}
	
	function PushBuyScreen( user )
	{
		PushEndGameScreen( ::TrialBuyGameScreen( ), user )
	}
	
	function PushEndGameScreen( screen, user )
	{
		if( !endGameStack )
		{
			endGameStack = ::EndGameScreenStack( user )
			MoveToGuiScreen( endGameStack )
			endGameStack.SetUser( user )
		}
		
		endGameStack.PushMenu( screen )
	}
	
	function OnTicketsCountChanged( count, player )
	{
		if( showTraumaTut )
		{
			local presentDelay = 3.5
			::PopupMessage( "trauma_tut", Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ), FONT_FANCY_MED, null, Math.Vec3.Construct(0,-200,0), null, presentDelay, 0.1 )
			::TutorialPresenter.Present( "score", ::GameApp.CurrentLevel.ControllingPlayer( ).User, presentDelay, true, Math.Vec2.Construct( -60, -140 ) )
			showTraumaTut = false
		}
		
		if( count == 0 && !GameApp.AAACantLose && !GameApp.CurrentLevel.VictoryOrDefeat ) 
		{
			GameApp.CurrentLevel.DefeatedPlayer = player
			GameApp.AddTimedCallback( 2.5, GameEnded.bindenv( this ) ) //delay defeat, in seconds
		}
	}
	
	function DelayedGameEnd( time )
	{
		::GameApp.AddTimedCallback( time, GameEnded.bindenv( this ) )
	}
	
	function OnNoWavesOrEnemiesLeft( )
	{
		if( miniGameStartStopFunction != null )
			return //this is a minigame, can't win this way
			
		if( (MapType) != MAP_TYPE_HEADTOHEAD && (MapType) != MAP_TYPE_SURVIVAL && !GameApp.CurrentLevel.VictoryOrDefeat )
		{
			GameApp.CurrentLevel.VictoriousPlayer = GameApp.FrontEndPlayer
			
			local delay = 1 //delay victory, in seconds
			GameApp.AddTimedCallback( delay, GameEnded.bindenv( this ) ) 
		}
		
		if( (MapType) == MAP_TYPE_SURVIVAL )
		{
			if( inSpecialWave )
			{
				inSpecialWave = false
				survivalMainWave.Activate( )			
			}
			else
			{
				inSpecialWave = true
				survivalSpecialWave.Reset( )
				survivalSpecialWave.Activate( )
				
				::DisplayImpactText( "Survival_BonusWave" )
			}
		}
	}
	
	function OnWaveListFinished( waveList )
	{
	}
	
	function OnWaveKilled( waveList )
	{
	}

	function BossDestroyed( )
	{
		// Do not move to victory immediately, wait for OnNoWavesOrEnemiesLeft( )
		//Victory( )
	}

	function OnRegisterBoss( boss )
	{
		SetCurrentLevelProgression( LEVEL_PROGRESSION_BOSS )
	}	

	function OnRegisterSpecialLevelObject( specialLevelObject )
	{
		switch( specialLevelObject.GetName( ) )
		{
			case "goldenArcade":
				RegisterGoldenArcade( specialLevelObject )
			break
			
			case "goldenBabushka":
				RegisterGoldenBabushka( specialLevelObject )
			break
			
			case "goldenDogTag":
				RegisterGoldenDogTag( specialLevelObject )
			break
		}
	}
	
	function RegisterGoldenArcade( specialLevelObject )
	{
		local deleteIt = true
		for( local i = 0; i < ::GameApp.PlayerCount; ++i )
		{
			if( !::GameApp.GetPlayer( i ).LevelScores.HasFoundGoldenArcade )
				deleteIt = false
		}
				
		if( deleteIt || DlcNumber != DLC_COLD_WAR )
			specialLevelObject.Delete( )
		else
			specialLevelObject.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnGoldenArcadeDestroyed.bindenv( this ) )
	}
	
	function OnGoldenArcadeDestroyed( unitLogic )
	{
		for( local i = 0; i < ::GameApp.PlayerCount; ++i )	
		{
			local player = ::GameApp.GetPlayer( i )
			
			if( player.LevelScores.HasFoundGoldenArcade )
				continue
			
			player.LevelScores.SetFoundGoldenArcade( )
			
			local total = player.TotalGoldenArcadeCount
			local prog = player.GoldenArcadeFoundCount
			
			// Decorations
			local earnedIndex = null
			if( prog == 1 )
				earnedIndex = 0
			if( prog == total )
				earnedIndex = 1
			
			if( earnedIndex != null )
			{
				// Ration Ticket
				RationTicketUI.AwardGoldenArcadeRationTicket( earnedIndex, player.User )
				
				// Earned Item
				local earnedItem = ::EarnedItemData( )
				earnedItem.Type = EARNED_ITEM_TYPE_GOLDEN_ARCADE
				earnedItem.Value = earnedIndex
				player.AddEarnedItem( earnedItem )
			}
			
			// Notification
			RationTicketUI.FindGoldenArcade( player.User )
			
			// Earned Item
			local earnedItem = ::EarnedItemData( )
			earnedItem.Type = EARNED_ITEM_TYPE_GOLDEN_ARCADE
			earnedItem.Value = -1
			player.AddEarnedItem( earnedItem )
		}
	}
	
	function RegisterGoldenBabushka( specialLevelObject )
	{
		local deleteIt = true
		for( local i = 0; i < ::GameApp.PlayerCount; ++i )
		{
			if( !::GameApp.GetPlayer( i ).LevelScores.HasFoundAllGoldenBabushkas )
				deleteIt = false
		}
				
		if( deleteIt || DlcNumber != DLC_EVIL_EMPIRE )
			specialLevelObject.Delete( )
		else
			specialLevelObject.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnGoldenBabushkaDestroyed.bindenv( this ) )
	}
	
	function OnGoldenBabushkaDestroyed( unitLogic )
	{
		goldenObjectsDestroyed++
		
		for( local i = 0; i < ::GameApp.PlayerCount; ++i )	
		{
			local player = ::GameApp.GetPlayer( i )
			
			if( player.LevelScores.HasFoundAllGoldenBabushkas )
				continue
				
			if( goldenObjectsDestroyed >= ::LevelScores.GoldenObjectsPerLevel( ) )
			{
				player.LevelScores.SetFoundAllGoldenBabushkas( )
				
				// Achievement
				if( player.FoundAllGoldenBabushkas )
					player.AwardAchievement( ACHIEVEMENTS_DLC1_FIND_15_DOLLS )
				
				// Notification
				RationTicketUI.FindAllGoldenBabushkas( player.User )
				
				// Earned Item
				local earnedItem = ::EarnedItemData( )
				earnedItem.Type = EARNED_ITEM_TYPE_GOLDEN_ARCADE
				earnedItem.Value = -2
				player.AddEarnedItem( earnedItem )
			}
			else
			{
				// Notification
				RationTicketUI.GoldenBabushkaProgress( goldenObjectsDestroyed, player.User )
			}
		}
	}
	
	function RegisterGoldenDogTag( specialLevelObject )
	{
		local deleteIt = true
		for( local i = 0; i < ::GameApp.PlayerCount; ++i )
		{
			if( !::GameApp.GetPlayer( i ).LevelScores.HasFoundAllGoldenDogTags )
				deleteIt = false
		}
				
		if( deleteIt || DlcNumber != DLC_NAPALM )
			specialLevelObject.Delete( )
		else
			specialLevelObject.Logic.RegisterForLevelEvent( LEVEL_EVENT_UNIT_DESTROYED, OnGoldenDogTagDestroyed.bindenv( this ) )
	}
	
	function OnGoldenDogTagDestroyed( unitLogic )
	{
		goldenObjectsDestroyed++
		
		for( local i = 0; i < ::GameApp.PlayerCount; ++i )	
		{
			local player = ::GameApp.GetPlayer( i )
			
			if( player.LevelScores.HasFoundAllGoldenDogTags )
				continue
				
			if( goldenObjectsDestroyed >= ::LevelScores.GoldenObjectsPerLevel( ) )
			{
				player.LevelScores.SetFoundAllGoldenDogTags( )
				
				// Achievement
				if( player.FoundAllGoldenDogTags )
					player.AwardAchievement( ACHIEVEMENTS_DLC2_FIND_DOG_TAGS )
				
				// Notification
				RationTicketUI.FindAllGoldenDogTags( player.User )
				
				// Earned Item
				local earnedItem = ::EarnedItemData( )
				earnedItem.Type = EARNED_ITEM_TYPE_GOLDEN_ARCADE
				earnedItem.Value = -3
				player.AddEarnedItem( earnedItem )
			}
			else
			{
				// Notification
				RationTicketUI.GoldenDogTagProgress( goldenObjectsDestroyed, player.User )
			}
		}
	}
	
	function QuitImmediately( )
	{
		::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
		::ResetGlobalDialogBoxSystem( )
		::GameApp.LoadFrontEnd( )
	}
	
	function NetQuitEarly( user )
	{
		SetCurrentLevelProgression( LEVEL_PROGRESSION_DEFEAT )
		PushEndGameScreen( ::NetQuitEarlyScreen( user ), ::GameApp.FrontEndPlayer.User )
	}
	
	function OnNetPlayerQuitEarly( userWhoQuit )
	{
		if( ::GameAppSession.IsQuickMatch )
		{
			::GameAppSession.SetFrontEndLoadBehavior( FRONTEND_LOAD_BEHAVIOR_DESTROY_SESSION )
			
			if( userWhoQuit.IsLocal )
				QuitImmediately( )
				
			return
		}
		
		NetQuitEarly( userWhoQuit )
	}
	
	function GameEnded( )
	{	
		local gameApp = ::GameApp
		local gameMode = ::GameApp.GameMode
		local coop = gameMode.IsCoOp
		local user = ::GameApp.FrontEndPlayer.User
		
		::GameApp.ForEachPlayer( function( player, x ) { player.OnMatchEnded( ) } )
		
		if( MapType == MAP_TYPE_HEADTOHEAD )
		{
			SetCurrentLevelProgression( LEVEL_PROGRESSION_VICTORY )
			PushEndGameScreen( ::VersusEndGameScreen( gameApp.CurrentLevel.VictoriousPlayer, gameApp.CurrentLevel.DefeatedPlayer ), user )
		}
		else if( MapType == MAP_TYPE_SURVIVAL )
		{
			SetCurrentLevelProgression( LEVEL_PROGRESSION_DEFEAT )
			PushEndGameScreen( ::SurvivalEndGameScreen( gameApp.FrontEndPlayer, (coop)? gameApp.SecondaryPlayer: null ), user )
		}
		else if( MapType == MAP_TYPE_CAMPAIGN )
		{
			if( gameApp.CurrentLevel.IsVictorious( gameApp.FrontEndPlayer ) || gameApp.CurrentLevel.IsVictorious( gameApp.SecondaryPlayer ) )
			{
				SetCurrentLevelProgression( LEVEL_PROGRESSION_VICTORY )
				PushEndGameScreen( ::CampaignVictoryScreen( gameApp.FrontEndPlayer, (coop)? gameApp.SecondaryPlayer: null ), user )
			}
			else
			{
				SetCurrentLevelProgression( LEVEL_PROGRESSION_DEFEAT )
				PushEndGameScreen( ::CampaignDefeatScreen( gameApp.FrontEndPlayer, (coop)? gameApp.SecondaryPlayer: null ), user )
			}
		}
		else if( MapType == MAP_TYPE_MINIGAME )
		{
			// ???
		}
	}
	
	function SetupBarrages( )
	{
		if( GameApp.CurrentLevelLoadInfo.ChallengeBarrages && !GameApp.OneManArmy )
		{
			GameApp.ForEachPlayer( ArtilleryBarrageImp.bindenv( this ), "" )
			if( !GameApp.GameMode.IsVersus )
				GameApp.ForEachPlayer( RamboBarrageImp.bindenv( this ), "" )
			GameApp.ForEachPlayer( B52BarrageImp.bindenv( this ), "" )
			GameApp.ForEachPlayer( NukeBarrageImp.bindenv( this ), "" )
			GameApp.ForEachPlayer( AC130BarrageImp.bindenv( this ), "" )
			GameApp.ForEachPlayer( LazerBarrageImp.bindenv( this ), "" )
			GameApp.ForEachPlayer( NapalmBarrageImp.bindenv( this ), "" )
		}
		else
		{
			TutDisableRandomPickups = 1
		}
	}
	
	function ArtilleryBarrageImp( player, nothing )
	{		
		if( !player.UnitLocked( "BARRAGE_ARTILLERY" ) )
		{
			if( player.Team == TEAM_BLUE )
			{
				local barrage2 = ArtilleryBarrage( )
				barrage2.DevName = "BARRAGE_ARTILLERY"
				barrage2.Name = "BARRAGE_ARTILLERY"
				barrage2.IconPath = "gui/textures/barrage/barrage_artillery_g.png"
				barrage2.WeaponID = "ARTILLERY_BARRAGE_1"
				barrage2.SetSpawnPtName( "artillerySpawn" )		
				barrage2.NumberOfExplosions = 20
				barrage2.DelayMin = 0.25
				barrage2.DelayMax = 1.25
				barrage2.AudioEventSelected = "Play_Barrage_Artillery_Selected"
				barrage2.AudioEventBegin = "Play_Barrage_Artillery_CallIn"
				barrage2.AudioEventEnd = "Play_Barrage_Artillery_Exit"
				player.AddBarrage( barrage2 )
			}
			else
			{
				local barrage2 = ArtilleryBarrage( )
				barrage2.DevName = "BARRAGE_ARTILLERY"
				barrage2.Name = "BARRAGE_ARTILLERY"
				barrage2.IconPath = "gui/textures/barrage/barrage_artillery_g.png"
				barrage2.WeaponID = "ARTILLERY_BARRAGE_1"
				barrage2.SetSpawnPtName( "artillerySpawn_USSR" )		
				barrage2.NumberOfExplosions = 20
				barrage2.DelayMin = 0.25
				barrage2.DelayMax = 1.25
				barrage2.AudioEventSelected = "Play_Barrage_Artillery_Selected"
				barrage2.AudioEventBegin = "Play_Barrage_Artillery_CallIn"
				barrage2.AudioEventEnd = "Play_Barrage_Artillery_Exit"
				player.AddBarrage( barrage2 )
			}
		}
	}
	
	function RamboBarrageImp( player, nothing )
	{	
		if( !player.UnitLocked( "BARRAGE_RAMBO" ) )
		{	 
			if( player.Team == TEAM_BLUE )
			{
				local barrage = ScriptRamboBarrage( )
				barrage.DevName = "BARRAGE_RAMBO"
				barrage.Name = "BARRAGE_RAMBO"
				barrage.IconPath = "gui/textures/barrage/barrage_rambo_g.png"
				barrage.CharacterPath = "gameplay/characters/officers/usa/officer_01_in_box.sigml"
				barrage.Duration = 30
				barrage.DropPtName = "ramboDrop"
				barrage.AudioEventSelected = "Play_Barrage_Commando_Selected"
				barrage.AudioEventBegin = "Play_Barrage_Commando_CallIn"
				barrage.AudioEventFirstUse = "Play_Barrage_Commando_Rolling"
				barrage.AudioEventEnd = "Play_Barrage_Commando_Exit"
				player.AddBarrage( barrage )
			}
			else
			{
				local barrage = ScriptRamboBarrage( )
				barrage.DevName = "BARRAGE_RAMBO"
				barrage.Name = "BARRAGE_IVAN"
				barrage.IconPath = "gui/textures/barrage/barrage_ivan_g.png"
				barrage.CharacterPath = "gameplay/characters/officers/ussr/officer_01_in_box.sigml"
				barrage.Duration = 30
				barrage.DropPtName = "ivanDrop"
				barrage.AudioEventSelected = "Play_Barrage_Commando_Selected"
				barrage.AudioEventBegin = "Play_Barrage_USSR_Commando_CallIn"
				barrage.AudioEventFirstUse = "Play_Barrage_Commando_Rolling"
				barrage.AudioEventEnd = "Play_Barrage_USSR_Commando_Exit"
				player.AddBarrage( barrage )
			}
		}
	}
	
	function B52BarrageImp( player, dropPts )
	{	
		if( !player.UnitLocked( "BARRAGE_B52" ) )
		{	
			if( player.Team == TEAM_BLUE )
			{ 
				local barrage = TargetedWaveLaunchBarrage( )
				barrage.DevName = "BARRAGE_B52"
				barrage.Name = "BARRAGE_B52"
				barrage.IconPath = "gui/textures/barrage/barrage_b52_g.png"
				barrage.Duration = 20
				barrage.TargetingDuration = 20
				barrage.WaveListName = "B52Barrage"
				barrage.AudioEventBegin = "Play_Barrage_B52_CallIn"
				barrage.AudioEventEnd = "Play_Barrage_B52_Exit"
				barrage.AudioEventLaunched = "Play_Barrage_B52_Rolling"
				barrage.AudioEventFire = "Play_Barrage_B52_Use"
				// dont use
				//barrage.AddUsableName( barrage.WaveListName ) // wave units are given name of wavelist
				barrage.SetPathSigml( OwnerEntity.FirstChildWithName( "B52_Barrage" ) )
				player.AddBarrage( barrage )
			}
			else
			{
				local barrage = TargetedWaveLaunchBarrage( )
				barrage.DevName = "BARRAGE_B52"
				barrage.Name = "BARRAGE_B52_USSR"
				barrage.IconPath = "gui/textures/barrage/barrage_b52_g.png"
				barrage.Duration = 20
				barrage.TargetingDuration = 20
				barrage.WaveListName = "B52BarrageUSSR"
				barrage.AudioEventBegin = "Play_Barrage_B52_CallIn"
				barrage.AudioEventEnd = "Play_Barrage_B52_Exit"
				barrage.AudioEventLaunched = "Play_Barrage_B52_Rolling"
				barrage.AudioEventFire = "Play_Barrage_B52_Use"
				// dont use
				//barrage.AddUsableName( barrage.WaveListName ) // wave units are given name of wavelist
				barrage.SetPathSigml( OwnerEntity.FirstChildWithName( "b52ussr_barrage" ) )
				player.AddBarrage( barrage )
			}
		}
	}
	
	function NukeBarrageImp( player, spawnPtName )
	{	
		if( !player.UnitLocked( "BARRAGE_NUKE" ) && !GameApp.PAXDemoMode )
		{
			if( player.Team == TEAM_BLUE )
			{ 
				local barrage2 = TargetedArtilleryBarrage( )
				barrage2.DevName = "BARRAGE_NUKE"
				barrage2.Name = "BARRAGE_NUKE"
				barrage2.IconPath = "gui/textures/barrage/barrage_nuke_g.png"
				barrage2.WeaponID = "ARTILLERY_BARRAGE_2"
				barrage2.SetSpawnPtName( "artillerySpawn" )		
				barrage2.NumberOfExplosions = 1
				barrage2.DelayMin = 0.25
				barrage2.DelayMax = 1.25
				barrage2.AudioEventBegin = "Play_Barrage_Nuke_CallIn"
				barrage2.AudioEventEnd = "Play_Barrage_Nuke_Exit"
				barrage2.AudioEventTargetSet = "Play_Barrage_Nuke_Rolling"
				barrage2.AudioEventUse = "Play_Barrage_Nuke_Use"
				barrage2.KillStatToIncrement = SESSION_STATS_MOST_KILLS_WITH_A_NUKE
				player.AddBarrage( barrage2 )
			}
			else
			{
				local barrage2 = TargetedArtilleryBarrage( )
				barrage2.DevName = "BARRAGE_NUKE"
				barrage2.Name = "BARRAGE_NUKE"
				barrage2.IconPath = "gui/textures/barrage/barrage_nuke_g.png"
				barrage2.WeaponID = "ARTILLERY_BARRAGE_2"
				barrage2.SetSpawnPtName( "artillerySpawn_USSR" )		
				barrage2.NumberOfExplosions = 1
				barrage2.DelayMin = 0.25
				barrage2.DelayMax = 1.25
				barrage2.AudioEventBegin = "Play_Barrage_Nuke_CallIn"
				barrage2.AudioEventEnd = "Play_Barrage_Nuke_Exit"
				barrage2.AudioEventTargetSet = "Play_Barrage_Nuke_Rolling"
				barrage2.AudioEventUse = "Play_Barrage_Nuke_Use"
				barrage2.KillStatToIncrement = SESSION_STATS_MOST_KILLS_WITH_A_NUKE
				player.AddBarrage( barrage2 )
			}
		}
	}
	
	
	function NapalmBarrageImp( player, spawnPtName )
	{	
		if( !player.UnitLocked( "BARRAGE_NAPALM" ) && !GameApp.PAXDemoMode && player.HasDLC( DLC_NAPALM ) )
		{
			if( player.Team == TEAM_BLUE )
			{ 
				local barrage = TargetedWaveLaunchBarrage( )
				barrage.DevName = "BARRAGE_NAPALM"
				barrage.Name = "BARRAGE_NAPALM"
				barrage.IconPath = "gui/textures/barrage/barrage_napalm_g.png"
				barrage.Duration = 20
				barrage.TargetingDuration = 20
				barrage.WaveListName = "NapalmBarrage"
				barrage.AudioEventBegin = "Play_Barrage_Naplam_CallIn_DLC02"
				barrage.AudioEventEnd = "Play_Barrage_Napalm_Exit_DLC02"
				barrage.AudioEventLaunched = "Play_Barrage_Napalm_TargetSet_DLC02"
				barrage.AudioEventFire = "Play_Barrage_Napalm_Use_DLC02"
				// dont use
				//barrage.AddUsableName( barrage.WaveListName ) // wave units are given name of wavelist
				barrage.SetPathSigml( OwnerEntity.FirstChildWithName( "B52_Barrage" ) )
				player.AddBarrage( barrage )
			}
			else
			{
				local barrage = TargetedWaveLaunchBarrage( )
				barrage.DevName = "BARRAGE_NAPALM"
				barrage.Name = "BARRAGE_NAPALM"
				barrage.IconPath = "gui/textures/barrage/barrage_napalm_g.png"
				barrage.Duration = 20
				barrage.TargetingDuration = 20
				barrage.WaveListName = "NapalmBarrageUSSR"
				barrage.AudioEventBegin = "Play_Barrage_Naplam_CallIn_DLC02"
				barrage.AudioEventEnd = "Play_Barrage_Napalm_Exit_DLC02"
				barrage.AudioEventLaunched = "Play_Barrage_Napalm_TargetSet_DLC02"
				barrage.AudioEventFire = "Play_Barrage_Napalm_Use_DLC02"
				// dont use
				//barrage.AddUsableName( barrage.WaveListName ) // wave units are given name of wavelist
				barrage.SetPathSigml( OwnerEntity.FirstChildWithName( "b52ussr_barrage" ) )
				player.AddBarrage( barrage )
			}
		}
	}
	
	function LazerBarrageImp( player, spawnPtName )
	{	
		if( !player.UnitLocked( "BARRAGE_LAZER" ) && !GameApp.PAXDemoMode && player.HasDLC( DLC_EVIL_EMPIRE ) )
		{
			if( player.Team == TEAM_BLUE )
			{ 
				print( "added blue lazer" )
				local barrage2 = TargetedLaserBarrage( )
				barrage2.DevName = "BARRAGE_LAZER"
				barrage2.Name = "BARRAGE_LAZER"
				barrage2.IconPath = "gui/textures/barrage/barrage_lazer_g.png"
				barrage2.WeaponID = "ARTILLERY_BARRAGE_LAZER"
				barrage2.SetSpawnPtName( "artillerySpawn" )
				barrage2.NumberOfExplosions = 1
				barrage2.DelayMin = 0.1
				barrage2.DelayMax = 0.2
				barrage2.SpawnStraightOver = true
				barrage2.AudioEventSelected = "Play_Barrage_Laser_Available_DLC01"
				barrage2.AudioEventBegin = "Play_Barrage_Laser_CallIn_DLC01"
				barrage2.AudioEventEnd = "Play_Barrage_Laser_Exit_DLC01"
				barrage2.AudioEventTargetSet = "Play_Barrage_Laser_TargetSet_DLC01"
				barrage2.AudioEventUse = "Play_Barrage_Laser_Use_DLC01"
				barrage2.KillStatToIncrement = SESSION_STATS_MOST_KILLS_WITH_A_NUKE
				barrage2.SetPlayer( player )
				player.AddBarrage( barrage2 )
			}
			else
			{
				print( "added red lazer" )
				local barrage2 = TargetedLaserBarrage( )
				barrage2.DevName = "BARRAGE_LAZER"
				barrage2.Name = "BARRAGE_LAZER"
				barrage2.IconPath = "gui/textures/barrage/barrage_lazer_g.png"
				barrage2.WeaponID = "ARTILLERY_BARRAGE_LAZER"
				barrage2.SetSpawnPtName( "artillerySpawn_USSR" )		
				barrage2.NumberOfExplosions = 1
				barrage2.DelayMin = 0.1
				barrage2.DelayMax = 0.2
				barrage2.SpawnStraightOver = true
				barrage2.AudioEventSelected = "Play_Barrage_Laser_Available_DLC01"
				barrage2.AudioEventBegin = "Play_Barrage_Laser_CallIn_DLC01"
				barrage2.AudioEventEnd = "Play_Barrage_Laser_Exit_DLC01"
				barrage2.AudioEventTargetSet = "Play_Barrage_Laser_TargetSet_DLC01"
				barrage2.AudioEventUse = "Play_Barrage_Laser_Use_DLC01"
				barrage2.KillStatToIncrement = SESSION_STATS_MOST_KILLS_WITH_A_NUKE
				barrage2.SetPlayer( player )
				player.AddBarrage( barrage2 )
			}
		}
	}
	
	function AC130BarrageImp( player, dropPts )
	{	
		if( !player.UnitLocked( "BARRAGE_AC130" ) )
		{	 
			if( player.Team == TEAM_BLUE )
			{ 
				local barrage = ScriptGunShipBarrage( )
				barrage.DevName = "BARRAGE_AC130"
				barrage.Name = "BARRAGE_AC130"
				barrage.IconPath = "gui/textures/barrage/barrage_ac130_g.png"
				barrage.Duration = 60
				barrage.WaveListName = "AC130Barrage"
				barrage.AudioEventSelected = "Play_Barrage_AC130_Selected"
				barrage.AudioEventBegin = "Play_Barrage_AC130_CallIn"
				barrage.AudioEventEnd = "Play_Barrage_AC130_Exit"
				barrage.AudioEventUse = "Play_Barrage_AC130_Use"
				barrage.AudioEventFire = "Play_Barrage_AC130_Fire"
				barrage.AudioEventKill = "Play_Barrage_AC130_Hit"
				barrage.UsingStatToIncrement = SESSION_STATS_TIME_USING_AC130
				barrage.AddUsableName( "AC130_MG" ) //the order of these determins which one he'll be put in first
				barrage.AddUsableName( "AC130_AUTOGUN" )
				barrage.AddUsableName( "AC130_CANNON" )
				player.AddBarrage( barrage )
			}
			else
			{
				local barrage = ScriptGunShipBarrage( )
				barrage.DevName = "BARRAGE_AC130"
				barrage.Name = "BARRAGE_AC130_USSR"
				barrage.IconPath = "gui/textures/barrage/barrage_ac130_g.png"
				barrage.Duration = 60
				barrage.WaveListName = "AC130BarrageUSSR"
				barrage.AudioEventSelected = "Play_Barrage_AC130_Selected"
				barrage.AudioEventBegin = "Play_Barrage_AC130_CallIn"
				barrage.AudioEventEnd = "Play_Barrage_AC130_Exit"
				barrage.AudioEventUse = "Play_Barrage_AC130_Use"
				barrage.AudioEventFire = "Play_Barrage_AC130_Fire"
				barrage.AudioEventKill = "Play_Barrage_AC130_Hit"
				barrage.UsingStatToIncrement = SESSION_STATS_TIME_USING_AC130
				barrage.AddUsableName( "AC130_MG_USSR" ) //the order of these determins which one he'll be put in first
				barrage.AddUsableName( "AC130_AUTOGUN_USSR" )
				barrage.AddUsableName( "AC130_CANNON_USSR" )
				player.AddBarrage( barrage )	
			}
		}
	}

	function AddOffensiveWaveUSABarrage( )
	{
		if( GameApp.CurrentLevelLoadInfo.ChallengeBarrages )
			AddOffensiveWave( offensiveWave_BarrageUSA )
	}

	function AddOffensiveWaveUSSRBarrage( )
	{
		if( GameApp.CurrentLevelLoadInfo.ChallengeBarrages )
			AddOffensiveWave( offensiveWave_BarrageUSSR )
	}
	
	function FillLoadLevelInfo( info )
	{		
		local currentInfo = ::GameApp.CurrentLevelLoadInfo;
		
		if( currentInfo.GameMode.IsCoOp )
			info.GameMode.AddCoOpFlag( )
		if( currentInfo.GameMode.IsNet )
			info.GameMode.AddOnlineFlag( )
		info.Difficulty = currentInfo.Difficulty
		info.ChallengeMode = currentInfo.ChallengeMode
	}
	
	function LoadNextLevel( )
	{
		local nextLevel = ::GameApp.NextCampaignLevel( LevelNumber, DlcNumber )
		local type = MapType
		local dlc = DlcNumber
		
		if( ::GameApp.E3Mode && type == MAP_TYPE_CAMPAIGN && dlc == DLC_COLD_WAR && nextLevel > 3 )
		{
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
			return
		}
		
		if( (nextLevel < 0) || (type != MAP_TYPE_CAMPAIGN)  )
		{
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
			return
		}

		::GameApp.LoadLevel( type, nextLevel, FillLoadLevelInfo.bindenv( this ) )
	}
	
	function SetupSurvival( )
	{		
		if( MapType == MAP_TYPE_SURVIVAL )
		{
			local mode = ::GameApp.GetChallengeMode( )
			if( mode == CHALLENGE_MODE_LOCKDOWN )
			{
				TutEnablePlatformLocking = 1
			}
			else if( mode == CHALLENGE_MODE_HARDCORE )
			{
				TutDisableRepair = 1
				TutDisableVehicleRespawn = 1
			}
			else if( mode == CHALLENGE_MODE_TRAUMA )
			{
				showTraumaTut = true
			}
			else if( mode == CHALLENGE_MODE_COMMANDO )
			{
				// Do Commando Setup
			}
		}
	}
	
	function SetSurvivalWaves( mainWave, specialWave )
	{
		survivalMainWave = mainWave
		survivalMainWave.SetLooping( true )
		
		survivalSpecialWave = specialWave
		survivalSpecialWave.BonusWave = 1
		survivalSpecialWave.SetLooping( false )		
	}
	
	function SurvivalStart( )
	{
		survivalMainWave.Activate( )
	}
	
	function StartBonusWaves( )
	{
		// call this in your main survival wave list when you want to start seeing bonus waves
		//  note it shouldnt actually do anything other than setting a flag
		//print( "bonus start" )
		survivalMainWave.BonusWaveActive = 1
	}
	
	function HandleLogicEventScript( event )
	{
		if( event.Id == GAME_EVENT_TUTORIAL_EVENT )
		{
			local te = TutorialEvent.Convert( event.Context )
			ProcessTutorialEvent( te )
			
			if( te.EventID == TUTORIAL_EVENT_UNIT_BUILT )
			{
				if( MapType == MAP_TYPE_SURVIVAL && ::GameApp.GetChallengeMode( ) == CHALLENGE_MODE_TRAUMA )
					te.Player.SubtractTickets( 1 )
			}
		}
	}
	
	function PlayerKilledWithOneOfTheseUnits( event, targetType, unitIDs )
	{
		if( event.EventID == TUTORIAL_EVENT_UNIT_DESTROYED && !is_null( event.Player ) )
		{
			//print( "i killed them" )
			if( targetType == -1 || event.Entity.Logic.UnitType == targetType )
			{
				local myUnit = event.PlayerKillerLogic
				
				if( !is_null( myUnit ) )
				{
					//print( "i'm in a unit" )
					local myUnitID = myUnit.UnitID
					for( local i = 0; i < unitIDs.len( ); ++i )
					{
						if( myUnitID == unitIDs[ i ] )
							return true
					}
				}
			}
		}
		
		return false
	}
	
	function PlayerKilledParticularUnitWithOneOfTheseUnits( event, targetIDs, playerUnitIDs )
	{
		if( event.EventID == TUTORIAL_EVENT_UNIT_DESTROYED && !is_null( event.Player ) )
		{
			local myUnit = event.PlayerKillerLogic
			{
				if( targetIDs == -1 || UnitIsOneOfTheseIds( event.Entity.Logic.UnitID, targetIDs ) )
				{
					if( !is_null( myUnit ) )
					{
						local myUnitID = myUnit.UnitID
						if( playerUnitIDs == -1 )
							return true
						for( local i = 0; i < playerUnitIDs.len( ); ++i )
						{
							if( myUnitID == playerUnitIDs[ i ] )
								return true
						}
					}
				}
			}
		}

		return false
	}
	
	function UnitIsOneOfTheseIds( unitID, targetIDs )
	{
		if( typeof targetIDs == "integer" || typeof targetIDs == "float" )
		{
			return unitID == targetIDs
		}
		else if( typeof targetIDs == "array" )
		{
			foreach( id in targetIDs )
			{
				if( unitID == id )
					return true
			}
		}
		
		return false
	}
		
	function PlayerKilledTheseUnitsWithOneOfTheseUnits( event, targetTypes, playerUnitIDs )
	{
		if( event.EventID == TUTORIAL_EVENT_UNIT_DESTROYED && !is_null( event.Player ) )
		{
			//print( "i killed them" )
			local myUnit = event.PlayerKillerLogic
			
			foreach( targetType in targetTypes )
			{
				if( targetType == -1 || event.Entity.Logic.UnitType == targetType )
				{
					if( !is_null( myUnit ) )
					{
						//print( "i'm in a unit" )
						local myUnitID = myUnit.UnitID
						for( local i = 0; i < playerUnitIDs.len( ); ++i )
						{
							if( myUnitID == playerUnitIDs[ i ] )
								return true
						}
					}
				}
			}
		}
		
		return false
	}
	
	function PlayerKilledWithOneOfTheseWeapons( event, targetType, weapons )
	{
		if( event.EventID == TUTORIAL_EVENT_UNIT_DESTROYED && !is_null( event.Player ) )
		{
			//print( "i killed them" )
			if( targetType == -1 || event.Entity.Logic.UnitType == targetType )
			{
				local myUnit = event.PlayerKillerLogic				
				if( !is_null( myUnit ) )
				{
					if( weapons == null || weapons == -1 )
						return true
					
					if( typeof weapons == "array" )
					{
						for( local i = 0; i < weapons.len( ); ++i )
						{
							if( event.WeaponID == weapons[ i ] )
								return true
						}
					}
					else if( typeof weapons == "string" )
					{
						return ( event.WeaponID == weapons )
					}
				}
			}
		}
		
		return false
	}
	
	function PlayerOverkillWithOneOfTheseWeapons( event, targetType, weapons )
	{
		return PlayerKillWithTargetWeaponFlag( event, targetType, weapons, KILL_FLAG_OVERKILL )
	}
	
	function PlayerAssistWithOneOfTheseWeapons( event, targetType, weapons )
	{
		return PlayerKillWithTargetWeaponFlag( event, targetType, weapons, KILL_FLAG_ASSIST )
	}
	
	function PlayerSpeedBonusWithOneOfTheseWeapons( event, targetType, weapons )
	{
		return PlayerKillWithTargetWeaponFlag( event, targetType, weapons, KILL_FLAG_SPEEDBONUS )
	}
	
	function PlayerKillWithTargetWeaponFlag( event, targetType, weapons, flag )
	{
		return ( PlayerKilledWithOneOfTheseWeapons( event, targetType, weapons ) && event.CheckFlag( flag ) )
	}
	
	function CheckDecorations( event )
	{
		foreach( data in decorationData )
		{
			if( data != null )
				data.Check( event )
		}
	}
	
	function InitDecoration( index, max, condition = null )
	{
		if( index in decorationData )
		{
			decorationData[ index ] = ::DecorationData( index, max, condition, this )
		}
		else
		{
			::print( "Decoration Error: Tried to initialize index:" + ( index == null ? "null" : index.tostring( ) ) + " but it does not exist" )
			return
		}
	}
	
	function IncDecoration( index )
	{
		if( !( index in decorationData ) )
		{
			::print( "Decoration Error: Tried to increment index:" + ( index == null ? "null" : index.tostring( ) ) + " but it does not exist" )
			return
		}
		else if( decorationData[ index ] == null )
		{
			::print( "Decoration Error: Tried to increment index:" + ( index == null ? "null" : index.tostring( ) ) + " but it is null. Wtf?" )
			return
		}
		
		decorationData[ index ].Inc( )
	}
	
	function AwardDecoration( index )
	{
		if( !( index in decorationData ) )
		{
			::print( "Decoration Error: Tried to award index:" + ( index == null ? "null" : index.tostring( ) ) + " but it does not exist" )
			return
		}
		else if( decorationData[ index ] == null )
		{
			::print( "Decoration Error: Tried to award index:" + ( index == null ? "null" : index.tostring( ) ) + " but it is null. Wtf?" )
			return
		}
		
		decorationData[ index ].Award( )
	}
	
	function InitChallenge( condition )
	{
		challengeCondition = condition
	}
	
	function CheckChallenge( event )
	{
		if( challengeCondition != null && challengeCondition( event ) )
			IncRankProgress( event.Player, 1 )
	}

	// override this if you need custom behavior
	function ProcessTutorialEvent( event )
	{
		CheckDecorations( event )
		CheckChallenge( event )
		
		if( event.EventID == TUTORIAL_EVENT_BARRAGE_ACTIVATED && event.WeaponID == "BARRAGE_NAPALM" && ::GameApp.GameMode.IsVersus )
		{
			if( !is_null( event.Player ) && event.Player.HasDLC( 1 ) )
				event.Player.AwardAchievement( ACHIEVEMENTS_DLC2_NAPALM_IN_VERSUS )
		}
	}
	
	// override this if you need custom behavior
	function PlatformPurchased( entity )
	{
		PlatformPrice *= 2.0
	}
	
	function ControllingPlayer( )
	{
		if(  ::GameApp.SingleScreenCoop )
			return ::GameApp.GetPlayer( ::GameApp.SingleScreenControlIndex )
		else
			return ::GameApp.GetPlayer( 0 )
	}
	
	function PlayerInProximity( player, proximityLogic )
	{
		if( !is_null( player.CurrentUnit )  )
		{
			for( local i = 0; i < proximityLogic.Proximity.EntityCount( ); ++i )
			{
				local logic = proximityLogic.Proximity.GetEntity( i ).Logic
				if( logic && logic.UnderUserControl )
				{
					return true
				}
			}
		}
		
		return false
	}
	
	function VehicleReachedVolume( proximityLogic )
	{
		local player = ControllingPlayer( )
		if( PlayerInProximity( player, proximityLogic ) )
		{
			player.CurrentUnit.Destroy( )
		
			local bonusEvent = LogicEvent.Construct( GAME_EVENT_TUTORIAL_EVENT )
			bonusEvent.Setup( GAME_EVENT_TUTORIAL_EVENT, TutorialEvent.Construct( TUTORIAL_EVENT_MINIGAME_BONUS, 2 ) )
			HandleLogicEvent( bonusEvent )
			
			HandleTutorialEvent( TUTORIAL_EVENT_MINIGAME_DEFEAT )
			
			player.PushTiltShiftCamera( )
		}
		
		//GameApp.CurrentLevel.VictoriousPlayer = player
		//MoveToGuiScreen( CampaignVictoryScreen( player, null ) )
	}
	
	function VehicleMiniGameDestroyed( logic )
	{
		HandleTutorialEvent( TUTORIAL_EVENT_MINIGAME_DEFEAT )
		
		local player = ControllingPlayer( )
		player.PushTiltShiftCamera( )
		
		//GameApp.CurrentLevel.DefeatedPlayer = player
		//GameApp.AddTimedCallback( 2.5, GameEnded.bindenv( this ) ) //delay defeat, in seconds	
	}
	
	function VehShutdown( logic )
	{
		
	}
	
	function MiniGameTurretDestroyed( logic )
	{
		return true //true if the turret should be deleted
	}
	
	function ExtraMiniGamePoints( points )
	{
		if( scoreUI && miniGameActive )
			scoreUI.AddPoints( points )
	}
	
	function AddMiniGameTime( time )
	{
		if( scoreUI && miniGameActive )
		{
			HandleTutorialEventObj( TutorialEvent.Construct( TUTORIAL_EVENT_TIME, time.tointeger( ) ) )
			scoreUI.AddTime( time )
		}
	}
	
	function CommonMiniGamePlayerSetup( player )
	{
		player.ShowScoreUI( false )
		player.ShowMiniMap( false )
		player.ShowOverchargeMeter( false )	
	}
	
	function CommonMiniGameSetup( )
	{
		// KEEP THIS UP TO DATE WITH RESET
		TutDisableRewind = 1
		TutDisableBarrage = 1
		TutDisableRandomPickups = 1
		TutDisableLaunchArrows = 1
		TutAllowCoopTurrets = 1
		TutDisableRTSRings = 1
		
		miniGamePlays = 0
		
		::GameApp.DifficultyOverride = DIFFICULTY_CASUAL
	}
	
	function CommonMiniGameReset( )
	{
		// KEEP THIS UP TO DATE WITH SETUP
		TutDisableRewind = 0
		TutDisableBarrage = 0
		TutDisableRandomPickups = 0
		TutDisableLaunchArrows = 0
		TutAllowCoopTurrets = 0
		
		miniGamePlays = 0	
		
		::GameApp.DifficultyOverride = -1
	}
	
	function CommonHTHSetup( )
	{
		TutDisableRewind = 1
		TutDisableRandomPickups = 1
	}
		
	function CommonMiniGameStartStop( start, preStart, waves, stage )
	{
		if( start )
		{
			if( preStart && stage == 0 )
			{
				SetCurrentLevelProgression( LEVEL_PROGRESSION_MINIGAME )
				++miniGamePlays
				if( miniGamePlays == 3 && ::GameApp.GameMode.IsSinglePlayer )
					ControllingPlayer( ).AwardAchievement( ACHIEVEMENTS_PERSISTENCE )
			}					
				
			if( preStart && stage == 1 )
			{
				local player = GameApp.GetPlayer( 0 )
				player.CancelOverCharge( )
				player.ResetAmmo( )		
			}
			
			miniGameActive = true
			
			if( !preStart && stage == 1 )
			{				
				if( GameApp.CurrentLevelLoadInfo.MapType == MAP_TYPE_MINIGAME )
				{
					if( ControllingPlayer( ).GetUserProfile( ).MinigameTriesRemaining == 0 )
						GameApp.DoAskPlayerToBuyGame( )
				}				
			}
		}
		else
		{
			miniGameActive = false
			
			if( stage == 0 )
			{
				// kill waves
				for( local i = 0; i < waves.len( ); ++i )
				{
					waves[ i ].KillEveryone( )
					waves[ i ].Pause( )
					waves[ i ].Reset( )
				}
			}
			else if( stage == 1 )
			{
				SetCurrentLevelProgression( LEVEL_PROGRESSION_PLAYING )
				
				if( GameApp.CurrentLevelLoadInfo.MapType == MAP_TYPE_MINIGAME )
					ControllingPlayer( ).GetUserProfile( ).IncrementMinigameCount( )
			}
		}
	}
	
	function InfantryReachedDeadEnd( logic )
	{
		//this is only called in minigames when an infantry reaches a path that doesnt end in a goalbox.
		logic.OwnerEntity.Delete( )
	}
	
	function MiniGameCargoDropSpawn( logic, index )
	{
		
	}
	
	function MinigamePrompt( )
	{
		//this gets called when the minigame ready thing shows up
		TutDisableVehicleInput = 1
	}
		
	function GiveAvatarAirWave( )
	{
		print( "air wave" )
		GiveAvatarAward( AVATAR_AWARDS_COMMANDO_MULLET )
	}
	
	function GiveAvatarAward( award )
	{
		GameApp.ForEachPlayer( GiveAvatarImp.bindenv( this ), award )
	}
	
	function GiveAvatarImp( player, data )
	{
		player.AwardAvatarAward( data )
	}
	
	function PlaySound( sound )
	{
		::GameApp.AudioEvent( sound )
	}
}

class StageBasedLevelLogic extends GameStandardLevelLogic
{
	stageEvents = null
	currentStage = 0
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		currentStage = 0
		stageEvents = { }
		stageEvents[ 0 ] <- ScriptedLevelEvent( MoveToNextStage.bindenv( this ) )
		stageEvents[ 0 ].AddCondition( "WavesComplete", 1 )
	}
	
	function OnNoWavesOrEnemiesLeft( )
	{
		stageEvents[ currentStage ].UpdateCondition( "WavesComplete" )
	}
	// Sample function for use above
	//function OnUnitDestroyed( unitLogic )
	//{
	//}
	function MoveToNextStage( )
	{
		currentStage += 1
	}
}

class HeadToHeadLevelLogic extends GameStandardLevelLogic
{
	constructor( )
	{
		GameStandardLevelLogic.constructor( )
	}
}
