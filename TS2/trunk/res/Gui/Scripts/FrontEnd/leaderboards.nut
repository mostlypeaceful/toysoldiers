
// Requires
sigimport "gui/scripts/frontend/common.nut"
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/frontend/userstatsscreen.nut"
sigimport "gui/scripts/frontend/leaderboarddisplay.nut"
sigimport "Gui/Scripts/DialogBox/globalmodaldialogbox.nut"
sigimport "gui/scripts/utility/modeutility.nut"

LeaderboardFilterNames <- {
	[ LBFilter.Overall ] = "LB_Filter_Overall",
	[ LBFilter.Friends ] = "LB_Filter_Friends",
	[ LBFilter.MyScore ] = "LB_Filter_MyScore",
}
	
class FrontEndLeaderboardsMenu extends FrontEndStaticScreen
{
	// Display
	displays = null
	
	// Data
	filter = null
	boardIndex = null
	boardIds = null
	levelInfos = null
	noInput = null
	levelInfo = null
	loadedBoards = null
	boardSwitchInputTimer = null
	eventHandler = null
	
	// Static
	static maxLoadedBoards = 1
	static timeBetweenBoardSwitches = 0.5
	
	constructor( boards = null, levelInfo_ = null, initialBoard = 0, eventHandler_ = null )
	{
		::FrontEndStaticScreen.constructor( )
		inputHook = true
		allowFastInput = true
		advancedInputDelay = { [ 0.0 ] = 0.2, [ 1.0 ] = 0.1, [ 4.0 ] = 0.025 }
		noInput = false
		boardIds = boards
		filter = LBFilter.Friends
		boardIndex = initialBoard
		levelInfo = levelInfo_
		loadedBoards = 0
		boardSwitchInputTimer = 0
		IgnoreBoundsChange = 1
		eventHandler = eventHandler_
	}
	
	function FinalizeIconSetup( )
	{
		if( boardIds == null )
			GetDefaultLeaderboardIds( )
		else
		{
			levelInfos = [ ]
			foreach( id in boardIds )
				levelInfos.push( levelInfo )
		}
			
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		// Leaderboard Displays
		displays = array( boardIds.len( ), null ) // This array needs to be this long, but no all will be set at first
		foreach( i, id in boardIds )
		{
			displays[ i ] = CreateBoard( id, levelInfo, user )
			loadedBoards++
			if( loadedBoards > maxLoadedBoards )
				break
		}

		// Controls
		if( boardIds.len( ) > 1 )
			secondaryControls.AddControl( [ "Gui/Textures/Gamepad/button_lshoulder_g.png", "Gui/Textures/Gamepad/button_rshoulder_g.png" ], "LB_Boards" )
		
		controls.AddControl( GAMEPAD_BUTTON_A, "LB_Menus_Stats" )
		controls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
		controls.AddControl( GAMEPAD_BUTTON_X, "LB_Menus_Filter" )
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
		
		ChangeBoard( 0 )
		ChangeFilter( 0 )
	}
	
	function CreateBoard( id, levelInfo, user )
	{
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		
		local board = ::LeaderboardDisplay( id, levelInfo, user )
		board.SetPosition( vpRect.Left + 20, vpRect.Top + 110, 0 )
		board.Invisible = true
		AddChild( board )
		return board
	}
	
	function GetDefaultLeaderboardIds( )
	{
		local player = ::GameApp.GetPlayerByUser( user )
		
		boardIds = [ LEADERBOARD_ARCADE, LEADERBOARD_H2H_TOTALS ]
		levelInfos = [ null, null ]
		
		local mapTypes = [ MAP_TYPE_CAMPAIGN, MAP_TYPE_SURVIVAL, MAP_TYPE_MINIGAME, MAP_TYPE_HEADTOHEAD ]
		
		foreach( mapType in mapTypes )
		{
			for( local levelIndex = 0; levelIndex < ::GameApp.NumLevelsInTable( mapType ); ++levelIndex )
			{
				local levelInfo = ::GameApp.GetLevelLoadInfo( mapType, levelIndex )
				local dlc = levelInfo.DlcNumber
				if( player.HasDLC( dlc ) )
				{
					for( local mode = 0; mode < ::ModeCount( mapType ); ++mode )
					{
						if( mapType == MAP_TYPE_SURVIVAL )
						{
							if( mode == CHALLENGE_MODE_TRAUMA && !player.HasDLC( DLC_EVIL_EMPIRE ) )
								continue
							if( mode == CHALLENGE_MODE_COMMANDO && !player.HasDLC( DLC_NAPALM ) )
								continue
						}
						local id = ::GameAppSession.GetLevelLeaderboardId( mapType, levelIndex, mode )
						if( id != 0 )
						{
							boardIds.push( id )
							levelInfos.push( levelInfo )
						}
					}
				}
			}
			
			if( mapType == MAP_TYPE_MINIGAME )
			{
				// add mobile leaderboards
				boardIds.push( 1000 + MOBILE_MINIGAME_FLY )
				levelInfos.push( null )
				boardIds.push( 1000 + MOBILE_MINIGAME_HALLWAY )
				levelInfos.push( null )
				boardIds.push( 1000 + MOBILE_MINIGAME_TRIAL_GAME_2 )
				levelInfos.push( null )
				boardIds.push( 1000 + MOBILE_MINIGAME_TOTALS )
				levelInfos.push( null )
			}
		}
	}
	
