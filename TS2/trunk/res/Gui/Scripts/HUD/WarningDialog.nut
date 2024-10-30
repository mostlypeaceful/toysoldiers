// Warning Dialog

// Requires
sigimport "gui/scripts/controls/gamepad.nut"

// Resources
sigimport "gui/textures/hud/warning_background_g.png"

class WarningDialog extends AnimatingCanvas
{
	// Display
	text = null
	
	// Data
	gamepad = null
	noInput = null
	
	// Callbacks
	onOk = null
	onCancel = null
	
	// Statics
	static TextWidth = 280
	
	constructor( loc, player )
	{
		::AnimatingCanvas.constructor( )
		audioSource = player.AudioSource
		noInput = true
		gamepad = ::FilteredGamepad( player.User )
		
		local bg = ::Gui.TexturedQuad( )
		bg.SetTexture( "gui/textures/hud/warning_background_g.png" )
		bg.SetPosition( 0, 0, 0.001 )
		AddChild( bg )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		if( typeof loc == "string" )
			text.BakeLocString( ::GameApp.LocString( loc ), TEXT_ALIGN_CENTER )
		else if( typeof loc == "instance" )
			text.BakeLocString( loc, TEXT_ALIGN_CENTER )
		text.SetPosition( bg.TextureDimensions( ).x * 0.5, 20 - text.Height * 0.5, 0 )
		AddChild( text )
		
		text.Compact( TextWidth )
		
		local controls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL )
		controls.AddControl( GAMEPAD_BUTTON_A, "Ok" )
		controls.AddControl( GAMEPAD_BUTTON_B, "Cancel" )
		controls.SetPosition( ( bg.TextureDimensions( ).x - controls.LocalRect.Width ) * 0.5, bg.TextureDimensions( ).y - 16, 0 )
		AddChild( controls )
		
		// Add to screen
		local vpRect = player.ComputeViewportSafeRect( )
		::GameApp.HudLayer( player.HudLayerName ).AddChild( this )
		SetPosition( vpRect.Center.x - 150, vpRect.Center.y - 60, 0.2 )
		SetAlpha( 0 )
		FadeInAnd( 0.2, function( canvas ):(player)
		{
			noInput = false
		}.bindenv( this ) )
		
		if( !player.User.IsLocal )
			Invisible = true
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( !noInput && gamepad )
		{
			local pad = gamepad.Get( )
			
			if( pad.ButtonDown( GAMEPAD_BUTTON_A ) )
			{
				PlaySound( "Play_UI_Select_Forward" )
				if( onOk )
					onOk( )
				Close( )
			}
			else if( pad.ButtonDown( GAMEPAD_BUTTON_B ) )
			{
				PlaySound( "Play_UI_Select_Backward" )
				if( onCancel )
					onCancel( )
				Close( )
			}
		}
	}
	
	function Close( )
	{
		if( gamepad )
		{
			gamepad.Release( )
			FadeOutAndDie( 0.3 )
		}
		gamepad = null
		noInput = true
	}
}