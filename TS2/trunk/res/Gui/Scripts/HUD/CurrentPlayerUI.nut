// Current Player

class CurrentPlayerUI extends AnimatingCanvas
{
	// Display
	gamerPicture = null
	gamerTag = null
	
	// Data
	player = null
	playerIndex = null
	
	constructor( startingPlayer )
	{
		::AnimatingCanvas.constructor( )
		
		// Label
		local label = ::Gui.Text( )
		label.SetFontById( FONT_SIMPLE_SMALL )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( ::GameApp.LocString( "Minigame_CurrentPlayer" ) )
		AddChild( label )
		
		// Player Image
		gamerPicture = ::Gui.GamerPictureQuad( )
		gamerPicture.SetPosition( 0, label.Height, 0 )
		gamerPicture.SetUniformScale( 0.5 )
		AddChild( gamerPicture )
		
		// Player Text
		gamerTag = ::Gui.Text( )
		gamerTag.SetFontById( FONT_SIMPLE_SMALL )
		gamerTag.SetRgba( COLOR_CLEAN_WHITE )
		gamerTag.BakeLocString( startingPlayer.User.GamerTag, TEXT_ALIGN_LEFT )
		gamerTag.SetPosition( 47, label.Height + 16 - gamerTag.Height * 0.5, 0 )
		AddChild( gamerTag )
		
		local vpRect = startingPlayer.User.ComputeScreenSafeRect( )
		SetPosition( vpRect.Left, vpRect.Top, 0.3 )
		SetAlpha( 0 )
		SetPlayer( startingPlayer )
		
		::GameApp.HudRoot.AddChild( this )
		FadeIn( )
	}
	
	function SetPlayer( newPlayer )
	{
		player = newPlayer
		playerIndex = ::GameApp.WhichPlayer( player.User )
		gamerPicture.SetTexture( ::GameApp.FrontEndPlayer.User, player.User, false )
		gamerTag.BakeLocString( player.User.GamerTag, TEXT_ALIGN_LEFT )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		if( playerIndex != ::GameApp.SingleScreenControlIndex )
			SetPlayer( ::GameApp.GetPlayer( ::GameApp.SingleScreenControlIndex ) )
	}
}

PreviousPlayerUIScores <- [ 0, 0 ]

function ResetPreviousPlayerScores( )
{
	::PreviousPlayerUIScores <- [ 0, 0 ]
}

function SetMinigameBestScore( player, score )
{
	local index = player.PlayerIndex
	::PreviousPlayerUIScores[ index ] = ::Math.Max( score, ::PreviousPlayerUIScores[ index ] )
}

class PreviousPlayerUI extends AnimatingCanvas
{
	constructor( playerZero, newScoreZero, playerOne, newScoreOne )
	{
		::AnimatingCanvas.constructor( )
		
		// Label
		local label = ::Gui.Text( )
		label.SetFontById( FONT_SIMPLE_SMALL )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( ::GameApp.LocString( "Previous_Best" ) % ":", TEXT_ALIGN_RIGHT )
		AddChild( label )
		
		local data = [ { }, { } ]
		
		// Get Actual Top Scores
		local score0 = ::PreviousPlayerUIScores[ 0 ]
		local score1 = ::PreviousPlayerUIScores[ 1 ]
		
		if( score0 > score1 )
		{
			data[ 0 ].player <- playerZero
			data[ 0 ].score <- score0
			data[ 1 ].player <- playerOne
			data[ 1 ].score <- score1
		}
		else
		{
			data[ 0 ].player <- playerOne
			data[ 0 ].score <- score1
			data[ 1 ].player <- playerZero
			data[ 1 ].score <- score0
		}
		
		local spacing = 38
		foreach( i, t in data )
		{
			// Player Image
			local gamerPicture = ::Gui.GamerPictureQuad( )
			gamerPicture.SetPosition( -32, label.Height + i * spacing, 0 )
			gamerPicture.SetUniformScale( 0.5 )
			gamerPicture.SetTexture( ::GameApp.FrontEndPlayer.User, t.player.User, false )
			AddChild( gamerPicture )
			
			// Player Text
			local gamerTag = ::Gui.Text( )
			gamerTag.SetFontById( FONT_SIMPLE_SMALL )
			gamerTag.SetRgba( COLOR_CLEAN_WHITE )
			gamerTag.BakeCString( ::StringUtil.AddCommaEvery3Digits( t.score.tointeger( ).tostring( ) ), TEXT_ALIGN_RIGHT )
			gamerTag.SetPosition( -47, label.Height + 16 - gamerTag.Height * 0.5 + i * spacing, 0 )
			AddChild( gamerTag )
		}
	}
}