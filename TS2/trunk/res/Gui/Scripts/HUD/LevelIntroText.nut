
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/controllerbutton.nut"
sigimport "gui/scripts/utility/rationticketscripts.nut"

class LevelIntroText extends AnimatingCanvas
{
	// Screen Data
	skipButton = null
	objectiveLabelText = null
	objectiveText = null
	rationTicketLabelText = null
	rationTicketRequirementText = null
	objectiveMargins = null // number: Width of margin on either side of the objective/ration ticket text
	alphaContainer = null
	blackScreen = null
	
	// Fade Data
	fadeItems = [ ]
	fadeVelocity = 0
	
	// Progression Data
	stages =
	{
		timer = 0,
		currentStage = 0,
		[0] = {
			time = 0
		},
		[1] = {
			time = 2
		},
		[2] = {
			time = 5
		},
		[3] = {
			time = 18
		}
	}
	
	constructor( levelNumber, mapType, skipable )
	{
		::AnimatingCanvas.constructor( )
		
		// Set initial values
		fadeItems = [ ]
		fadeVelocity = 0
		stages.timer = 0
		stages.currentStage = 0
		objectiveMargins = GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( ).Width * 0.025
		
		// The level number of this level
		local n = levelNumber
		
		// Get the string data for this level
		local objectiveLabel = null
		local objective = null
		local rationTicketLabel = null
		local rationTicketRequirement = null
		
		if( mapType == null )
			mapType = MAP_TYPE_CAMPAIGN

		objectiveLabel = ::GameApp.LocString( ::GameApp.LevelNameInTable( mapType, n ) + "_Global_Intro" )
		objective = ::GameApp.LocString( ::GameApp.LevelNameInTable( mapType, n ) + "_Global_Objective" )
		rationTicketLabel = ::GameApp.LocString( "Ration_Tickets" )
		
		if( mapType == MAP_TYPE_CAMPAIGN )
		{
			local difficulty = ::GameApp.CurrentLevelLoadInfo.Difficulty
			if( difficulty == DIFFICULTY_ELITE )
				objective = ::GameApp.LocString( "Campaign_Elite_Global_Objective" )
			else if( difficulty == DIFFICULTY_GENERAL )
				objective = ::GameApp.LocString( "Campaign_General_Global_Objective" )
		}
		else if( mapType == MAP_TYPE_SURVIVAL )
		{
			local mode = ::GameApp.CurrentLevelLoadInfo.ChallengeMode
			if( mode == CHALLENGE_MODE_LOCKDOWN )
				objective = ::GameApp.LocString( "Challenge_Lockdown_Global_Objective" )
			else if( mode == CHALLENGE_MODE_HARDCORE )
				objective = ::GameApp.LocString( "Challenge_Hardcore_Global_Objective" )
			else if( mode == CHALLENGE_MODE_TRAUMA )
				objective = ::GameApp.LocString( "Challenge_Trauma_Global_Objective" )
			else if( mode == CHALLENGE_MODE_COMMANDO )
				objective = ::GameApp.LocString( "Challenge_HardLock_Global_Objective" )
		}
		
		local info = ::GameApp.GetLevelLoadInfo( mapType, n )
		rationTicketRequirement = ::RationTicketFullDescLocString( info.LevelIndex, 0 ) % "\n" % ::RationTicketFullDescLocString( info.LevelIndex, 1 )
		
		local vpRect = GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		local z = 0
		
		// Level Intro Texts
		objectiveLabelText = Gui.Text( ) 
		objectiveLabelText.SetFontById( FONT_FANCY_LARGE ) 
		if( n >= 0 ) objectiveLabelText.BakeLocString( objectiveLabel, TEXT_ALIGN_CENTER )
		else		objectiveLabelText.BakeCString( "Level number is less than 1", TEXT_ALIGN_CENTER )
		objectiveLabelText.SetRgba( COLOR_CLEAN_WHITE ) 
		objectiveLabelText.SetAlpha( 0 )
		
		objectiveText = Gui.Text( ) 
		objectiveText.SetFontById( FONT_SIMPLE_MED ) 
		if( n >= 0 ) objectiveText.BakeBoxLocString( vpRect.Width - 2.0 * objectiveMargins, objective, TEXT_ALIGN_CENTER ) 
		else		objectiveText.BakeCString( "You must set the level number in your level's OnSpawn method by using SetLevelNumber( # ).", TEXT_ALIGN_CENTER )
		objectiveText.SetRgba( COLOR_CLEAN_WHITE ) 
		objectiveText.SetAlpha( 0 )
		
		local hasRationTickets = ( mapType == MAP_TYPE_CAMPAIGN )
		
		if( hasRationTickets )
		{
			rationTicketLabelText = Gui.Text( ) 
			rationTicketLabelText.SetFontById( FONT_FANCY_MED ) 
			if( n >= 0 ) rationTicketLabelText.BakeLocString( rationTicketLabel, TEXT_ALIGN_CENTER ) 
			else		rationTicketLabelText.BakeCString( "\"Sorry.\"", TEXT_ALIGN_CENTER )
			rationTicketLabelText.SetRgba( COLOR_CLEAN_WHITE ) 
			rationTicketLabelText.SetAlpha( 0 )

			rationTicketRequirementText = Gui.Text( ) 
			rationTicketRequirementText.SetFontById( FONT_SIMPLE_SMALL ) 
			if( n >= 0 ) rationTicketRequirementText.BakeBoxLocString( vpRect.Width - 2.0 * objectiveMargins, rationTicketRequirement, TEXT_ALIGN_CENTER ) 
			else		rationTicketRequirementText.BakeCString( "<3 Randy" )
			rationTicketRequirementText.SetRgba( COLOR_CLEAN_WHITE ) 
			rationTicketRequirementText.SetAlpha( 0 )
		}
		
		if( skipable )
		{
			// Skip Button
			skipButton = ControllerButton( "Gui/Textures/Gamepad/button_a_g.png", "Skip", ControllerButtonAlignment.LEFT, FONT_SIMPLE_MED )
			skipButton.SetAlpha( 0 )
		}
		
		// Positions
		objectiveLabelText.SetPosition( Math.Vec3.Construct( vpRect.Center.x, vpRect.Center.y - objectiveLabelText.LocalRect.Height, z ) )
		objectiveText.SetPosition( Math.Vec3.Construct( vpRect.Center.x, vpRect.Center.y + 0, z ) )
		if( hasRationTickets )
		{
			rationTicketLabelText.SetPosition( Math.Vec3.Construct( vpRect.Center.x, vpRect.Center.y + 100, z ) )
			rationTicketRequirementText.SetPosition( Math.Vec3.Construct( vpRect.Center.x, vpRect.Center.y + 140, z ) )
		}
		if( skipable )
		{
			skipButton.SetPosition( Math.Vec3.Construct( vpRect.Right - skipButton.GetSize( ).Width, vpRect.Bottom - (skipButton.GetSize( ).Height * 0.5), z ) )
		}
		
		// Lines
		// TODO: Make Lines and stuff
		
		// Create Alpha-able container
		alphaContainer = ::AnimatingCanvas( )
		alphaContainer.AddChild( objectiveLabelText )
		alphaContainer.AddChild( objectiveText )
		if( rationTicketLabelText && rationTicketRequirementText )
		{
			alphaContainer.AddChild( rationTicketLabelText )
			alphaContainer.AddChild( rationTicketRequirementText )
		}
		if( skipButton )
		{
			alphaContainer.AddChild( skipButton )
		}
		
		// Create Black Screen
		blackScreen = ::AnimatingCanvas( )
			local blackQuad = ::Gui.ColoredQuad( )
			blackQuad.SetRgba( ::Math.Vec4.Construct( 0, 0, 0, 1 ) )
			blackQuad.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
			blackScreen.AddChild( blackQuad )
		blackScreen.SetPosition( 0, 0, -0.01 )
		blackScreen.SetAlpha( 0 )
		AddChild( blackScreen )
		
		// Set position
		SetPosition( 0, 0, 0.6 )
		
		// Add to screen
		AddChild( alphaContainer )
		
		// Set up progression
		stages[0].action <- function() 
		{
			fadeVelocity = 2
			fadeItems = [ objectiveLabelText, skipButton ]
		}.bindenv(this)
		stages[1].action <- function() 
		{
			fadeVelocity = 2
			fadeItems = [ objectiveText ]
		}.bindenv(this)
		stages[2].action <- function() 
		{
			fadeVelocity = 2
			fadeItems = [ rationTicketLabelText, rationTicketRequirementText ]
		}.bindenv(this)
		stages[3].action <- function() 
		{
			fadeVelocity = -2
			fadeItems = [ objectiveLabelText, objectiveText, rationTicketLabelText, rationTicketRequirementText ]
		}.bindenv(this)
	}
	
