// Achievement Buy Notification

// Resources

sigimport "gui/textures/score/score_decoration_g.png"
sigimport "gui/scripts/utility/achievementscripts.nut"
sigimport "gui/scripts/controls/verticalmenu.nut"

sigexport function CanvasCreateAchievementBuyNotification( cppObj )
{	
	return ::BuyNotificationMenuStack( cppObj )
}

class BuyNotification extends AnimatingCanvas
{
	// Data
	startPos = null
	endPos = null
	shown = null
	gamepad = null
	
	// Data
	static Width = 330
	static ShowTime = 0.5
	static TextWidth = 320
	
	constructor( text1, text2, imagePath, player )
	{
		::AnimatingCanvas.constructor( )
		shown = false
		gamepad = ::FilteredGamepad( player.User, false )
		
		// Image
		local image = ::Gui.AsyncTexturedQuad( )
		image.SetTexture( imagePath )
		AddChild( image )
		
		// You earned text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_MED )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( text1, TEXT_ALIGN_LEFT )
		if( text.Width > TextWidth )
			text.SetScale( TextWidth / text.Width, 1.0 )
		text.SetPosition( ::AchievementImage.Width + 10, 0, 0 )
		AddChild( text )
		
		// Line
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		line.SetPosition( text.GetXPos( ), text.GetYPos( ) + text.LineHeight, 0 )
		AddChild( line )
		
		// Buy Text
		local buyText = ::Gui.Text( )
		buyText.SetFontById( FONT_SIMPLE_SMALL )
		buyText.SetRgba( COLOR_CLEAN_WHITE )
		buyText.BakeBoxLocString( TextWidth, text2, TEXT_ALIGN_LEFT )
		buyText.SetPosition( line.GetXPos( ), line.GetYPos( ) + 8, 0 )
		AddChild( buyText )
		
		// What do do
		local controls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL )
		controls.AddControl( GAMEPAD_BUTTON_A, "Trial_BuyGameForAchievement" )
		controls.AddControl( GAMEPAD_BUTTON_START, "EndGame_Continue" )
		controls.SetPosition( buyText.GetXPos( ), buyText.GetYPos( ) + buyText.Height + 16, 0 )
		AddChild( controls )
		
		local vpRect = player.ComputeViewportSafeRect( )
		SetPosition( vpRect.Center.x - Width * 0.5, vpRect.Center.y - LocalRect.Height * 0.5, 0 )
	}
}

class BuyNotificationMenuStack extends VerticalMenuStack
{
	cppObj = null
	
	constructor( obj, achievement = true )
	{
		cppObj = obj
		::VerticalMenuStack.constructor( cppObj.Player.User, false )
		minStackCount = 0
		PushMenu( ::BuyNotificationMenu( cppObj.Player, cppObj.Index, achievement ) )
	}
	
	function Show( show )
	{
		local rootMenu = ::GameApp.CurrentLevel.GetRootMenu( )
		
		if( show )
		{
			// Add to hud
			rootMenu.AddMenuStack( cppObj.Player, this )
			::GameApp.Pause( true, audioSource )
			if( ::GameApp.CurrentLevelLoadInfo.MapType != MAP_TYPE_MINIGAME 
				&& cppObj.Index != ACHIEVEMENTS_DEMOLITION_MAN )
				::GameApp.ForEachPlayer( function( player, x ) { if( !player.DisableTiltShift ) player.PushTiltShiftCamera( ) } )
		}
	}
	
	function OnTick( dt )
	{
		if( IsEmpty( ) )
		{
			::GameApp.Pause( false, audioSource )
			DeleteSelf( )
			if( user.IsLocal )
				::GameApp.HudRoot.Invisible = false
			if( ::GameApp.CurrentLevelLoadInfo.MapType != MAP_TYPE_MINIGAME )
				::GameApp.ForEachPlayer( function( player, x ) { if( !player.DisableTiltShift ) player.PopTiltShiftCamera( ) } )
		}
		
		::VerticalMenuStack.OnTick( dt )
	}
}

class BuyNotificationMenu extends VerticalMenu
{
	constructor( player, index, achievement = true )
	{
		::VerticalMenu.constructor( )
		BackButtons = GAMEPAD_BUTTON_START
		
		local name = null
		local buyText = null
		local imagePath = null
		if( achievement )
		{
			name = ::GameApp.LocString( "You_Earned" ).Replace( "itemName", ::AchievementName( index ) )
			buyText = ::GameApp.LocString( ::AchievementBuyText( index ) )
			imagePath = ::AchievementImagePath( index )
		}
		else
		{
			name = ::GameApp.LocString( "You_Earned" ).Replace( "itemName", ::AvatarAwardName( index ) )
			buyText = ::GameApp.LocString( ::AvatarAwardBuyText( index ) )
			imagePath = ::AvatarAwardImagePath( index )
		}
			
		// Notification box
		local buyNotification = ::BuyNotification( name, buyText, imagePath, player )
		AddChild( buyNotification )
	}
	
	function SelectActiveIcon( )
	{
		// Show buy menu
		local buyScreen = ::TrialBuyGameScreen( )
		buyScreen.backOutBehavior = function( ) { AutoExit = true; return false }.bindenv( buyScreen )
		buyScreen.SetBackButtonText( "Menus_Back" )
		return PushNextMenu( buyScreen )
	}
	
	function PurchaseComplete( )
	{
		if( GameApp.IsFullVersion )
		{
			AutoExit = true
		}
	}
	
	function HandleCanvasEvent( event )
	{
		switch( event.Id )
		{
			case ON_UPGRADE_TO_FULL_VERSION:
				PurchaseComplete( )
				break;
		}
		
		return ::VerticalMenu.HandleCanvasEvent( event )
	}
}
