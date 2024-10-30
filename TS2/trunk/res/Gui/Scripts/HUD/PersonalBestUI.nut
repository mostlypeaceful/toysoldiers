// Personal Best UI for in-game

// Requires
sigimport "gui/scripts/hud/statsleaderboarddisplay.nut"

sigexport function CanvasCreatePersonalBestUI( cppObj )
{
	return ::PersonalBestUI( cppObj )
}

class PersonalBestUI extends AnimatingCanvas
{
	// Display 
	owner = null
	display = null
	
	// Data
	player = null
	currentStatID = null
	inGoalX = null
	outGoalX = null
	startingY = null
	hideTimer = null
	maxTimer = 0
	
	constructor( cppObj )
	{
		owner = cppObj
		::AnimatingCanvas.constructor( )
		player = cppObj.Player
		hideTimer = 0
		maxTimer = 0
		
		local vpSafeRect = player.ComputeViewportSafeRect( )
		
		inGoalX = vpSafeRect.Left
		outGoalX = vpSafeRect.Left - 256
		startingY = vpSafeRect.Center.y
		
		::GameApp.HudLayer( "viewport" + player.User.ViewportIndex.tostring( ) ).AddChild( this )
		SetZPos( 0.4 )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( hideTimer < 0 || maxTimer > 8.0 )
		{
			if( display )
			{
				display.Hide( CleanUp.bindenv( this ) )
				display = null
			}
		}
		else
		{
			hideTimer -= dt
			maxTimer += dt
		}
	}
	
	function CleanUp( canvas )
	{
		canvas.DeleteSelf( )
		owner.Shown = 0 //this is on the tPersonalBestUI
		maxTimer = 0
	}
	
	function NewPersonalBest( statID, newValue )
	{
		if( ::GameApp.GameMode.IsSplitScreen && ::GameApp.GameMode.IsCoOp )
			return
		
		local level =::GameApp.CurrentLevel
		if( level.TutHideKillHoverText || level.MapType == MAP_TYPE_MINIGAME  )
			return
			
		if( display )
		{
			if( display.statID == statID )
			{
				Update( newValue )
			}
			else 
			{
				local currentPriority = ::GameSessionStats.Priority( display.statID )
				local newPriority = ::GameSessionStats.Priority( statID )
			
				if( newPriority < currentPriority )
				{
					display.HideAndDie( )
					MakeNewDisplay( statID, newValue )
				}
			}
		}
		else
		{
			MakeNewDisplay( statID, newValue )
		}
	}
	
	function MakeNewDisplay( statID, newValue )
	{
		if( !::GameApp.GameMode.IsSplitScreen )
		{
			display = ::StatsLeaderboard( statID, newValue, player )
			display.SetPosition( outGoalX, startingY, 0 )
			display.SetGoalPositions( inGoalX, outGoalX )
			AddChild( display )
			Show( )
		}
	}
	
	function Update( newValue )
	{
		display.SetPlayerScore( newValue )
		Show( )
	}
	
	function Show( )
	{
		display.Show( )
		hideTimer = 4.0
		owner.Shown = 1 //this is on the tPersonalBestUI
	}
}