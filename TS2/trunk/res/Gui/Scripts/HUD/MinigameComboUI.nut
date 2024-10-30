// Minigame Combo

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

class MinigameComboUI extends AnimatingCanvas
{
	// Display
	combo = null
	
	// Data
	comboValue = null
	vpRect = null
	
	// Statics
	static slideDistance = 256
	static slideTime = 0.5
	
	constructor( player, labelText = "Minigame_ComboLabel" )
	{
		::AnimatingCanvas.constructor( )
		comboValue = 0
		
		local label = Gui.Text( )
		label.SetFontById( FONT_SIMPLE_MED )
		label.SetRgba( COLOR_CLEAN_WHITE )
		label.BakeLocString( ::GameApp.LocString( labelText ) )
		label.SetYPos( -label.Height )
		AddChild( label )
		
		combo = ::Gui.Text( )
		combo.SetFontById( FONT_FANCY_LARGE )
		combo.SetRgba( COLOR_CLEAN_WHITE )
		SetValueString( )
		combo.SetPosition( label.Width + 10, -combo.Height, 0 )
		AddChild( combo )
		
		local line = ::Gui.TexturedQuad( )
		line.SetTexture( "gui/textures/score/score_decoration_g.png" )
		AddChild( line )
		
		vpRect = player.User.ComputeViewportSafeRect( )
		SetPosition( vpRect.Left - slideDistance, vpRect.Bottom, 0.1 )
		::GameApp.HudLayer( "alwaysShow" ).AddChild( this )
		
		SetAlpha( 0 )
	}
	
	function ProcessTutorialEvent( event )
	{
		if( event.EventID == TUTORIAL_EVENT_UNIT_DESTROYED && !is_null( event.Player ) )
		{
			local eventCombo = event.Combo
			if( eventCombo != comboValue )
			{
				if( comboValue <= 1 && eventCombo > 1 )
					Show( )
				else if( eventCombo <= 1 )
					Hide( )
				comboValue = eventCombo
				local str = comboValue.tostring( ) + "x"
				combo.BakeCString( str )
			}
		}
		if( event.EventID == TUTORIAL_EVENT_COMBO_LOST )
		{
			if( comboValue > 1 )
			{
				Hide( )
				comboValue = 0
			}
		}
	}
	
	function SetValue( value )
	{
		if( value != comboValue )
		{
			if( comboValue <= 1 && value > 1 )
				Show( )
			else if( value <= 1 )
				Hide( )
			comboValue = value
			SetValueString( )
		}
	}
	
	function SetValueString( )
	{
		local str = comboValue.tostring( ) + "x"
		combo.BakeCString( str )
	}
	
	function Show( )
	{
		ClearActions( )
		AddAction( ::XMotionTween( GetXPos( ), vpRect.Left, slideTime ) )
		AddAction( ::AlphaTween( GetAlpha( ), 1.0, slideTime ) )
	}
	
	function Hide( )
	{
		ClearActions( )
		AddAction( ::XMotionTween( GetXPos( ), vpRect.Left - slideDistance, slideTime ) )
		AddAction( ::AlphaTween( GetAlpha( ), 0.0, slideTime ) )
	}
}

class MinigamePassengerUI extends MinigameComboUI
{
	constructor( player )
	{
		::MinigameComboUI.constructor( player, "Minigame_Passengers" )
		SetYPos( vpRect.Bottom - 35 )
		Show( )
	}
	
	function ProcessTutorialEvent( event )
	{ 
		// Override base to not use it
	}
	
	function SetValue( value )
	{
		if( value != comboValue )
		{
			comboValue = value
			SetValueString( )
		}
	}
	
	function SetValueString( )
	{
		local str = comboValue.tostring( )
		combo.BakeCString( str )
	}
}
