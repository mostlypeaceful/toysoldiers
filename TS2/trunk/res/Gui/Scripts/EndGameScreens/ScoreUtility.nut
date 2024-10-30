// Utility classes for the score

// Requires
sigimport "gui/scripts/hud/swoopnotification.nut"

class BonusPointsFloatingTextBase extends AnimatingCanvas
{
	constructor( locString, color )
	{
		::AnimatingCanvas.constructor( )
		
		local text = Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		text.SetRgba( color )
		text.SetUniformScale( 0.9 )
		text.BakeLocString( locString, TEXT_ALIGN_CENTER )
		AddChild( text )
		
		SetAlpha( 0 )
	}
	
	function Animate( delay )
	{
		AddDelayedAction( delay, ::YMotionTween( GetYPos( ), GetYPos( ) - 100, 1.4, EasingTransition.Quintic ) )
		AddDelayedAction( delay, ::AlphaTween( 0.0, 1.0, 0.2 ) )
		AddDelayedAction( delay + 0.8, ::AlphaTween( 1.0, 0.0, 0.5 ) )
		AddDelayedAction( delay, ::SoundAction( 0.0, "Play_HUD_Points_Bonus", null, audioSource ) )
	}
}

class BonusPointsFloatingText extends BonusPointsFloatingTextBase
{
	constructor( statID, value, source = null )
	{
		audioSource = source
		if( !::GameSessionStats.IsBonus( statID ) )
			return
		
		local locString = ::GameApp.LocString( "Bonus_MulitplierLabelFormat" ).Replace( "name", ::GameSessionStats.StatLocName( statID ) ).Replace( "value", value.tointeger( ) )
		local color = ::GameSessionStats.DisplayColor( statID )
		
		::BonusPointsFloatingTextBase.constructor( locString, color )
	}

}

class BonusPointsSpawner extends AnimatingCanvas
{
	// Data
	display = null
	
	// Statics
	static timeBetweenSpawns = 1.0
	
	constructor( bonusData, source = null )
	{
		::AnimatingCanvas.constructor( )
		audioSource = source
		display = [ ]
		
		foreach( key, score in bonusData )
		{
			local bonus = ::BonusPointsFloatingText( key, score.count, audioSource )
			bonus.SetZPos( -0.01 )
			AddChild( bonus )
			display.push( bonus )
		}
	}
	
	function Animate( delay )
	{
		foreach( i, bonus in display )
		{
			bonus.Animate( delay + i * timeBetweenSpawns )
		}
	}
	
	function Count( )
	{
		return display.len( )
	}
	
	function Skip( )
	{
		foreach( bonus in display )
		{
			bonus.ClearActions( )
			RemoveChild( bonus )
			bonus.DeleteSelf( )
		}
	}
}

function GetBonuses( player )
{
	local scores = player.LevelScoreAndStats
	local bonuses = { }
	
	//::print( "Bonus (" + player.User.GamerTag.ToCString( ) + ")" )
	
	for( local k = SESSION_STATS_SCORE; k < SESSION_STATS_COUNT; ++k )
	{
		if( !::GameSessionStats.IsBonus( k ) )
			continue
		
		local key = ::GameSessionStats.DisplayLocID( k )
		if( key in scores && scores[ key ] > 0 )
		{
			if( k == SESSION_STATS_WAVE_CHAIN && scores[ key ] < 2 )
				continue
				
			bonuses[ k ] <- { }
			bonuses[ k ].count <- scores[ key ]
			bonuses[ k ].points <- scores[ key + "_Points" ]
			
			//::print( k.tostring( ) + " = " + scores[ key ].tostring( ) )
		}
	}
	
	//::print( "Called from:" )
	//DumpCallstack( )
	
	return bonuses
}

class ScoreItem extends AnimatingCanvas
{
	// Display
	text = null
	score = null
	
	// Statics
	static col = 340
	
