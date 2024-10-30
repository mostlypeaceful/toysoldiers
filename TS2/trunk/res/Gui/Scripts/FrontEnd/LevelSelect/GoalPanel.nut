// Panel to show bonus goals

// Resources
sigimport "gui/scripts/controls/checkbox.nut"

class GoalPanelLine extends Gui.CanvasFrame
{
	// Display
	checkbox = null // Checkbox
	text = null // Gui.Text
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		checkbox = ::Checkbox( )
		checkbox.SetPosition( Math.Vec3.Construct( 0, 3, 0 ) )
		AddChild( checkbox )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.SetPosition( checkbox.WorldRect.Width + 5, 0, 0 )
		AddChild( text )
	}
	
	function Set( complete, descLocString )
	{
		checkbox.SetCheckedStatus( complete )
		text.BakeLocString( descLocString )
		
		text.CompactAndUnscale( 710 )
	}
}

class GoalPanel extends Gui.CanvasFrame
{
	// Display
	goals = null // array of GoalPanelLine objects
	title = null // title
	
	constructor( startingInfo, startingScores )
	{
		::Gui.CanvasFrame.constructor( )
		local asianLanguage = ::GameApp.IsAsianLanguage( )
		
		title = ::Gui.Text( )
		title.SetFontById( FONT_FANCY_MED )
		title.SetRgba( COLOR_CLEAN_WHITE )
		title.BakeLocString( ::GameApp.LocString( "Ration_Tickets" ), TEXT_ALIGN_CENTER )
		title.SetPosition( 375, 1, -0.01 )
		if( asianLanguage )
			title.SetYPos( 2 )
		else
			title.SetUniformScale( 0.7 )
		AddChild( title )
		
		goals = []
		
		local startY = ( title.Height + ( asianLanguage ? 10 : 2 ) )
		for( local i = 0; i < 2; ++i )
		{
			goals.push( ::GoalPanelLine( ) )
			goals[ i ].SetPosition( 7, startY + (goals[ i ].WorldRect.Height + 3) * i, 0 )
			AddChild( goals[ i ] )
		}
		
		SetLevelInfo( startingInfo, startingScores )
	}
	
	function SetLevelInfo( info, scores )
	{
		local n = info.LevelIndex
		local goalData = [ ]
		for( local i = 0; i < 2; ++i )
		{
			goalData.push( { complete = scores.IsGoalComplete( i ), desc = ::RationTicketFullDescLocString( n, i ) } )
		}
		
		DisplayGoals( goalData )
	}
	
	function DisplayGoals( goalData ) // Array of objects{ bool, locID }
	{
		goals[ 0 ].SetAlpha( 0.0 )
		goals[ 1 ].SetAlpha( 0.0 )
		
		foreach( i, goal in goalData )
		{
			goals[ i ].SetAlpha( 1.0 )
			goals[ i ].Set( goal.complete, goal.desc )
		}
	}
}