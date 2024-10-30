// Versus Custom Match

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/frontend/levelselect/profilebadge.nut"
sigimport "gui/scripts/frontend/settings.nut"
sigimport "gui/scripts/frontend/levelselect/levelpreviewpanel.nut"

// Resources
sigimport "gui/textures/frontend/versus_custom_bg_d.png"

class VersusCustomMatchLobby extends FrontEndStaticScreen
{
	// Display
	mapSelect = null
	teamSelect = null
	vehicleSelect = null
	barrageSelect = null
	turretAiSelect = null
	toyboxHpSelect = null
	cashSelect = null
	preview = null
	twoPlayerController = null
	
	// Data
	levelInfos = null // array of LevelLoadInfo(C++) objects
	
	hpValues = null
	cashValues = null
	
	constructor( )
	{
		::FrontEndStaticScreen.constructor( )
		ForwardButtons = GAMEPAD_BUTTON_A
		SetTitle( "CustomMatch_Title" )
		menuPositionOffset = ::Math.Vec3.Construct( 150, 150, 0 )
		inputHook = true
		
		local vpSafeRect = ::GameApp.FrontEndPlayer.ComputeViewportSafeRect( )
		
		// Background
		local background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/frontend/versus_custom_bg_d.png" )
		background.SetPosition( vpSafeRect.Left, vpSafeRect.Top + 120, 0.04 )
		AddChild( background )
		
		// Get Level Infos
		levelInfos = [ ]
		local numLevels = ::GameApp.NumLevelsInTable( MAP_TYPE_HEADTOHEAD )
		for( local i = 0; i < numLevels; ++i )
		{
			local levelInfo = ::GameApp.GetLevelLoadInfo( MAP_TYPE_HEADTOHEAD, i )
			levelInfos.push( levelInfo )
		}
		
		twoPlayerController = ::TwoPlayerMenuController( ::GameApp.FrontEndPlayer.User )
		twoPlayerController.onUserChange = UpdateControls.bindenv( this )
		twoPlayerController.SetPosition( vpSafeRect.Left, vpSafeRect.Top, 0 )
		AddChild( twoPlayerController )
		
		// Preview box
		preview = ::LevelPreviewPanel( levelInfos[ 0 ] )
		preview.SetPosition( 150, 380, 0 )
		AddChild( preview )
		
		if( twoPlayerController.user1.SignedInOnline )
			::GameAppSession.HostGame( twoPlayerController.user1, CONTEXT_GAME_MODE_VERSUS, CONTEXT_GAME_TYPE_STANDARD )
	}
	
	function FinalizeIconSetup( )
	{
		// Items
		local startGame = ::FrontEndMenuEntry( "CustomMatch_StartGame", null, StartGame.bindenv( this ) )
		icons.push( startGame )
		AddChild( startGame )
		
		local mapNames = [ ]
		foreach( level in levelInfos )
			mapNames.push( level.MapDisplayName )
		mapSelect = ::SettingItem_Choice( "CustomMatch_Map", mapNames, 0, 0, function( index ) { UpdatePreview( index ) }.bindenv(this), 300, 200 )
		icons.push( mapSelect )
		AddChild( mapSelect )
		
		teamSelect = ::SettingItem_Choice( "CustomMatch_Team", [ "CustomMatch_USA", "CustomMatch_USSR" ], 0 )
		icons.push( teamSelect )
		AddChild( teamSelect )
		
		vehicleSelect = ::SettingItem_Choice( "CustomMatch_Vehicles", [ "CustomMatch_Yes", "CustomMatch_No" ], 0 )
		icons.push( vehicleSelect )
		AddChild( vehicleSelect )
		
		barrageSelect = ::SettingItem_Choice( "CustomMatch_Barrages", [ "CustomMatch_Yes", "CustomMatch_No" ], 0 )
		icons.push( barrageSelect )
		AddChild( barrageSelect )
		
		turretAiSelect = ::SettingItem_Choice( "CustomMatch_TurretAI", [ "CustomMatch_On", "CustomMatch_Off" ], 0 )
		icons.push( turretAiSelect )
		AddChild( turretAiSelect )
		
		hpValues = [ 10, 20, 30, 50, 100 ]
		toyboxHpSelect = ::SettingItem_Choice( "CustomMatch_ToyboxHP", hpValues, 1 )
		icons.push( toyboxHpSelect )
		AddChild( toyboxHpSelect )
		
		cashValues = [
			1000,
			2000,
			4000,
			6000,
			10000
		]
		
		local cashOptions = array( cashValues.len( ) )
		for( local i = 0; i < cashValues.len( ); ++i )
			cashOptions[ i ] = ::LocString.ConstructMoneyString( cashValues[ i ].tostring( ) )
		cashSelect = ::SettingItem_Choice( "CustomMatch_Cash", cashOptions, 1 )
		cashSelect.SetLooping( true )
		icons.push( cashSelect )
		AddChild( cashSelect )
		
		UpdateControls( )
		
		::FrontEndStaticScreen.FinalizeIconSetup( )
	}
	
