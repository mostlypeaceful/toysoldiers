

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/asyncstatus.nut"
sigimport "gui/textures/weapons/ammo/reload_symbol_g.png"

sigexport function CanvasCreateNetUI( cppObj )
{
	return NetUIDisplay( )
}

// Display for Net UI
class NetUIDisplay extends AnimatingCanvas
{
	lagUITimer = null
	lagUIWantsHide = null
	lagUI = null
	
	netWaitUITimer = null
	netWaitUIWantsHide = null
	netWaitUI = null
	
	desyncUITimer = null
	desyncUIWantsHide = null
	desyncUI = null
	
	constructor( )
	{
		AnimatingCanvas.constructor( )
		
		lagUITimer = -1.0
		lagUIWantsHide = false
		lagUI = ::AsyncStatus( "Menus_Lagged", "gui/textures/weapons/ammo/reload_symbol_g.png" )
		
		netWaitUITimer = -1.0
		netWaitUIWantsHide = false
		netWaitUI = ::AsyncStatus( "Menus_NetworkLoadDelay", "gui/textures/weapons/ammo/reload_symbol_g.png" )
	
		desyncUITimer = -1.0
		desyncUIWantsHide = false
		desyncUI = ::AsyncStatus( "Menus_Desync", "gui/textures/weapons/ammo/reload_symbol_g.png" )
		
		::GameApp.StatusCanvas.AddChild( this )
	}
	
	function OnTick( dt )
	{
		AnimatingCanvas.OnTick( dt )
		
		// Lag UI
		if( lagUITimer >= 0.0 )
			lagUITimer += dt
		
		if( lagUIWantsHide && lagUITimer > 2.0 )
			ReallyHideLagUI( )
			
		// Net WaitUI
		if( netWaitUITimer >= 0.0 )
			netWaitUITimer += dt
		
		if( netWaitUIWantsHide && netWaitUITimer > 2.0 )
			ReallyHideNetWaitUI( )
			
		// Desync UI
		if( desyncUITimer >= 0.0 )
			desyncUITimer += dt
			
		if( desyncUIWantsHide && desyncUITimer > 2.0 )
			ReallyHideDesyncUI( )
	}
	
	function ShowLagUI( )
	{	
		lagUITimer = 0.0
		lagUIWantsHide = false
		lagUI.ClearActions( )
		lagUI.AddAction( 
			::AlphaTween( 
				lagUI.GetAlpha( ), 1.0, 0.5, 
				EasingTransition.Quadratic, 
				EasingEquation.In ) )
				
		AddChild( lagUI )
	}
	
	function HideLagUI( )
	{
		lagUIWantsHide = true
	}
	
	function ReallyHideLagUI( )
	{
		lagUITimer = -1
		lagUIWantsHide = false
		lagUI.ClearActions( )
		lagUI.AddAction( 
			::AlphaTween( 
				lagUI.GetAlpha( ), 0.0, 0.5, 
				EasingTransition.Quadratic, 
				EasingEquation.Out, null, 
				function( canvas ) { RemoveChild( lagUI ) }.bindenv( this ) ) )
	}
	
	function ShowNetWaitUI( )
	{	
		netWaitUITimer = 0.0
		netWaitUIWantsHide = false
		netWaitUI.ClearActions( )
		netWaitUI.AddAction( 
			::AlphaTween( 
				netWaitUI.GetAlpha( ), 1.0, 0.5, 
				EasingTransition.Quadratic, 
				EasingEquation.In ) )
				
		AddChild( netWaitUI )
	}
	
	function HideNetWaitUI( )
	{
		netWaitUIWantsHide = true
	}
	
	function ReallyHideNetWaitUI( )
	{
		netWaitUITimer = -1
		netWaitUIWantsHide = false
		netWaitUI.ClearActions( )
		netWaitUI.AddAction( 
			::AlphaTween( 
				netWaitUI.GetAlpha( ), 0.0, 0.5, 
				EasingTransition.Quadratic, 
				EasingEquation.Out, null, 
				function( canvas ) { RemoveChild( netWaitUI ) }.bindenv( this ) ) )
	}
	
	function ShowDesyncUI( )
	{	
		desyncUITimer = 0
		desyncUIWantsHide = false
		desyncUI.ClearActions( )
		desyncUI.AddAction( 
			::AlphaTween( 
				desyncUI.GetAlpha( ), 1.0, 0.5, 
				EasingTransition.Quadratic, 
				EasingEquation.In ) )
				
		AddChild( desyncUI )
	}
	
	function HideDesyncUI( )
	{
		desyncUIWantsHide = true
	}
	
	function ReallyHideDesyncUI( )
	{
		desyncUITimer = -1
		desyncUIWantsHide = false
		desyncUI.ClearActions( )
		desyncUI.AddAction( 
			::AlphaTween( 
				desyncUI.GetAlpha( ), 0.0, 0.5, 
				EasingTransition.Quadratic, 
				EasingEquation.Out, null, 
				function( canvas ) { RemoveChild( desyncUI ) }.bindenv( this ) ) )
	}
}