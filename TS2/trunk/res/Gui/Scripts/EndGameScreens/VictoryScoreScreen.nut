// Screen to show the scores of the victorious 

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/endgamescreens/scoreutility.nut"
sigimport "gui/scripts/utility/medalscripts.nut"
sigimport "gui/scripts/controls/asyncstatus.nut"

// Resources
sigimport "gui/textures/endgamescreens/victory_bg1_d.png"
sigimport "gui/textures/endgamescreens/medals/bronze_xl_g.png"
sigimport "gui/textures/endgamescreens/medals/bronze_lg_g.png"
sigimport "gui/textures/endgamescreens/medals/bronze_med_g.png"
sigimport "gui/textures/endgamescreens/medals/gold_xl_g.png"
sigimport "gui/textures/endgamescreens/medals/gold_lg_g.png"
sigimport "gui/textures/endgamescreens/medals/gold_med_g.png"
sigimport "gui/textures/endgamescreens/medals/platinum_xl_g.png"
sigimport "gui/textures/endgamescreens/medals/platinum_lg_g.png"
sigimport "gui/textures/endgamescreens/medals/platinum_med_g.png"
sigimport "gui/textures/endgamescreens/medals/silver_xl_g.png"
sigimport "gui/textures/endgamescreens/medals/silver_lg_g.png"
sigimport "gui/textures/endgamescreens/medals/silver_med_g.png"
sigimport "effects/fx/gui/small_silver_fireworks.fxml"
sigimport "effects/fx/gui/small_bronze_fireworks.fxml"
sigimport "effects/fx/gui/small_gold_fireworks.fxml"
sigimport "effects/fx/gui/big_silver_fireworks.fxml"
sigimport "effects/fx/gui/big_bronze_fireworks.fxml"
sigimport "effects/fx/gui/big_gold_fireworks.fxml"
sigimport "effects/fx/gui/small_silver_glow.fxml"
sigimport "effects/fx/gui/small_bronze_glow.fxml"
sigimport "effects/fx/gui/small_gold_glow.fxml"
sigimport "effects/fx/gui/big_silver_glow.fxml"
sigimport "effects/fx/gui/big_bronze_glow.fxml"
sigimport "effects/fx/gui/big_gold_glow.fxml"
sigimport "effects/fx/gui/large_firework_sequence_01.fxml"
sigimport "effects/fx/gui/large_firework_sequence_02.fxml"
sigimport "effects/fx/gui/large_firework_sequence_03.fxml"
sigimport "effects/fx/gui/large_firework_sequence_04.fxml"


class DefenseCountUpTweener extends Tweener
{
	text = null
	totalHP = null
	
	constructor( textObj, start, end, duration, total )
	{
		::Tweener.constructor( start, end, duration )
		text = textObj
		totalHP = total
	}
	
	function OnTick( dt, canvas )
	{
		local toyboxHpLeft = easer.Evaluate( timer ).tointeger( )
		text.BakeCString( toyboxHpLeft.tostring( ) + "/" + totalHP, TEXT_ALIGN_RIGHT )
	}
}

class AggressionCountUpTweener extends Tweener
{
	text = null
	
	constructor( textObj, start, end, duration )
	{
		::Tweener.constructor( start, end, duration )
		text = textObj
	}
	
	function OnTick( dt, canvas )
	{
		local secondsSkipped = easer.Evaluate( timer )
		text.BakeLocString( ::LocString.ConstructTimeString( secondsSkipped, false ), TEXT_ALIGN_RIGHT )
	}
}

class ProfitCountUpTweener extends Tweener
{
	text = null
	
	constructor( textObj, start, end, duration )
	{
		::Tweener.constructor( start, end, duration )
		text = textObj
	}
	
	function OnTick( dt, canvas )
	{
		local profitRatio = easer.Evaluate( timer )
		text.BakeCString( (profitRatio * 100).tointeger( ).tostring( ) + "%", TEXT_ALIGN_RIGHT )
	}
}

class OverallCountUpTweener extends Tweener
{
	text = null
	
	constructor( textObj, start, end, duration )
	{
		::Tweener.constructor( start, end, duration )
		text = textObj
	}
	
