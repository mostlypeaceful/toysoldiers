// Control to show the new turrets onlocked

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

class UnlockedUnitIcon extends AnimatingCanvas
{
	image = null
	
	constructor( unitID, country )
	{
		::AnimatingCanvas.constructor( )
		
		local newUnitID = ((country == COUNTRY_USA)? "USA_": "USSR_") + unitID
		
		image = ::Gui.AsyncTexturedQuad( )
		image.SetTexture( "gui/textures/endgamescreens/unlockicons/" + newUnitID + "_unlock_g.png" )
		image.SetPosition( ::Math.Vec3.Construct( -160, -120, 0 ) ) 
		
		local unitDesc = ::GameApp.LocString( "Unlock_UnitDescFormat" ).Replace( "name", newUnitID ).Replace( "class", newUnitID + "_Class" ).Replace( "desc", newUnitID + "_FullDescription" )
		
		local text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeBoxLocString( 360, unitDesc, TEXT_ALIGN_CENTER )
		text.SetPosition( ::Math.Vec3.Construct( 0, 120 + 10, 0 ) )
		AddChild( text )
		
		FadeIn( 0.3 )
		AddChild( image )
	}
}

class UnlockScreen extends AnimatingCanvas
{
	icons = null
	
	constructor( rect, player1 )
	{
		::AnimatingCanvas.constructor( )
		
		// Unlocked Text
		local text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_LARGE )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeLocString( ::GameApp.LocString( "EndGame_Unlocked" ), TEXT_ALIGN_CENTER )
		text.SetPosition( rect.Center.x, rect.Top + 50, 0 )
		AddChild( text )
		
		// Decoration
		local decoration = ::Gui.TexturedQuad( )
		decoration.SetTexture( "gui/textures/score/score_decoration_g.png" )
		decoration.CenterPivot( )
		decoration.SetPosition( text.GetXPos( ), text.GetYPos( ) + text.Height + 5, 0 )
		AddChild( decoration )

		// Unlock Icons
		icons = [ ]
		local unlockedUnitIds = player1.GetUnlockedUnitIDs( ::GameApp.CurrentLevel.LevelNumber )
		local spacing = 395
		local startPos = rect.Center.x - (unlockedUnitIds.len( ) - 1) * 0.5 * spacing
		local i = 0
		foreach( key, id in unlockedUnitIds )
		{
			local icon = ::UnlockedUnitIcon( key, player1.Country )
			icon.SetPosition( startPos + i * spacing, rect.Center.y, 0 )
			icons.push( icon )
			AddChild( icon )
			i++
		}
		
		// Unlock general and elite modes
		local unlockText = ::Gui.Text( )
		unlockText.SetFontById( FONT_FANCY_MED )
		unlockText.SetRgba( COLOR_CLEAN_WHITE )
		unlockText.BakeLocString( ::GameApp.LocString( "Unlock_EliteGeneral" ), TEXT_ALIGN_CENTER )
		unlockText.SetPosition( rect.Center.x, decoration.GetYPos( ) + 10, 0 )
		AddChild( unlockText )
	}
	
	function HasUnlocks( )
	{
		if( icons.len( ) > 0 )
			return true
		
		return false
	}
	
	function OnDelete( )
	{
		foreach( icon in icons )
			icon.image.Unload( )
	}
}