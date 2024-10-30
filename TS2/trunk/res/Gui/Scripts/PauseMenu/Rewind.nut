// Rewind Page

// Requires
sigimport "gui/scripts/controls/verticalmenu.nut"
sigimport "gui/scripts/hud/wavelist_sp.nut"
sigimport "gui/scripts/hud/wavelistdisplay.nut"
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/pausemenu/rewindpreview.nut"

// Resources
sigimport "gui/textures/trial/tutorial_bg_g.png"

class RewindMenuStack extends VerticalMenuStack
{
	constructor( user )
	{
		::VerticalMenuStack.constructor( user )
		minStackCount = 0
		if( !user.IsLocal )
			Invisible = true
	}
	
	function OnTick( dt )
	{
		if( IsEmpty( ) )
		{
			GameApp.Pause( false )
			DeleteSelf( )
			if( user.IsLocal )
				::GameApp.HudRoot.Invisible = false
		}
		else
		{
			local pad = filteredGamepad.Get( )
			if( pad.ButtonDown( GAMEPAD_BUTTON_SELECT | GAMEPAD_BUTTON_B ) )
			{
				PopMenu( )
			}
		}
		
		::VerticalMenuStack.OnTick( dt )
	}
	
	function DeleteSelf( )
	{
		local curMenu = CurrentMenu( )
		if( curMenu )
			curMenu.OnBackOut( )
		::VerticalMenuStack.DeleteSelf( )
	}
}

class RewindController extends AnimatingCanvas
{
	// Display
	display = null // WaveListDisplay
	toyboxHealthLabel = null // Gui.Text
	moneyLabel = null // Gui.Text
	moneyLabelPlayer2 = null // Gui.Text
	turretsLabel = null // Gui.Text
	turretIcons = null // array of RewindPreviewIcon objects
	disabledText = null
	
	// Data
	wavelist = null // tWaveList*
	disabled = null
	user = null
	
	// Events
	onRewind = null
	
	constructor( user_ )
	{
		::AnimatingCanvas.constructor( )
		user = user_
		audioSource = ::GameApp.GetPlayerByUser( user ).AudioSource
		turretIcons = [ ]
		
		wavelist = ::GameApp.CurrentLevel.CurrentOrLastDisplayedWaveList( )
		local highest = ::GameApp.HighestRewindableWaveIndex
		local startingIndex = ( is_null( wavelist )? 0: wavelist.CurrentUIWaveListID )
		startingIndex = ::Math.Min( startingIndex, highest )
		display = ::WaveListDisplay( wavelist, startingIndex, highest, true, true )
		display.audioSource = audioSource
		display.SetPosition( 0, -30, 0 )
		AddChild( display )
		
		local decoration2 = ::Gui.TexturedQuad( )
		decoration2.SetTexture( "gui/textures/score/score_decoration_g.png" )
		decoration2.CenterPivot( )
		decoration2.SetPosition( 0, 40, 0 )
		AddChild( decoration2 )
		
		local blackBar = ::Gui.TexturedQuad( )
		blackBar.SetTexture( "gui/textures/trial/tutorial_bg_g.png" )
		blackBar.SetRect( ::Math.Vec2.Construct( 1280, 100 ) )
		blackBar.CenterPivot( )
		blackBar.SetPosition( 0, -30, 0.05 )
		AddChild( blackBar )
		
		disabled = ( ::GameApp.HighestRewindableWaveIndex == -1 || !::GameApp.RewindEnabled )
		
		if( disabled )
		{
			// Disabled notification
			local disabledText = ::Gui.Text( )
			disabledText.SetFontById( FONT_SIMPLE_MED )
			disabledText.BakeLocString( ::GameApp.LocString( "Rewind_Disabled" ), TEXT_ALIGN_CENTER )
			disabledText.SetPosition( 0, -30, 0 )
			AddChild( disabledText )
			
			display.SetRgba( 0.4, 0.4, 0.4, 0.5 )
		}
		else
		{
			toyboxHealthLabel = ::Gui.Text( )
			toyboxHealthLabel.SetFontById( FONT_SIMPLE_MED )
			toyboxHealthLabel.SetRgba( COLOR_CLEAN_WHITE )
			toyboxHealthLabel.SetPosition( 8, 50, 0 )
			AddChild( toyboxHealthLabel )
			
			moneyLabel = ::Gui.Text( )
			moneyLabel.SetFontById( FONT_SIMPLE_MED )
			moneyLabel.SetRgba( COLOR_CLEAN_WHITE )
			moneyLabel.SetPosition( 8, 80, 0 )
			AddChild( moneyLabel )
			
			if( !::GameApp.GameMode.IsSinglePlayer )
			{
				moneyLabelPlayer2 = ::Gui.Text( )
				moneyLabelPlayer2.SetFontById( FONT_SIMPLE_MED )
				moneyLabelPlayer2.SetRgba( COLOR_CLEAN_WHITE )
				moneyLabelPlayer2.SetPosition( 8, 110, 0 )
				AddChild( moneyLabelPlayer2 )
			}
			
			turretsLabel = ::Gui.Text( )
			turretsLabel.SetFontById( FONT_SIMPLE_MED )
			turretsLabel.SetRgba( COLOR_CLEAN_WHITE )
			turretsLabel.SetPosition( -200, 50, 0 )
			turretsLabel.BakeLocString( ::GameApp.LocString( "Rewind_Turrets" ) )
			AddChild( turretsLabel )
			
			Update( display.CurrentWaveIndex( ) )
		}
		
		SetAlpha( 1.0 )
	}
	
