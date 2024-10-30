// Base class of all weapon huds

// SigVars
@[AmmoPos] { "Ammo Counts Position", (0,0,0), [ -2000:2000 ] }

// Requires
sigimport "gui/scripts/hud/ammocounter.nut"
sigimport "gui/scripts/controls/progressbar.nut"
sigimport "gui/scripts/hud/unitcontrolring.nut"
sigimport "gui/scripts/weapons/reticle.nut"
sigimport "gui/scripts/weapons/hitindicator.nut"
sigimport "gui/textures/cursor/barbackground_g.png"
sigimport "gui/textures/cursor/bar_g.png"

class WheelStats extends Gui.CanvasFrame
{
	circle = null
	line = null
	g = null
	suspension = null
	slip = null
	tempBar = null
	
	constructor( )
	{
		::Gui.CanvasFrame.constructor( )
		
		circle = Gui.LineList( )
		AddChild( circle )
		
		line = Gui.LineList( )
		AddChild(line )
		
		g = Gui.LineList( )
		AddChild( g )
		
		suspension = Gui.ColoredQuad( )
		AddChild(suspension )
		
		slip = Gui.ColoredQuad( )
		AddChild( slip )
		
		tempBar = Gui.ColoredQuad( )
		AddChild( tempBar )
	}	
	
	// every thing is g's
	function Set( max, kinetic, acc, normalForce, slipRatio, temp )
	{
		local circleScale = 16
		local barScale = 30
		
		circle.Circle( Math.Vec3.Construct( 0,0,0 ), max * circleScale, kinetic ? Math.Vec4.Construct(1,0,0,1) : Math.Vec4.Construct(0,1,0,1) )
		line.Line( Math.Vec3.Construct( 0,0,0 ), acc * circleScale, Math.Vec4.Construct(0,0,1,1) )
		
		local barSpace = 30
		local width = 20
		local startPt2 = Math.Vec3.Construct( 40, 20 )
		local startPt = Math.Vec3.Construct( startPt2.x, startPt2.y, 0 )
		suspension.SetRect( startPt2 + Math.Vec2.Construct( 0, -normalForce * barScale ), Math.Vec2.Construct( width, normalForce * barScale ) )	
		
		g.Line( startPt + Math.Vec3.Construct(0, -barScale, 0), startPt + Math.Vec3.Construct(width, -barScale, 0), Math.Vec4.Construct(1,1,0,1) )
		
		startPt2.x += barSpace
		slip.SetRect( startPt2 + Math.Vec2.Construct( 0, -slipRatio * barScale ), Math.Vec2.Construct( width, slipRatio * barScale ) )	
		
		startPt2.x += barSpace
		tempBar.SetRect( startPt2 + Math.Vec2.Construct( 0, -temp * barScale ), Math.Vec2.Construct( width, temp * barScale ) )	
		
	}
}

class WeaponBase extends Gui.CanvasFrame
{
	// Display
	ammoCounters = null // array of AmmoCounter objects
	controls = null // UnitControlRing
	altControls = null // UnitControlRing
	reticle = null // Reticle (or derived)
	shellCamOverlay = null // AnimatingCanvas
	shellCamReticle = null // Reticle (or derived)
	tempReticle = null
	
	spSetting = null
	spAltSetting = null
	isSouthpaw = null
	isAltSouthpaw = null
	spControls = null
	spAltControls = null
	profile = null
	
	// Data
	vpIndex = null
	userHudPos = null // vec3, for managing where the center of the user's viewport is
	shellCamActive = null // bool
	weaponUI = null // C++ object
	
	//debugging for vehicle stats
	vehicleStats = null
	wheelStats = null
	
	constructor( owner, reticle_ = null )
	{
		::Gui.CanvasFrame.constructor( )
		
		weaponUI = owner
		ammoCounters = [ ]		
		vehicleStats = null
		wheelStats = null
		shellCamActive = false
		userHudPos = ::Math.Vec3.Construct( 0.0, 0.0, 0.0 )
		isSouthpaw = false
		isAltSouthpaw = false
		
		controls = ::UnitControlRing( )
		controls.SetAlpha( 0 )
		AddChild( controls )
		
		spControls = ::UnitControlRing( )
		spControls.SetAlpha( 0 )
		AddChild( spControls )
		
		altControls = ::UnitControlRing( )
		altControls.SetAlpha( 0 )
		AddChild( altControls )
		
		spAltControls = ::UnitControlRing( )
		spAltControls.SetAlpha( 0 )
		AddChild( spAltControls )
		
		if( reticle_ )
			reticle = reticle_
		else
			reticle = Reticle( )
		AddChild( reticle )
	}
	