	function OnTick( dt, canvas )
	{
		text.BakeCString( ::StringUtil.AddCommaEvery3Digits( easer.Evaluate( timer ).tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT )
	}
}


class FireworksBurstWithAfterglow
{
	fireworks = null
	glow = null
	constructor( victoryscorescreen, medalCanvas, type, size, delay)
	{
		local fireworkspath = null
		local glowpath = null
		if( size == "small" )
		{
			if( type == "silver" )
			{
				fireworkspath = "effects/fx/gui/small_silver_fireworks.fxml"
				glowpath = "effects/fx/gui/small_silver_glow.fxml"
			}
			else if( type == "bronze" )
			{
				fireworkspath = "effects/fx/gui/small_bronze_fireworks.fxml"
				glowpath = "effects/fx/gui/small_bronze_glow.fxml"
			}
			else if( type == "gold" )
			{
				fireworkspath = "effects/fx/gui/small_gold_fireworks.fxml"
				glowpath = "effects/fx/gui/small_gold_glow.fxml"
			}
		}
		if( size == "big" )
		{
			if( type == "silver" )
			{
				fireworkspath = "effects/fx/gui/big_silver_fireworks.fxml"
				glowpath = "effects/fx/gui/big_silver_glow.fxml"
			}
			else if( type == "bronze" )
			{
				fireworkspath = "effects/fx/gui/big_bronze_fireworks.fxml"
				glowpath = "effects/fx/gui/big_bronze_glow.fxml"
			}
			else if( type == "gold" )
			{
				fireworkspath = "effects/fx/gui/big_gold_fireworks.fxml"
				glowpath = "effects/fx/gui/big_gold_glow.fxml"
			}
		}
		
		fireworks = ::Gui.ScreenSpaceFxSystem( )
		glow = ::Gui.ScreenSpaceFxSystem( )
		
		fireworks.SetSystem( fireworkspath, 1, true )	//path, playcount(-1=loop), localSystem
		glow.SetSystem( glowpath, -1, true )	//path, playcount(-1=loop), localSystem
		
		if( medalCanvas != null )
		{
			fireworks.SetPosition( medalCanvas.GetXPos( ), 	medalCanvas.GetYPos( ), 	medalCanvas.GetZPos( ) + 0.725 )
			glow.SetPosition( medalCanvas.GetXPos( ), 	medalCanvas.GetYPos( ), 	medalCanvas.GetZPos( ) + 0.725 )
		}
		
		fireworks.SetDelay( delay-0.1 )
		glow.SetDelay( delay-0.1 )
		
		victoryscorescreen.AddChild( fireworks )
		victoryscorescreen.AddChild( glow )
	}
	
	function SetDelay( delay )
	{
		fireworks.SetDelay( delay )
		glow.SetDelay( delay )
	}
	
	function FadeOutSystems( time )
	{
		fireworks.FadeOutSystems( time )
		glow.FadeOutSystems( time )
	}
	
	function OverrideSystemAlphas( newAlpha )
	{
		fireworks.OverrideSystemAlphas( newAlpha )
		glow.OverrideSystemAlphas( newAlpha )		
	}
	
	function FastForward( amount )
	{
		fireworks.FastForward( amount )
		glow.FastForward( amount )		
	}
}

class VictoryScreenFriendScore extends AnimatingCanvas
{
	// Display
	nameDisplay = null
	scoreDisplay = null
	
	// Data
	friendName = null
	score = null
	
	// Statics
	static Spacing = 84
	
	constructor( friendName_, score_, medal )
	{
		::AnimatingCanvas.constructor( )
		friendName = friendName_
		score = score_
		
		nameDisplay = ::Gui.Text( )
		nameDisplay.SetFontById( FONT_SIMPLE_MED )
		nameDisplay.SetRgba( COLOR_CLEAN_WHITE )
		nameDisplay.BakeLocString( friendName, TEXT_ALIGN_LEFT )
		nameDisplay.SetPosition( 0, -nameDisplay.Height, 0 )
		AddChild( nameDisplay )

		nameDisplay.Compact( 282 - 38 - 10 )
		
		scoreDisplay = ::Gui.Text( )
		scoreDisplay.SetFontById( FONT_SIMPLE_MED )
		scoreDisplay.SetRgba( COLOR_CLEAN_WHITE )
		scoreDisplay.BakeCString( ::StringUtil.AddCommaEvery3Digits( score.tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT )
		scoreDisplay.SetPosition( 0, 0, 0 )
		AddChild( scoreDisplay )
		
		local medalDisplay = ::Gui.AsyncTexturedQuad( )
		medalDisplay.OnLoaded = function( quad )
		{
			quad.CenterPivot( )
			quad.SetPosition( 282, 0, 0 )
		}
		local medalTexture = ::VictoryScreenMedalImagePath( medal, MEDAL_SIZE_FRIEND )
		if( medalTexture )
		{
			medalDisplay.SetTexture( medalTexture )
			AddChild( medalDisplay )
		}
	}
	
	function SetAsPlayer( )
	{
		nameDisplay.SetRgba( 1.0, 1.0, 0.0, 1.0 )
		scoreDisplay.SetRgba( 1.0, 1.0, 0.0, 1.0 )
	}
}

class VictoryScoreScreen extends AnimatingCanvas
{
	// Display
	labelCanvases = null
	overallLabelCanvas = null
	defenseScore = null
	defensePoints = null
	aggressionScore = null
	aggressionPoints = null
	profitScore = null
	profitPoints = null
	overallScore = null
	defenseMedal = null
	medalCanvases = null
	bonuses = null
	perfect = null
	playerItem = null
	player2Item = null
	friendsStartPos = null
	asyncStatus = null
	
