// Campaign Pause Menu Content

// Requires
sigimport "gui/scripts/frontend/levelselect/medalpanel.nut"
sigimport "gui/scripts/pausemenu/pausecontentbase.nut"
sigimport "gui/scripts/utility/goldenarcadescripts.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"
sigimport "gui/textures/cursor/barbackground_g.png"
sigimport "gui/textures/cursor/bar_g.png"
sigimport "gui/textures/cursor/powerbar_g.png"

class RationTicketProgressDisplay extends AnimatingCanvas
{
	constructor( levelIndex, ticketIndex, status )
	{
		::AnimatingCanvas.constructor( )
		local red = ::Math.Vec4.Construct( 1.0, 0.694, 0.694, 1.0 )
		
		// Loc String
		local loc = null
		if( status.complete )
			loc = ::GameApp.LocString( "Complete" )
		else if( status.failed )
			loc = ::GameApp.LocString( "Incomplete" ) // Edit for DR: Don't show failed status
		else if( status.hasProgress )
			loc = ::RationTicketProgressLocString( levelIndex, ticketIndex, status.progress, status.max )
		else
			loc = ::GameApp.LocString( "Incomplete" )
		
		// Text
		local progressText = ::Gui.Text( )
		progressText.SetFontById( FONT_SIMPLE_MED )
		progressText.SetRgba( /*status.failed? red: */COLOR_CLEAN_WHITE )
		progressText.BakeLocString( loc, TEXT_ALIGN_CENTER )
		progressText.SetPosition( 184 / 2, 0, 0 )
		AddChild( progressText )
		
		// Image
		local bar = ::ProgressBar( "gui/textures/cursor/barbackground_g.png", status.complete? "gui/textures/cursor/powerbar_g.png": "gui/textures/cursor/bar_g.png" )
		bar.SetMode( PROGRESSBAR_MODE_TEXTURE )
		bar.SetPosition( 0, progressText.Height, 0.001 )
		AddChild( bar )
		
		if( status.hasProgress )
			bar.SetMeterHorizontal( status.progress.tofloat( ) / status.max.tofloat( ) )
		else
			bar.SetMeterHorizontal( ( status.complete )? 1.0: 0.0 )
	}
}

class RationTicketProgressPanel extends AnimatingCanvas
{
	constructor( level, ticketIndex, user )
	{
		::AnimatingCanvas.constructor( )
		local levelIndex = level.LevelNumber
		
		// Image
		local medalTexture = ::RationTicketImagePath( levelIndex, ticketIndex )
		local medalWidth = 128 * 0.6
		local width = 540 + 124
		
		local image = ::Gui.AsyncTexturedQuad( )
		image.SetTexture( medalTexture )
		image.SetUniformScale( 0.6 )
		AddChild( image )
		
		// Title
		local title = ::Gui.Text( )
		title.SetFontById( FONT_FANCY_MED )
		title.SetRgba( COLOR_CLEAN_WHITE )
		title.SetPosition( medalWidth + 10, 0, 0 )
		title.BakeLocString( ::RationTicketNameLocString( levelIndex, ticketIndex ), TEXT_ALIGN_LEFT )
		AddChild( title )
		
		title.Compact( width - 184 )
		
		// Line
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		line.SetPosition( medalWidth, title.LineHeight + 3, 0 )
		AddChild( line )
		
		// Desc
		local desc = ::Gui.Text( )
		desc.SetFontById( FONT_SIMPLE_SMALL )
		desc.SetRgba( COLOR_CLEAN_WHITE )
		desc.SetPosition( medalWidth + 10, line.GetYPos( ) + 10, 0 )
		desc.BakeBoxLocString( width, ::RationTicketDescLocString( levelIndex, ticketIndex ), TEXT_ALIGN_LEFT )
		AddChild( desc )
		
		desc.Compact( width )
		
		// Progress Text
		local status = level.RationTicketStatus( ticketIndex, user )
		if( status != null )
		{
			local progress = ::RationTicketProgressDisplay( levelIndex, ticketIndex, status )
			progress.SetPosition( medalWidth + 10 + width - 184, 0, 0 )
			AddChild( progress )
			
			if( !status.complete )
				image.SetRgba( 0, 0, 0, 0.5 )
		}
		else
		{
			image.SetRgba( 0, 0, 0, 0.5 )
		}
	}
}

class CampaignPauseContent extends PauseContentBase
{
	constructor( player )
	{
		::PauseContentBase.constructor( player )
		SetBackground( "gui/textures/pausemenu/pausemenu_background_g.png" )
		AddLabel( "topMiddle", origin.x + contentSize.x * 0.5 + 10, background.GetYPos( ) + 3 )
		SetLabel( "topMiddle", "Ration_Tickets" )
		
		// Ration Ticket Progress
		local startX = 350
		local startY = 5
		local spacing = 106
		
		for( local i = 0; i < 2; ++i )
		{
			local ticketProgress = ::RationTicketProgressPanel( ::GameApp.CurrentLevel, i, player.User )
			ticketProgress.SetPosition( startX, startY + spacing * i, 0 )
			AddChild( ticketProgress )
		}
		
		local profile = player.GetUserProfile( )
		local levelScores = profile.GetLevelScores( levelInfo.MapType, levelInfo.LevelIndex )
		
		// Rank
		local rank = ::MedalPanel( player.Stats, levelInfo, levelScores, RANKPANEL_TYPE_WIDE )
		rank.SetPosition( /*origin.x + 20*/ 710 - 325, startY + 2 * spacing + 10, 0 )
		AddChild( rank )
		
		// Golden Arcade
		local dlc = ::GameApp.CurrentLevel.DlcNumber
		local goldenArcade = null
		if( dlc == DLC_COLD_WAR )
			goldenArcade = ::GoldenArcadePanel( levelScores )
		else if( dlc == DLC_EVIL_EMPIRE )
			goldenArcade = ::GoldenBabushkaPanel( levelScores )
		else if( dlc == DLC_NAPALM )
			goldenArcade = ::GoldenDogTagPanel( levelScores )
		else
			goldenArcade = ::GoldenArcadePanel( levelScores )
		goldenArcade.SetPosition( origin.x + contentSize.x - 32, origin.y + contentSize.y, 0 )
		AddChild( goldenArcade )
	}
}