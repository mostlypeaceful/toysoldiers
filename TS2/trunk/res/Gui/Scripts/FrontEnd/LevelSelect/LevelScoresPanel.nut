// Show the scores for the selected level

// Requires
sigimport "gui/scripts/frontend/levelselect/difficultyselector.nut"
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/utility/medalscripts.nut"
sigimport "gui/scripts/utility/modeutility.nut"

// Resources
sigimport "gui/textures/endgamescreens/medals/bronze_sm_g.png"
sigimport "gui/textures/endgamescreens/medals/gold_sm_g.png"
sigimport "gui/textures/endgamescreens/medals/platinum_sm_g.png"
sigimport "gui/textures/endgamescreens/medals/silver_sm_g.png"
sigimport "gui/textures/endgamescreens/medals/bronze_overall_sm_g.png"
sigimport "gui/textures/endgamescreens/medals/gold_overall_sm_g.png"
sigimport "gui/textures/endgamescreens/medals/platinum_overall_sm_g.png"
sigimport "gui/textures/endgamescreens/medals/silver_overall_sm_g.png"
sigimport "gui/textures/frontend/locked_level_small_g.png"

class BaseLevelScoresPanel extends AnimatingCanvas
{
	// Data
	active = null
	autoLaunch = null
	selectedIndex = null
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		active = false
		autoLaunch = false
		selectedIndex = -1
	}
	
	function AutoLaunch( )
	{
		return autoLaunch
	}
	
	function SelectedIndex( )
	{
		return selectedIndex
	}

	function SetActive( active_ )
	{
		active = active_
	}
	
	function ModeLocked( mode = null )
	{
		return false
	}
	
	function GetDescriptor( )
	{
		return null
	}

	// Virtual
	function SetSelection( index ) { }
	function SetLevelInfo( scores, info ) { }
	function FillLoadInfo( info ) { }
	function ResetHeadingsAndScores( ) { }
}

class LevelScoresPanel extends BaseLevelScoresPanel
{
	// Display
	headers = null
	columns = null // array of Gui.Text objects
	selector = null // DifficultySelector
	highlight = null // Gui.TexturedQuad
	//descText = null
	//descLabel = null
	medalIcons = null
	mapType = null
	lockIcons = null
	
	// Functionality
	startPos = null // number
	spacing = null // number
	selectorGoal = null // number, x value of selector
	modeCount = null // number
	startingSelection = null
	hasPlayedMode = null
	player = null
	hasDlc = null // function
	
	// "Protected" Data
	modeCount = null // number
	modeNames = null // array of LocIDs
	startingY = null
	
	// Statics
	static lineHeight = 30
	
	constructor( mapType_, startingScores, labels_, player_, initialSelection = 1, hasDlcFunc = null )
	{
		::BaseLevelScoresPanel.constructor( )
		mapType = mapType_
		player = player_
		hasPlayedMode = [ false, false, false, false, false ]
		startingY = 30
		hasDlc = ( hasDlcFunc == null ? StandardHasDlc : hasDlcFunc )

		if( labels_ )
		{
			modeNames = labels_
			modeCount = labels_.len( )
			//modeDesc = desc
		}
		startingSelection = initialSelection
		
		ResetHeadingsAndScores( )
		
		local height = 138
		selector = ::DifficultySelector( height )
		selector.SetPosition( Math.Vec3.Construct( startPos, height / 2, 0.01 ) )
		selector.SetAlpha( 0.0 )
		AddChild( selector )
		
		highlight = ::PanelHighlight( ::Math.Vec2.Construct( 751, height ), 3 )
		highlight.SetPosition( Math.Vec3.Construct( 0, 0, -0.02 ) )
		AddChild( highlight )
		
		if( startingScores )
		{
			SetLevelInfo( startingScores, null, player )
			SetActive( false )
			SetSelection( initialSelection )
		}
	}
	
