// Ration Ticket notifications

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/utility/rationticketscripts.nut"
sigimport "gui/scripts/utility/goldenarcadescripts.nut"

// Resources
sigimport "gui/textures/rationtickets/rationticket_notify_background_g.png"
sigimport "gui/textures/rationtickets/rationticket_progress_g.png"
sigimport "gui/textures/rationtickets/rationticket_complete_g.png"
sigimport "gui/textures/rationtickets/rationticket_failed_g.png"
sigimport "gui/textures/frontend/rank_star_01_g.png"
sigimport "gui/textures/frontend/rank_star_02_g.png"
sigimport "gui/textures/frontend/rank_star_03_g.png"
sigimport "gui/textures/frontend/rank_star_04_g.png"
sigimport "gui/textures/radialmenus/blank_yellow_g.png"
sigimport "gui/textures/rationtickets/goldenarcade1_g.png"
sigimport "gui/textures/rationtickets/goldenarcade11_g.png"
sigimport "gui/textures/rationtickets/decoration_bg_g.png"
// dog tags and babooskas are imported in dlc_imports.nut

class RationTicketNotification extends AnimatingCanvas
{
	// Display
	background = null
	icon = null
	text1 = null
	text1Canvas = null
	text2 = null
	text2Canvas = null
	
	// Consts
	static Width = 300
	static Height = 60
	
	// Data
	inGoalX = null
	outGoalX = null
	shown = null // bool, true after Show is finished, false after Hide is started
	hideTimer = null
	
	constructor( locString1, locString2, iconPath, alt = false )
	{
		::AnimatingCanvas.constructor( )
		
		shown = false
		hideTimer = 0
		local green = ::Math.Vec4.Construct( 0.768, 0.851, 0.768, 1.0 )
		
		background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/rationtickets/rationticket_notify_background_g.png" )
		background.SetPosition( 0, 0, 0.002 )
		AddChild( background )
		
		local iconImage = null
		icon = ::AnimatingCanvas( )
			if( iconPath )
			{
				iconImage = ::Gui.TexturedQuad( )
				iconImage.SetTexture( iconPath )
				icon.AddChild( iconImage )
			}
		icon.CenterPivot( )
		icon.SetPosition( Width - 36, Height / 2, 0.001 )
		AddChild( icon )
		
		local align = ( alt? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT )
		
		text1Canvas = ::AnimatingCanvas( )
			text1 = ::Gui.Text( )
			text1.SetFontById( FONT_SIMPLE_SMALL )
			text1.SetRgba( green )
			text1.BakeLocString( locString1, align )
			text1Canvas.AddChild( text1 )
		text1Canvas.SetPosition( 12, 5, 0 )
		text1Canvas.SetAlpha( 0 )
		AddChild( text1Canvas )
		
		text2Canvas = ::AnimatingCanvas( )
			text2 = ::Gui.Text( )
			text2.SetFontById( FONT_SIMPLE_MED )
			text2.SetRgba( green )
			text2.BakeLocString( locString2, align )
			text2Canvas.AddChild( text2 )
		text2Canvas.SetPosition( 15, 24, 0 )
		text2Canvas.SetAlpha( 0 )
		AddChild( text2Canvas )
		
		// Resize to fit
		local availableTextSpace = ( Width - 72 )
		if( iconImage )
			availableTextSpace = ( Width - 50 - iconImage.TextureDimensions( ).x * 0.5 )
		text1.Compact( availableTextSpace )
		text2.Compact( availableTextSpace )
		
		if( alt )
		{
			icon.SetXPos( 36 )
			text1Canvas.SetXPos( Width - 12 )
			text2Canvas.SetXPos( Width - 15 )
		}
		
		inGoalX = 0
		outGoalX = 0
	}
	
	function SetGoalPositions( inGoal, outGoal )
	{
		inGoalX = inGoal
		outGoalX = outGoal
	}
	
	function OnTick( dt )
	{
		if( ::GameApp.Paused( ) )
			return
			
		::AnimatingCanvas.OnTick( dt )
		
		if( hideTimer < 0 )
			Hide( )
		else
			hideTimer -= dt
	}

