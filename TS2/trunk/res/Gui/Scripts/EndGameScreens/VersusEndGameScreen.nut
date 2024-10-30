// Versus End Game

// Requires
sigimport "gui/scripts/endgamescreens/baseendgamescreen.nut"
sigimport "gui/scripts/endgamescreens/versusstatsscreen.nut"
sigimport "gui/scripts/endgamescreens/earnedscreen.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

const VERSUS_ENDTEXT_COUNT = 4

function GetRandomVersusEndText( victory )
{
	local base = "EndGame_VersusDefeat"
	if( victory )
		base = "EndGame_VersusVictory"

	return ( base + ::SubjectiveRand.Int( 0, VERSUS_ENDTEXT_COUNT - 1 ).tostring( ) )
}

class VersusEndGameScreen extends BaseEndGameScreen
{
	constructor( victoriousPlayer, defeatedPlayer )
	{
		local screenRect = victoriousPlayer.User.ComputeScreenSafeRect( )
		::BaseEndGameScreen.constructor( screenRect, true )
		if( victoriousPlayer.User.IsLocal )
			audioSource = victoriousPlayer.AudioSource
		else
			audioSource = defeatedPlayer.AudioSource
		BackButtons = 0
		local startY = screenRect.Center.y - 100
		local earnedY = 150

		if( victoriousPlayer.User.IsLocal )
		{
			local vpRect = victoriousPlayer.User.ComputeViewportSafeRect( )
			
			local victoryDisplay = ::VersusEndgameDisplay( "EndGame_Victor", victoriousPlayer, GetRandomVersusEndText( true ) )
			victoryDisplay.SetPosition( vpRect.Center.x, startY, 0 )
			victoryDisplay.SetZPos( 0 )
			AddChild( victoryDisplay )
			
			local victoryEarned = ::EarnedItemsDisplayCanvas( victoriousPlayer )
			if( victoryEarned.HasEarnings( ) )
			{
				victoryEarned.SetPosition( vpRect.Center.x, startY + earnedY, 0 )
				AddChild( victoryEarned )
			}
		}
		
		if( defeatedPlayer.User.IsLocal )
		{
			local vpRect = defeatedPlayer.User.ComputeViewportSafeRect( )
			
			local defeatDisplay = ::VersusEndgameDisplay( "EndGame_Defeated", defeatedPlayer, GetRandomVersusEndText( false ) )
			defeatDisplay.SetPosition( vpRect.Center.x, startY, 0  )
			defeatDisplay.SetZPos( 0 )
			AddChild( defeatDisplay )
			
			local defeatEarned = ::EarnedItemsDisplayCanvas( defeatedPlayer )
			if( defeatEarned.HasEarnings( ) )
			{
				defeatEarned.SetPosition( vpRect.Center.x, startY + earnedY, 0 )
				AddChild( defeatEarned )
			}
		}
		
		controls.AddControl( GAMEPAD_BUTTON_A, "EndGame_Continue" )
	}
	
	function SelectActiveIcon( )
	{
		if( noInput )
			return
			
		noInput = true
		PlaySound( "Play_UI_Select_Forward" )
		PushNextMenu( ::VersusStatsScreen( ) )
		return true
	}
}

class VersusEndgameDisplay extends AnimatingCanvas
{
	constructor( titleLocID, player, extraLocId )
	{
		::AnimatingCanvas.constructor( )
		
		local title = ::Gui.Text( )
		title.SetFontById( FONT_FANCY_LARGE )
		title.SetRgba( COLOR_CLEAN_WHITE )
		title.BakeLocString( ::GameApp.LocString( titleLocID ), TEXT_ALIGN_CENTER )
		title.SetPosition( 0, -title.Height, 0 )
		AddChild( title )
		
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		line.SetPosition( -128, 0, 0 )
		AddChild( line )

		local str = ::GameApp.LocString( "Versus_EndGameQuoteFormat" ).Replace( "quote", extraLocId )
		
		local text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_MED )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeBoxLocString( 500, str, TEXT_ALIGN_CENTER )
		text.SetPosition( 0, line.GetYPos( ) + 10, 0 )
		AddChild( text )
		
		local speakerText = ::Gui.Text( )
		speakerText.SetFontById( FONT_SIMPLE_MED )
		speakerText.SetRgba( COLOR_CLEAN_WHITE )
		speakerText.BakeBoxLocString( 500, ::LocString.FromCString( "- " ) % player.User.GamerTag, TEXT_ALIGN_RIGHT )
		speakerText.SetPosition( -250, text.GetYPos( ) + text.Height + 10, 0 )
		AddChild( speakerText )
	}
}