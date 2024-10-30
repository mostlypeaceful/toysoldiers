// The credits!

sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"

class CreditsCanvas extends AnimatingCanvas
{
	// Display
	movie = null
	
	// Data
	currentY = null
	velocity = null
	goalVelocity = null
	acceleration = null
	speedEvents = null
	specialEvents = null
	progress = null
	
	// Events
	onFinish = null
	
	// Statics
	static spacing = 50
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		currentY = 0
		velocity = 20
		goalVelocity = 60
		acceleration = 50
		speedEvents = { }
		specialEvents = { }
		progress = 0
		
		// DO CREDITS HERE: ////////////////////////////////////////////////////
		AddHeading( "Signal_Studios" )
		AddPerson( "D.R. Albright", "Title_President" )
		AddPerson( "Max Wagner", "Title_CTO" )
		AddPerson( "Jason Ilano", "Title_VicePresTechDir" )
		AddPerson( "Colin Tennery", "Title_VicePresProjDir" )
		AddSpace( )

		AddHeading( "Heading_ColdWarTeam" )
		AddSpace( )
		
		AddSubheading( "Heading_Art" )
		AddPerson( "Nick Vigna", "Title_LeadArtist" )
		AddPerson( "Corbett Sachter", "Title_LeadCharacterArtist" )
		AddPerson( "Brian Bassir", "Title_3DArtist" )
		AddPerson( "Steven Carroll", "Title_3DArtist" )
		AddPerson( "Jason Hammon", "Title_3DArtist" )
		AddPerson( "Jesse Havens", "Title_3DArtist" )
		AddPerson( "Ryan Gaule", "Title_FxArtist" )
		AddSpace( )
		
		AddSubheading( "Heading_Animation" )
		AddPerson( "Ben Hopper", "Title_Animator" )
		AddPerson( "Nick Kondo", "Title_Animator" )
		AddSpace( )
		
		AddSubheading( "Heading_Design" )
		AddPerson( "Robin Vincent", "Title_LeadDesigner" )
		AddPerson( "Darryl Taverner", "Title_Designer" )
		AddPerson( "Evan Pongress", "Title_Designer" )
		AddSpace( )
		
		AddSubheading( "Heading_Programming" )
		AddPerson( "Matt Kincaid", "Title_LeadProgrammer" )
		AddPerson( "Randall Knapp", "Title_Programmer" )
		AddPerson( "Josh Wittner", "Title_LeadEngineProgrammer" )
		AddPerson( "Robert West", "Title_Programmer" )
		AddPerson( "Chad Bramwell", "Title_Programmer" )
		AddPerson( "Patrick Walker", "Title_Programmer" )
		AddPerson( "Kyle Sorge-Toomey", "Title_ToolsPC" )
		AddSpace( )
		
		AddSubheading( "Header_QA" )
		AddPerson( "Rick Maher", "Title_QALead" )
		AddSpace( )
		
		AddSubheading( "Header_Memory" )
		AddPerson( "Peggy Jo Albright" )
		AddPerson( "Katie West" )
		AddPerson( "Amy Pashe" )
		AddPerson( "Gwendolyn Jaymes Easley" )
		AddSpace( )
		AddSpace( )
		AddSpace( )
		
		AddSpeedEvent( 180 )
		AddHeading( "Header_Microsoft" )
		AddSubheading( "Header_XBLATeam" )
		AddSubheading( "Header_Producer" )
		AddPerson( "Caesar Filori" )
		AddPerson( "Ross Perez" )
		
		AddSubheading( "Header_LeadProducer" )
		AddPerson( "Sean O'Connor" )
		
		AddSubheading( "Header_TestLead" )
		AddPerson( "Paul Loynd" )
		
		AddSubheading( "Header_ReleaseMgmt" )
		AddPerson( "Josh Mulanax" )
		AddPerson( "Tyler Keenan (Volt)" )
		
		AddSubheading( "Header_ProductPlanning" )
		AddPerson( "Cherie Lutz" )
		
		AddSubheading( "Header_Marketing" )
		AddPerson( "Michael Wolf" )
		AddPerson( "Daniel McConnell" )
		
		AddSubheading( "Header_ExecutiveProducer" )
		AddPerson( "Earnest Yuen" )
		
		AddSubheading( "Header_PublishingDirector" )
		AddPerson( "Ted Woolsey" )
		AddSpace( )
		
