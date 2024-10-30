// Level Select Display

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut" 
sigimport "gui/scripts/controls/rollingmenu.nut"
sigimport "gui/scripts/frontend/levelselect/profilebadge.nut"
sigimport "gui/scripts/controls/controllerbuttoncontainer.nut"
sigimport "gui/scripts/frontend/levelselect/levelpreviewpanel.nut"
sigimport "gui/scripts/frontend/levelselect/medalpanel.nut"
sigimport "gui/scripts/frontend/levelselect/goalpanel.nut"
sigimport "gui/scripts/frontend/levelselect/friendleaderboardpanel.nut"
sigimport "gui/scripts/frontend/levelselect/levelscorespanel.nut"
sigimport "gui/scripts/frontend/levelselect/leveldisplay.nut"
sigimport "gui/scripts/frontend/levelselect/versusoptionspanel.nut"
sigimport "gui/scripts/frontend/levelselect/minigamescorespanel.nut"
sigimport "gui/scripts/frontend/levelselect/tipspanel.nut"
sigimport "gui/scripts/frontend/frontendscripts.nut"
sigimport "gui/scripts/hud/trialrestrictionui.nut"
sigimport "gui/scripts/utility/goldenarcadescripts.nut"
sigimport "gui/scripts/dialogbox/suspendedgamedialogbox.nut"

// Resources

class LevelSelectDisplay extends RollingMenu
{
	// Display
	background = null // Gui.TexturedQuad
	background2 = null // Gui.TexturedQuad
	dlcLabel = null // Gui.Text
	controls = null // ControllerButtonContainer, to show controls at the bottom
	navControls = null // ControllerButtonContainer, to show the navigation for tabs and stuff
	scores = null // LevelScoresPanel
	preview = null // LevelPreviewPanel
	medals = null // MedalPanel
	goals = null // GoalPanel
	friendScores = null // FriendLeaderboardPanel
	tips = null
	levels = null // LevelDisplay
	descriptor = null // Gui.Text
	goldenArcadePanel = null
	restrictUI = null
	resumeDialog = null

	// Data
	mapType = null
	dlc = null
	levelInfos = null // array of LevelLoadInfo(C++) objects
	player1Scores = null // array of LevelScores(C++) objects
	player2Scores = null // array of LevelScores(C++) objects
	initialSelectedLevel = null
	suspendedLevelIndex = null
	suspendedWaveIndex = null
	allowSuspendedPlay = null
	player = null
	dialogOpen = null
	hasDlcFunc = null
	
	// Events
	onHighlightChange = null
	
	constructor( mapType_, dlc_, hasDlc = null )
	{
		::RollingMenu.constructor( )
		inputHook = true
		
		mapType = mapType_
		dlc = dlc_
		dialogOpen = false
		hasDlcFunc = ( hasDlc == null ? HasDlc : hasDlc )
	}
	
