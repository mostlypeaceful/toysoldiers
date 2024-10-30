// AT 3

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/hud/scoretimer.nut"
sigimport "gui/scripts/weapons/autoaimreticle.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png"
sigimport "gui/textures/weapons/overlays/flybywire_overlay_g.png"
sigimport "gui/textures/weapons/overlays/reticle01_missile_g.png"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return AT03WeaponScript( weaponUI )
}

class FlyByWireOverlay extends AnimatingCanvas
{
	// Display
	tof = null // Gui.Text
	lat = null // Gui.Text
	lon = null // Gui.Text
	alt = null // Gui.Text
	dto = null // Gui.Text
	extra = null // Gui.Text
	
	// Data
	entity = null
	timeOfFlight = null
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		
		entity = null
		timeOfFlight = 0
		
		local background = ::Gui.TexturedQuad( )
		background.SetTexture( "gui/textures/weapons/overlays/flybywire_overlay_g.png" )
		background.SetPosition( ::Math.Vec3.Construct( 0, 0, 0.01 ) )
		AddChild( background )
		
		tof = ::ScoreTimer_TimeLeftText( )
		tof.text.SetFontById( FONT_FIXED_SMALL )
		tof.SetPosition( ::Math.Vec3.Construct( 565, 134, 0 ) )
		tof.SetScale( ::Math.Vec2.Construct( 1.5, 1.5 ) )
		tof.SetAlpha( 0.5 )
		AddChild( tof )
		
		local bottomRow = 581
		