	function Enabled( )
	{
		return !disabled
	}
	
	function Update( waveIndex )
	{
		if( disabled )
			return
			
		local saveData = ::GameApp.RewindPreview( waveIndex )
		
		if( !is_null( saveData ) )
		{
			UpdateToyboxHealthLabel( saveData.Tickets )
			UpdateMoneyLabel( saveData )
			UpdateTurrets( saveData )
		}
	}

	function UpdateToyboxHealthLabel( health )
	{
		local str = ::GameApp.LocString( "Rewind_Toybox_Health" ).Replace( "health", health )
		toyboxHealthLabel.BakeLocString( str, TEXT_ALIGN_LEFT )
	}
	
	function UpdateMoneyLabel( saveData )
	{
		if( ::GameApp.GameMode.IsSinglePlayer )
		{
			local str = ::GameApp.LocString( "Rewind_Money" ).Replace( "money", ::LocString.ConstructMoneyString( saveData.Money.tostring( ) ) )
			moneyLabel.BakeLocString( str, TEXT_ALIGN_LEFT )
		}
		else
		{
			local str = ::GameApp.FrontEndPlayer.User.GamerTag % ": " % ::LocString.ConstructMoneyString( saveData.Money.tostring( ) )
			moneyLabel.BakeLocString( str, TEXT_ALIGN_LEFT )
			local str2 = ::GameApp.SecondaryPlayer.User.GamerTag % ": " % ::LocString.ConstructMoneyString( saveData.MoneyPlayer2.tostring( ) )
			moneyLabelPlayer2.BakeLocString( str2, TEXT_ALIGN_LEFT )			
		}
	}
	
	function UpdateTurrets( saveData )
	{
		local pos = ::Math.Vec2.Construct( -200 + 16, 80 + 16 )
		local enemySpace = 10
		local width = 50
		local height = 42

		// Clear turrets
		foreach( icon in turretIcons )
			icon.FadeOutAndDie( 0.1 )
		turretIcons.clear( )
		
		//local enemyTurrets = [ ]
		//local playerTurrets = [ ]
		
		local playerCountry = ::GameApp.GetPlayerByUser( user ).Country
		for( local i = 0; i < saveData.TurretCount; ++i )
		{
			local turretData = saveData.Turret( i )
			if( turretData.Country == playerCountry )
			{
				local icon = ::RewindPreviewIcon( turretData.UnitID, turretData.Country )
				icon.SetAlpha( 0 )
				icon.FadeIn( 0.1 )
				AddChild( icon )
				turretIcons.push( icon )
			}
			
			//if( turretData.Country == COUNTRY_USA )
			//	playerTurrets.push( icon )
			//else
			//	enemyTurrets.push( icon )
		}
		
		foreach( i, icon in turretIcons )
			icon.SetPosition( pos.x + (i % 4) * width, pos.y + (i / 4).tointeger( ) * height, 0 )
		
		//local enemyStartY = pos.y + ( ( playerTurrets.len( ) > 0 )? (((playerTurrets.len( ) - 1) / 4).tointeger( ) + 1) * height + enemySpace : 0 )
		//foreach( i, icon in enemyTurrets )
		//	icon.SetPosition( pos.x + (i % 4) * width, enemyStartY + (i / 4).tointeger( ) * height, 0 )
	}
	