	constructor( textLoc, scoreLoc )
	{
		::AnimatingCanvas.constructor( )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( textLoc )
		AddChild( text )
		
		if( scoreLoc )
		{
			local scoreAlign = TEXT_ALIGN_RIGHT
			
			score = ::Gui.Text( )
			score.SetFontById( FONT_SIMPLE_SMALL )
			score.SetRgba( COLOR_CLEAN_WHITE )
			score.BakeLocString( scoreLoc, scoreAlign )
			score.SetXPos( col )
			AddChild( score )
		}
	}
}

class ItemizedScoreDisplay extends AnimatingCanvas
{
	// Display
	items = null
	
	// Statics
	static spacing = 22
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		items = [ ]
	}
	
	function Add( textLoc, scoreLoc )
	{
		local item = ::ScoreItem( textLoc, scoreLoc )
		item.SetYPos( items.len( ) * spacing )
		items.push( item )
		AddChild( item )
	}
	
	function AddValue( textLoc, score, time = false )
	{
		local scoreLoc = null
		if( score != null )
		{
			if( time )
				scoreLoc = ::LocString.ConstructTimeString( score, true )
			else
				scoreLoc = ::LocString.FromCString(  ::StringUtil.AddCommaEvery3Digits( score.tointeger( ).tostring( ) ) )
		}
			
		Add( textLoc, scoreLoc )
	}
}

class ScoreController extends AnimatingCanvas
{
	// Display
	scorePresenter = null
	scoreText = null
	anims = null
	itemizedScores1 = null
	itemizedScores2 = null
	earnedItems1 = null
	earnedItems2 = null
	
	// Data
	currentScore = null
	totalScore = null
	player = null
	coopPlayer = null
	delay = null
	scores = null
	scores2 = null
	gamertag1 = null
	gamertag2 = null
	startX = null
	startY = null
	startX1 = null
	startY1 = null
	startX2 = null
	startY2 = null
	scoreEndPos = null
	
	// Statics
	static delayExtra = 0.5
	
	// Events
	onFinish = null
	
	constructor( player_, coopPlayer_ = null, onFinish_ = null, itemizedY = 60 )
	{
		::AnimatingCanvas.constructor( )
		player = player_
		audioSource = player.AudioSource
		coopPlayer = coopPlayer_
		onFinish = onFinish_
		currentScore = 0
		anims = [ ]
		scoreEndPos = -40
		
		local bottomLine = 140
		if( coopPlayer )
			bottomLine += 50
			
		scorePresenter = ::AnimatingCanvas( )
		
		// Score Label
		local scoreLabel = ::Gui.Text( )
		scoreLabel.SetFontById( FONT_FANCY_LARGE )
		scoreLabel.SetRgba( COLOR_CLEAN_WHITE )
		scoreLabel.BakeLocString( ::GameApp.LocString( "resultsKillMiniGame" ), TEXT_ALIGN_LEFT )
		scoreLabel.SetPosition( ::Math.Vec3.Construct( -128, -scoreLabel.Height * 0.5, 0 ) )
		scorePresenter.AddChild( scoreLabel )
		
		// Score Text
		scoreText = ::Gui.Text( )
		scoreText.SetFontById( FONT_FANCY_LARGE )
		scoreText.SetRgba( COLOR_CLEAN_WHITE )
		scoreText.BakeCString( "0", TEXT_ALIGN_LEFT )
		scoreText.SetPosition( ::Math.Vec3.Construct( scoreLabel.GetXPos( ) + scoreLabel.Width + 8, -scoreLabel.Height * 0.5, 0 ) )
		scorePresenter.AddChild( scoreText )
		
		scorePresenter.SetPosition( ::Math.Vec3.Construct( 0, bottomLine - scoreLabel.LineHeight * 0.5, 0 ) )
		AddChild( scorePresenter )
		
		scores = player.LevelScoreAndStats
		totalScore = scores.OverallScore
		
		scores2 = null
		delay = 0
		startX = -426
		startY = -100
		startX1 = null
		startY1 = null
		startX2 = null
		startY2 = null
		gamertag1 = null
		gamertag2 = null
		
		if( coopPlayer )
		{
			scores2 = coopPlayer.LevelScoreAndStats
			
			startX1 = -626
			startX2 = -226
			startY1 = -150
			startY2 = -40
			
			gamertag1 = player.User.GamerTag
			gamertag2 = coopPlayer.User.GamerTag
		}
		
		itemizedScores1 = ::ItemizedScoreDisplay( )
		itemizedScores1.SetPosition( -::ScoreItem.col * 0.5, itemizedY, 0 )
		itemizedScores1.SetAlpha( 0 )
		AddChild( itemizedScores1 )
		
		if( coopPlayer )
		{
			itemizedScores2 = ::ItemizedScoreDisplay( )
			itemizedScores2.SetPosition( 50, itemizedY, 0 )
			itemizedScores2.SetAlpha( 0 )
			AddChild( itemizedScores2 )
			
			itemizedScores1.SetPosition( -::ScoreItem.col - 50, itemizedY, 0 )
		}
	}
	
