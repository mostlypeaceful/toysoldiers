// Rations Dump

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/frontend/levelselect/goalpanel.nut"
sigimport "gui/scripts/frontend/levelselect/profilebadge.nut"

// Resources
sigimport "gui/textures/rationtickets/rationticket_selector_g.png"

class RationsDumpScreen extends FrontEndStaticScreen
{
	// Display
	selector = null
	titleText = null
	descText = null
	textCanvas = null
	background = null
	displayItems = null
	profileBadge = null
	
	// Data
	dlc = null
	currentSelection = null
	ticketCount = null
	slots = null
	
	// Constants
	static numColumns = 8
	static numRows = 3
	static inactiveAlpha = 0.5
	
	constructor( dlc_ )
	{
		::FrontEndStaticScreen.constructor( )
		allowFastInput = false
		inputHook = true
		displayItems = [ ]
		audioSource = ::GameApp.FrontEndPlayer.AudioSource
		
		dlc = dlc_
		currentSelection = 0
		ticketCount = 0
		slots = { }
		
		inputDelay = 0.3
	}
	
	function FinalizeIconSetup( )
	{
		SetTitle( "Menus_CampaignRationsDump" )
		
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
		
		local rect = ::GameApp.ComputeScreenSafeRect( )
		
		local player = ::GameApp.GetPlayerByUser( user )
		audioSource = player.AudioSource
		
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		// Profile Badge
		profileBadge = ::ProfileBadge( )
		profileBadge.SetUser( ::GameApp.FrontEndPlayer.User, user )
		profileBadge.SetPosition( rect.Left, rect.Top, 0 )
		profileBadge.EnableControl( false )
		AddChild( profileBadge )
		
		// Create Selector
		selector = ::AnimatingCanvas( )
			local pulser = ::AnimatingCanvas( )
				local selectorImage = ::Gui.TexturedQuad( )
				selectorImage.SetTexture( "gui/textures/rationtickets/rationticket_selector_g.png" )
				pulser.AddChild( selectorImage )
			pulser.AddAction( ::AlphaPulse( 0.8, 1.5, 1.0 ) )
			selector.AddChild( pulser )
		selector.CenterPivot( )
		selector.SetPosition( CalculatePosition( currentSelection ) )
		selector.SetZPos( 0.001 )
		AddChild( selector )
		
		// Create Text
		textCanvas = ::AnimatingCanvas( )
			titleText = ::Gui.Text( )
			titleText.SetFontById( FONT_SIMPLE_MED )
			titleText.SetRgba( COLOR_CLEAN_WHITE )
			titleText.BakeCString( " " )
			textCanvas.AddChild( titleText )
			
			descText = ::Gui.Text( )
			descText.SetFontById( FONT_SIMPLE_SMALL )
			descText.SetRgba( COLOR_CLEAN_WHITE )
			descText.BakeCString( " " )
			descText.SetPosition( titleText.LocalRect.Width + 20, titleText.GetYPos( ) + titleText.Height - descText.Height, 0 )
			textCanvas.AddChild( descText )
		textCanvas.SetPosition( rect.Center.x - 440, rect.Bottom - titleText.Height, 0 )
		AddChild( textCanvas )
		
		// Create Ticket Icons
		local levelInfos = [ ]
		local numLevels = ::GameApp.NumLevelsInTable( MAP_TYPE_CAMPAIGN )
		for( local i = 0; i < numLevels; ++i )
		{
			local info = ::GameApp.GetLevelLoadInfo( MAP_TYPE_CAMPAIGN, i )
			if( info.DlcNumber == dlc )
				levelInfos.push( info )
		}
		
		local scores = { }
		local userProfile = player.GetUserProfile( )
		foreach( i, info in levelInfos )
		{
			local score = userProfile.GetLevelScores( MAP_TYPE_CAMPAIGN, info.LevelIndex )
			scores[ i ] <- ( is_null( score )? null: score )
		}
		
		ticketCount = levelInfos.len( ) * 2
		
		// Create ration tickets
		local start = rect.Top + 100
		local deltaY = 30
		foreach( n, score in scores )
		{
			for( local i = 0; i < 2; ++i )
			{
				local index = (n * 2 + i).tointeger( )
				local display = AddDisplayItem( index, ::RationTicketImagePath( levelInfos[ n ].LevelIndex, i ) )
				
				if( !score.IsGoalComplete( i ) )
				{
					display.SetRgba( 0, 0, 0, inactiveAlpha )
				}
				
				local descLoc = ::GameApp.LocString( "DecorationsCase_Desc" ).Replace( "desc", ::RationTicketDescLocString( levelInfos[ n ].LevelIndex, i ) ).Replace( "levelName", levelInfos[ n ].MapDisplayName )
				AddSlotData( index, ::RationTicketNameLocString( levelInfos[ n ].LevelIndex, i ), descLoc )
			}
		}
		
		// Golden Arcade
		if( dlc == DLC_COLD_WAR )
		{
			local total = player.TotalGoldenArcadeCount
			local prog = player.GoldenArcadeFoundCount
			
			if( prog >= 1 )
			{
				local index = ticketCount
				AddDisplayItem( index, ::GoldenRationTicketImagePath( 0 ) )
				AddSlotData( index, ::GoldenRationTicketNameLocString( 0 ), ::GoldenRationTicketDescLocString( 0 ) )
				ticketCount++
			}
			
			if( prog == total )
			{
				local index = ticketCount
				AddDisplayItem( index, ::GoldenRationTicketImagePath( 1 ) )
				AddSlotData( index, ::GoldenRationTicketNameLocString( 1 ), ::GoldenRationTicketDescLocString( 1 ) )
				ticketCount++
			}
		}
		
		// Finalize
		Update( )
	}
	