	// Data
	animating = null
	defenseValue = null
	defenseTotal = null
	secondsSkipped = null
	profitRatio = null
	points = null
	scores = null
	currentPoints = null
	totalScore = null
	friendsScores = null
	prevInit = null
	screenWidth = null
	screenHeight = null
	
	// Effects
	fireworksTimer = null
	launchFireworks = null
	medalFx01 = null
	medalFx02 = null
	medalFx03 = null
	medalFx04 = null
	
	constructor( rect, player1, player2 = null )
	{
		::AnimatingCanvas.constructor( )
		audioSource = player1.AudioSource
		animating = false
		prevInit = false
		screenWidth = rect.Right
		screenHeight = rect.Bottom
		fireworksTimer = 7.0
		launchFireworks  = true
		
		// Player1 Stats
		local stats = player1.Stats
		
		// Title Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( "EndGame_Victory" ), TEXT_ALIGN_CENTER )
		text.SetPosition( rect.Center.x, rect.Top + 10, 0 )
		text.SetUniformScale( 1.6 )
		AddChild( text )
		
		// Background 1
		local bg1 = ::Gui.TexturedQuad( )
		bg1.SetTexture( "gui/textures/endgamescreens/victory_bg1_d.png" )
		bg1.SetPosition( rect.Left, rect.Top + 92, 0.01 )
		AddChild( bg1 )
		
		local startX = rect.Left
		local startY = rect.Top + 92
		
		// Title 1
		local title1 = ::Gui.Text( )
		title1.SetFontById( FONT_FANCY_LARGE )
		title1.SetRgba( COLOR_CLEAN_WHITE )
		title1.BakeLocString( ::GameApp.LocString( "EndGame_Medals" ), TEXT_ALIGN_RIGHT )
		title1.SetPosition( startX + 806, startY + 8, 0 )
		AddChild( title1 )
		
		// Title 2
		local title2 = ::Gui.Text( )
		title2.SetFontById( FONT_FANCY_MED )
		title2.SetRgba( COLOR_CLEAN_WHITE )
		title2.BakeLocString( ::GameApp.LocString( "EndGame_Friends" ), TEXT_ALIGN_LEFT )
		title2.SetPosition( startX + 838, startY + 12, 0 )
		title2.SetUniformScale( 0.8 )
		AddChild( title2 )
		
		local modeText = ::Gui.Text( )
		modeText.SetFontById( FONT_SIMPLE_SMALL )
		modeText.SetRgba( COLOR_CLEAN_WHITE )
		modeText.BakeLocString( ::LocString.FromCString( "(" ) % ::GetDifficultyName( ::GameApp.CurrentLevelLoadInfo.Difficulty ) % ")", TEXT_ALIGN_LEFT )
		modeText.SetPosition( title2.GetXPos( ) + title2.Width * 0.8 + 15, title2.GetYPos( ) + title2.Height * 0.8 * 0.5 - modeText.Height * 0.5 - 2, 0 )
		AddChild( modeText )
		
		local labelColumnX = 350
		local scoreColumnX = 683
		local medalColumnX = 747
		
		local asianSpacing = ( ::GameApp.IsAsianLanguage( ) ? 6 : 0 )
		
		local rowSpacing = 132
		local defenseRowY = 136
		local aggressionRowY = defenseRowY + rowSpacing
		local profitRowY = aggressionRowY + rowSpacing - 2
		local overallRowY = 377
		
		local baseLabelFont = FONT_SIMPLE_MED
		local baseScoreFont = FONT_FANCY_MED
		
		// Labels and Descriptions
		local scoreData = [
			{ name = "Defense", y = defenseRowY }, 
			{ name = "Aggression", y = aggressionRowY }, 
			{ name = "Profit", y = profitRowY }
		]
		labelCanvases = [ ]
		foreach( data in scoreData )
		{
			local label = ::Gui.Text( )
			label.SetFontById( baseLabelFont )
			label.SetRgba( COLOR_CLEAN_WHITE )
			label.BakeLocString( ::GameApp.LocString( "EndGame_" + data.name + "_Label" ), TEXT_ALIGN_LEFT )
			label.SetPosition( 0, -label.LineHeight - asianSpacing, 0 )
			
			local desc = ::Gui.Text( )
			desc.SetFontById( FONT_SIMPLE_SMALL )
			desc.SetRgba( COLOR_CLEAN_WHITE )
			desc.BakeLocString( ::GameApp.LocString( "EndGame_" + data.name + "_Desc" ), TEXT_ALIGN_LEFT )
			desc.SetPosition( 0, 0 + asianSpacing, 0 )
			
			local canvas = ::AnimatingCanvas( )
			canvas.SetPosition( startX + labelColumnX, startY + data.y, 0 )
			canvas.SetRgba( 0.5, 0.5, 0.5, 1.0 )
			canvas.AddChild( label )
			canvas.AddChild( desc )
			AddChild( canvas )
			
			labelCanvases.push( canvas )
		}
		
		scores = player1.LevelScoreAndStats
		points = { DefensePoints = scores.DefensePoints, AggressionPoints = scores.AggressionPoints, ProfitPoints = scores.ProfitPoints, BonusPoints = scores.BonusPoints, OverallScore = scores.OverallScore }

		local scoreFont = FONT_FANCY_MED
		local pointsFont = FONT_FANCY_LARGE
		
		// Defense Score
		defenseValue = scores.TicketsLeft
		defenseTotal = scores.TicketsMax
		defenseScore = ::Gui.Text( )
		defenseScore.SetFontById( scoreFont )
		defenseScore.SetRgba( COLOR_CLEAN_WHITE )
		defenseScore.BakeCString( "0/" + defenseTotal, TEXT_ALIGN_RIGHT )
		defenseScore.SetPosition( rect.Left + scoreColumnX, startY + defenseRowY + asianSpacing, 0 )
		AddChild( defenseScore )
		
		defensePoints = ::Gui.Text( )
		defensePoints.SetFontById( pointsFont )
		defensePoints.SetRgba( COLOR_CLEAN_WHITE )
		defensePoints.BakeCString( "0", TEXT_ALIGN_RIGHT )
		defensePoints.SetPosition( rect.Left + scoreColumnX, startY + defenseRowY - defensePoints.LineHeight - asianSpacing, 0 )
		AddChild( defensePoints )
		
		// Aggression Score
		secondsSkipped = scores.SecondsSkipped
		aggressionScore = ::Gui.Text( )
		aggressionScore.SetFontById( scoreFont )
		aggressionScore.SetRgba( COLOR_CLEAN_WHITE )
		aggressionScore.BakeLocString( ::LocString.ConstructTimeString( 0.0, false ), TEXT_ALIGN_RIGHT )
		aggressionScore.SetPosition( rect.Left + scoreColumnX, startY + aggressionRowY + asianSpacing, 0 )
		AddChild( aggressionScore )
		
		aggressionPoints = ::Gui.Text( )
		aggressionPoints.SetFontById( pointsFont )
		aggressionPoints.SetRgba( COLOR_CLEAN_WHITE )
		aggressionPoints.BakeCString( "0", TEXT_ALIGN_RIGHT )
		aggressionPoints.SetPosition( rect.Left + scoreColumnX, startY + aggressionRowY - aggressionPoints.LineHeight - asianSpacing, 0 )
		AddChild( aggressionPoints )
		
		// Profit Score
		profitRatio = scores.ProfitRatio
		profitScore = ::Gui.Text( )
		profitScore.SetFontById( scoreFont )
		profitScore.SetRgba( COLOR_CLEAN_WHITE )
		profitScore.BakeCString( "0%", TEXT_ALIGN_RIGHT )
		profitScore.SetPosition( rect.Left + scoreColumnX, startY + profitRowY + asianSpacing, 0 )
		AddChild( profitScore )
		
		profitPoints = ::Gui.Text( )
		profitPoints.SetFontById( pointsFont )
		profitPoints.SetRgba( COLOR_CLEAN_WHITE )
		profitPoints.BakeCString( "0", TEXT_ALIGN_RIGHT )
		profitPoints.SetPosition( rect.Left + scoreColumnX, startY + profitRowY - profitPoints.LineHeight - asianSpacing, 0 )
		AddChild( profitPoints )
		
		// Overall label
		local overallLabel = ::Gui.Text( )
		overallLabel.SetFontById( baseLabelFont )
		overallLabel.SetRgba( COLOR_CLEAN_WHITE )
		overallLabel.BakeLocString( ::GameApp.LocString( "EndGame_Overall_Label" ), TEXT_ALIGN_RIGHT )
		overallLabel.SetPosition( 0, 0, 0 )
		
		overallLabelCanvas = ::AnimatingCanvas( )
		overallLabelCanvas.SetPosition( rect.Left + 145, startY + overallRowY - overallLabel.LineHeight, 0 )
		overallLabelCanvas.SetRgba( 0.5, 0.5, 0.5, 1.0 )
		overallLabelCanvas.AddChild( overallLabel )
		AddChild( overallLabelCanvas )
		
		// Overall score
		overallScore = ::Gui.Text( )
		overallScore.SetFontById( FONT_FANCY_MED )
		overallScore.SetRgba( COLOR_CLEAN_WHITE )
		overallScore.BakeCString( "0", TEXT_ALIGN_LEFT )
		overallScore.SetPosition( rect.Left + 153, startY + overallRowY - overallScore.LineHeight, 0 )
		AddChild( overallScore )		
		
		// Medals
		medalCanvases = [ ]
		local medalSizes = [ MEDAL_SIZE_LARGE, MEDAL_SIZE_LARGE, MEDAL_SIZE_LARGE, MEDAL_SIZE_XL ]
		local medals = [ scores.DefenseMedal, scores.AggressionMedal, scores.ProfitMedal, scores.OverallMedal ]
		local medalXPositions = [ medalColumnX, medalColumnX, medalColumnX, 159 ]
		local medalYPositions = [ defenseRowY, aggressionRowY, profitRowY, 203 ]
		foreach( i, m in medals )
		{
			local imagePath = ::VictoryScreenMedalImagePath( m, medalSizes[ i ] )
			local medal = ::Gui.AsyncTexturedQuad( )
			if( imagePath )
			{
				medal.OnLoaded = function( quad )
				{
					quad.CenterPivot( )
					quad.SetPosition( 0, 0, 0 )
				}
				medal.SetTexture( imagePath )
			}
			
			local canvas = ::AnimatingCanvas( )
			canvas.SetPosition( rect.Left + medalXPositions[ i ], startY + medalYPositions[ i ], 0.0 )
			canvas.SetAlpha( 0 )
			canvas.AddChild( medal )
			AddChild( canvas )
			
			medalCanvases.push( canvas )
		}

		// Bonuses!
		local bonusData = ::GetBonuses( player1 )
		bonuses = ::BonusPointsSpawner( bonusData, audioSource )
		bonuses.SetPosition( overallScore.GetPosition( ) )
		AddChild( bonuses )
		
		// Friends data
		local startFriendsY = 88
		local friendsColumnX = 836
		friendsStartPos = ::Math.Vec3.Construct( rect.Left + friendsColumnX, startY + startFriendsY, 0 )
		
		asyncStatus = ::AsyncStatusSmall( )
		asyncStatus.SetPosition( modeText.GetXPos( ) + modeText.Width + 23, modeText.GetYPos( ) + modeText.Height * 0.5 + 2, 0 )
		AddChild( asyncStatus )
		
		playerItem = ::VictoryScreenFriendScore( player1.User.GamerTag, points.OverallScore, scores.OverallMedal )
		playerItem.SetAsPlayer( )
		
		player2Item = null
		if( player2 )
		{
			player2Item = ::VictoryScreenFriendScore( player2.User.GamerTag, points.OverallScore, scores.OverallMedal )
			player2Item.SetAsPlayer( )
		}
		
		local levelInfo = ::GameApp.CurrentLevelLoadInfo
		friendsScores = stats.RequestLevelData( levelInfo.MapType, levelInfo.LevelIndex )
		if( is_null( friendsScores ) )
			friendsScores = null
		if( friendsScores )
		{
			if( friendsScores.Initialized )
			{
				AddFriendScores( )
			}
			prevInit = friendsScores.Initialized
		}

		// DO IT
		BeginAnimations( )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( friendsScores )
		{
			if( !prevInit && friendsScores.Initialized )
			{
				AddFriendScores( )
			}
			
			prevInit = friendsScores.Initialized
		}
		
		if( launchFireworks )
			TickFireworks( dt )
	}

	
	function TickFireworks( dt )
	{
		fireworksTimer -= dt
		if( fireworksTimer < 0.0 )
		{
			fireworksTimer = ::SubjectiveRand.Float( 0.2, 0.9 )
			
			local pathchoice = ::SubjectiveRand.Int( 1, 4 )
			local path = "effects/fx/gui/large_firework_sequence_01.fxml"
			if( pathchoice == 2 )
				path = "effects/fx/gui/large_firework_sequence_02.fxml"
			else if( pathchoice == 3 )
				path = "effects/fx/gui/large_firework_sequence_03.fxml"
			else if( pathchoice == 4 )
				path = "effects/fx/gui/large_firework_sequence_04.fxml"
			
			local confetti = ::Gui.ScreenSpaceFxSystem( )
			confetti.SetSystem( path, 1, true )	//path, playcount(-1=loop), localSystem
			PlaySound( "Play_HUD_Victory_Firework" )
			
			local x = ::SubjectiveRand.Float( 0, screenWidth )
			local y = ::SubjectiveRand.Float( 0, screenHeight )
			confetti.SetPosition( x, y	0.725 )
			confetti.SetDelay( 0 )
			AddChild( confetti )
		}
	}
	