	function SetSouthpawSetting( setting )
	{
		spSetting = setting
	}
	
	function SetAltSouthpawSetting( setting )
	{
		spAltSetting = setting
	}
	
	function SetViewportIndex( index )
	{
		vpIndex = index
		
		if( shellCamOverlay )
		{
			local vpRect = ::GameApp.ComputeViewportRect( vpIndex )
			shellCamOverlay.SetScissorRect( vpRect )
			shellCamOverlay.SetPosition( vpRect.Center.x, vpRect.Center.y, 0.4 )
			
			if( "SetViewportIndex" in shellCamOverlay )
				shellCamOverlay.SetViewportIndex( index )
		}
	}
	
	function SetAmmoValues( index, percent, count, reloading, forceRefresh ) // index: number, percent: number [0:1], count: number, reloading: bool
	{
		if( index < 0 || index >= ammoCounters.len( ) )
			return;
		
		ammoCounters[ index ].SetAmmoValues( percent, count, reloading, forceRefresh )
	}
	
	function SetVehicleStats( speed, rpm, load )
	{
		if( vehicleStats == null )
		{	
			vehicleStats = [ ]
			local pos = ::Math.Vec3.Construct( 50, 50, 0.1 )
			for( local i = 0; i < 3; ++i )
			{
				local bar = ::ProgressBar( "gui/textures/cursor/barbackground_g.png", "gui/textures/cursor/bar_g.png" )
				bar.SetPosition( pos.x, pos.y + i * 30, pos.z )
				bar.SetMeterHorizontal( 1.0 )
				vehicleStats.push( bar )
				AddChild( bar )
			}
		}
		
		vehicleStats[ 0 ].SetMeterHorizontal( speed )
		vehicleStats[ 1 ].SetMeterHorizontal( rpm )
		vehicleStats[ 2 ].SetMeterHorizontal( load )
	}
	
	function SetWheelStats( index, max, kinetic, acc, normalForce, slipRatio, temp )
	{
		if( wheelStats == null )
		{	
			wheelStats = [ ]
			local pos = Math.Vec3.Construct( 250, 100, 0.1 )
			wheelStats.push( WheelStats( ) )
			wheelStats[ 0 ].SetPosition( pos )
			AddChild( wheelStats[ 0 ] )
			
			wheelStats.push( WheelStats( ) )
			wheelStats[ 1 ].SetPosition( pos + Math.Vec3.Construct( 0, 100, 0 ) )
			AddChild( wheelStats[ 1 ] )
		}
		
		wheelStats[ index ].Set( max, kinetic, acc, normalForce, slipRatio, temp )
	}
	
	function AddAmmoCounter( ammoIcon, tickMarkIcon, maxAmmo )
	{
		// Create and add an ammo counter
		local a = AmmoCounter( ammoIcon, tickMarkIcon, maxAmmo )
		ammoCounters.push( a )
		
		// Display the ammo counter
		AddChild( a )
	}
	
	function SetAmmoIcon( index, iconPath )
	{
		if( index < 0 || index >= ammoCounters.len( ) )
			return;
			
		if( iconPath ) ammoCounters[ index ].SetAmmoIcon( iconPath )
	}
	
	function SouthpawTexture( buttonTexture )
	{
		if( typeof buttonTexture == "string" )
			buttonTexture = buttonTexture.tolower( )
			
		if( buttonTexture == GAMEPAD_BUTTON_LTHUMB_MINMAG || buttonTexture == "gui/textures/gamepad/button_lstick_g.png" )
			return GAMEPAD_BUTTON_RTHUMB_MINMAG
		else if( buttonTexture == GAMEPAD_BUTTON_RTHUMB_MINMAG || buttonTexture == "gui/textures/gamepad/button_rstick_g.png")
			return GAMEPAD_BUTTON_LTHUMB_MINMAG
		else if( buttonTexture == GAMEPAD_BUTTON_RTHUMB )
			return GAMEPAD_BUTTON_LTHUMB
		else if( buttonTexture == GAMEPAD_BUTTON_LTHUMB )
			return GAMEPAD_BUTTON_RTHUMB
		else
			return buttonTexture
	}
	
	function AddControl( buttonTexture, locID, combo = false )
	{
		controls.AddControl( buttonTexture, locID, combo )
		spControls.AddControl( SouthpawTexture( buttonTexture ), locID, combo )
		RepositionUserHUD( )
	}
	
	function AddAltControl( buttonTexture, locID, combo = false )
	{
		altControls.AddControl( buttonTexture, locID, combo )
		spAltControls.AddControl( SouthpawTexture( buttonTexture ), locID, combo )
		RepositionUserHUD( )
	}
	
