// Personal Best for the load screen

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/hud/swoopnotification.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

class PersonalBest extends StandardSwoop
{
	constructor( gamertag, statID )
	{
		local labelLocString = gamertag
		local statLocName = ::GameSessionStats.StatLocName( statID )
		local valueString = null
		
		local tryingToBeatIndex = ::GameApp.FrontEndPlayer.Stats.TryingToBeatFriendIndex( statID )
		
		if( tryingToBeatIndex == -1 )
			valueString = ::GameSessionStats.StatLocValueString( ::GameApp.FrontEndPlayer.GetUserProfile( ).Stat( statID ), statID )
		else
		{
			local friendStat = ::GameApp.FrontEndPlayer.Stats.FriendStat( statID, tryingToBeatIndex )
			valueString = ::GameSessionStats.StatLocValueString( friendStat.Stat, statID )
			labelLocString = friendStat.GamerTag
		}
		
		::StandardSwoop.constructor( labelLocString, statLocName, valueString )
	}
}

class PersonalBestController extends AnimatingCanvas
{
	// Data
	timer = null
	profile = null
	stats = null
	
	// Consts
	static delay = 5.0
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		timer = delay
		profile = ::GameApp.FrontEndPlayer.GetUserProfile( )
		stats = ::GameApp.FrontEndPlayer.Stats
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( timer < 0 )
		{
			timer = delay
			MakeRandom( )
		}
		else
		{
			timer -= dt
		}
	}
	
	function IsValidStatId( statID )
	{
		if( ::GameSessionStats.StatTemp( statID ) )
			return false

		if( profile.Stat( statID ) <= 0 )
			return false
		
		return true
	}
	
	function FindValidStatId( )
	{
		local statID  = ::SubjectiveRand.Int( SESSION_STATS_SCORE, SESSION_STATS_COUNT - 1 )
		
		local search = 0
		while( !IsValidStatId( statID ) && search < SESSION_STATS_COUNT )
		{
			statID = ::Math.Wrap( statID + 1, 0, SESSION_STATS_COUNT )
			search++
		}
		
		if( search >= SESSION_STATS_COUNT )
			return -1
		
		return statID
	}
	
	function MakeRandom( )
	{
		if( is_null( ::GameApp.FrontEndPlayer ) || is_null( ::GameApp.FrontEndPlayer.GetUserProfile( ) ) )
			return

		local statID = FindValidStatId( )
		if( statID == -1 )
			return
		
		local personalBest = ::PersonalBest( ::GameApp.FrontEndPlayer.User.GamerTag, statID )
		AddChild( personalBest )
		personalBest.Swoop( )
	}
}