		AddSubheading( "Header_Audio" )
		AddPerson( "Mike Cody", "Title_AudioDirector" )
		AddPerson( "Jeff Linsenbigler", "Title_SoundSupervisor" )
		AddPerson( "Robbie Elias", "Title_SoundDesign" )
		AddPerson( "Brian Brocket", "Title_SoundDesign" )
		AddPerson( "Daniel Raimo", "Title_SoundDesign" )
		AddPerson( "Kristen Quebe", "Title_SoundDesign" )
		AddPerson( "David Liu", "Title_SoundDesign" )
		AddPerson( "Jordan Stock", "Title_AudioImplementation" )
		AddPerson( "Kira Anderson", "Title_AudioEngineer" )
		AddPerson( "Jason Shirley", "Title_VocalProducerForeign" )
		AddPerson( "Bill Black", "Title_VocalDirectorForeign" )
		AddPerson( "Nathaniel Papadakis", "Title_MusicComposer" )
		
		AddSubheading( "Header_VoiceActorsEnglish" )
		AddPerson( "Dex Manley" )
		AddPerson( "Tom Abernathy" )
		AddPerson( "Boyd Post" )
		AddPerson( "Mike Caviezel" )
		AddPerson( "Robbie Elias" )
		
		//AddSubheading( "Header_VoiceActorsRussian" )
		//AddPerson( "Place Holder" )
		//AddPerson( "Place Holder" )
		//AddPerson( "Place Holder" )
		//
		//AddSubheading( "Header_VoiceActorsVietnamese" )
		//AddPerson( "Place Holder" )
		AddSpace( )
		
		AddSubheading( "Header_MGSFirstParty" )
		AddPerson( "Andy Beaudoin", "Title_DesignDirector" )
		AddPerson( "Justin Swan", "Title_TestManager" )
		AddPerson( "Mac Smith", "Title_UserResearchEngineer" )
		AddPerson( "Deborah Henderson", "Header_VMCTestManager" )
		AddSpace( )
		
		AddPerson( "The Lillingtons" )
		AddPerson( "\"Russian Attack\"" )
		AddPerson( "Courtesy of Red Scare Records" )
		AddPerson( "Words and Music by Kody Templeman" )
		AddPerson( "Copyright 2006 Murdercar Music/(BMI)" )
		AddPerson( "All Rights Reserved.  Used by Permission." )
		AddSpace( )
		
		AddSubheading( "Header_VMCTestManager" )
		AddPerson( "Scott R. Griffiths" )
		
		AddSubheading( "Header_VMCSeniorTestLeads" )
		AddPerson( "Justin Davis" )
		AddPerson( "Anthony Tregre" )
		
		AddSubheading( "Header_VMCMATTestLead" )
		AddPerson( "Brian McCarty" )
		
		AddSubheading( "Header_VMCMATTester" )
		AddPerson( "Chris Hodges" )
		
		AddSubheading( "Header_VMCTestLead" )
		AddPerson( "Justin Davis" )

		AddSubheading( "Header_VMCTesters" )
		AddPerson( "John Davis Jr." )
		AddPerson( "Carlton Hagler" )
		AddPerson( "Jessika Jenkins" )
		AddPerson( "Adam Long" )
		AddPerson( "Robert C. Minnick" )
		AddPerson( "Brian Mishler" )
		AddPerson( "Joshua Pedersen" )
		AddPerson( "Brittany Wentink" )
		AddSpace( )
		AddSpace( )
		AddSpace( )
		AddSpeedEvent( 0 )
		AddSpace( )
		currentY -= 20
		
		AddHeading( "Signal_Studios" )
		AddPeople( "D.R. Albright", "Eric Arroyo", "Brian Bassir" )
		AddPeople( "Chad Bramwell", "Kristie Brigham", "Keith Canzoneri" )
		AddPeople( "Steven Carroll", "Ryan Gaule", "Jason Hammon" )
		AddPeople( "Jesse Havens", "Ben Hopper", "Jason Ilano" )
		AddPeople( "Matt Kincaid", "Randall Knapp", "Nick Kondo" )
		AddPeople( "Hardy LeBel", "Rick Maher", "Fred Pashe" )
		AddPeople( "Evan Pongress", "Corbett Sachter", "Kyle Sorge-Toomey" )
		AddPeople( "Darryl Taverner", "Colin Tennery", "Nick Vigna" )
		AddPeople( "Robin Vincent", "Max Wagner", "Patrick Walker" )
		AddPeople( "Robert West", null, "Josh Wittner" )