	function FinalizeIconSetup( )
	{
		local wideLanguage = ::GameApp.IsWideLanguage( )
		
		player = ::GameApp.GetPlayerByUser( user )
		audioSource = player.AudioSource
		allowSuspendedPlay = true
		
		initialSelectedLevel = 0
		local profile = player.GetUserProfile( )
		if( mapType == MAP_TYPE_CAMPAIGN )
		{
			//::print( "profile.HighestLevelReached(" + dlc.tostring( ) + "):" + profile.HighestLevelReached( dlc ).tostring( ) )
			initialSelectedLevel = ::Math.Clamp( profile.HighestLevelReached( dlc ), 0, ::GameApp.NumLevelsInTable( mapType ) - 1 )
		}
		else if( mapType == MAP_TYPE_MINIGAME && !::GameApp.IsFullVersion )
			initialSelectedLevel = 5
		else if( mapType == MAP_TYPE_MINIGAME && ::GameApp.E3Mode )
			initialSelectedLevel = 3
		
		local initialScores = null
		local initialLevelInfo = null
		
		player.DoesntWantRewind = 0 //we're considering rewinding again.
		
		GetLevelScores( player )
		foreach( i, info in levelInfos )
		{
			if( info.LevelIndex == initialSelectedLevel )
			{
				initialLevelInfo = info
				initialScores = player1Scores[ i ]
				break
			}
		}
		
		if( !initialLevelInfo && levelInfos.len( ) > 0 )
		{
			initialLevelInfo = levelInfos[ 0 ]
			initialScores = player1Scores[ 0 ]
		}
		
		local profile = player.GetUserProfile( )
		if( mapType == MAP_TYPE_CAMPAIGN && profile.LastPlayedRewindValid && ::GameApp.GameMode.IsFrontEnd )
		{
			suspendedLevelIndex = profile.LastPlayedRewindLevel
			suspendedWaveIndex = profile.LastPlayedRewindWave
			
			if( suspendedLevelIndex == 0 )
			{
				suspendedLevelIndex = null
				suspendedWaveIndex = null
			}
		}
		
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		menuPositionOffset.x = vpSafeRect.Left + 30
		menuPositionOffset.y = vpSafeRect.Top + 241
		
		background = ::Gui.ColoredQuad( )
		background.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		background.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		background.SetPosition( 0, 0, 0.09 )
		AddChild( background )
		
		controls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL, 20 )
		controls.AddControl( "Gui/Textures/Gamepad/button_b_g.png", "Menus_Back" )
		navControls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL, 15 )
		navControls.AddControl( "gui/textures/gamepad/button_rtrigger_g.png", "Toggle_Player_Score" )
		controls.SetPosition( vpSafeRect.Left, vpSafeRect.Bottom - 12, 0 )
		navControls.SetPosition( controls.GetXPos( ), controls.GetYPos( ) - 26, 0 )
		AddChild( controls )
		AddChild( navControls )
		
		// Description Text
		descriptor = Gui.Text( )
		descriptor.SetFontById( FONT_SIMPLE_SMALL )
		descriptor.SetRgba( COLOR_CLEAN_WHITE )
		descriptor.SetPosition( vpSafeRect.Right, vpSafeRect.Bottom - descriptor.LineHeight, 0 )
		AddChild( descriptor )
		
		local startX = vpSafeRect.Left
		local startY = vpSafeRect.Top + 120
		
		background2 = ::Gui.AsyncTexturedQuad( )
		if( mapType == MAP_TYPE_CAMPAIGN || mapType == MAP_TYPE_SURVIVAL || mapType == MAP_TYPE_MINIGAME )
			background2.SetTexture( "gui/textures/frontend/levelselect_background2_g.png" )
		else if( mapType == MAP_TYPE_HEADTOHEAD )
			background2.SetTexture( "gui/textures/frontend/versus_custom_bg_d.png" )
		else
			background2.SetTexture( "gui/textures/frontend/versus_custom_bg_d.png" )
		background2.SetPosition( Math.Vec3.Construct( startX, startY, 0.04 ) )
		AddChild( background2 )
		
		if( dlc != null )
		{
			dlcLabel = ::Gui.Text( )
			dlcLabel.SetFontById( FONT_FANCY_MED )
			dlcLabel.SetRgba( COLOR_CLEAN_WHITE )
			dlcLabel.SetPosition( startX + 20, startY - 40, 0 )
			UpdateDlcName( )
			AddChild( dlcLabel )
		}

		levels = ::LevelDisplay( mapType, 0 )
		levels.SetPosition( startX + 0, startY + 10, 0 )
		AddChild( levels )
		
		// Trial restriction
		if( !::GameApp.IsFullVersion && ( mapType == MAP_TYPE_SURVIVAL || mapType == MAP_TYPE_MINIGAME ) )
		{			
			restrictUI = ::TrialRestrictionUI( player )
			restrictUI.mapType = mapType
			restrictUI.SetPosition( vpSafeRect.Center.x + 80, vpSafeRect.Top + 50, 0 )
			AddChild( restrictUI )
		}
		
		if( mapType == MAP_TYPE_CAMPAIGN )
			scores = ::CampaignLevelScoresPanel( initialScores, dlc )
		else if( mapType == MAP_TYPE_SURVIVAL )
			scores = ::ChallengeLevelScoresPanel( initialScores, player, hasDlcFunc )
		else if( mapType == MAP_TYPE_MINIGAME )
			scores = ::MinigameScoresPanel( initialScores, initialLevelInfo )
		else if( mapType == MAP_TYPE_HEADTOHEAD )
			scores = ::VersusOptionsPanel( audioSource )		
		
		if( scores )
		{
			scores.audioSource = audioSource
			scores.SetPosition( startX + 401, startY + 10, 0 )
			AddChild( scores )
		}
			
		if( initialLevelInfo != null )
		{
			preview = ::LevelPreviewPanel( initialLevelInfo, initialScores, LevelLocked.bindenv( this ) )
			preview.SetPosition( startX + 1, startY + 238 - 2, 0 )
			AddChild( preview )
			
			if( mapType != null )
			{
				player.Stats.ClearLevelData( )
				friendScores = ::FriendLeaderboardPanel( player.User.GamerTag, player.Stats, initialLevelInfo, initialScores, DefaultGameMode( ) )
				friendScores.SetPosition( startX + 770, startY + 151, 0 )
				AddChild( friendScores )
			}
			
			if( mapType == MAP_TYPE_HEADTOHEAD )
			{
				friendScores.SetYPos( startY + 10 )
			}
			else if( mapType != null )
			{
				medals = ::MedalPanel( player.Stats, initialLevelInfo, initialScores )
				medals.SetPosition( startX + 401, startY + 151, 0 )
				AddChild( medals )
			}
			
			if( mapType == MAP_TYPE_CAMPAIGN )
			{
				goals = ::GoalPanel( initialLevelInfo, initialScores )
				goals.SetPosition( startX + 401, startY + 360, 0 )
				AddChild( goals )
				
				if( dlc == DLC_COLD_WAR )
					goldenArcadePanel = ::GoldenArcadePanel( initialScores )
				else if( dlc == DLC_EVIL_EMPIRE )
					goldenArcadePanel = ::GoldenBabushkaPanel( initialScores )
				else if( dlc == DLC_NAPALM )
					goldenArcadePanel = ::GoldenDogTagPanel( initialScores )
				goldenArcadePanel.SetPosition( vpSafeRect.Right - 32, vpSafeRect.Bottom - 100 + 38, 0 )
				AddChild( goldenArcadePanel )
			}
			else if( mapType != null )
			{
				tips = ::TipsPanel( initialLevelInfo )
				tips.SetPosition( startX + 411, startY + 360, 0 )
				AddChild( tips )
			}
		}
		
		IconSetup( )
	}
	
	function UnloadResources( )
	{
		background2.Unload( )
		if( preview )
			preview.Unload( )
	}
	
	function LevelLocked( scores, levelInfo )
	{
		if( ::Player.RandyMode( ) )
			return false
			
		if( ::GameApp.E3Mode )
		{
			local type = levelInfo.MapType
			local levelIndex = levelInfo.LevelIndex
			if( type == MAP_TYPE_CAMPAIGN && levelIndex > 3 )
				return true
			else if( type == MAP_TYPE_SURVIVAL && levelIndex != 0 )
				return true
			else if( type == MAP_TYPE_MINIGAME && levelIndex != 3 && levelIndex != 5 )
				return true
			else
				return false
		}
		
		if( !::GameApp.IsFullVersion )
		{
			if( !levelInfo.AvailableInTrial )
				return true
			else
				return false
		}
		
		if( scores.Locked )
			return true
		else
			return false
	}
	
	function IconSetup( )
	{
		local initialIndex = 0
		foreach( i, levelInfo in levelInfos )
		{
			local player1Score = player1Scores[ i ]
			local image = LEVELSELECT_MENUICON_NONE
			if( LevelLocked( player1Score, levelInfo ) )
				image = LEVELSELECT_MENUICON_LOCKED
			else if( suspendedLevelIndex == levelInfo.LevelIndex )
				image = LEVELSELECT_MENUICON_SUSPENDED
			
			icons.push( ::RollingMenuIcon( levelInfo.MapDisplayName, null, image ) )
			if( levelInfo.LevelIndex == initialSelectedLevel )
				initialIndex = i
		}
		
		::RollingMenu.FinalizeIconSetup( )
		
		InstantHighlight( initialIndex )
		UpdateDescriptorText( )
	}
	
	function AllowSuspendedPlay( allow )
	{
		allowSuspendedPlay = allow
		
		if( resumeDialog && !allowSuspendedPlay )
			::ClearGlobalDialog( resumeDialog )
		
		if( suspendedLevelIndex )
		{
			foreach( i, levelInfo in levelInfos )
			{
				if( suspendedLevelIndex == levelInfo.LevelIndex )
				{
					local icon = icons[ i ]
					if( !player1Scores[ i ].Locked )
						icon.SetIcon( allow? LEVELSELECT_MENUICON_SUSPENDED: LEVELSELECT_MENUICON_NONE )
					break
				}
			}
		}
	}
		
	function UnlockAllLevelIcons( )
	{
		if( mapType != MAP_TYPE_SURVIVAL && mapType != MAP_TYPE_MINIGAME )
			return
			
		foreach( icon in icons )
			icon.SetIcon( LEVELSELECT_MENUICON_NONE )
	}
		
	function ChangeHorizontalHighlight( delta )
	{
		if( scores && scores.active )
		{
			local prevIndex = scores.SelectedIndex( )
			scores.ChangeHorizontalHighlight( delta )
			StatusChangeNotify( )
			if( scores.SelectedIndex( ) != prevIndex )
			{
				if( delta < 0 )
					PlaySound( "Play_UI_Select_Left" )
				else if( delta > 0 )
					PlaySound( "Play_UI_Select_Right" )
				UpdateFriendScores( )
				UpdateDescriptorText( )
			}
		}
	}
	
	function ChangeHighlight( indexDelta, force = false )
	{
		if( !dialogOpen || force )
		{
			if( scores && scores.active )
			{
				scores.ChangeHighlight( indexDelta )
				StatusChangeNotify( )
				UpdateDescriptorText( )
			}
			else
			{
				local prev = highlightIndex
				::RollingMenu.ChangeHighlight( indexDelta )
				
				if( prev != highlightIndex || force )
				{
					UpdateScores( )
					UpdateDescriptorText( )
				}
			}
		}
	}
		
	function SetDescriptorText( textId )
	{
		descriptor.SetAlpha( 1 )
		if( typeof textId == "string" )
			descriptor.BakeLocString( ::GameApp.LocString( textId ), TEXT_ALIGN_RIGHT )
		else if( typeof textId == "instance" )
			descriptor.BakeLocString( textId, TEXT_ALIGN_RIGHT )
		else
			descriptor.SetAlpha( 0 )
	}
	
	function PreviousLevelInfo( levelInfo )
	{
		local prevInfo = null
		foreach( info in levelInfos )
		{
			if( levelInfo.LevelIndex == info.LevelIndex )
				return prevInfo
			
			if( info.DlcNumber == levelInfo.DlcNumber )
				prevInfo = info
		}
		return null
	}
	
	function UpdateDescriptorText( )
	{
		if( !( highlightIndex in levelInfos ) )
			return
			
		local levelInfo = levelInfos[ highlightIndex ]
		local player1Score = player1Scores[ highlightIndex ]
		
		if( scores && scores.active )
		{
			SetDescriptorText( scores.GetDescriptor( ) )
		}
		else
		{
			// Set descriptor text to the helptext for the current level
			if( LevelLocked( player1Score, levelInfo ) )
			{
				if( ::GameApp.E3Mode )
				{
					SetDescriptorText( "LevelSelect_LevelLockedUnknownPrev" )
				}
				else if( ::GameApp.IsFullVersion )
				{
					local prevInfo = PreviousLevelInfo( levelInfo )
					if( prevInfo )
						SetDescriptorText( ::GameApp.LocString( "LevelSelect_LevelLocked" ).Replace( "levelName", prevInfo.MapDisplayName ) )
					else
						SetDescriptorText( "LevelSelect_LevelLockedUnknownPrev" )
				}
				else
				{
					SetDescriptorText( "LevelSelect_LevelLockedTrial" )
				}
			}
			else
			{
				local descId = levelInfo.DescriptionLocId
				if( descId && typeof descId == "string" && descId.len( ) > 0 )
					SetDescriptorText( descId )
			}
		}
	}
	
	function UpdateScores( )
	{
		local levelInfo = levelInfos[ highlightIndex ]
		local player1Score = player1Scores[ highlightIndex ]
		
		if( scores )
			scores.SetLevelInfo( player1Score, levelInfo, player )
		if( preview )
			preview.SetLevelInfo( levelInfo, player1Score )
		if( medals )
			medals.SetLevelInfo( levelInfo, player1Score )
		if( goals )
			goals.SetLevelInfo( levelInfo, player1Score )
		if( tips )
			tips.SetLevelInfo( levelInfo )
		if( goldenArcadePanel )
			goldenArcadePanel.SetLevelInfo( player1Score )
		UpdateFriendScores( )
		
		StatusChangeNotify( )
	}
	
	function HasDlc( player, dlc )
	{
		::print( "  using standard HasDLC" )
		return player.HasDLC( dlc )
	}
	
	function Set( highlightedLevel, scoresActive, highlightedMode, extra ) 
	{
		//::print( "highlight:" + highlightedLevel.tostring( ) + " scoresActive:" + scoresActive.tostring( ) + " scoresHighlight:" + highlightedMode.tostring( ) + " extra:" + extra.tostring( ) )
		
		if( extra != ~0 && mapType == MAP_TYPE_CAMPAIGN )
			ChangeDLC( extra )
			
		if( highlightIndex != highlightedLevel )
			::RollingMenu.HighlightByIndex( highlightedLevel )
		
		if( scores && scores.active != scoresActive )
		{
			levels.SetActive( !scoresActive )
			scores.SetActive( scoresActive )
		}
		
		if( scores && scoresActive )
			scores.SetSelection( highlightedMode )
		
		if( scores && scoresActive && extra != ~0 && mapType == MAP_TYPE_HEADTOHEAD )
			scores.SetExtra( highlightedMode, extra )
		
		UpdateScores( )
		UpdateDescriptorText( )
	}
		
	function UpdateFriendScores( )
	{
		if( friendScores )
		{
			local difficulty = DefaultGameMode( )
			if( scores && scores.active && ( mapType == MAP_TYPE_CAMPAIGN || mapType == MAP_TYPE_SURVIVAL ) )
				difficulty = scores.SelectedIndex( )
			friendScores.SetLevelInfo( levelInfos[ highlightIndex ], player1Scores[ highlightIndex ], difficulty )
		}
	}
	
	function DefaultGameMode( )
	{
		if( mapType == MAP_TYPE_SURVIVAL )
			return CHALLENGE_MODE_SURVIVAL
		if( mapType == MAP_TYPE_HEADTOHEAD )
			return DIFFICULTY_NORMAL
		if( mapType == MAP_TYPE_MINIGAME )
			return DIFFICULTY_CASUAL
		else
			return DIFFICULTY_NORMAL
	}
	
	function StatusChangeNotify( )
	{
		if( onHighlightChange )
		{
			local extra = ~0
			if( mapType == MAP_TYPE_CAMPAIGN && dlc != null )
				extra = dlc
			else if( mapType == MAP_TYPE_HEADTOHEAD && scores )
				extra = scores.GetExtra( )
			
			if( mapType == MAP_TYPE_HEADTOHEAD && scores )
			{
				for( local i = 0; i < scores.Count( ); ++i )
					onHighlightChange( highlightIndex, 1, i, scores.GetExtra( i ), player )
			}
			
			local selectedScoresIndex = ( ( scores ) ? scores.SelectedIndex( ) : 0 )
			local scoresActive = ( ( scores && scores.active == true )? 1: 0 )
			onHighlightChange( highlightIndex, scoresActive, selectedScoresIndex, extra, player )
		}
	}
	
	function UpdateDlcName( )
	{
		if( dlc != null )
		{
			local dlcNames = [ "CampaignName_ColdWar", "CampaignName_SpecOps", "CampaignName_Napalm" ]
			dlcLabel.BakeLocString( ::GameApp.LocString( dlcNames[ dlc ] ) )
		}
	}
	
	function ChangeDLC( dlc_ = null )
	{
		if( scores && !scores.active && dlc_ != dlc )
		{
			//::print( "change dlc" )
			InstantHighlight( 0 )
			dlc = dlc_
			if( "ChangeDLC" in scores )
				scores.ChangeDLC( dlc )
			ForceUpdateLists( )
		}
	}
	
	function ForceUpdateLists( )
	{
		GetLevelScores( player )
		finalized = false
		ClearLevels( )
		IconSetup( )
		::RollingMenu.OnTick( 0 )
		if( icons.len( ) > 0 )
			icons[ 0 ].OnHighlight( true )
		UpdateDlcName( )
		::RollingMenu.ChangeHighlight( -1 )
		UpdateScores( )
	}
	
	function GetLevelScores( player )
	{
		local gameApp = ::GameApp
		levelInfos = [ ]
		local numLevels = gameApp.NumLevelsInTable( mapType )
		for( local i = 0; i < numLevels; ++i )
		{
			local levelInfo = gameApp.GetLevelLoadInfo( mapType, i )
			if( mapType == MAP_TYPE_CAMPAIGN )
			{
				if( levelInfo.DlcNumber == dlc )
					levelInfos.push( levelInfo )
			}
			else
			{
				if( gameApp.IsFullVersion && !gameApp.PAXDemoMode )
				{
					if( HasDlc( player, levelInfo.DlcNumber ) )
					{
						levelInfos.push( levelInfo )
					}
				}
				else
				{
					if( levelInfo.DlcNumber == DLC_COLD_WAR )
						levelInfos.push( levelInfo )
				}
			}
		}
		
		// Sort level info
		levelInfos.sort( function( a, b )
		{
			if( a.DlcNumber > b.DlcNumber )
				return 1
			else if( a.DlcNumber < b.DlcNumber )
				return -1
			else
			{
				if( a.LevelIndex > b.LevelIndex )
					return 1
				else if( a.LevelIndex < b.LevelIndex )
					return -1
				else
					return 0
			}
		} )
		
		player1Scores = { }
		local userProfile = player.GetUserProfile( )
		foreach( i, info in levelInfos )
		{
			local score = userProfile.GetLevelScores( mapType, info.LevelIndex )
			player1Scores[ i ] <- ( is_null( score )? null: score )
		}
	}
	
	function ClearLevels( )
	{
		foreach( i in icons )
		{
			i.onSelectCallback = null
			i.DeleteSelf( )
		}
		icons = [ ]
	}
	
	function HandleInput( gamepad, canStart )
	{
		if( dialogOpen )
			return
			
		// Handle selecting difficulty
		if( scores && scores.active )
		{
			if( canStart && gamepad.ButtonDown( GAMEPAD_BUTTON_A ) )
			{
				// Load level
				if( highlightIndex in levelInfos )
				{
					local info = levelInfos[ highlightIndex ]
					if( TryToLoadLevel( mapType, info.LevelIndex ) )
					{
						PlaySound( "Play_UI_Select_Forward" )
						scores.SetActive( false )
						SetControls( )
						AutoAdvance = true
						return
					}
					else
					{
						PlaySound( errorSound )
					}
				}
			}
			else if( gamepad.ButtonDown( GAMEPAD_BUTTON_B ) )
			{
				TransitionFromScoresToLevels( )
				return
			}
		}
		else
		{
			if( canStart && gamepad.ButtonDown( GAMEPAD_BUTTON_A ) ) 
			{
				local levelInfo = null
				local player1Score = null
				
				if( highlightIndex in levelInfos )
				{
					levelInfo = levelInfos[ highlightIndex ]
					player1Score = player1Scores[ highlightIndex ]
				}
				
				if( levelInfo && !LevelLocked( player1Score, levelInfo ) )
				{
					if( !scores || scores.AutoLaunch( ) )
					{
						if( TryToLoadLevel( levelInfo.MapType, levelInfo.LevelIndex ) )
						{
							PlaySound( "Play_UI_Select_Forward" )
							SetControls( )
							AutoAdvance = true
						}
						else
						{
							PlaySound( errorSound )
						}
						return
					}
					else if( allowSuspendedPlay && levelInfo.LevelIndex == suspendedLevelIndex )
					{
						AskToPlaySuspendedGame( )
					}
					else
					{
						if( mapType == MAP_TYPE_CAMPAIGN && !player.DoesntWantRewind && player.NeedsToChooseSaveDevice )
						{
							player.ChooseSaveDeviceId( 0 )
							player.StorageDeviceSelectionCallback = function( selected )
							{
								//if( selected ) // we want to move them on to the next no matter what they choose.
									TransitionFromLevelsToScores( )
								player.StorageDeviceSelectionCallback = null
							}.bindenv( this )
						}
						else
							TransitionFromLevelsToScores( )
					}
					return
				}
				else
				{
					PlaySound( errorSound )
					return
				}
			}
			else if( gamepad.ButtonDown( GAMEPAD_BUTTON_B ) )
			{
				// Warning message
				if( TryToBackOut( ) )
					AutoBackOut = true
				return
			}
		}
		
		// Change DLC
		if( dlc != null && ( scores && !scores.active ) )
		{
			local newDlc = dlc
			local offset = 0
			if( gamepad.ButtonDown( GAMEPAD_BUTTON_LSHOULDER ) )
				offset = -1
			else if( gamepad.ButtonDown( GAMEPAD_BUTTON_RSHOULDER ) )
				offset = 1
			
			if( offset != 0 )
			{
				do
				{
					if( newDlc + offset > DLC_COUNT )
						newDlc = 0
					else if( newDlc + offset < 0 )
						newDlc = DLC_COUNT - 1
					else
						newDlc += offset
				}
				while( !HasDlc( player, newDlc ) )
				
				PlaySound( "Play_UI_Scroll" )
				ChangeDLC( newDlc )
				return
			}
		}
		
		// Show Leaderboards
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_Y ) )
		{
			ShowLeaderboards( )
		}
	}
	
	function TryToBackOut( )
	{
		return true
	}
	
	function TryToLoadLevel( type, levelIndex )
	{
		if( !::GameApp.GameMode.IsFrontEnd )
		{
			//::print( player.User.GamerTag.ToCString( ) + " trying to load new level" )
			local dialog = ::ModalConfirmationBox( "NewLevel_Confirm", player.User, "Ok", "Cancel" )
			dialog.onAPress = function( ):( type, levelIndex )
			{
				//::print( "got here" )
				LoadLevel( type, levelIndex )
				//if( LoadLevel( type, levelIndex ) )
				//	AutoExit = true
			}.bindenv( this )
			return false
		}
		else
		{
			return LoadLevel( type, levelIndex )
		}
	}
	
	function TransitionFromLevelsToScores( )
	{
		PlaySound( "Play_UI_Select_Forward" )
		levels.SetActive( false )
		if( scores )
			scores.SetActive( true )
		SetControls( )
		StatusChangeNotify( )
		UpdateFriendScores( )
		UpdateDescriptorText( )
	}
	
	function TransitionFromScoresToLevels( )
	{
		// Back to level select
		PlaySound( "Play_UI_Select_Backward" )
		levels.SetActive( true )
		if( scores )
			scores.SetActive( false )
		SetControls( )
		StatusChangeNotify( )
		UpdateFriendScores( )
		UpdateDescriptorText( )
	}
	
	function AskToPlaySuspendedGame( )
	{
		// If the player needs a storage device:
		if( player.NeedsToChooseSaveDevice && !player.DoesntWantRewind )
		{
			player.ChooseSaveDeviceId( 0 )
			player.StorageDeviceSelectionCallback = function( selected )
			{
				if( selected )
					ActuallyAskToPlaySuspendedGame( )
				//else
					//AllowSuspendedPlay( false )
				player.StorageDeviceSelectionCallback = null
			}.bindenv( this )
		}
		else
			ActuallyAskToPlaySuspendedGame( )
	}
	
	function ActuallyAskToPlaySuspendedGame( )
	{
		if( resumeDialog )
			::ClearGlobalDialog( resumeDialog )
			
		// Dialog
		resumeDialog = ::SuspendedGameDialogBox( user )
		PlaySound( "Play_UI_Select_Forward" )
		
		resumeDialog.onFadedOut = function( )
		{
			ActuallyStartSuspendedGame( )
		}.bindenv( this )
		
		resumeDialog.onAPress = function( )
		{
			resumeDialog = null
		}.bindenv( this )
		
		resumeDialog.onCanceled = function( )
		{
			local dialog = ::ModalConfirmationBox( "Error_ResumeRestartConfirm", user, "Menus_CampaignContinue" )
			
			dialog.onFadedOut = function( )
			{
				TransitionFromLevelsToScores( )
				resumeDialog = null
			}.bindenv( this )
			
			local onPress = function( )
			{
				::ClearGlobalDialogBoxQueue( ::LocalGlobalDialogBoxQueue )
				resumeDialog = null
			}.bindenv( this )
			
			dialog.onAPress = onPress
			dialog.onBPress = onPress

		}.bindenv( this )
	}
	
	function ActuallyStartSuspendedGame( )
	{
		if( allowSuspendedPlay )
		{
			::GameApp.Rewind( user, suspendedWaveIndex )
			SetLevelStarting( )
			DialogAnswerExtra( )
		}
	}
	
	function SetLevelStarting( localCoopUserId = null )
	{
		::SetFrontEndLevelSelectRestart( mapType, dlc, localCoopUserId )
		twoPlayerController.levelStarting = true
		nextAction = VerticalMenuAction.ExitStack
	}
	
	function DialogAnswerExtra( )
	{
		SetControls( )
		AutoAdvance = true
	}
	
	function ShowLeaderboards( )
	{
		// Create a new leaderboard object
		local levelInfo = levelInfos[ highlightIndex ]
		local levelIndex = levelInfo.LevelIndex
		local boards = [ ]
		
		local count = 0
		if( mapType == MAP_TYPE_CAMPAIGN ) 
			count = DIFFICULTY_COUNT
		else if( mapType == MAP_TYPE_MINIGAME )
			count = 1
		else if( mapType == MAP_TYPE_SURVIVAL )
		{
			count = CHALLENGE_MODE_HARDCORE + 1
			if( HasDlc( player, DLC_EVIL_EMPIRE ) )
				count = CHALLENGE_MODE_TRAUMA + 1
			if( HasDlc( player, DLC_NAPALM ) )
				count = CHALLENGE_MODE_COMMANDO + 1
		}
		else if( mapType == MAP_TYPE_HEADTOHEAD )
			count = 1
		
		local initial = 0
		if( mapType == MAP_TYPE_CAMPAIGN )
		{
			initial = 1
			if( scores && scores.active )
				initial = scores.GetSelection( )
		}
		else if( mapType == MAP_TYPE_SURVIVAL )
		{
			if( scores && scores.active )
				initial = scores.GetSelection( )
		}
			
		for( local diff = 0; diff < count; ++diff )
		{
			local id = ::GameAppSession.GetLevelLeaderboardId( mapType, levelIndex, diff )
			if( id != ~0 )
				boards.push( id )
		}
		
		if( boards.len( ) > 0 )
		{
			PlaySound( "Play_UI_Select_Forward" )
			PushNextMenu( ::FrontEndLeaderboardsMenu( boards, levelInfo, initial, this ) )
			AutoAdvance = true
		}
		else
		{
			PlaySound("Play_HUD_WeaponMenu_Error" )
			::print( "Warning, tried to load a leaderboard menu that has no leaderboards. MapType:" + mapType.tostring( ) + " LevelIndex:" + levelIndex.tostring( ) )
			return false
		}
	}
	
	function SelectActiveIcon( )
	{
		return true
	}
	
	function SetControls( )
	{
	}
	
	function LoadLevel( front, levelIndex )
	{
		return false
	}
	
	function PurchaseComplete( )
	{
		if( GameApp.IsFullVersion )
		{
			if( restrictUI )
				restrictUI.FadeOutAndDie( 0.2 )
		}
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				PurchaseComplete( )
				UnlockAllLevelIcons( )
				break;
		}
		
		return ::RollingMenu.HandleCanvasEvent( event )
	}
}
