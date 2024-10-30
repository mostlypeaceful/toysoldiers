// Speed Bonus UI

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

class SpeedBonusUI extends AnimatingCanvas
{
	constructor( player )
	{
		local label = Gui.Text( )
		label.SetFontById( FONT_SIMPLE_MED )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( ::GameApp.LocString( "Minigame_ComboLabel" ) )
		label.SetYPos( -label.Height )
		AddChild( label )
		
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		AddChild( line )
		
		local vpRect = player.User.ComputeViewportSafeRect( )
		SetPosition( vpRect.Left, vpRect.Bottom, 0.3 )
		::GameApp.HudLayer( "alwaysShow" ).AddChild( this )
	}
	
	function TargetHit( killValue, speedBonus )
	{
	}
}