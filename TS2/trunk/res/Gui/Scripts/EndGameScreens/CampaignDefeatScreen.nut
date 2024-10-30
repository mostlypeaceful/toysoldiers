
sigimport "Gui/Scripts/EndGameScreens/BaseEndGameScreen.nut"
sigimport "gui/scripts/pausemenu/rewind.nut"
sigimport "gui/scripts/utility/protipscripts.nut"

class CampaignDefeatScreen extends BaseEndGameScreen
{
	rewindController = null
	
	constructor( defeatedPlayer, coopPlayer = null )
	{
		local screenRect = defeatedPlayer.User.ComputeScreenSafeRect( )
		::BaseEndGameScreen.constructor( screenRect )
		audioSource = defeatedPlayer.AudioSource

		// Stats Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( "EndGame_Defeat" ), TEXT_ALIGN_CENTER )
		text.SetPosition( ::Math.Vec3.Construct( screenRect.Center.x, screenRect.Top + 50, 0 ) )
		AddChild( text )
		
		// Decoration
		local decoration = ::Gui.TexturedQuad( )
		decoration.SetTexture( "gui/textures/score/score_decoration_g.png" )
		decoration.CenterPivot( )
		decoration.SetPosition( text.GetXPos( ), text.GetYPos( ) + text.Height + 5, 0 )
		AddChild( decoration )
		
		// Quote
		local currentDLC = ::GameApp.CurrentLevel.DlcNumber
		local quoteCount = 10
		local quoteLocString = ::GameApp.LocString( "CampaignDefeat_Format" ).Replace( "name", coopPlayer? "Defeat_MultiplayerName": defeatedPlayer.User.GamerTag )
		quoteLocString.Replace( "quote", ::GameApp.LocString( "DefeatQuote" + ( currentDLC == DLC_EVIL_EMPIRE ? "USSR" : "" ) + ::SubjectiveRand.Int( 0, quoteCount - 1 ).tostring( ) ) )
		quoteLocString.StripNewlines( )
		
		local quote = ::Gui.Text( )
		quote.SetFontById( FONT_FANCY_MED )
		quote.SetRgba( COLOR_CLEAN_WHITE )
		quote.BakeBoxLocString( 1000, quoteLocString, TEXT_ALIGN_CENTER )
		quote.SetPosition( ::Math.Vec3.Construct( screenRect.Center.x, decoration.GetYPos( ) + 10, 0 ) )
		AddChild( quote )
		
		// Pro Tip
		local tip = ::Gui.Text( )
		tip.SetFontById( FONT_SIMPLE_SMALL )
		tip.SetRgba( COLOR_CLEAN_WHITE )
		tip.BakeLocString( ::GetRandomProTipLocStringWithLabel( ), TEXT_ALIGN_CENTER )
		tip.SetPosition( ::Math.Vec3.Construct( screenRect.Center.x, screenRect.Center.y + 220, 0 ) )
		AddChild( tip )
		
		if( ::GameApp.RewindEnabled )
		{
			// Rewind
			rewindController = ::RewindController( defeatedPlayer.User )
			rewindController.SetPosition( screenRect.Center.x, screenRect.Center.y, 0 )
			AddChild( rewindController )
		}
		else
		{
			tip.SetYPos( screenRect.Center.y + 50 )
		}
		
		SetControls( )
		
	}
	
	function SetControls( )
	{
		controls.Clear( )
		if( ::GameAppSession.IsHost )
		{
			if( rewindController && rewindController.Enabled( ) )
			{
				controls.AddControl( GAMEPAD_BUTTON_DPAD_LEFT | GAMEPAD_BUTTON_DPAD_RIGHT, "Rewind_Select_Wave" )
				controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Confirm" )
			}
			controls.AddControl( GAMEPAD_BUTTON_X, "EndGame_RestartLevel" )
		}
		else
		{
			if( twoPlayerController && twoPlayerController.user1 )
			{
				if( twoPlayerController.lobbyController.IsClientReady( ) )
					controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Unready" )
				else
					controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Ready" )
			}
		}
		controls.AddControl( GAMEPAD_BUTTON_B, "EndGame_QuitToMainMenu" )
		
		if( ::GameAppSession.IsHost && twoPlayerController && twoPlayerController.user2 )
			controls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
		else if( twoPlayerController && twoPlayerController.user1 )
			controls.AddControl( GAMEPAD_BUTTON_SELECT, "LB_Menus_Gamer_Card" )
	}
	
	function HandleInput( gamepad )
	{
		if( noInput )
			return
			
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_X ) && ::GameAppSession.IsHost )
		{
			if( twoPlayerController && !twoPlayerController.lobbyController.IsClientReady( ) )
				return
					
			PlaySound( "Play_UI_Select_Backward" )
			noInput = true
			RestartCurrentLevel( )
			return
		}
		
		// Rewind controls
		local delta = 0
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_LEFT ) )
			delta = -1
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_RIGHT ) )
			delta = 1
		
		if( delta != 0 )
			rewindController.ChangeHorizontalHighlight( delta )
		
		if( twoPlayerController && twoPlayerController.HandleInput( gamepad ) )
			return true
	}
	
	function ChangeHorizontalHighlight( delta )
	{
		return false
	}
	
	function OnPressA( ) // Press A
	{
		if( ::GameAppSession.IsHost )
		{
			if( rewindController && rewindController.Enabled( ) )
			{
				if( !::GameApp.GameMode.IsNet || ( twoPlayerController && twoPlayerController.lobbyController.IsClientReady( ) ) )
				{
					noInput = true
					
					rewindController.onRewind = function( didRewind )
					{
						noInput = didRewind
						if( didRewind )
							CloseEndGameScreen( )
					}.bindenv( this )
					
					rewindController.Rewind( )
					return true
				}
			}
		}
		
		return false
	}
	
	function OnPressB( )
	{
		if( ::GameApp.IsFullVersion )
		{
			local dialog = ::ModalConfirmationBox( "Quit_ConfirmDisplayCase", user, "Ok", "Cancel" )
			dialog.onAPress = function( )
			{
				noInput = true
				
			}.bindenv( this )
			
			dialog.onFadedOut = function( )
			{
				::BaseEndGameScreen.OnPressB( )
				
			}.bindenv( this )
			
			return false
		}
		else
		{
			::BaseEndGameScreen.OnPressB( )
		}
	}
}
