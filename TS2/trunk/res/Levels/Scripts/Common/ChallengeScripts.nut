// Scripts and Functions for Challenge mode

function WaveLocString( waveNumber )
{
	return ::GameApp.LocString( "Wave" ) % " " % waveNumber.tostring( )
}

function RoundLocString( roundNumber )
{
	return ::GameApp.LocString( "RoundNum" ) % " " % roundNumber.tostring( )
}

function TimeCString( time )
{
	time = time.tointeger( )
	local minutes = ( time / 60 ).tointeger( )
	local seconds = time % 60
	local formatString = ( minutes < 10 ? "0" : "" ) + minutes + ":" + ( seconds < 10 ? "0" : "" ) + seconds
			
	return formatString
}

function FindNextThreshold( progress, info )
{
	local thresholds = [ info.RankThreshold( 0 ), info.RankThreshold( 1 ), info.RankThreshold( 2 ), info.RankThreshold( 3 ) ]
	foreach( i, v in thresholds )
	{
		if( progress < v )
			return v
	}
	return null
}

function FindRank( progress, info )
{
	local thresholds = [ info.RankThreshold( 0 ), info.RankThreshold( 1 ), info.RankThreshold( 2 ), info.RankThreshold( 3 ) ]
	foreach( i, v in thresholds )
	{
		if( progress < v )
			return i - 1
	}
	return 3
}