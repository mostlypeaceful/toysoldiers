// Focus Prompt

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

sigexport function CanvasCreateFocalPrompt( scriptedControl )
{
	return FocalPrompt( )
}

class FocalPrompt extends AnimatingCanvas
{
	// Data
	vpRect = null
	shown = null
	text = null
	control = null
	
	// Statics
	static time = 0.3
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		shown = false 
		vpRect = null
	}
	
	function Setup( user )
	{		
		ClearChildren( )
		
		// Create Text
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_MED )
		text.SetRgba( COLOR_CLEAN_WHITE )
		AddChild( text )
		
		// Create icon
		control = ::ControllerButton( GAMEPAD_BUTTON_Y, "Focus_Explain", null, FONT_SIMPLE_SMALL )
		control.SetPosition( -control.GetSize( ).Width * 0.5, 15, 0 )
		AddChild( control )
		
		// Add Self to HUD Root
		::GameApp.HudRoot.AddChild( this )
		
		vpRect = user.ComputeViewportSafeRect( )
		SetPosition( vpRect.Center.x, vpRect.Center.y - 100, 0.3 )
		
		SetAlpha( 0 )
	}
	
	function Show( locID )
	{
		text.BakeLocString( ::GameApp.LocString( locID ), TEXT_ALIGN_CENTER )
		text.SetPosition( 0, -text.LineHeight, 0 )
		
		Hide( false )
	}
	
	function Hide( hide )
	{
		if( hide )
		{
			if( shown )
			{
				shown = false
				ClearActions( )
				AddPulse( )
				AddDelayedAction( 3.0, ::AlphaTween( GetAlpha( ), 0.0, time * GetAlpha( ), null, EasingEquation.In, null, function( canvas )
				 {
					 canvas.SetAlpha( 0 )
				 } ) )
				 
				 
				control.ClearActions( )
				control.AddAction( ::AlphaTween( GetAlpha( ), 0.0, time * GetAlpha( ), null, EasingEquation.In, null, function( canvas )
				 {
					 canvas.SetAlpha( 0 )
				 } ) )
			}
		}
		else if( !shown )
		{
			shown = true
			ClearActions( )
			AddAction( ::AlphaTween( GetAlpha( ), 1.0, time ) )
			AddPulse( )

			control.ClearActions( )
			control.AddAction( ::AlphaTween( GetAlpha( ), 1.0, time ) )
			control.AddAction( ::RgbPulse( ::Math.Vec3.Construct( 0.8, 0.8, 0.8 ), ::Math.Vec3.Construct( 1.5, 1.5, 1.5 ), 0.5 ) )
		}
	}
	
	function AddPulse( )
	{
		AddAction( ::RgbPulse( ::Math.Vec3.Construct( 1.0, 1.0, 1.0 ), ::Math.Vec3.Construct( 2.0, 2.0, 2.0 ), 0.3 ) )
	}
}