	function ClearText( )
	{
		local containers = [ headers, columns, lockIcons, medalIcons ]
		
		foreach( container in containers )
		{
			if( container != null )
			{
				foreach( item in container )
				{
					if( item != null )
						RemoveChild( item )
				}
			}
		}
		
		headers = [ ]
		columns = [ ]
		lockIcons = [ ]
		medalIcons = [ ]
	}
	
	function SetupText( )
	{
		local wideLanguage = ::GameApp.IsWideLanguage( )
		local asianLanguage = ::GameApp.IsAsianLanguage( )
		
		local pad = 10
		local labelXPos = 108
		startPos = 170
		spacing = 125
		
		local labelFont = ( wideLanguage ? FONT_SIMPLE_SMALL : FONT_SIMPLE_MED )
		local labelLocs = [ "High_Score", "Score_Medals", "Score_Overall" ]
		
		foreach( i, loc in labelLocs )
		{
			local label = ::Gui.Text( )
			label.SetFontById( labelFont )
			label.SetRgba( COLOR_CLEAN_WHITE )
			label.BakeLocString( ::GameApp.LocString( loc ) % ":", TEXT_ALIGN_RIGHT )
			label.SetPosition( labelXPos, startingY + lineHeight * i, 0 )
			AddChild( label )
			
			if( mapType != MAP_TYPE_CAMPAIGN )
				break
		}
		
		for( local i = 0; i < modeCount; ++i )
		{
			local header = ::Gui.Text( )
			header.SetFontById( FONT_FANCY_MED )
			header.SetRgba( COLOR_CLEAN_WHITE )
			header.SetPosition( startPos + spacing * i, -2, 0 )
			if( asianLanguage )
				header.SetYPos( 2 )
			header.BakeLocString( modeNames[ i ], TEXT_ALIGN_CENTER )
			if( !asianLanguage )
				header.SetUniformScale( 0.85 )
			header.CompactMaintainScale( spacing - 16 )
			headers.push( header )
			AddChild( header )
			
			local column = ::Gui.Text( )
			column.SetFontById( FONT_SIMPLE_MED )
			column.SetRgba( COLOR_CLEAN_WHITE )
			column.SetPosition( startPos + spacing * i, startingY, 0 )
			columns.push( column )
			AddChild( column )
			
			if( ( mapType == MAP_TYPE_CAMPAIGN && i >= DIFFICULTY_ELITE ) || ( mapType == MAP_TYPE_SURVIVAL && i >= CHALLENGE_MODE_TRAUMA ) )
			{
				local lock = ::Gui.TexturedQuad( )
				lock.SetTexture( "gui/textures/frontend/locked_level_small_g.png" )
				lock.CenterPivot( )
				lock.SetPosition( startPos + spacing * i, startingY + 16, 0 )
				lock.SetAlpha( 0 )
				lockIcons.push( lock )
				AddChild( lock )
			}
			else
				lockIcons.push( null )
		}
	}
	
	function ResetHeadingsAndScores( )
	{
		ClearText( )
		
		if( mapType == MAP_TYPE_SURVIVAL )
		{
			modeCount = 3
			
			//if( player.HasDLC( DLC_EVIL_EMPIRE ) )
			if( hasDlc( player, DLC_EVIL_EMPIRE ) )
				modeCount = 4
			
			//if( player.HasDLC( DLC_NAPALM ) )
			if( hasDlc( player, DLC_NAPALM ) )
				modeCount = 5
			
			::print( "reset modeCount: " + modeCount.tostring() )
		}
		
		SetupText( )
	}
	
	function StandardHasDlc( player, dlc )
	{
		::print( "using standard dlc (scores)" )
		return player.HasDLC( dlc )
	}
	
	function SetActive( active_ )
	{
		::BaseLevelScoresPanel.SetActive( active_ )
		selector.Show( active )
		if( active == true )
			SetSelection( startingSelection )
		
		highlight.SetActive( active )
	}
	
	function ChangeHorizontalHighlight( delta )
	{
		local i = selectedIndex + delta
		if( i < 0 ) i = modeCount - 1
		if( i > modeCount - 1 ) i = 0
		
		SetSelection( i )
	}
	