	function AddFriendScores( )
	{
		if( !friendsScores || !friendsScores.Initialized )
			return
			
		asyncStatus.SetAlpha( 0 )
			
		// Friends data
		local friendsData = [ ]
		local difficulty = ::GameApp.CurrentLevelLoadInfo.Difficulty
		
		local numFriends = ::Math.Min( 4, friendsScores.ScoreCount( difficulty ) )
		local friendMedals = [ MEDAL_RANK_BRONZE, MEDAL_RANK_BRONZE, MEDAL_RANK_BRONZE, MEDAL_RANK_BRONZE ]
		for( local i = 0; i < numFriends; ++i )
		{
			local friendStats = friendsScores.GetScore( difficulty, i )
			if( player2Item && friendStats.GamerTag.ToCString( ) == player2Item.friendName.ToCString( ) )
				continue
			local friendsItem = ::VictoryScreenFriendScore( friendStats.GamerTag, friendStats.Score, friendStats.OverallMedal )
			friendsData.push( friendsItem )
			AddChild( friendsItem )
		}
		
		friendsData.push( playerItem )
		AddChild( playerItem )
		if( player2Item )
		{
			friendsData.push( player2Item )
			AddChild( player2Item )
		}
		
		friendsData.sort( function( a, b )
		{
			if( a.score < b.score ) return 1
			else if( a.score > b.score ) return -1
			else return 0
		} )
		
		// Position Friends data
		foreach( i, item in friendsData )
		{
			item.SetPosition( friendsStartPos.x, friendsStartPos.y + i * ::VictoryScreenFriendScore.Spacing, 0 )
		}
	}
	
