// Results dialog box for the trial

// Requires
sigimport "gui/scripts/endgamescreens/scoreutility.nut"
sigimport "gui/scripts/endgamescreens/earnedscreen.nut"

class TrialResultsDialog extends AnimatingCanvas
{
	// Display
	timesUpText = null
	skipButton = null
	scoreController = null
	controls = null
	
	// Data
	totalPoints = null
	currentScore = null
	weaponStation = null
	player = null
	filteredGamepad = null
	noInput = null
	canceled = null
	audio = null
	
	// Events
	onAPress = null
	onBPress = null
	onFadedOut = null
	onCanceled = null
		
	constructor( user, points, personalBest, liveBest, friendsBest = false )
	{
		::AnimatingCanvas.constructor( )
		player = ::GameApp.CurrentLevel.ControllingPlayer( )
		audio = player.AudioSource
		
		filteredGamepad = ::FilteredGamepad( user, false, false )
		canceled = false
		
		local aText = null
		local bText = null
		
		if( ::GameApp.CurrentLevel.IsTrial )
		{
			aText = "Continue Tutorial"
			bText = "Replay"
		}
		else
		{
			if( ::GameApp.SingleScreenCoop )
				aText = "Minigame_CoopPass"
			else
				aText = "Replay"
			bText = ( ::GameApp.GameMode.IsNet ? "EndGame_QuitToLobby" : "EndGame_Quit" )
		}

		if( player.User.IsLocal )
		{
			controls = ::ControllerButtonContainer( FONT_SIMPLE_MED, 24 )
			controls.AddControl( GAMEPAD_BUTTON_A, aText )
			controls.AddControl( GAMEPAD_BUTTON_B, bText )
			controls.SetPosition( -controls.LocalRect.Width * 0.5, 25, 0 )
			controls.SetAlpha( 0 )
			AddChild( controls )
		}
		
		// Text
		timesUpText = ::ImpactText( ::GameApp.CurrentLevel.miniGameBeginLocKey + "_finish", player )
		timesUpText.SetPosition( 0, -200, 0 )
		AddChild( timesUpText )
		
		// Earned Items
		local earnedItems = ::EarnedItemsDisplayCanvas( player, true, FONT_SIMPLE_MED, ::GameApp.LocString( "You_Earned" ).ReplaceCString( "itemName", "" ) )
		if( earnedItems.HasEarnings( ) )
		{
			earnedItems.SetPosition( -500, -200, 0 )
			earnedItems.SetAlpha( 0 )
			earnedItems.FadeIn( )
			AddChild( earnedItems )
		}
		
		// Scores
		scoreController = ::ScoreController( player, null, EnableInput.bindenv( this ) )
		scoreController.SetPosition( 0, 0, 0 )
		scoreController.SetAlpha( 0 )
		scoreController.FadeIn( )
		AddChild( scoreController )
		
		// Hack it for minigames...
		scoreController.totalScore = points
		scoreController.scores.MinigameScore <- ( points - scoreController.scores.BonusPoints )
		
		// Do scores
		scoreController.DoResult( "MinigameScore", "MinigameScore", "MinigameScore" )
		scoreController.DoBonuses( )
		scoreController.PresentScore( )
		if( personalBest )
			scoreController.DoResult( "Personal_Best", null, null )
		if( liveBest )
			scoreController.DoResult( "LIVE_Best", null, null )
		if( friendsBest )
			scoreController.DoResult( "Friends_Best", null, null )
		
		local currentUnit = player.CurrentUnit
		if( currentUnit && currentUnit.Weapon )
		{
			weaponStation = currentUnit.WeaponStation( 0 )
			if( !is_null( weaponStation ) )
			{
				local weaponUI = weaponStation.WeaponUI
				
				if( !is_null( weaponUI ) )
				{
					//::GameApp.CurrentLevel.TutAlwaysShowUnitControls = false
					weaponUI.ShowHideReticle( 0 )
					weaponUI.GetControls( true ).SetAlpha( 0 )
					weaponUI.GetControls( false ).SetAlpha( 0 )
				}
			}
		}
		
		local vpRect = player.ComputeViewportSafeRect( )
		
		// Setup Skippable
		noInput = false
		skipButton = ::ControllerButton( GAMEPAD_BUTTON_A, "Skip" )
		skipButton.SetPosition( -skipButton.GetSize( ).Width * 0.5, vpRect.Height * 0.5 - skipButton.GetSize( ).Height * 0.5, 0 )
		if( player.User.IsLocal )
			AddChild( skipButton )
		onAPress = function( )
		{
			scoreController.Skip( )
			skipButton.FadeOut( 0.3 )
		}.bindenv( this )
		
		SetPosition( vpRect.Center.x, vpRect.Center.y, 0.4 )
		SetAlpha( 0 )
		FadeIn( 0.5 )
		::GameApp.HudRoot.AddChild( this )
	}
	