	function DoTimeResult( locID, resultKey, pointsKey, coop = false, itemizedPoints = true )
	{
		DoResult( locID, resultKey, pointsKey, coop, true, itemizedPoints )
	}
	
	function DoResult( locID, resultKey, pointsKey, coop = false, time = false, itemizedPoints = true )
	{
		local text = null
		if( !time )
			text = ::ResultsSwoop( ::GameApp.LocString( locID ), resultKey? scores[ resultKey ]: null, false, (coop && coopPlayer)? gamertag1: null, audioSource )
		else
			text = ::TimeResultsSwoop( ::GameApp.LocString( locID ), resultKey? scores[ resultKey ]: null, false, (coop && coopPlayer)? gamertag1: null )
		text.SetPosition( ( coop && coopPlayer )? startX1: startX, ( coop && coopPlayer )? startY1: startY, 0.01 )
		if( ( resultKey == null && pointsKey == null ) )
			text.SetYPos( startY - 50 ) // higher
		text.Swoop( delay )
		AddChild( text )
		anims.push( text )
		
		if( coop && coopPlayer )
		{
			local text2 = null
			if( !time )
				text2 = ::ResultsSwoop( ::GameApp.LocString( locID ), resultKey? scores2[ resultKey ]: null, false, gamertag2, coopPlayer.AudioSource )
			else
				text2 = ::TimeResultsSwoop( ::GameApp.LocString( locID ), resultKey? scores2[ resultKey ]: null, false, gamertag2 )
			text2.SetPosition( startX2, startY2, 0.01 )
			text2.Swoop( delay )
			AddChild( text2 )
			anims.push( text2 )
			
			if( resultKey && scores[ resultKey ] != scores2[ resultKey ] )
			{
				if( scores[ resultKey ] > scores2[ resultKey ] )
				{
					text.line3Text.SetRgba( 0.0, 1.0, 0.0, 1.0 )
					text2.showTime = 1.0
				}
				else
				{
					text2.line3Text.SetRgba( 0.0, 1.0, 0.0, 1.0 )
					text.showTime = 1.0
				}
			}
		}

		local itemizedKey = ( itemizedPoints? ( ( pointsKey )? pointsKey: null ): resultKey )
		if( itemizedKey )
		{
			itemizedScores1.AddValue( ::GameApp.LocString( locID ), itemizedKey? scores[ itemizedKey ]: null, time )
			if( coopPlayer )
				itemizedScores2.AddValue( ::GameApp.LocString( locID ), itemizedKey? scores2[ itemizedKey ]: null, time )
		}
		
		if( pointsKey )
			CountUpScore( scores[ pointsKey ] )
		delay += ::StandardSwoop.standardShowTime + delayExtra
	}
	
