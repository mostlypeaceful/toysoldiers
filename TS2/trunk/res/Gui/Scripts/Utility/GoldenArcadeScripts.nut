// Golden Arcade

// Resources
sigimport "gui/textures/hud/golden_arcade_icon_g.png"
sigimport "gui/textures/hud/golden_arcade_small_g.png"
sigimport "gui/textures/score/score_decoration_g.png"
// dog tags and babookshkas imported in dlc_imports.nut

function GoldenArcadeIconTexture( small = false )
{
	if( small )
		return "gui/textures/hud/golden_arcade_small_g.png"
	else
		return "gui/textures/hud/golden_arcade_icon_g.png"
}

class GoldenArcadeIcon extends AnimatingCanvas
{
	constructor( small = false )
	{
		::AnimatingCanvas.constructor( )
		
		local icon = ::Gui.TexturedQuad( )
		icon.SetTexture( ::GoldenArcadeIconTexture( small ) )
		AddChild( icon )
	}
}

class GoldenObjectPanel extends AnimatingCanvas
{
	constructor( icon, textID, startingScores )
	{
		::AnimatingCanvas.constructor( )
		
		AddChild( icon )
		
		local text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( textID ), TEXT_ALIGN_RIGHT )
		text.SetPosition( -8, 4, 0 )
		AddChild( text )
		
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		line.SetPosition( -text.Width - 8, 32, 0 )
		line.SetRect( ::Math.Vec2.Construct( text.Width + 8 + 32, line.TextureDimensions( ).y + 1 ) )
		AddChild( line )
		
		SetLevelInfo( startingScores )
	}
	
	function CheckStatus( scores )
	{
		return true
	}
	
	function SetLevelInfo( scores )
	{
		if( CheckStatus( scores ) )
			SetAlpha( 1 )
		else
			SetAlpha( 0 )
	}
}

class GoldenArcadePanel extends GoldenObjectPanel
{
	constructor( startingScores )
	{
		::GoldenObjectPanel.constructor( ::GoldenArcadeIcon( true ), "Golden_Arcade_Destroyed", startingScores )
	}
	
	function CheckStatus( scores )
	{
		return scores.HasFoundGoldenArcade
	}
}

function GoldenBabushkaIconTexture( small = false )
{
	if( small )
		return "gui/textures/hud/golden_babushka_small_g.png"
	else
		return "gui/textures/hud/golden_babushka_icon_g.png"
}

class GoldenBabushkaIcon extends AnimatingCanvas
{
	constructor( small = false )
	{
		::AnimatingCanvas.constructor( )
		
		local icon = ::Gui.TexturedQuad( )
		icon.SetTexture( ::GoldenBabushkaIconTexture( small ) )
		AddChild( icon )
	}
}

class GoldenBabushkaPanel extends GoldenObjectPanel
{
	constructor( startingScores )
	{
		::GoldenObjectPanel.constructor( ::GoldenBabushkaIcon( true ), "Golden_Babushka_All_Found", startingScores )
	}
	
	function CheckStatus( scores )
	{
		return scores.HasFoundAllGoldenBabushkas
	}
}

function GoldenDogTagIconTexture( small = false )
{
	if( small )
		return "gui/textures/hud/golden_dogtag_small_g.png"
	else
		return "gui/textures/hud/golden_dogtag_icon_g.png"
}

class GoldenDogTagIcon extends AnimatingCanvas
{
	constructor( small = false )
	{
		::AnimatingCanvas.constructor( )
		
		local icon = ::Gui.TexturedQuad( )
		icon.SetTexture( ::GoldenDogTagIconTexture( small ) )
		AddChild( icon )
	}
}

class GoldenDogTagPanel extends GoldenObjectPanel
{
	constructor( startingScores )
	{
		::GoldenObjectPanel.constructor( ::GoldenDogTagIcon( true ), "Golden_DogTag_All_Found", startingScores )
	}
	
	function CheckStatus( scores )
	{
		return scores.HasFoundAllGoldenBabushkas
	}
}