	function FillLoadInfo( info )
	{
		info.Country = teamSelect.CurrentIndex( ) == 0 ? COUNTRY_USA : COUNTRY_USSR
		info.Tickets = hpValues[ toyboxHpSelect.CurrentIndex( ) ]
		info.Money = cashValues[ cashSelect.CurrentIndex( ) ]
		info.ChallengeVehicles = 1 - vehicleSelect.CurrentIndex( ) //this will be 1 if currentIndex is zero, or 0 otherwise
		info.ChallengeBarrages = 1 - barrageSelect.CurrentIndex( )
		info.ChallengeTurretAI = 1 - turretAiSelect.CurrentIndex( )
	}
	
	function OnBackOut( )
	{
		if( ::GameApp.GameMode.IsFrontEnd )
		{
			::GameAppSession.CancelSession( )
		}
		return ::FrontEndStaticScreen.OnBackOut( )
	}
	
	function ChangeHighlight( delta )
	{
		::FrontEndStaticScreen.ChangeHighlight( delta )
		UpdateControls( )
	}
	
	function HandleInput( gamepad )
	{
		if( twoPlayerController )
			twoPlayerController.HandleInput( gamepad )
	}
	
	function HandleCanvasEvent( event )
	{
		/*if( event.Id == SESSION_CREATED )
		{
			if( BoolEventContext.Convert( event.Context ).Bool == false && twoPlayerController.user1.IsOnlineEnabled )
			{
				WarnNoLive( )
				return true
			}
		}*/
		
		local twoPlayerEventHandled = twoPlayerController.HandleCanvasEvent( event )
		if( twoPlayerEventHandled )
			return true
		
		return ::FrontEndStaticScreen.HandleCanvasEvent( event )
	}
	
	function WarnNoLive( )
	{	
		local dialogBox = ::ModalConfirmationBox( "Menus_NoXboxLiveNoStats", twoPlayerController.user1, "Accept", "Cancel" )

		dialogBox.onBPress = function( )
		{
			AutoBackOut = true
		}.bindenv(this)
		
		dialogBox.onAPress = function( )
		{
			UpdateControls( )
		}.bindenv(this)
	}
	
	function UpdatePreview( index )
	{
		preview.SetLevelInfo( levelInfos[ index ] )
	}
	
	function UpdateControls( )
	{
		controls.Clear( )
		
		if( highlightIndex == 0 )
			controls.AddControl( GAMEPAD_BUTTON_A, "Menus_Select" )
		
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
		
		if( twoPlayerController.InvitesAllowed( ) )
		{
			if( !twoPlayerController.user1.IsInActiveParty )
				controls.AddControl( "gui/textures/gamepad/button_x_g.png", "Invite_Friend" )
			else
				controls.AddControl( "gui/textures/gamepad/button_x_g.png", "Invite_Party" )	
		}
	}
	
	function StartGame( )
	{
		if( !twoPlayerController.HasTwoPlayers( ) )
			return false
		
		local levelIndex = mapSelect.CurrentIndex( )
		local isLocal = twoPlayerController.user2.IsLocal
		
		if( isLocal )
			::GameApp.LoadLevel( MAP_TYPE_HEADTOHEAD, levelIndex, FillLoadInfo.bindenv( this ) )
		else
			::GameAppSession.StartMap( MAP_TYPE_HEADTOHEAD, levelIndex, FillLoadInfo.bindenv( this ) )
		
		nextAction = VerticalMenuAction.ExitStack
		return true
	}
}