	function SetUser( _user )
	{
		::FrontEndStaticScreen.SetUser( _user )
		if( !user.SignedInOnline )
		{
			WarnNoLive( )
		}		
	}
	
	function WarnNoLive( locString = "Menus_NoXboxLiveNoLeaderboards" )
	{
		local dialogBox = ::ModalInfoBox( locString, user, "Accept" )

		dialogBox.onAPress = function( )
		{
			::ResetGlobalDialogBoxSystem( )
			AutoBackOut = true
		}.bindenv(this)
	}
	
	function HandleCanvasEvent( event )
	{
		if( event.Id == ON_PLAYER_NO_LIVE || event.Id == ON_PLAYER_SIGN_OUT )
		{
			WarnNoLive( "Menus_XboxLiveConnectionLost" )
			return ::GameApp.GameMode.IsFrontEnd
		}
		else if( event.Id == SESSION_LOAD_LEVEL )
		{
			AutoExit = true
			return true
		}
		else if( eventHandler && eventHandler.HandleCanvasEvent( event ) )
			return true;
		
		return ::FrontEndStaticScreen.HandleCanvasEvent( event )
	}
	
	function OnBackOut( )
	{
		noInput = true
		return ::FrontEndStaticScreen.OnBackOut( )
	}
	
	function ChangeFilter( delta )
	{
		filter += delta
		if( filter == LBFilter.LBFilterCount )
			filter = LBFilter.FirstUnused + 1
		displays[ boardIndex ].ChangeFilter( filter )
		SetSubtitle( ::LeaderboardFilterNames[ filter ] )
	}
	
	function ChangeBoard( delta )
	{
		if( displays.len( ) == 0 )
			return
		
		local display = displays[ boardIndex ]
			
		// Hide the current board
		if( display != null )
		{
			display.Invisible = true
			display.onCountChange = null
			
			if( delta != 0 && loadedBoards + 1 > maxLoadedBoards )
			{
				loadedBoards--
				display.DeleteSelf( )
				RemoveChild( display )
				displays[ boardIndex ] = null
			}
		}
		
		boardIndex += delta;
		if( boardIndex < 0 )
			boardIndex = boardIds.len( ) - 1
		else if( boardIndex >= displays.len( ) )
			boardIndex = 0
		
		display = displays[ boardIndex ]
		
		if( display == null )
		{
			loadedBoards++
			local newBoard = CreateBoard( boardIds[ boardIndex ], levelInfos[ boardIndex ], user )
			displays[ boardIndex ] = newBoard
			display = newBoard
		}
		
		display.StartReading( )
		
		// Show the board
		display.Invisible = false
		display.ChangeFilter( filter )
		display.onCountChange = OnCountChange.bindenv( this )
		
		// Update the title
		SetTitle( display.name )
		OnCountChange( )
	}
	
	function OnCountChange( )
	{
		local display = displays[ boardIndex ]
		SetDescriptorText( display.CountText( ) )
	}
	
	function OnTick( dt )
	{
		boardSwitchInputTimer += dt
		::FrontEndStaticScreen.OnTick( dt )
	}
	
	function HandleInput( gamepad )
	{
		if( noInput )
			return
			
		local board = displays[ boardIndex ]
		
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_X ) )
		{
			PlaySound( "Play_UI_Select_Forward" )
			ChangeFilter( 1 )
		}
		
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_A ) )
		{
			local userID = board.HighlightedUserId( )
			local gamertag = board.HighlightedGamertag( )
			if( userID && gamertag )
			{
				PlaySound( "Play_UI_Select_Forward" )
				PushNextMenu( ::UserStatsScreen( userID, gamertag ) )
				AutoAdvance = true
			}
			else
			{
				PlaySound( "Play_HUD_WeaponMenu_Error" )
			}
		}
		
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_SELECT ) && !::GameApp.GamercardViewDisabled( ) )
		{
			if( user.IsLocal )
			{
				PlaySound( "Play_UI_Select_Forward" )
				if( !board.ShowGamerCard( ) )
					PlaySound( "Play_HUD_WeaponMenu_Error" )
			}
		}
		
		if( boardSwitchInputTimer > timeBetweenBoardSwitches )
		{
			local buttonPressed = false
			
			if( gamepad.ButtonHeld( GAMEPAD_BUTTON_LSHOULDER ) )
			{
				buttonPressed = true
				ChangeBoard( -1 )
			}
			else if( gamepad.ButtonHeld( GAMEPAD_BUTTON_RSHOULDER ) )
			{
				buttonPressed = true
				ChangeBoard( 1 )
			}
			
			if( buttonPressed )
			{
				PlaySound( "Play_UI_Select_Forward" )
				boardSwitchInputTimer = 0
			}
		}
	}

	function ChangeHighlight( indexDelta )
	{
		displays[ boardIndex ].ChangeHighlight( indexDelta )
	}
	
	function SelectActiveIcon( )
	{
		return true
	}
}