	function BeginAnimations( )
	{
		animating = true
		
		local gray = ::Math.Vec3.Construct( 0.5, 0.5, 0.5 )
		local defenseLabel = labelCanvases[ 0 ]
		local aggressionLabel = labelCanvases[ 1 ]
		local profitLabel = labelCanvases[ 2 ]
		
		local defenseMedal = medalCanvases[ 0 ]
		local aggressionMedal = medalCanvases[ 1 ]
		local profitMedal = medalCanvases[ 2 ]
		local overallMedal = medalCanvases[ 3 ]

		// Medal Drops
		local dropTime = 0.4

		// Scores Count Up
		local pauseTime = 1.0 + dropTime
		currentPoints = 0
		local currentTime = 0
		local delay = 0
		
		defenseLabel.AddAction( ::RgbTween( gray, COLOR_CLEAN_WHITE, 0.5 ) )
		defenseMedal.AddDelayedAction( delay, ::DefenseCountUpTweener( defenseScore, 0.0, defenseValue, 1.0, defenseTotal ) )
		CountUpScore( 0, points.DefensePoints, defensePoints )
		currentPoints += points.DefensePoints
		
		defenseMedal.AddDelayedAction( delay, ::UniformScaleTween( 1.2, 1.0, dropTime ) )
		defenseMedal.AddDelayedAction( delay, ::AlphaTween( 0.0, 1.0, dropTime ) )
		defenseMedal.AddDelayedAction( delay, ::SoundAction( 0.0, "Play_UI_Medal_Small", null, audioSource ) )
		medalFx01 = FireworksBurstWithAfterglow( this, defenseMedal, "gold", "small", delay )
		delay += pauseTime
	
	
		// Perfect Game
		if( scores.Bonus_Rainbow_Points > 0 )
		{
			// Count Up
			CountUpScore( delay, scores.Bonus_Rainbow_Points, defensePoints, points.DefensePoints )
			currentPoints += scores.Bonus_Rainbow_Points
			
			// Animation
			perfect = ::BonusPointsFloatingTextBase( ::GameApp.LocString( "Bonus_Perfect" ), ::Math.Vec4.Construct( 1.0, 0.895, 0.122, 1.0 ) )
			perfect.audioSource = audioSource
			perfect.CenterPivot( )
			perfect.SetPosition( defensePoints.GetXPos( ), defensePoints.GetYPos( ) + 100, 0 )
			AddChild( perfect )
			perfect.Animate( delay )
			
			delay += 1.0
		}
		
		aggressionLabel.AddDelayedAction( delay, ::RgbTween( gray, COLOR_CLEAN_WHITE, 0.5 ) )
		aggressionMedal.AddDelayedAction( delay, ::AggressionCountUpTweener( aggressionScore, 0.0, secondsSkipped, 1.0 ) )
		CountUpScore( delay, points.AggressionPoints, aggressionPoints )
		currentPoints += points.AggressionPoints
		
		aggressionMedal.AddDelayedAction( delay, ::UniformScaleTween( 1.2, 1.0, dropTime ) )
		aggressionMedal.AddDelayedAction( delay, ::AlphaTween( 0.0, 1.0, dropTime ) )
		medalFx02 = FireworksBurstWithAfterglow( this, aggressionMedal, "silver", "small", delay )
		aggressionMedal.AddDelayedAction( delay, ::SoundAction( 0.0, "Play_UI_Medal_Small", null, audioSource ) )
		
		delay += pauseTime
		
		profitLabel.AddDelayedAction( delay, ::RgbTween( gray, COLOR_CLEAN_WHITE, 0.5 ) )
		profitMedal.AddDelayedAction( delay, ::ProfitCountUpTweener( profitScore, 0.0, profitRatio, 1.0 ) )
		CountUpScore( delay, points.ProfitPoints, profitPoints )
		currentPoints += points.ProfitPoints
		
		profitMedal.AddDelayedAction( delay, ::UniformScaleTween( 1.2, 1.0, dropTime ) )
		profitMedal.AddDelayedAction( delay, ::AlphaTween( 0.0, 1.0, dropTime ) )
		medalFx03 = FireworksBurstWithAfterglow( this, profitMedal, "gold", "small", delay )
		profitMedal.AddDelayedAction( delay, ::SoundAction( 0.0, "Play_UI_Medal_Small", null, audioSource ) )
		
		delay += pauseTime
		
		// Bonuses!
		bonuses.Animate( delay )
		CountUpScore( delay, points.BonusPoints - scores.Bonus_Rainbow_Points, null, 0, bonuses.Count( ) * ::BonusPointsSpawner.timeBetweenSpawns )
		delay += 1.0
		
		// Final action
		overallLabelCanvas.AddDelayedAction( delay, ::RgbTween( gray, COLOR_CLEAN_WHITE, 0.5 ) )
		overallMedal.AddAction( ::CanvasAction( delay, null, null, function( canvas ) { animating = false }.bindenv(this) ) )
		overallMedal.AddDelayedAction( delay, ::UniformScaleTween( 1.4, 1.0, dropTime ) )
		overallMedal.AddDelayedAction( delay, ::AlphaTween( 0.0, 1.0, dropTime ) )
		medalFx04 = FireworksBurstWithAfterglow( this, overallMedal, "gold", "big", delay )
		local medalSounds = [ "Play_UI_Medal_Bronze", "Play_UI_Medal_Bronze", "Play_UI_Medal_Silver", "Play_UI_Medal_Gold", "Play_UI_Medal_Platinum" ]
		overallMedal.AddDelayedAction( delay, ::SoundAction( 0.0, medalSounds[ scores.OverallMedal ], null, audioSource ) )
	}
	