	function DoBonuses( )
	{
		// Do bonuses 
		local bonuses = ::GetBonuses( player )
		local coopBonuses = null
		local combinedBonuses = { }
		foreach( id, bonus in bonuses )
			combinedBonuses[ id ] <- { score1 = bonus, score2 = { count = 0, points = 0 } }
		
		if( coopPlayer )
		{
			local coopBonuses = ::GetBonuses( coopPlayer )
			foreach( id, bonus in coopBonuses )
			{
				if( id in combinedBonuses )
					combinedBonuses[ id ].score2 = bonus
				else
					combinedBonuses[ id ] <- { score1 = { count = 0, points = 0 }, score2 = bonus }
			}
		}
		
		foreach( id, bonus in combinedBonuses )
		{
			local score1 = bonus.score1
			local score2 = bonus.score2
			local bonusText = null
			local bonusText2 = null
			
			local bonusLabelLocString = ::GameApp.LocString( "Bonus" ).Replace( "bonusName", ::GameSessionStats.DisplayLocString( id ) )
			
			if( score1.count > 0 )
			{
				local locString = ::GameApp.LocString( "Bonus_MulitplierLabelFormat" ).Replace( "name", ::GameSessionStats.DisplayLocString( id ) ).Replace( "value", score1.count.tointeger( ) )
				
				bonusText = ::ResultsSwoop( locString, score1.points, false, gamertag1, audioSource )
				bonusText.SetPosition( startX, startY, 0.01 )
				if( coopPlayer )
					bonusText.SetPosition( startX1, startY1, 0.01 )
				AddChild( bonusText )
				anims.push( bonusText )
				
				itemizedScores1.AddValue( bonusLabelLocString, score1.points )
			}
			
			if( score2.count > 0 )
			{
				local locString2 = ::GameApp.LocString( "Bonus_MulitplierLabelFormat" ).Replace( "name", ::GameSessionStats.DisplayLocString( id ) ).Replace( "value", score2.count.tointeger( ) )
				
				bonusText2 = ::ResultsSwoop( locString2, score2.points, false, gamertag2, coopPlayer.AudioSource )
				bonusText2.SetPosition( startX2, startY2, 0.01 )
				AddChild( bonusText2 )
				anims.push( bonusText )
				
				itemizedScores2.AddValue( bonusLabelLocString, score2.points )
			}
			
			if( coopPlayer && score1.count > 0 && score2.count > 0 && score1.count != score2.count )
			{
				if( score1.count > score2.count )
				{
					bonusText.line2Text.SetRgba( 0.0, 1.0, 0.0, 1.0 )
					bonusText2.showTime = 1.0
				}
				else if( score1.count < score2.count )
				{
					bonusText2.line2Text.SetRgba( 0.0, 1.0, 0.0, 1.0 )
					bonusText.showTime = 1.0
				}
			}
			
			if( bonusText )
				bonusText.Swoop( delay )
			if( bonusText2 )
				bonusText2.Swoop( delay )
			
			if( score1.points > 0 )
				CountUpScore( score1.points ) // Is this right?
			else if( score2.points > 0 )
				CountUpScore( score2.points ) 
			delay += ::StandardSwoop.standardShowTime + delayExtra
		}
	}
	