	function ShowControls( alt = false )
	{
		//::print( "Show" + ( alt ? " alt " : " " ) + "controls" )
		if( ::GameApp.CurrentLevel && ::GameApp.CurrentLevel.TutAlwaysShowUnitControls )
		{
			HideControls( !alt )
			ResetVanishingControls( alt )
		}
		else
			ResetVanishingControls( alt )
	}

	function GetAltControls( )
	{
		if( spAltSetting != null && isAltSouthpaw )
			return spAltControls
		else
			return altControls
	}
	
	function GetRegularControls( )
	{
		if( spSetting != null && isSouthpaw )
			return spControls
		else
			return controls
	}
	
	function GetControls( alt = false )
	{
		if( alt )
			return GetAltControls( )
		else
			return GetRegularControls( )
	}
	
	function HideControls( alt = false )
	{
		//::print( "Hide" + ( alt ? " alt " : " " ) + "controls" )
		if( !::GameApp.GameMode.IsSplitScreen )
		{
			if( ::GameApp.CurrentLevel && ::GameApp.CurrentLevel.TutAlwaysShowUnitControls )
			{
				//GetControls( !alt ).SetAlpha( 0 )
				GetControls( alt ).SetAlpha( 0 )
			}
			else
			{
				GetControls( alt ).SetAlpha( 0 )
			}
		}
	}
	
	function FadeOutControls( alt = false )
	{
		//::print( "FadeOut" + ( alt ? " alt " : " " ) + "controls" )
		if( !::GameApp.GameMode.IsSplitScreen )
		{
			if( ::GameApp.CurrentLevel && ::GameApp.CurrentLevel.TutAlwaysShowUnitControls )
			{
				GetControls( !alt ).SetAlpha( 1 )
				GetControls( alt ).SetAlpha( 0 )
			}
			else
			{
				GetControls( alt ).vanishTimer = 0
			}
		}
	}
	
	function ResetVanishingControls( alt = false )
	{
		if( !::GameApp.GameMode.IsSplitScreen )
			GetControls( alt ).ResetVanishingControls( )
	}
	
	function UserControl( userControlled, player )
	{
		local alpha = 0

		if( userControlled )
			alpha = GetAlpha( )
		
		if( userControlled )
			HideControls( )
		
		if( userControlled )
		{
			profile = player.GetUserProfile( )
			if( userControlled && spSetting != null )
				isSouthpaw = ( ( profile.GetSetting( spSetting ) == 1 )? true: false )
			if( userControlled && spAltSetting != null )
				isAltSouthpaw = ( ( profile.GetSetting( spAltSetting ) == 1 )? true: false )
		}
		else
			profile = null
			
		foreach( a in ammoCounters )
			a.SetAlphaClamp( alpha )

		//if( !::GameApp.GameMode.IsSplitScreen )
		//	GetControls( ).SetAlphaClamp( alpha )
		
		if( userControlled )
			ResetVanishingControls( )
	}
	
	function OnTick( dt )
	{
		if( ::GameApp.Paused( ) )
			return
			
		::Gui.CanvasFrame.OnTick( dt )
		
		if( profile )
		{
			if( spSetting != null )
			{
				local newSetting = ( ( profile.GetSetting( spSetting ) == 1 )? true: false )
				if( newSetting != isSouthpaw )
				{
					HideControls( )
					isSouthpaw = newSetting
					ResetVanishingControls( )
				}
			}
			if( spAltSetting != null )
			{
				local newSetting = ( ( profile.GetSetting( spAltSetting ) == 1 )? true: false )
				if( newSetting != isAltSouthpaw )
				{
					HideControls( true )
					isAltSouthpaw = newSetting
					ResetVanishingControls( true )
				}
			}
		}
	}
	
	function ShowHideReticle( show )
	{
		reticle.ShowHide( show )
	}
	
	function SetCenterPos( pos, safeRect )
	{
		// Set the center
		userHudPos = pos
		
		// Set the reticle object's position (not call SetReticlePos)
		reticle.SetPosition( userHudPos )
		
		// Set the edge based on splitscreen or not
		local counterSpacing = 32
		local vpRect = safeRect
		local offset
		local alignment = AmmoCounterAlign.Right
		if( ::GameApp.GameMode.IsSplitScreen && !::GameApp.SingleScreenCoop && (::GameApp.GameMode.IsCoOp || ::GameApp.GameMode.IsVersus) && vpIndex == 0 )
		{
			offset = Math.Vec3.Construct( vpRect.Left, vpRect.Bottom - counterSpacing / 2, 0 )
			alignment = AmmoCounterAlign.Left
		}
		else
			offset = Math.Vec3.Construct( vpRect.Right, vpRect.Bottom - counterSpacing / 2, 0 )
		
		// Set the positions of the ammo counters
		foreach( i, a in ammoCounters )
		{
			local newPos = offset.Clone( )
			newPos.y -=( counterSpacing * i )
			a.SetPosition( newPos )
			a.SetAlignment( alignment )
		}
		
		// Set the positions of the controls
		RepositionUserHUD( ) 
	}
	