	function CountUpScore( delay, value, secondaryText = null, secondaryStart = 0, time = 1.0 )
	{
		local a = medalCanvases[ 3 ]
		a.AddDelayedAction( delay, ::TextCountTween( overallScore, currentPoints, currentPoints + value, time, function( text, v )
		{
			text.BakeCString( ::StringUtil.AddCommaEvery3Digits( v.tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT )
		}, "Play_HUD_Points", audioSource ) )
		if( points.OverallScore > 0 && value > 0 )
			a.AddDelayedAction( delay, ::SoundRtpcTween( "Points_Aquired", currentPoints / points.OverallScore, (currentPoints + value) / points.OverallScore, time, audioSource ) )
		
		if( secondaryText )
		{
			a.AddDelayedAction( delay, ::TextCountTween( secondaryText, secondaryStart, secondaryStart + value, time, function( text, v )
			{
				text.BakeCString( ::StringUtil.AddCommaEvery3Digits( v.tointeger( ).tostring( ) ), TEXT_ALIGN_RIGHT )
			} ) )
		}
	}
	
	function HandleInput( gamepad )
	{
		// Handle Skipping
		if( animating && gamepad.ButtonDown( GAMEPAD_BUTTON_A ) )
		{
			Skip( )
			return true
		}

		return false
	}
	
	function FadeOutMedalEffects( )
	{
		launchFireworks = false
		
		if( medalFx01 != null )
			medalFx01.OverrideSystemAlphas( 0.0 )
		if( medalFx02 != null )
			medalFx02.OverrideSystemAlphas( 0.0 )
		if( medalFx03 != null )
			medalFx03.OverrideSystemAlphas( 0.0 )
		if( medalFx04 != null )
			medalFx04.OverrideSystemAlphas( 0.0 )
	}
	
	function Skip( )
	{
		animating = false
		
		// Set final label colors
		foreach( canvas in labelCanvases )
		{
			canvas.ClearActions( )
			canvas.SetRgba( COLOR_CLEAN_WHITE )
		}
		
		overallLabelCanvas.ClearActions( )
		overallLabelCanvas.SetRgba( COLOR_CLEAN_WHITE )
		
		// Drop in medals
		foreach( canvas in medalCanvases )
		{
			canvas.ClearActions( )
			canvas.SetAlpha( 1.0 )
			canvas.SetUniformScale( 1.0 )
		}
		
		// Hide bonuses
		bonuses.Skip( )
		if( perfect )
		{
			RemoveChild( perfect )
			perfect.DeleteSelf( )
		}
		
		// Set final scores
		profitScore.BakeCString( (profitRatio * 100).tointeger( ).tostring( ) + "%", TEXT_ALIGN_RIGHT )
		aggressionScore.BakeLocString( ::LocString.ConstructTimeString( secondsSkipped, false ), TEXT_ALIGN_RIGHT )
		defenseScore.BakeCString( defenseValue.tostring( ) + "/" + defenseTotal, TEXT_ALIGN_RIGHT )
		overallScore.BakeCString( ::StringUtil.AddCommaEvery3Digits( points.OverallScore.tointeger( ).tostring( ) ), TEXT_ALIGN_LEFT )
		
		profitPoints.BakeCString( ::StringUtil.AddCommaEvery3Digits( points.ProfitPoints.tointeger( ).tostring( ) ), TEXT_ALIGN_RIGHT )
		aggressionPoints.BakeCString( ::StringUtil.AddCommaEvery3Digits( points.AggressionPoints.tointeger( ).tostring( ) ), TEXT_ALIGN_RIGHT )
		local totalDefensePoints = points.DefensePoints + scores.Bonus_Rainbow_Points
		defensePoints.BakeCString( ::StringUtil.AddCommaEvery3Digits( totalDefensePoints.tointeger( ).tostring( ) ), TEXT_ALIGN_RIGHT )
		
		medalFx01.SetDelay( 0.0 )
		medalFx02.SetDelay( 0.0 )
		medalFx03.SetDelay( 0.0 )
		medalFx04.SetDelay( 0.0 )
		
		medalFx01.FastForward( 0.5 )
		medalFx02.FastForward( 0.5 )
		medalFx03.FastForward( 0.5 )
		medalFx04.FastForward( 0.5 )
	}
}