	function SelectedIndex( )
	{
		return selectedIndex
	}
	
	function ChangeHighlight( delta )
	{
		// Do nothing
	}
	
	function SetSelection( index )
	{
		if( selectedIndex != index )
		{
			selectedIndex = index
			// Move selector
			local pos = selector.GetPosition( )
			pos.x = startPos + spacing * index
			selector.SetPosition( pos )
		}
	}

	function SetLevelInfo( scores, info, player_ )
	{
		player = player_
		
		local height = lineHeight
		local startY = startingY + height * 0.5
		
		// Clear previous medal icons
		foreach( icon in medalIcons )
		{
			icon.DeleteSelf( )
			RemoveChild( icon )
		}
		medalIcons.clear( )
		
		// Get High Scores
		local highScores = array( modeCount, -1 )
		foreach( i, value in highScores )
			highScores[ i ] = scores.GetHighScore( i )

		// Get Medal Progress
		local medals = array( modeCount, -1 )		
		local medalSpacing = 26
		local medalX = { 
			[ MEDAL_TYPE_DEFENSE ] = -medalSpacing,
			[ MEDAL_TYPE_AGGRESSION ] = 0,
			[ MEDAL_TYPE_PROFIT ] = medalSpacing,
			[ MEDAL_TYPE_OVERALL ] = 0
		}
		local medalY = { 
			[ MEDAL_TYPE_DEFENSE ] = height,
			[ MEDAL_TYPE_AGGRESSION ] = height,
			[ MEDAL_TYPE_PROFIT ] = height,
			[ MEDAL_TYPE_OVERALL ] = height + height
		}
		local medalIsOverall = [ false, false, false, true ]
		
		if( mapType == MAP_TYPE_CAMPAIGN )
		{
			foreach( i, value in medals )
			{
				medals[ i ] = {
					[ MEDAL_TYPE_DEFENSE ] = scores.MedalProgress( i, MEDAL_TYPE_DEFENSE ),
					[ MEDAL_TYPE_AGGRESSION ] = scores.MedalProgress( i, MEDAL_TYPE_AGGRESSION ),
					[ MEDAL_TYPE_PROFIT ] = scores.MedalProgress( i, MEDAL_TYPE_PROFIT ),
					[ MEDAL_TYPE_OVERALL ] = scores.MedalProgress( i, MEDAL_TYPE_OVERALL )
				}
			}
		}
		
		hasPlayedMode = [ false, false, false, false, false ]
		
		// Create scores
		foreach( i, col in columns )
		{
			// Create Points
			local str = null
			local havePlayed = highScores[ i ] >= 0
			if( havePlayed )
				str = ::StringUtil.AddCommaEvery3Digits( highScores[ i ].tointeger( ).tostring( ) )
			else
				str = "---"
			hasPlayedMode[ i ] = havePlayed
			local locked = ModeLocked( i )
			
			if( locked )
			{
				col.SetAlpha( 0 )
				if( lockIcons[ i ] )
					lockIcons[ i ].SetAlpha( 1 )
				headers[ i ].SetRgba( COLOR_LOCKED_GREEN )
			}
			else
			{
				col.SetAlpha( 1 )
				if( lockIcons[ i ] )
					lockIcons[ i ].SetAlpha( 0 )
				headers[ i ].SetRgba( COLOR_CLEAN_WHITE )
				col.BakeBoxCString( 145, str, TEXT_ALIGN_CENTER )
			}
			
			if( havePlayed && mapType == MAP_TYPE_CAMPAIGN )
			{
				// Create Medals
				foreach( m, value in medals[ i ] )
				{
					local icon = ::Gui.TexturedQuad( )
					local medalTexture =  ::VictoryScreenMedalImagePath( value, MEDAL_SIZE_SMALL, medalIsOverall[ m ] )
					if( medalTexture )
						icon.SetTexture( medalTexture )
					icon.CenterPivot( )
					icon.SetPosition( startPos + spacing * i + medalX[ m ], startY + medalY[ m ], 0 )
					AddChild( icon )
					medalIcons.push( icon )
				}
			}
		}
	}
	