	function Rewind( )
	{
		if( disabled )
		{
			return false
		}
		else
		{
			local dialog = ::ModalConfirmationBox( "Rewind_Confirm", user, "Ok", "Cancel" )
			dialog.onFadedOut = function( )
			{
				local player = ::GameApp.GetPlayerByUser( user )
				player.AwardAchievement( ACHIEVEMENTS_LIKE_IT_NEVER_HAPPENED );
				
				local userToGetRewindsFrom = (::GameApp.GameMode.IsSplitScreen) ? ::GameApp.FrontEndPlayer.User : user
				
				::GameApp.Rewind( userToGetRewindsFrom, display.CurrentWaveIndex( ) )
				if( onRewind )
					onRewind( true )
			}.bindenv( this )
			dialog.onCanceled = function( )
			{
				if( onRewind )
					onRewind( false )
			}.bindenv( this )
			return false
		}
		return false
	}
	
	function ChangeHorizontalHighlight( delta )
	{
		if( disabled )
			return false
		
		local currentWaveIndex = display.CurrentWaveIndex( )
		if( delta < 0 )
		{
			display.PreviousWave( )
			if( currentWaveIndex != display.CurrentWaveIndex( ) )
				Update( display.CurrentWaveIndex( ) )
			return true
		}
		else if( delta > 0 )
		{
			display.NextWave( )
			if( currentWaveIndex != display.CurrentWaveIndex( ) )
				Update( display.CurrentWaveIndex( ) )
			return true
		}
		return false
	}
}

class RewindMenu extends FrontEndStaticScreen
{
	// Display
	rewindController = null // RewindController
	noInput = null
	
	// Player
	player = null
	
	constructor( )
	{
		::FrontEndStaticScreen.constructor( )
		inputHook = true
		noInput = false
		
		ForwardButtons = GAMEPAD_BUTTON_A
		BackButtons = GAMEPAD_BUTTON_B
		
		title.BakeLocString( ::GameApp.LocString( "Menus_Rewind" ), TEXT_ALIGN_CENTER )
		
		controls.font = FONT_SIMPLE_MED
		controls.padding = 20
		controls.AddControl( GAMEPAD_BUTTON_A, "Menus_Select" )
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
		controls.AddControl( GAMEPAD_BUTTON_DPAD_LEFT | GAMEPAD_BUTTON_DPAD_RIGHT, "Rewind_Select_Wave" )
	}
	
	function FinalizeIconSetup( )
	{
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		local center = vpSafeRect.Center
		center.y -= 80
		
		rewindController = ::RewindController( user )
		rewindController.SetPosition( center.x, center.y, 0 )
		AddChild( rewindController )
		
		rewindController.onRewind = function( rewind )
		{
			noInput = rewind
			if( rewind )
				AutoExit = true
		}.bindenv( this )
		
		player = ::GameApp.GetPlayerByUser( user )
		player.PushTiltShiftCamera( )
	}
	
	function SelectActiveIcon( )
	{
		if( noInput )
			return false
		
		noInput = true
		PlaySound( "Play_UI_Select_Forward" )
		rewindController.Rewind( )
		return true
	}
	
	function HandleInput( gamepad )
	{
		if( noInput )
			return
			
		local delta = 0
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_LEFT ) )
			delta = -1
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_RIGHT ) )
			delta = 1
		
		if( delta != 0 )
		{
			rewindController.ChangeHorizontalHighlight( delta )
		}
	}
	
	function ChangeHorizontalHighlight( delta )
	{
		// DPAD only
		return false
	}
	
	function OnBackOut( )
	{
		PlaySound( "Play_UI_Select_Backward" )
		noInput = true
		player.PopTiltShiftCamera( )
		return true
	}
}