	function FadeThroughBlackAndDelete( )
	{
		local fadeOutTime = 0.3
		local fadeInTime = 0.5
		alphaContainer.AddAction( ::AlphaTween( 1.0, 0.0, fadeOutTime, null, null, null, function( canvas )
		 {
			 DeleteSelf( )
		 }.bindenv( this ) ) )
		//blackScreen.AddAction( ::AlphaTween( 0.0, 1.0, fadeOutTime ) )
		//blackScreen.AddDelayedAction( fadeOutTime, ::AlphaTween( 1.0, 0.0, fadeInTime, null, null, null, function( canvas )
		// {
		//	 DeleteSelf( )
		// }.bindenv( this ) ) )
	}
	
	function OnTick( dt )
	{
		if( ::GameApp.Paused( ) )
		{
			alphaContainer.SetAlpha( 0 )
			return
		}
		else
		{
			alphaContainer.SetAlpha( 1 )
		}
			
		::AnimatingCanvas.OnTick( dt )

		foreach( i,item in fadeItems )
		{
			if( item == null )
				continue
				
			local a = item.GetAlpha( ) + (fadeVelocity * dt)
			item.SetAlpha( a )
			
			if( a < 0 || a > 1 )
				fadeItems.remove(i)
		}
		
		stages.timer += dt
		if( (stages.currentStage in stages) && stages.timer > stages[ stages.currentStage ].time )
		{
			stages[ stages.currentStage ].action()
			stages.currentStage++
		}
	}
}