	function GetSelection( ) 
	{
		return Math.Clamp( selectedIndex, 0, modeCount )
	}
	
	function AutoLaunch( )
	{
		return false
	}
	
	function ModeLocked( mode = null )
	{
		if( ::Player.RandyMode( ) )
			return false
			
		if( mode == null )
			mode = GetSelection( ) 
			
		if( mapType == MAP_TYPE_CAMPAIGN )
		{
			if( mode < DIFFICULTY_ELITE )	
				return false
			
			if( !hasPlayedMode[ DIFFICULTY_CASUAL ] && !hasPlayedMode[ DIFFICULTY_NORMAL ] && !hasPlayedMode[ DIFFICULTY_HARD ] )
				return true
		}
		else if( mapType == MAP_TYPE_SURVIVAL )
		{
			//if( mode == CHALLENGE_MODE_TRAUMA && !player.HasDLC( DLC_EVIL_EMPIRE ) )
			if( mode == CHALLENGE_MODE_TRAUMA && !hasDlc( player, DLC_EVIL_EMPIRE ) )
				return true
		}
		
		return false
	}
	
	function FillLoadInfo( info ) {} // Virtual
}

class CampaignLevelScoresPanel extends LevelScoresPanel
{
	dlc = null
	
	constructor( startingScores, dlc_ )
	{
		dlc = dlc_
		::LevelScoresPanel.constructor( MAP_TYPE_CAMPAIGN, startingScores, ::DifficultyNames, null, 1 )
	}
	
	function ChangeDLC( dlc_ )
	{
		dlc = dlc_
	}

	function FillLoadInfo( info )
	{
		info.Difficulty = GetSelection( )
		info.ChallengeMode = 0
	}
	
	function GetDescriptor( )
	{
		local difficultyDesc = [
			dlc == DLC_EVIL_EMPIRE ? "EE_Difficulty_Casual_Description": "Difficulty_Casual_Description",
			dlc == DLC_EVIL_EMPIRE ? "EE_Difficulty_Normal_Description": "Difficulty_Normal_Description",
			"Difficulty_Hard_Description",
			ModeLocked( DIFFICULTY_ELITE )? "Difficulty_Elite_Locked_Description": "Difficulty_Elite_Description",
			ModeLocked( DIFFICULTY_GENERAL )? "Difficulty_General_Locked_Description": "Difficulty_General_Description"
		]

		return difficultyDesc[ GetSelection( ) ]
	}
}

class ChallengeLevelScoresPanel extends LevelScoresPanel
{
	constructor( startingScores, player_, hasDlcFunc = null )
	{
		::LevelScoresPanel.constructor( MAP_TYPE_SURVIVAL, startingScores, ::SurvivalModeNames, player_, 0, hasDlcFunc )
	}
	
	function FillLoadInfo( info )
	{
		info.Difficulty = DIFFICULTY_NORMAL// TODO: Find default difficulty for challenge modes
		info.ChallengeMode = GetSelection( )
	}
	
	function GetDescriptor( )
	{
		local selection = GetSelection( )
		if( selection == CHALLENGE_MODE_COMMANDO && ModeLocked( CHALLENGE_MODE_COMMANDO ) ) 
			return "Survival_Trauma_LockedDesc"
		return ::SurvivalModeDesc[ GetSelection( ) ]
	}
}
/*
class MinigameLevelScoresPanel extends LevelScoresPanel
{
	constructor( startingScores )
	{
		local difficultyNames = [
			::GameApp.LocString( "Difficulty_Casual" ),
			::GameApp.LocString( "Difficulty_Normal" ),
			::GameApp.LocString( "Difficulty_Hard" )
		]
		::LevelScoresPanel.constructor( MAP_TYPE_MINIGAME, startingScores, difficultyNames, 0 )
	}
	
	function FillLoadInfo( info )
	{
		info.Difficulty = GetSelection( )
		info.ChallengeMode = 0
	}
}*/
