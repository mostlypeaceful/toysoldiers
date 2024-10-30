// Versus Pause Menu Content

// Requires

sigimport "gui/scripts/pausemenu/pausecontentbase.nut"
sigimport "gui/scripts/hud/scoreui.nut"
sigimport "gui/scripts/pausemenu/rewindpreview.nut"
sigimport "gui/scripts/hud/flaghealthbar.nut"

// Resources
sigimport "gui/textures/misc/versus_decoration_g.png"

class VersusPausePlayerInfo extends AnimatingCanvas
{
	// Statics
	static width = 360
	static flagHeight = 64
	static flagWidth = 104
	static spacing = 10
	
	constructor( player, turretData )
	{
		::AnimatingCanvas.constructor( )
		
		// Toybox HP
		local toyboxHpLabel = ::Gui.Text( )
		toyboxHpLabel.SetFontById( FONT_SIMPLE_MED )
		toyboxHpLabel.SetRgba( COLOR_CLEAN_WHITE )
		toyboxHpLabel.BakeLocString( ::GameApp.LocString( "Rewind_Toybox_Health" ).ReplaceCString( "health", "" ), TEXT_ALIGN_LEFT )
		toyboxHpLabel.SetPosition( 0, ( flagHeight - toyboxHpLabel.Height ) * 0.5, 0 )
		AddChild( toyboxHpLabel )
		
		toyboxHpLabel.Compact( width - flagWidth - 10 )
		
		local toyboxHp = ::Gui.Text( )
		toyboxHp.SetFontById( FONT_FANCY_MED )
		toyboxHp.SetRgba( COLOR_CLEAN_WHITE )
		toyboxHp.BakeCString( player.Tickets.tostring( ), TEXT_ALIGN_CENTER )
		toyboxHp.SetPosition( width - flagWidth * ( 1.0 / 3.0 ), ( flagHeight - toyboxHp.Height ) * 0.5, 0 )
		AddChild( toyboxHp )
		
		local flag = ::FlagHealthBar( player.Tickets / ::GameApp.CurrentLevelLoadInfo.Tickets.tofloat( ), player.Country )
		flag.SetPosition( width - flagWidth, 0, 0.001 )
		AddChild( flag )
		
		// Cash
		local line2Y = flagHeight + spacing + 4
		local cashLabel = ::Gui.Text( )
		cashLabel.SetFontById( FONT_SIMPLE_MED )
		cashLabel.SetRgba( COLOR_CLEAN_WHITE )
		cashLabel.BakeLocString( ::GameApp.LocString( "Rewind_Money" ).ReplaceCString( "money", "" ), TEXT_ALIGN_LEFT )
		cashLabel.SetPosition( 0, line2Y, 0 )
		AddChild( cashLabel )
		
		local cash = ::Gui.Text( )
		cash.SetFontById( FONT_FANCY_MED )
		cash.SetRgba( COLOR_CLEAN_WHITE )
		cash.BakeLocString( ::LocString.ConstructMoneyString( player.Money.tostring( ) ), TEXT_ALIGN_RIGHT )
		cash.SetPosition( width, line2Y, 0 )
		AddChild( cash )
		
		// Turrets
		local line3Y = line2Y + cash.Height + spacing
		local turretsLabel = ::Gui.Text( )
		turretsLabel.SetFontById( FONT_SIMPLE_MED )
		turretsLabel.SetRgba( COLOR_CLEAN_WHITE )
		turretsLabel.BakeLocString( ::GameApp.LocString( "Rewind_Turrets" ), TEXT_ALIGN_LEFT )
		turretsLabel.SetPosition( 0, line3Y, 0 )
		AddChild( turretsLabel )
		
		local turrets = ::TurretIconDisplay( player.Country, 5, TURRET_ALIGN_RIGHT )
		turrets.SetPosition( width - 20, line3Y + 16, 0 )
		AddChild( turrets )
		for( local i = 0; i < turretData.TurretCount; ++i )
		{
			local turret = turretData.Turret( i )
			if( turret.Country == player.Country )
				turrets.AddIcon( turret.UnitID )
		}
	}
}

class VersusPauseContent extends PauseContentBase
{
	function _typeof( ) { return "VersusPauseContent" }
	
	constructor( player )
	{
		::PauseContentBase.constructor( player )
		SetBackground( "gui/textures/pausemenu/pausemenu_background_g.png" )
		AddLabel( "topMiddle", origin.x + contentSize.x * 0.5, origin.y + 1 )
		SetLabel( "topMiddle", ::GameApp.LocString( "EndGame_VersusVs" ) )
		
		// Names
		local hostPlayer = null
		if( ::GameAppSession.IsHost )
			hostPlayer = ::GameApp.FrontEndPlayer
		else
			hostPlayer = ::GameApp.SecondaryPlayer
		local clientPlayer = ::GameApp.OtherPlayer( hostPlayer )
		local players = [ hostPlayer, clientPlayer ]
		
		local leftName = AddLabel( "leftName", origin.x + contentSize.x * 0.25, origin.y + 1 )
		SetLabel( "leftName", hostPlayer.User.GamerTag )
		local rightName = AddLabel( "rightName", origin.x + contentSize.x * 0.75, origin.y + 1 )
		SetLabel( "rightName", clientPlayer.User.GamerTag )
		
		local availableTextSpace = contentSize.x * 0.5 - 40
		leftName.Compact( availableTextSpace )
		rightName.Compact( availableTextSpace )
		
		// Data
		local spacing = 36
		local turretData = ::GameApp.CurrentLevel.GetAllTurretData( )
		foreach( i, player in players )
		{
			local infoDisplay = ::VersusPausePlayerInfo( player, turretData )
			infoDisplay.SetPosition( origin.x + 20 + ( i * ( ::VersusPausePlayerInfo.width + spacing ) ), origin.y + 60, 0 )
			AddChild( infoDisplay )
		}
		
		// Line
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/misc/versus_decoration_g.png" )
		line.SetPosition( origin.x + 20 + ::VersusPausePlayerInfo.width + spacing * 0.5 - 2, origin.y + 50, 0 )
		AddChild( line )

		// Tips
		local tips = ::TipsPanel( ::GameApp.CurrentLevelLoadInfo )
		tips.SetPosition( origin.x + 20, origin.y + 276, 0 )
		AddChild( tips )
	}
}