	function OnTick( dt )
	{
		if( !::GameApp.Paused( ) )
			::AnimatingCanvas.OnTick( dt )
			
		if( noInput )
			return
			
		if( filteredGamepad )
		{
			local gamepad = filteredGamepad.Get( )
			
			if( gamepad.ButtonDown( GAMEPAD_BUTTON_A ) && onAPress )
			{
				noInput = true
				audio.AudioEvent( "Play_UI_Select_Forward" )
				onAPress( )
			}
			else if( gamepad.ButtonDown( GAMEPAD_BUTTON_B ) && onBPress )
			{
				audio.AudioEvent( "Play_UI_Select_Backward" )
				
				if( !::GameApp.CurrentLevel.IsTrial )
				{
					local dialog = ::ModalConfirmationBox( "Quit_Confirm", player.User, "Ok", "Cancel" )
					dialog.onAPress = function( )
					{
						noInput = true
					}.bindenv( this )
					dialog.onFadedOut = function( )
					{
						onBPress( )
					}.bindenv( this )
				}
				else
				{
					onBPress( )
				}
			}
		}
	}
	
	function CloseDialog( )
	{
		noInput = true
		FadeOutAnd( 0.5, FadeOutProc.bindenv( this ) )
	}

	function EnableInput( )
	{
		skipButton.ClearActions( )
		skipButton.AddAction( ::AlphaTween( skipButton.GetAlpha( ), 0.0, 0.3 ) )
		
		if( controls )
		{
			controls.ClearActions( )
			controls.AddAction( ::AlphaTween( controls.GetAlpha( ), 1.0, 0.4 ) )
		}
		noInput = false
		
		onAPress = Cleanup.bindenv( this )		
		onBPress = OnExit.bindenv( this )
	}
	
	function OnExit( )
	{				
		if( !::GameApp.IsFullVersion )
		{
			noInput = true
			::GameApp.AskPlayerToBuyGameCallback = function( )
			{
				noInput = false
			}.bindenv( this )
			::GameApp.DoAskPlayerToBuyGame( )
			return
		}
		
		if( !is_null( weaponStation ) )
		{
			local weaponUI = weaponStation.WeaponUI
			if( !is_null( weaponUI ) )
			{
				weaponUI.ShowHideReticle( 1 )
				weaponUI.GetControls( true ).SetAlpha( 0 )
				weaponUI.GetControls( false ).SetAlpha( 1 )
				//::GameApp.CurrentLevel.TutAlwaysShowUnitControls = levelShowUnitControlsSetting
			}
		}
		
		canceled = true
		CloseDialog( )
	}
	
	function Cleanup( )
	{
		local currentUnit = player.CurrentUnit
		if( currentUnit && currentUnit.Weapon )
		{
			weaponStation = currentUnit.WeaponStation( 0 )
			if( !is_null( weaponStation ) )
			{
				local weaponUI = weaponStation.WeaponUI
				
				if( !is_null( weaponUI ) )
				{
					weaponUI.ShowHideReticle( 1 )
					weaponUI.GetControls( true ).SetAlpha( 0 )
					weaponUI.GetControls( false ).SetAlpha( 1 )
					//::GameApp.CurrentLevel.TutAlwaysShowUnitControls = levelShowUnitControlsSetting
				}
			}
		}
		
		timesUpText.Clear( )
		CloseDialog( )
	}
	
	function FadeOutProc( canvas )
	{
		if( filteredGamepad )
			filteredGamepad.Release( )
		filteredGamepad = null
		DeleteSelf( )
		
		if( canceled )
		{
			if( onCanceled )
				onCanceled( )
		}
		else
		{
			if( onFadedOut )
				onFadedOut( )
		}
		
		onCanceled = null
		onFadedOut = null
	}
}
