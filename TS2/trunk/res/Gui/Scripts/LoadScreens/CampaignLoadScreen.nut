// Campaign load screen

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

function LevelHasBriefing( levelIndex )
{
	switch( levelIndex )
	{
		case 1:
		case 3:
		case 6:
		case 8:
		case 10:
		case 11:
		case 14:
		return true
		
		default:
		return false
	}
}

function BriefingBackground( levelIndex, slide, dlc = DLC_COLD_WAR )
{
	local dlcPrefixes = [ "", "evilempire_", "napalm_" ]
	local dlcPrefix = dlcPrefixes[ dlc ]
	
	if( slide == 0 )
		return "gui/textures/loadscreens/campaign/" + dlcPrefix + "briefing_bg_g.png"
	else
		return "gui/textures/loadscreens/campaign/" + dlcPrefix + "briefing_bg" + levelIndex.tostring( ) + "_g.png"
}

function BriefingSubjectLoc( levelIndex )
{
	return ( "Briefing" + levelIndex.tostring( ) + "_Subject" )
}

function BriefingSlideLoc( levelIndex, slide )
{
	return ( "Briefing" + levelIndex.tostring( ) + "_Slide" + slide.tostring( ) )
}

class TypingText extends AnimatingCanvas
{
	// Display
	text = null
	
	// Data
	currentIndex = null
	loc = null
	width = null
	speed = null
	timer = null
	delay = null
	align = null
	finished = null
	Height = null
	length = null
	
	// Events
	onFinished = null
	
	constructor( loc_, width_, speed_, delay_, align_ = TEXT_ALIGN_LEFT )
	{
		::AnimatingCanvas.constructor( )
		currentIndex = 0
		loc = ::GameApp.LocString( loc_ )
		width = width_
		speed = speed_
		timer = 0
		delay = delay_
		align = align_
		finished = false
		Height = 0
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		AddChild( text )
		
		length = loc.Length( )
		if( length > 0 )
			speed = speed / length
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( delay > 0 )
		{
			delay -= dt
		}
		else if( !finished )
		{
			timer -= dt
			while( timer < 0 && !finished )
			{
				currentIndex++
				
				if( currentIndex > length )
				{
					finished = true
					if( onFinished )
						onFinished( )
				}
				
				// Set the next letter
				text.BakeBoxLocString( width, loc.Slice( 0, currentIndex ), align )
				Height = text.Height
				
				// Play the sound
				if( !loc.IsWhiteSpace( currentIndex - 1 ) )
					::GameApp.AudioEvent( "Play_UI_TextScroll" )
				
				timer += speed
			}
		}
	}
}

class CampaignLoadScreenSlide extends AnimatingCanvas
{
	// Events
	onFinished = null
	
	constructor( levelIndex, slide, dlc )
	{
		::AnimatingCanvas.constructor( )
		
		local bg = ::Gui.AsyncTexturedQuad( )
		bg.SetTexture( ::BriefingBackground( levelIndex, slide, dlc ) )
		bg.SetPosition( 0, 0, 0.001 )
		AddChild( bg )
		
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		local fadeInTime = 0.5
		
		if( slide == 0 )
		{
			local sovietSuffix = ( dlc == DLC_EVIL_EMPIRE ? "_USSR" : "" );
			local headerLeftLoc = "Briefing_HeaderLeft" + sovietSuffix
			local headerRightLoc = "Briefing_HeaderRight" + sovietSuffix
			local beginLoc = "Briefing_BeginMessage" + sovietSuffix
			local subjectLoc = ::BriefingSubjectLoc( levelIndex )
			local contentLoc = ::BriefingSlideLoc( levelIndex, slide )
			local endLoc = "Briefing_EndMessage" + sovietSuffix
			
			// Headers and begin
			local headerLeft = ::TypingText( headerLeftLoc, 300, 0.6, fadeInTime + 0.0, TEXT_ALIGN_LEFT )
			headerLeft.SetPosition( vpRect.Left + 20, vpRect.Top + 100, 0 )
			AddChild( headerLeft )
			
			local headerRight = ::TypingText( headerRightLoc, 1000, 0.6, fadeInTime + 0.6, TEXT_ALIGN_LEFT )
			headerRight.SetPosition( vpRect.Left + 540, vpRect.Top + 100, 0 )
			AddChild( headerRight )
			
			local begin = ::TypingText( beginLoc, 1000, 0.6, fadeInTime + 1.2, TEXT_ALIGN_LEFT )
			begin.SetPosition( vpRect.Left + 40, vpRect.Top + 200, 0 )
			AddChild( begin )
			
			// Text
			local subject = ::TypingText( subjectLoc, 1000, 1.0, fadeInTime + 2.0, TEXT_ALIGN_LEFT )
			subject.text.SetFontById( FONT_SIMPLE_MED )
			subject.SetPosition( vpRect.Left + 80, vpRect.Top + 280, 0 )
			AddChild( subject )
			
			local content = ::TypingText( contentLoc, 1000, 4.0, fadeInTime + 3.4, TEXT_ALIGN_LEFT )
			content.SetPosition( vpRect.Left + 80, vpRect.Top + 350, 0 )
			AddChild( content )
			
			// End
			local end = ::TypingText( endLoc, 1000, 0.4, fadeInTime + 8.0, TEXT_ALIGN_LEFT )
			end.SetPosition( vpRect.Right - 200, vpRect.Top + 600, 0 )
			AddChild( end )
			
			end.onFinished = function( ):( fadeInTime ) 
			{
				AddDelayedAction( 2.0, ::AlphaTween( 1.0, 0.0, fadeInTime, null, null, null, function( canvas )
				{
					if( onFinished )
						onFinished( ) 
				}.bindenv( this ) ) ) 
			}.bindenv( this )
		}
		else
		{
			local contentLoc = ::BriefingSlideLoc( levelIndex, slide )

			local content = ::TypingText( contentLoc, 1000, 2.0, fadeInTime + 0.0, TEXT_ALIGN_LEFT )
			content.SetPosition( vpRect.Center.x - 500, vpRect.Bottom - 80, 0 )
			AddChild( content )
			
			content.onFinished = function( ) 
			{
				AddDelayedAction( 1.0, ::CanvasAction( 0.0, null, function( canvas )
				{
					if( onFinished )
						onFinished( ) 
				}.bindenv( this ) ) )
			}.bindenv( this )
		}
		
		SetAlpha( 0 )
		FadeIn( fadeInTime )
	}
}

class CampaignLoadScreen extends AnimatingCanvas
{
	// Display
	slide0 = null
	slide1 = null
	
	// Events
	onFinished = null
	
	constructor( levelIndex, dlc )
	{
		::AnimatingCanvas.constructor( )
		
		slide0 = ::CampaignLoadScreenSlide( levelIndex, 0, dlc )
		slide0.onFinished = function( )
		{
			RemoveChild( slide0 )
			AddChild( slide1 )
		}.bindenv( this )
		AddChild( slide0 )
		
		slide1 = ::CampaignLoadScreenSlide( levelIndex, 1, dlc )
		slide1.onFinished = function( )
		{
			if( onFinished )
				onFinished( )
		}.bindenv( this )
	}
}
