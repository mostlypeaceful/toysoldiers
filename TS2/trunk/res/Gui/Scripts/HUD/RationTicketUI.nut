// Ration Ticket UI Controller

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/hud/rationticketnotification.nut"

sigexport function CanvasCreateRationTicketUI( cppObj )
{
	return RationTicketUI( )
}

class RationTicketUI extends AnimatingCanvas
{
	// Display
	notifications = null // array of RationTicketNotification objects
	
	// Data
	levelIndex = null
	inGoalXRight = null
	outGoalXRight = null
	inGoalXLeft = null
	outGoalXLeft = null
	startingY = null
	
	static goldenArcadePositionIndex = 2
	static rankPositionIndex = 3
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		SetPosition( 0, 0, 0.4 )
		
		local screenRect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		
		levelIndex = ::GameApp.CurrentLevel.LevelNumber
		inGoalXRight = screenRect.Right - RationTicketNotification.Width
		outGoalXRight = screenRect.Right + 100
		inGoalXLeft = screenRect.Left
		outGoalXLeft = screenRect.Left - RationTicketNotification.Width - 100
		startingY = screenRect.Bottom - 200
		
		notifications = array( 2 )
		notifications[ 0 ] = array( rankPositionIndex + 1, null )
		notifications[ 1 ] = array( rankPositionIndex + 1, null )
	}
	
	function OnTick( dt )
	{
		if( !::GameApp.Paused( ) )
			::AnimatingCanvas.OnTick( dt )
	}
	
	function SideIndex( user = null )
	{
		if( user != null && user.IsLocal && ::GameApp.GameMode.IsSplitScreen && user.ViewportIndex == 0 )
			return 0
		else if( user != null && !user.IsLocal )
			return null
		else
			return 1
	}
	
	function RationTicketProgress( index, progress, max, user = null )
	{
		local vpIndex = SideIndex( user )
		if( vpIndex == null )
			return
		
		local alt = ( vpIndex == 0 )
		
		local note = notifications[ vpIndex ][ index ]
		
		if( note )
			note.Update( progress )
		else
			AddNotification( index, ::RationTicketProgress( levelIndex, index, progress, max, alt ), user )
	}
	
	function HideAndDestroyNotification( index, user )
	{
		local vpIndex = SideIndex( user )
		if( vpIndex == null )
			return
		
		local note = notifications[ vpIndex ][ index ]
		if( note && note.shown )
		{
			note.HideAndDie( )
			notifications[ vpIndex ][ index ] = null
		}
	}
	
	function PositionNotification( index, user = null )
	{
		local vpIndex = SideIndex( user )
		if( vpIndex == null )
			return
		
		local note = notifications[ vpIndex ][ index ]
		if( note )
		{
			local outGoal = outGoalXRight
			
			if( user != null && user.IsLocal && ::GameApp.GameMode.IsSplitScreen )
			{
				if( vpIndex == 0 )
					outGoal = outGoalXLeft
			}
			
			note.SetPosition( outGoal, startingY - index * 76, 0 )
		}
	}
	
	function AddNotification( index, note, user = null )
	{
		local vpIndex = SideIndex( user )
		if( vpIndex == null )
			return
		
		notifications[ vpIndex ][ index ] = note
		note.SetGoalPositions( inGoalXRight, outGoalXRight )
		if( user != null && user.IsLocal && ::GameApp.GameMode.IsSplitScreen && vpIndex == 0 )
			note.SetGoalPositions( inGoalXLeft, outGoalXLeft )
		AddChild( note )
		PositionNotification( index, user )
		note.Show( )
	}
	
	////////////////////////////////////////////////////////////////////////////
	
	function AwardRationTicket( index, user = null )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( index, user )
		AddNotification( index, ::RationTicketEarned( levelIndex, index, alt ), user )
	}
	
	function FailRationTicket( index, user = null )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( index, user )
		AddNotification( index, ::RationTicketFailed( levelIndex, index, alt ), user )
	}
	
	function AwardNewRank( rankIndex, user )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( rankPositionIndex, user )
		AddNotification( rankPositionIndex, ::NewRankEarned( rankIndex, user, alt ), user )
	}
	
	function FindGoldenArcade( user = null )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( goldenArcadePositionIndex, user )
		AddNotification( goldenArcadePositionIndex, ::GoldenArcadeFoundNotification( alt ), user )
	}
	
	function AwardGoldenArcadeRationTicket( index, user = null ) // 0 or 1
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( index, user )
		AddNotification( index, ::GoldenArcadeRationTicket( index, alt ), user )
	}
	
	function GoldenBabushkaProgress( progress, user = null )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( goldenArcadePositionIndex, user )
		AddNotification( goldenArcadePositionIndex, ::GoldenBabushkasProgressNotification( progress, alt ), user )
	}
	
	function FindAllGoldenBabushkas( user = null )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( goldenArcadePositionIndex, user )
		AddNotification( goldenArcadePositionIndex, ::GoldenBabushkasFoundNotification( alt ), user )
	}
	
	function GoldenDogTagProgress( progress, user = null )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( goldenArcadePositionIndex, user )
		AddNotification( goldenArcadePositionIndex, ::GoldenDogTagsProgressNotification( progress, alt ), user )
	}
	
	function FindAllGoldenDogTags( user = null )
	{
		local alt = ( SideIndex( user ) == 0 )
		HideAndDestroyNotification( goldenArcadePositionIndex, user )
		AddNotification( goldenArcadePositionIndex, ::GoldenDogTagsFoundNotification( alt ), user )
	}
}