	function CountUpScore( value )
	{
		local a = ::AnimatingCanvas( )
		a.AddDelayedAction( delay, ::TextCountTween( scoreText, currentScore, currentScore + value, 0.8,
			function( text, v ) { text.BakeCString( ::StringUtil.AddCommaEvery3Digits( v.tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT ) }, ( value > 0 )? "Play_HUD_Points": null, audioSource ) )
		if( totalScore > 0 && value > 0 )
			a.AddDelayedAction( delay, ::SoundRtpcTween( "Points_Aquired", currentScore / totalScore, (currentScore + value) / totalScore, 0.8, audioSource ) )
		AddChild( a )
		anims.push( a )
		
		currentScore += value
	}
	
	scoreEndPos = -40
	
	function PresentScore( )
	{
		// Swoop back
		local backSwoopTime = 0.4
		local startPos = scorePresenter.GetYPos( )
		local midPos = 0
		local midScale = 0.4
		local transition = EasingTransition.Quadratic
		
		scorePresenter.AddDelayedAction( delay, ::YMotionTween( startPos, midPos, backSwoopTime, EasingTransition.Linear, EasingEquation.Out ) )
		scorePresenter.AddDelayedAction( delay, ::UniformScaleTween( 1.0, midScale, backSwoopTime, transition, EasingEquation.Out ) )
		scorePresenter.AddDelayedAction( delay, ::SoundAction( 0.0, "Play_HUD_Points_Center", null, audioSource ) )
		
		// Swoop forward
		local forwardSwoopTime = 0.3
		local endScale = 1.05
		
		scorePresenter.AddDelayedAction( delay + backSwoopTime, ::YMotionTween( midPos, scoreEndPos, forwardSwoopTime, EasingTransition.Linear, EasingEquation.In ) )
		scorePresenter.AddDelayedAction( delay + backSwoopTime, ::UniformScaleTween( midScale, endScale, forwardSwoopTime, transition, EasingEquation.In ) )
		
		// Slam
		local slamTime = 0.1
		local slamScale = 1.0
		
		scorePresenter.AddDelayedAction( delay + backSwoopTime + forwardSwoopTime, ::UniformScaleTween( endScale, slamScale, slamTime, null, null, null,
			function( canvas ) { if( onFinish ) onFinish( ); onFinish = null }.bindenv( this ) ) )
		scorePresenter.AddDelayedAction( delay + backSwoopTime + forwardSwoopTime, ::SoundAction( 0.0, "Play_HUD_Points_Center_Impact", null, audioSource ) )
		
		// Show Itemized lists
		itemizedScores1.AddDelayedAction( delay + backSwoopTime, ::AlphaTween( 0.0, 1.0, 0.5 ) )
		if( itemizedScores2 )
			itemizedScores2.AddDelayedAction( delay + backSwoopTime, ::AlphaTween( 0.0, 1.0, 0.5 ) )
			
		delay += backSwoopTime + forwardSwoopTime
	}
	
	function Skip( )
	{
		// Cancel all the swoops and actions
		foreach( anim in anims )
		{
			anim.ClearActions( )
			RemoveChild( anim )
			anim.DeleteSelf( )
		}
		scorePresenter.ClearActions( )
		
		// Set score in the middle
		scorePresenter.SetYPos( scoreEndPos )
		scorePresenter.SetUniformScale( 1.0 )
		
		// Show end scores instantly
		scoreText.BakeCString( ::StringUtil.AddCommaEvery3Digits( totalScore.tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT )
		itemizedScores1.ClearActions( )
		itemizedScores1.SetAlpha( 1 )
		if( itemizedScores2 )
		{
			itemizedScores2.ClearActions( )
			itemizedScores2.SetAlpha( 1 )
		}
		
		// Call onFinish
		if( onFinish )
			onFinish( )
	}
}

class TimeResultsSwoop extends ResultsSwoop
{
	function Swoop( delay = null )
	{
		if( score )
		{
			AddDelayedAction( delay, ::TextCountTween( line3Text, 0.0, score, 0.8, function( text, value )
			{
				text.BakeLocString( ::LocString.ConstructTimeString( value, true ), TEXT_ALIGN_RIGHT )
			} ) )
		}
		::StandardSwoop.Swoop( delay )
	}
}

// Minigame scoring utility scripts
function GetMinigameScore( player )
{
	local miniScoreIndex = ::GameApp.CurrentLevel.miniGameScoreIndex
		
	if( ::GameApp.CurrentLevel.MapType == MAP_TYPE_MINIGAME )
	{
		if( miniScoreIndex != -1 )
			::print( "minigame score index mismatch!" )
		
		return player.CurrentLevelHighScore
	}
	else
	{		
		local miniGameIndex = miniScoreIndex		
		if( miniScoreIndex > 1 || miniScoreIndex < 0 )
			::print( "minigame score index mismatch 2!" )
			
		//get trial minigame scores from other levels
		local scores = player.GetUserProfile( ).GetLevelScores( MAP_TYPE_MINIGAME, miniGameIndex )
		return scores.GetHighScore( 0 ) //difficulty 0
	}
}