	function ShowText( )
	{
		text1Canvas.ClearActions( )
		text2Canvas.ClearActions( )
		text1Canvas.AddAction( ::AlphaTween( text1Canvas.GetAlpha( ), 1, 0.3 ) )
		text2Canvas.AddAction( ::AlphaTween( text2Canvas.GetAlpha( ), 1, 0.3 ) )
	}
	
	function HideText( )
	{
		text1Canvas.ClearActions( )
		text2Canvas.ClearActions( )
		text1Canvas.AddAction( ::AlphaTween( text1Canvas.GetAlpha( ), 0, 0.2 ) )
		text2Canvas.AddAction( ::AlphaTween( text2Canvas.GetAlpha( ), 0, 0.2 ) )
	}
	
	function Show( )
	{
		hideTimer = 4.0
		
		if( !shown )
		{
			shown = true
			ClearActions( )
			AddAction( ::XMotionTween( GetXPos( ), inGoalX, 0.4, null, null, null, function( canvas )
			{
				canvas.ShowText( )
			} ) )
		}
	}
	
	function Hide( )
	{
		if( shown )
		{
			shown = false
			HideText( )
			ClearActions( )
			AddAction( ::XMotionTween( GetXPos( ), outGoalX, 0.4, null, EasingEquation.In ) )
		}
	}
	
	function HideAndDie( )
	{
		if( shown )
		{
			shown = false
			HideText( )
			ClearActions( )
			AddAction( ::XMotionTween( GetXPos( ), outGoalX, 0.4, null, EasingEquation.In, null, function( canvas ) { canvas.DeleteSelf( ) } ) )
		}
	}
}

class RationTicketEarned extends RationTicketNotification
{
	constructor( levelIndex, ticketIndex, alt = false )
	{
		local youEarnedLocString = ::GameApp.LocString( "You_Earned" ).ReplaceCString( "itemName", "" )
		local ticketNameLocString = ::RationTicketNameLocString( levelIndex, ticketIndex )
		
		::RationTicketNotification.constructor( youEarnedLocString, ticketNameLocString, null, alt )

		local image = ::Gui.AsyncTexturedQuad( )
		image.SetTexture( ::RationTicketImagePath( levelIndex, ticketIndex ) )
		local scale = 0.7
		image.SetUniformScale( scale )
		image.SetPosition( -64 * scale, -80 * scale, -0.001 )
		icon.AddChild( image )
	}
}

class NewRankEarned extends RationTicketNotification
{
	constructor( rankIndex, user = null, alt = false )
	{
		local levelInfo = ::GameApp.CurrentLevelLoadInfo
		local newRankLocString = ::GameApp.LocString( "Earned_RankName" ).Replace( "i", rankIndex + 1 )
		local descLocString = ::GameApp.LocString( "Slide_RankDescFormat" ).Replace( "threshold", levelInfo.RankThreshold( rankIndex ) ).Replace( "desc", levelInfo.RankDesc )
		
		::RationTicketNotification.constructor( newRankLocString, descLocString,  "gui/textures/frontend/rank_star_0" + (rankIndex + 1).tostring( ) + "_g.png", alt )
	}
}

class RationTicketFailed extends RationTicketNotification
{
	constructor( levelIndex, ticketIndex, alt = false )
	{
		local ticketNameLocString = ::RationTicketNameLocString( levelIndex, ticketIndex )
		local failedLocString = ::GameApp.LocString( "Failed" )
		
		::RationTicketNotification.constructor( ticketNameLocString, failedLocString, "gui/textures/rationtickets/rationticket_failed_g.png", alt )
		local red = ::Math.Vec4.Construct( 1.0, 0.694, 0.694, 1.0 )
		text1.SetRgba( red )
		text2.SetRgba( red )
	}
}

class RationTicketProgress extends RationTicketNotification
{
	// Data
	levelIndex = null
	ticketIndex = null
	value = null
	max = null
	align = null
	
	constructor( levelIndex_, ticketIndex_, value_, max_, alt = false )
	{
		levelIndex = levelIndex_
		ticketIndex = ticketIndex_
		value = value_
		max = max_
		align = ( alt? TEXT_ALIGN_RIGHT: TEXT_ALIGN_LEFT )
		
		local ticketNameLocString = ::RationTicketNameLocString( levelIndex, ticketIndex ) % ":"
		local progressLocString = ::RationTicketProgressLocString( levelIndex, ticketIndex, value.tointeger( ), max.tointeger( ) )
		
		::RationTicketNotification.constructor( ticketNameLocString, progressLocString, "gui/textures/rationtickets/rationticket_progress_g.png", alt )
	}
	