	function SetReticlePos( pos )
	{
		if( reticle )
			reticle.SetReticlePos( pos )
	}
	function SetReticleSpread( spread )
	{
		if( reticle )
			reticle.SetReticleSpread( spread )
	}
	function SetReticleOverTarget( over )
	{
		if( reticle )
			reticle.SetReticleOverTarget( over )
	}
	
	function RepositionUserHUD( )
	{
		local pos = userHudPos.Clone( )
		pos.y += 100
		pos.z = -0.08
		controls.SetPosition( pos )
		spControls.SetPosition( pos )
		altControls.SetPosition( pos )
		spAltControls.SetPosition( pos )
	}
	
	function AddShellCamCanvas( canvas )
	{
		shellCamOverlay = canvas
		shellCamOverlay.SetAlpha( 0 )
		AddChild( shellCamOverlay )
	}
	
	function AddShellCamOverlay( texture )
	{
		local canvas = ::AnimatingCanvas( )
		local overlayTexture = ::Gui.TexturedQuad( )
		overlayTexture.SetTexture( texture )
		canvas.AddChild( overlayTexture )
		AddShellCamCanvas( canvas )
	}
	
	function ShowShellCamOverlay( show, entity, player )
	{
		shellCamActive = show
		
		if( shellCamOverlay )
		{
			if( "SetEntity" in shellCamOverlay )
			{
				if( is_null( entity ) )
					shellCamOverlay.SetEntity( null )
				else
					shellCamOverlay.SetEntity( entity )
			}
				
			
			local vpRect = ::GameApp.ComputeViewportRect( vpIndex )
			shellCamOverlay.SetScissorRect( vpRect )
			shellCamOverlay.SetPosition( vpRect.Center.x, vpRect.Center.y, 0.45 )
			shellCamOverlay.ClearActions( )
			shellCamOverlay.AddAction( AlphaTween( shellCamOverlay.GetAlpha( ), show? 1.0: 0.0, 0.5 ) )
			
			::GameApp.HudLayer( "viewport" + vpIndex.tostring( ) ).Invisible = show
			::GameApp.HudLayer( "hover" + vpIndex.tostring( ) ).Invisible = show
			::GameApp.HudLayer( "alwaysHide" ).Invisible = show
			player.SetFullScreenOverlayActive( show )
		}
		
		if( shellCamReticle )
		{
			if( show )
			{
				tempReticle = reticle
				RemoveChild( reticle )
				reticle = shellCamReticle
				AddChild( reticle )
			}
			else
			{
				RemoveChild( reticle )
				reticle = tempReticle
				AddChild( reticle )
			}
		}
	}
	
	function SetShellCamReticle( ret )
	{
		shellCamReticle = ret
	}
	
	function GetHit( )
	{
		local hitIndicator = ::HitIndicator( weaponUI )
		hitIndicator.SetPosition( userHudPos )
		AddChild( hitIndicator )
	}
	
	// For Advanced Targeting
	////////////////////////////////////////////////////////////////////////////
	function TargetingBoxSize( ) { return ::Math.Vec2.Construct( 0, 0 ) }
	function AutoAimBoxSize( ) { return ( reticle )? reticle.AutoAimBoxSize( ): ::Math.Vec2.Construct( 0, 0 ) }
	function ReticleSize( ) { return ( reticle )? reticle.ReticleSize( ): ::Math.Vec2.Construct( 0, 0 ) }
	function TargetLockSize( ) { return ::Math.Vec2.Construct( 0, 0 ) }
	
	// For GunWeapon
	////////////////////////////////////////////////////////////////////////////
	function SetScopeBlend( blend ) {} // Virtual
	
	// For TargetingWeapon
	////////////////////////////////////////////////////////////////////////////
	function AddTarget( uID, pos ) {} // Virtual
	function RemoveTarget( uID ) {} // Virtual
	function SetTargetPosition( uID, pos ) {} // Virtual
	function AddLock( uID, pos ) {} // Virtual
	function RemoveLock( uID ) {} // Virtual
	function SetLockPosition( uID, pos ) {} // Virtual
	function ClearTargets( ) {} // Virtual
}