		////////////////////////////////////////////////////////////////////////
	}
	
	function AddHeading( heading )
	{
		_addText( FONT_FANCY_LARGE, ::GameApp.LocString( heading ), true )
		AddSpace( )
	}
	
	function AddSubheading( heading )
	{
		_addText( FONT_FANCY_MED, ::GameApp.LocString( heading ) )
		AddSpace( )
	}
	
	function AddPerson( name, title = null )
	{
		local locString = ::LocString.FromCString( name )
		if( title )
			locString = locString % " - " % ::GameApp.LocString( title )
		_addText( FONT_SIMPLE_SMALL, locString )
		AddSpace( )
	}
	
	function AddPeople( name1, name2, name3 )
	{
		local names = [ name1, name2, name3 ]
		local pos = [ -200, 0, 200 ]
		
		foreach( i, name in names )
		{
			if( name )
			{
				local name = _addText( FONT_SIMPLE_SMALL, ::LocString.FromCString( name ) )
				name.SetXPos( pos[ i ] )
			}
		}
		AddSpace( )
	}
	
	function AddSpace( )
	{
		currentY += spacing
	}
	
	function AddMovie( moviePath )
	{
		local movieSize = ::Math.Vec2.Construct( 1024, 576 )
		local moviePos = ::Math.Vec3.Construct( 0, currentY + movieSize.x * 0.5, 0 )
		
		// Background
		local bgSize = ::Math.Vec2.Construct( 1044, 596 )
		local bg = ::Gui.ColoredQuad( )
		bg.SetRgba( 0, 0, 0, 1 )
		bg.SetRect( bgSize )
		bg.SetPosition( moviePos.x - bgSize.x * 0.5, moviePos.y - bgSize.y * 0.5, moviePos.z )
		AddChild( bg )
		
		local movieStartOffset = 400
		currentY += movieStartOffset
		AddSpecialEvent( function( ):( moviePath, movieSize, moviePos )
		{
			movie = ::Gui.MovieQuad( )
			movie.SetMovie( moviePath, 1 )
			movie.SetRect( movieSize )
			movie.SetPosition( moviePos.x - movieSize.x * 0.5, moviePos.y - movieSize.y * 0.5, moviePos.z )
			AddChild( movie )
		}.bindenv( this ) )
		currentY -= movieStartOffset
		
		currentY += movieSize.y
		AddSpace( )
	}
	
	function _addText( font, locString, withUnderline = false )
	{
		local text = ::Gui.Text( )
		text.SetFontById( font )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( locString, TEXT_ALIGN_CENTER )
		text.SetPosition( 0, currentY, 0 )
		AddChild( text )
		
		if( withUnderline )
		{
			local line = ::Gui.TexturedQuad( )
			line.SetTexture( "gui/textures/score/score_decoration_g.png" )
			line.CenterPivot( )
			line.SetPosition( 0, currentY + text.Height, 0 )
			AddChild( line )
			
			AddSpace( )
		}
		
		return text
	}
	
	function AddSpeedEvent( newVelocity )
	{
		speedEvents[ currentY ] <- newVelocity
	}
	
	function AddSpecialEvent( func )
	{
		specialEvents[ currentY ] <- func
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		// Check Speed Events
		local toDelete = [ ]
		foreach( y, vel in speedEvents )
		{
			if( progress >= y )
			{
				goalVelocity = vel
				toDelete.push( y )
			}
		}
		foreach( y in toDelete )
			delete speedEvents[ y ]
			
		// Check Special Events
		toDelete = [ ]
		foreach( y, func in specialEvents )
		{
			if( progress >= y )
			{
				func( )
				toDelete.push( y )
			}
		}
		foreach( y in toDelete )
			delete specialEvents[ y ]
		
		// Update velocity
		local epsilon = 0.5
		if( velocity < goalVelocity - epsilon )
			velocity += acceleration * dt
		else if( velocity > goalVelocity + epsilon )
			velocity -= acceleration * dt
		
		// Move
		Translate( 0, -velocity * dt, 0 )
		progress += velocity * dt
		
		if( GetYPos( ) < -(currentY + 200) )
		{
			velocity = 0
			acceleration = 0
			
			if( onFinish )
			{
				onFinish( )
				onFinish = null
			}
		}
		
		if( velocity <= 0 && onFinish )
		{
			onFinish( )
			onFinish = null
		}
	}
}