	function Update( value_ )
	{
		value = value_
		local progressLocString = ::RationTicketProgressLocString( levelIndex, ticketIndex, value.tointeger( ), max.tointeger( ) )
		text2.BakeLocString( progressLocString, align )
		Show( )
	}
}

class GoldenArcadeRationTicket extends RationTicketNotification
{
	constructor( index, alt = false )
	{
		local youEarnedLocString = ::GameApp.LocString( "You_Earned" ).ReplaceCString( "itemName", "" )
		local ticketNameLocString = ::GoldenRationTicketNameLocString( index )
		
		::RationTicketNotification.constructor( youEarnedLocString, ticketNameLocString, null, alt )
		local gray = ::Math.Vec4.Construct( 0.5, 0.5, 0.5, 1.0 )
		local gold = ::Math.Vec4.Construct( 1.0, 0.85, 0.0, 1.0 )
		text1.SetRgba( gray )
		text2.SetRgba( gold )

		local image = ::Gui.AsyncTexturedQuad( )
		image.SetTexture( ::GoldenRationTicketImagePath( index ) )
		image.SetUniformScale( 0.5 )
		image.SetPosition( -32, -40, -0.001 )
		icon.AddChild( image )
	}
}

class GoldenObjectNotification extends RationTicketNotification
{
	constructor( line1, line2, texture, alt = false )
	{
		::RationTicketNotification.constructor( line1, line2, texture, alt )
		local gray = ::Math.Vec4.Construct( 0.5, 0.5, 0.5, 1.0 )
		local gold = ::Math.Vec4.Construct( 1.0, 0.85, 0.0, 1.0 )
		text1.SetRgba( gray )
		text2.SetRgba( gold )
	}
}

class GoldenArcadeFoundNotification extends GoldenObjectNotification
{
	constructor( alt = false )
	{
		local line1 = ::GameApp.LocString( "Earned_GoldenArcade" )
		local line2 = ::GameApp.LocString( "Golden_Arcade_Destroyed" )
		
		::GoldenObjectNotification.constructor( line1, line2, ::GoldenArcadeIconTexture( ), alt )
	}
}

class GoldenBabushkasFoundNotification extends GoldenObjectNotification
{
	constructor( alt = false )
	{
		local line1 = ::GameApp.LocString( "Earned_GoldenBabushka" )
		local line2 = ::GameApp.LocString( "Golden_Babushka_All_Found" )
		
		::GoldenObjectNotification.constructor( line1, line2, ::GoldenBabushkaIconTexture( ), alt )
	}
}

class GoldenBabushkasProgressNotification extends GoldenObjectNotification
{
	constructor( progress, alt = false )
	{
		local line1 = ::GameApp.LocString( "Golden_Babushka_Destroyed" )
		local line2 = ::LocString.FromCString( progress.tostring( ) + " / " + ::LevelScores.GoldenObjectsPerLevel( ).tostring( ) )
		
		::GoldenObjectNotification.constructor( line1, line2, "gui/textures/hud/golden_babushka_progress_g.png", alt )
	}
}

class GoldenDogTagsFoundNotification extends GoldenObjectNotification
{
	constructor( alt = false )
	{
		local line1 = ::GameApp.LocString( "Earned_GoldenDogTag" )
		local line2 = ::GameApp.LocString( "Golden_DogTag_All_Found" )
		
		::GoldenObjectNotification.constructor( line1, line2, ::GoldenDogTagIconTexture( ), alt )
	}
}

class GoldenDogTagsProgressNotification extends GoldenObjectNotification
{
	constructor( progress, alt = false )
	{
		local line1 = ::GameApp.LocString( "Golden_DogTag_Destroyed" )
		local line2 = ::LocString.FromCString( progress.tostring( ) + " / " + ::LevelScores.GoldenObjectsPerLevel( ).tostring( ) )
		
		::GoldenObjectNotification.constructor( line1, line2, "gui/textures/hud/golden_dogtag_progress_g.png", alt )
	}
}