		lat = ::Gui.Text( )
		lat.SetFontById( FONT_FIXED_SMALL )
		lat.SetRgba( ::Math.Vec3.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		lat.SetPosition( ::Math.Vec3.Construct( 383, bottomRow, 0 ) )
		lat.SetAlpha( 0.5 )
		AddChild( lat )
		
		lon = ::Gui.Text( )
		lon.SetFontById( FONT_FIXED_SMALL )
		lon.SetRgba( ::Math.Vec3.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		lon.SetPosition( ::Math.Vec3.Construct( 537, bottomRow, 0 ) )
		lon.SetAlpha( 0.5 )
		AddChild( lon )
		
		alt = ::Gui.Text( )
		alt.SetFontById( FONT_FIXED_SMALL )
		alt.SetRgba( ::Math.Vec3.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		alt.SetPosition( ::Math.Vec3.Construct( 684, bottomRow, 0 ) )
		alt.SetAlpha( 0.5 )
		AddChild( alt )
		
		dto = ::Gui.Text( )
		dto.SetFontById( FONT_FIXED_SMALL )
		dto.SetRgba( ::Math.Vec3.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		dto.SetPosition( ::Math.Vec3.Construct( 836, bottomRow, 0 ) )
		dto.SetAlpha( 0.5 )
		AddChild( dto )
		
		extra = ::Gui.Text( )
		extra.SetFontById( FONT_FIXED_SMALL )
		extra.SetRgba( ::Math.Vec3.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		extra.SetPosition( ::Math.Vec3.Construct( 1152, 327, 0 ) )
		extra.SetAlpha( 0.5 )
		AddChild( extra )
		
		CenterPivot( )
	}
	
	function SetEntity( entity_ )
	{
		entity = entity_
		timeOfFlight = 0
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		// Update Time of Flight
		timeOfFlight += dt
		tof.SetTime( timeOfFlight )
		
		if( !is_null( entity ) )
		{
			// Update position stats
			local pos = entity.GetPosition( )
			local d = pos.Length( )
			
			lat.BakeCString( ": " + pos.x.tostring( ) )
			lon.BakeCString( ": " + pos.z.tostring( ) )
			alt.BakeCString( ": " + pos.y.tostring( ) )
			dto.BakeCString( ": " + d.tostring( ) )
		}
		
		// Set Extra Text
		local num1 = 16
		local num2 = 30
		local num3 = 10
		extra.BakeCString( num1.tostring( ) + "\n" + num2.tostring( ) + "\n" + num3.tostring( ) )
	}
}

class JitteryReticle extends AnimatingCanvas
{
	// Display
	image = null
	
	// Data
	distance = null
	time = null
	
	constructor( texturePath, jitteryDistance = 10, jitteryTime = 0.4 )
	{
		::AnimatingCanvas.constructor( )
		
		distance = jitteryDistance
		time = Math.Max( jitteryTime, 0.1 )
		
		image = ::AnimatingCanvas( )
		local ret = ::Gui.TexturedQuad( )
		ret.SetTexture( texturePath )
		ret.CenterPivot( )
		ret.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		image.AddChild( ret )
		AddChild( image )
		
		AddJitter( )
	}
	
	function AddJitter( )
	{
		local jitterReticle = this
		local newPos = ::Math.Vec3.Construct( SubjectiveRand.Float( -distance, distance ), SubjectiveRand.Float( -distance, distance ), 0 )
		image.AddAction( MotionTween( image.GetPosition( ), newPos, SubjectiveRand.Float( 0.1, time ), EasingTransition.Quadratic, EasingEquation.InOut, null, 
			function( canvas ) { AddJitter( ) }.bindenv(this) ) )
	}
	
	function TextureDimensions( ) { return image.TextureDimensions( ) }
}

class AT03_JitteryReticle extends JitteryReticle
{
	constructor( )
	{
		::JitteryReticle.constructor( "gui/textures/weapons/overlays/reticle01_missile_g.png" )
	}
}

class AT03WeaponScript extends GunWeaponUI
{
	constructor( weaponUI )
	{
		::GunWeaponUI.constructor( weaponUI, Reticle( "Gui/Textures/Weapons/MachineGun/reticle_circular01_g.png" ) )
		SetSouthpawSetting( SETTINGS_TURRETSSOUTHPAW )
		SetAltSouthpawSetting( SETTINGS_SHELLCAMSOUTHPAW )

		AddControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fire" )
		AddControl( GAMEPAD_BUTTON_RTRIGGER, "Fly_By_Wire" )
		
		AddAltControl( GAMEPAD_BUTTON_RTHUMB_MINMAG, "Aim" )
		AddAltControl( GAMEPAD_BUTTON_LTHUMB_MINMAG, "Speed" )
		AddAltControl( GAMEPAD_BUTTON_RTRIGGER, "Shell_Cam_Exit" )
		
		local alternateCamEffect = weaponUI.ScreenEffects( WEAPON_SCREEN_EFFECT_SHELLCAM )
		local alternateCam = alternateCamEffect.Data
		
		alternateCam.UnitTint = ::Math.Vec4.Construct( 10, 1.0, 1.0, 1.0 )
		
		local filmGrain = alternateCam.FilmGrainOverride
		filmGrain.TextureKey = "video_noise"
		filmGrain.Exposure = ::Math.Vec3.Construct( 1.2, 1.2, 1.2 )
		filmGrain.Saturation = ::Math.Vec3.Construct( 0.0, 0.0, 0.0 )
		filmGrain.GrainFreq = 0
		filmGrain.GrainScale = 0.65
		filmGrain.GrainSpeed = ::Math.Vec2.Construct( 3.5, 0.6 )
		filmGrain.HairsFreq = 0
		filmGrain.HairsScale = -0.85
		filmGrain.HairsSpeed = ::Math.Vec2.Construct( -4.0, 12.5 )
		filmGrain.LinesFreq = 0.0
		filmGrain.LinesScale = 0.4
		filmGrain.LinesSpeed = ::Math.Vec2.Construct( -1.1, -6.7 )
		filmGrain.SmudgeFreq = 1.0
		filmGrain.SmudgeScale = 0.55
		filmGrain.SmudgeSpeed = ::Math.Vec2.Construct( 20.0, 3.0 )

		AddShellCamCanvas( ::FlyByWireOverlay( ) )
		
		// Holy hacks, Batman!
		local ret = AutoAimReticle( )
		ret.size = ::Math.Vec2.Construct( 514, 376 )
		ret.autoAimBox.SetAlpha( 0 )
		ret.RemoveChild( ret.image )
		ret.image = ::AT03_JitteryReticle( )
		ret.image.CenterPivot( )
		ret.image.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		ret.AddChild( ret.image )
		SetShellCamReticle( ret )
	}

	function TargetingBoxSize( )
	{
		if( shellCamActive )
			return ::Math.Vec2.Construct( 1280, 720 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
	
	function TargetLockSize( )
	{
		if( shellCamActive )
			return ::Math.Vec2.Construct( 514, 376 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
	
	function AutoAimBoxSize( ) 
	{ 
		if( shellCamActive )
			return ::Math.Vec2.Construct( 514, 376 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
	
	function ReticleSize( )
	{
		if( shellCamActive )
			return ::Math.Vec2.Construct( 256, 256 )
		else
			return ::GunWeaponUI.TargetingBoxSize( )
	}
}