class FrontEndCreditsMenu extends FrontEndStaticScreen
{
	// Display
	titleCanvas = null
	credits = null
	movie = null
	
	constructor( endGame = false )
	{
		::FrontEndStaticScreen.constructor( )
		SetTitle( "Menus_Credits" )
		controls.Clear( )
		if( ::GameApp.GameMode.IsFrontEnd || !::GameApp.CurrentLevel.VictoryOrDefeat )
			controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
		else
			controls.AddControl( GAMEPAD_BUTTON_B, "EndGame_Quit" )
		
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		// Set up title to hide
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		titleCanvas = ::AnimatingCanvas( )
			title.SetPosition( 0, 0, 0 )
			titleCanvas.AddChild( title )
			decoration.SetPosition( title.GetXPos( ), title.GetYPos( ) + title.Height + 5, 0  )
			titleCanvas.AddChild( decoration )
		titleCanvas.SetPosition( vpSafeRect.Center.x, vpSafeRect.Top + 10, 0 )
		AddChild( titleCanvas )
		
		// Credits
		credits = ::CreditsCanvas( )
		credits.SetPosition( vpSafeRect.Center.x, vpSafeRect.Center.y, 0.001 )
		credits.SetAlpha( 0.0 )
		AddChild( credits )
		
		credits.FadeIn( 2.0 )
		if( endGame )
			credits.onFinish = AddMovie.bindenv( this )
		
		// Hide Title after a bit
		titleCanvas.AddDelayedAction( 2.5, ::AlphaTween( 1.0, 0.0, 2.0 ) )
		
		// Music
		::GameApp.AudioEvent( "Play_UI_Credits" )
	}
	
	function AddMovie( )
	{
		local vpSafeRect = ::GameApp.ComputeScreenSafeRect( )
		local movieSize = ::Math.Vec2.Construct( 1024, 576 )
		local moviePos = ::Math.Vec3.Construct( vpSafeRect.Center.x, vpSafeRect.Center.y, 0 )
		
		// Background
		local bgSize = ::Math.Vec2.Construct( 1044, 596 )
		local bgCanvas = ::AnimatingCanvas( )
			local bg = ::Gui.ColoredQuad( )
			bg.SetRgba( 0, 0, 0, 1 )
			bg.SetRect( bgSize )
			bgCanvas.AddChild( bg )
		bgCanvas.SetPosition( moviePos.x - bgSize.x * 0.5, moviePos.y - bgSize.y * 0.5, moviePos.z )
		bgCanvas.SetAlpha( 0 )
		AddChild( bgCanvas )
		
		movie = ::Gui.MovieQuad( )
		movie.SetMovie( "game:\\video\\endgame.wmv", 0 )
		movie.SetRect( movieSize )
		movie.SetPosition( moviePos.x - movieSize.x * 0.5, moviePos.y - movieSize.y * 0.5, moviePos.z )
		
		movie.OnFinished = function( ) { ::print( "movie finish" ); AutoBackOut = true }.bindenv( this )
		
		bgCanvas.AddDelayedAction( 5.0, ::AlphaTween( 0.0, 1.0, 1.0, null, null, null, function( canvas )
		{
			::print( "movie start" )
			credits.FadeOut( 0.3 )
			AddChild( movie )
			canvas.ClearActions( )
		}.bindenv( this ) ) )
	}
	
	function OnBackOut( )
	{
		::GameApp.AudioEvent( "Play_UI_Select_Backward" )
		
		if( movie )
		{
			::print( "movie delete" )
			RemoveChild( movie )
			movie.DeleteSelf( )
			movie = null
		}
		
		// Music Stop
		::GameApp.AudioEvent( "Stop_UI_Credits" )
		
		if( ::GameApp.GameMode.IsFrontEnd )
			return true
		else if( ::GameApp.CurrentLevel.VictoryOrDefeat )
		{
			::SetFrontEndMainMenuRestart( )
			::ResetGlobalDialogBoxSystem( )
			::GameApp.LoadFrontEnd( )
			return false
		}
		else
			return true
	}
}