	function AddDisplayItem( index, texture )
	{
		local display = ::Gui.AsyncTexturedQuad( )
				
		local pos = CalculatePosition( index )
		display.OnLoaded = function( quad ):(pos)
		{
			quad.CenterPivot( )
			quad.SetPosition( pos )
			quad.SetZPos( 0.0 )
		}
		
		display.SetTexture( texture )
		AddChild( display )
		displayItems.push( display )
		
		return display
	}
	
	function AddSlotData( index, displayName, displayDesc )
	{
		slots[ index ] <- {
			x = index % numColumns, 
			y = (index / numColumns).tointeger( ),
			name = displayName % ":",
			desc = displayDesc
		}
	}
	
	function HandleInput( gamepad )
	{
		/*
		local setAllItemsRgba = function( r, g, b, a )
		{
			foreach( item in displayItems )
				item.SetRgba( r, g, b, a )
		}
		
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_Y ) )
		{
			setAllItemsRgba( 1, 1, 1, 1 )
		}
		else if( gamepad.ButtonUp( GAMEPAD_BUTTON_Y ) )
		{
			setAllItemsRgba( 0, 0, 0, inactiveAlpha )
		}
		*/
	}
	
	function Update( )
	{
		MoveSelector( currentSelection )
		UpdateText( )
	}
	
	function UpdateText( )
	{
		// Update Text
		textCanvas.ClearActions( )
		textCanvas.AddAction( ::AlphaTween( textCanvas.GetAlpha( ), 0.0, 0.15 ) )
		textCanvas.AddDelayedAction( 0.15, ::AlphaTween( 0.0, 1.0, 0.4, null, null, function( canvas )
		{
			local slot = slots[ currentSelection ]
			titleText.BakeLocString( slot.name )
			//descText.BakeBoxLocString( 800, slot.desc )
			descText.BakeLocString( slot.desc )
			descText.SetPosition( titleText.Width + 10, titleText.GetYPos( ) + titleText.Height - descText.Height - 3, 0 )
			local rect = ::GameApp.ComputeScreenSafeRect( )
			textCanvas.SetXPos( rect.Center.x - ( titleText.Width + 10 + descText.Width ) * 0.5 )
		}.bindenv( this ) ) )
	}
	
	function CalculatePosition( index )
	{
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		local horizSpacing = 128
		local vertSpacing = 128 + 32 + 8
		
		local startX = vpSafeRect.Center.x - ((numColumns / 2) * horizSpacing) + (horizSpacing / 2)
		local startY = vpSafeRect.Top + 164
		
		local x = index % numColumns
		local y = (index / numColumns).tointeger( )
		
		return ::Math.Vec3.Construct( startX + x * horizSpacing, startY + y * vertSpacing, 0.01 )
	}
	
	function ChangeHighlight( delta )
	{
		if( delta != 0 && ticketCount > numColumns )
		{
			do
			{
				local x = currentSelection % numColumns
				local y = (currentSelection / numColumns).tointeger( )
				
				y += delta
				
				if( y * numColumns + x > ticketCount - 1 )
					y = 0
				if( y < 0 )
					y = (ticketCount / numColumns).tointeger( )
				
				currentSelection = y * numColumns + x
			}
			while( currentSelection < 0 || currentSelection > ticketCount - 1 )
			
			Update( )
		}
	}
	
	function ChangeHorizontalHighlight( delta )
	{
		if( delta != 0 )
		{
			currentSelection += delta
			while( currentSelection < 0 )
				currentSelection = ticketCount + currentSelection
			while( currentSelection > ticketCount - 1 )
				currentSelection -= ticketCount
			Update( )
		}
	}
	
	function MoveSelector( index )
	{
		PlaySound( "Play_UI_Scroll" )
		selector.ClearActions( )
		selector.AddAction( ::MotionTween( selector.GetPosition( ), CalculatePosition( index ), 0.3, null, EasingEquation.InOut ) )
	}
}