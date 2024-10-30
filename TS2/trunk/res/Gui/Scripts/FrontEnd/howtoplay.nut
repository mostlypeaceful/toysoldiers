
// Requires
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"

// Resources
sigimport "gui/textures/misc/versus_decoration_g.png"

HowToPlayData <- [ // Array of pages (as tables)
	{
		title = "HTP_Title_01",
		paragraphs = [
			"HTP_Paragraph_01A",
		],
		bullets = [  ]
	},
	///////////////////////////
	{
		title = "HTP_Title_02",
		paragraphs = [
			"HTP_Paragraph_02A",
		],
		bullets = [  ]
	},
	///////////////////////////
	{
		title = "HTP_Title_03",
		paragraphs = [
			"HTP_Paragraph_03A",
		],
		bullets = [  ]
	},
	///////////////////////////
	{
		title = "HTP_Title_04",
		paragraphs = [
			"HTP_Paragraph_04A",
		],
		bullets = [  ]
	},
	///////////////////////////
	{
		title = "HTP_Title_05",
		paragraphs = [
			"HTP_Paragraph_05A",
		],
		bullets = [  ]
	},
	///////////////////////////
	{
		title = "HTP_Title_06",
		paragraphs = [
			"HTP_Paragraph_06A",
		],
		bullets = [  ]
	},
	///////////////////////////
	{
		title = "HTP_Title_07",
		paragraphs = [
			"HTP_Paragraph_07A",
		],
		bullets = [  ]
	},
]

HowToPlay_UseMediumFont <- false
HowToPlay_BulletsUseMediumFont <- false

class FrontEndHowToPlayMenu extends FrontEndStaticScreen
{
	// Display
	pages = null
	titles = null
	pageCounter = null
	
	// Data
	currentPage = null
	
	// Statics
	static pageWidth = 860 
	
	constructor( )
	{
		::FrontEndStaticScreen.constructor( )
		local vpSafeRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		//inputHook = true
		SetTitle( "Menus_HowToPlay" )
		menuPositionOffset = ::Math.Vec3.Construct( vpSafeRect.Left + 15, subtitle.GetYPos( ) + 100 + 24, 0 )
		//controls.AddControl( [ GAMEPAD_BUTTON_LSHOULDER, GAMEPAD_BUTTON_RSHOULDER ], "LB_Pages" )
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
		
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		pages = [ ]
		titles = [ ]
		currentPage = 0
		
		local lineX = ( ::GameApp.IsAsianLanguage( ) ? 270 : 300 )
		
		// Set Up
		foreach( pageData in ::HowToPlayData )
		{
			local title = pageData.title
			titles.push( title )
			
			local page = AddPage( pageData )
			page.SetPosition( vpSafeRect.Left + lineX + 20, subtitle.GetYPos( ) + 100, 0 )
			page.SetAlpha( 0 )
		}
		
		// Page Number Text
		/*pageCounter = ::Gui.Text( )
		pageCounter.SetFontById( FONT_SIMPLE_SMALL )
		pageCounter.SetRgba( COLOR_CLEAN_WHITE )
		pageCounter.SetPosition( vpSafeRect.Center.x, vpSafeRect.Bottom - 100, 0 )
		AddChild( pageCounter )*/
		
		// Line
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/misc/versus_decoration_g.png" )
		line.SetPosition( vpSafeRect.Left + lineX, subtitle.GetYPos( ) + 100 , 0 )
		AddChild( line )
		
		// Init
		if( pages.len( ) > 0 )
			ShowPage( currentPage )
	}
	
	function FinalizeIconSetup( )
	{
		foreach( pageData in ::HowToPlayData )
		{
			local entry = ::FrontEndMenuEntry( pageData.title, null, null )
			if( ::GameApp.IsWideLanguage( ) )
				entry.SetActiveInactiveScale( ::Math.Vec2.Construct( 0.6, 0.6 ), ::Math.Vec2.Construct( 0.8, 0.9 ) )
			icons.push( entry )
		}
		::VerticalMenu.FinalizeIconSetup( )
	}
	
	function AddPage( data )
	{
		local page = ::AnimatingCanvas( )
		local y = 0
		local paragraphSpacing = 10
		local bulletSpacing = 4
		
		// Paragraphs
		foreach( para in data.paragraphs )
			y = AddParagraph( page, y, para ) + paragraphSpacing
			
		y += paragraphSpacing
		
		// Bullets
		foreach( bullet in data.bullets )
			y = AddBulletpoint( page, y, bullet ) + bulletSpacing
		
		pages.push( page )
		AddChild( page )
		return page
	}
	
	function AddParagraph( page, y, locId )
	{
		local paraText = ::Gui.Text( )
		paraText.SetFontById( ::HowToPlay_UseMediumFont? FONT_SIMPLE_MED: FONT_SIMPLE_SMALL )
		paraText.SetRgba( COLOR_CLEAN_WHITE )
		paraText.SetPosition( 0, y, 0 )
		paraText.BakeBoxLocString( pageWidth - ( ::GameApp.IsAsianLanguage( ) ? 0 : 10 ), ::GameApp.LocString( locId ), TEXT_ALIGN_LEFT )
		page.AddChild( paraText )
		
		return y + paraText.Height
	}
	
	function AddBulletpoint( page, y, locId )
	{
		local bulletIndent = 30
		
		local bulletText = ::ControllerButton( -1, locId, null, HowToPlay_BulletsUseMediumFont? FONT_SIMPLE_MED: FONT_SIMPLE_SMALL )
		bulletText.SetPosition( bulletIndent, y, 0 )
		page.AddChild( bulletText )
		
		return y + bulletText.GetSize( ).Height
	}
	
	/*function TurnPage( shift )
	{
		if( shift < 0 )
			shift = -1
		else if( shift > 0 )
			shift = 1
			
		local newPage = currentPage + shift
		if( currentPage == newPage )
			return
		
		if( newPage < 0 )
			newPage = pages.len( ) - 1
		if( newPage > pages.len( ) - 1 )
			newPage = 0
		
		HidePage( currentPage )
		currentPage = newPage
		ShowPage( currentPage )
	}*/
	
	function HidePage( index )
	{
		local page = pages[ index ]
		page.SetAlpha( 0 )
	}
	
	function ShowPage( index )
	{
		SetSubtitle( titles[ index ] )
		local page = pages[ index ]
		page.SetAlpha( 1 )
		//SetPageNumber( index )
	}
	
	/*function SetPageNumber( index )
	{
		local pageCountString = ::GameApp.LocString( "Page" ).Replace( "current", ( index + 1 ).tointeger( ) ).Replace( "total", pages.len( ).tointeger( ) )
		
		pageCounter.SetAlpha( 1 )
		if( pages.len( ) > 1 )
			pageCounter.BakeLocString( pageCountString, TEXT_ALIGN_CENTER )
		else
			pageCounter.SetAlpha( 0 )
	}*/
	
	function ChangeHighlight( delta )
	{
		HidePage( currentPage )
		::FrontEndStaticScreen.ChangeHighlight( delta )
		currentPage = highlightIndex
		ShowPage( currentPage )
		ShowPage( highlightIndex )
	}
	
	/*function HandleInput( gamepad )
	{
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_LSHOULDER ) )
		{
			TurnPage( -1 )
		}
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_RSHOULDER ) )
		{
			TurnPage( 1 )
		}
	}*/
}
