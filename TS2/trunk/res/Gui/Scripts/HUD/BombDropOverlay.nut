// Bomb Drop Overlay

// Resources

sigimport "gui/textures/weapons/overlays/bombdrop_overlay_g.png"
sigimport "gui/textures/weapons/overlays/bombdrop_reticle_g.png"
sigimport "gui/textures/weapons/overlays/b52_overlay_staticreticule_g.png"
sigimport "gui/textures/weapons/overlays/b52_overlay_dialright_g.png"
sigimport "gui/textures/weapons/overlays/b52_overlay_dialleft_g.png"
sigimport "gui/textures/weapons/overlays/bombdrop_reticle_nuke_g.png"
sigimport "gui/textures/weapons/overlays/bombdrop_adjustment_nuke_g.png" 
sigimport "gui/textures/weapons/overlays/bombdrop_radial_nuke_g.png"
// laser imported in dlc_imports.nut

sigexport function CanvasCreateBombDropOverlay( cppObj )
{
	local player = ::GameApp.GetPlayer( ::GameApp.WhichPlayer( cppObj.User ) )
	local barrage = player.CurrentBarrage( )
	if( barrage && barrage.Name == "BARRAGE_NUKE" )
		return ::NukeBombDropOverlay( cppObj )
	else if( barrage && barrage.Name == "BARRAGE_LAZER" )
		return ::LaserTargetingOverlay( cppObj )
	else
		return ::B52BombDropOverlay( cppObj )
}

class BombDropOverlay extends AnimatingCanvas
{
	// Display
	reticle = null
	buttonPrompt = null
	timeDisplay = null
	
	// Data
	user = null
	timeCountdown = null
	rotating = null
	tickCounter = null
	
	constructor( cppObj )
	{
		::AnimatingCanvas.constructor( )
		
		user = cppObj.User
		audioSource = ::GameApp.GetPlayerByUser( user ).AudioSource
		local vpRect = user.ComputeViewportRect( )
		timeCountdown = 20.0
		rotating = true
		tickCounter = 0
		
		SetPosition( vpRect.Center.x, vpRect.Center.y, 0.45 )
		if( user.IsViewportVirtual )
			Invisible = true

		SetScissorRect( vpRect )

		reticle = ::Gui.TexturedQuad( )
		reticle.SetTexture( "gui/textures/weapons/overlays/bombdrop_reticle_g.png" )
		reticle.CenterPivot( )
		reticle.SetPosition( 0, 0, 0.02 )
		AddChild( reticle )
		
		// Prompt
		buttonPrompt = ::ControllerButton( GAMEPAD_BUTTON_A, "Drop_Bomb" )
		buttonPrompt.CenterPivot( )
		buttonPrompt.SetPosition( 0, 230, 0 )
		AddChild( buttonPrompt )

		timeDisplay = ::Gui.Text( )
		timeDisplay.SetFontById( FONT_FANCY_MED )
		timeDisplay.SetRgba( COLOR_CLEAN_WHITE )
		timeDisplay.SetPosition( 0, -267, 0.01 )
		AddChild( timeDisplay )

		// Setup PostEffect
		local postEffectData = cppObj.PostEffectData
		postEffectData.UnitTint = ::Math.Vec4.Construct(10, 1.0, 1.0, 1.0 )
		
		local filmGrain = postEffectData.FilmGrainOverride
		filmGrain.TextureKey = "video_noise"
		filmGrain.Exposure = ::Math.Vec3.Construct( 0.40, 0.90, 0.50 )
		filmGrain.Saturation = ::Math.Vec3.Construct( 0.0, 0.0, 0.0 )
		filmGrain.GrainFreq = 0
		filmGrain.GrainScale = 0.45
		filmGrain.GrainSpeed = ::Math.Vec2.Construct( 3.5, 0.6 )
		filmGrain.HairsFreq = 0
		filmGrain.HairsScale = -0.65
		filmGrain.HairsSpeed = ::Math.Vec2.Construct( -4.0, 12.5 )
		filmGrain.LinesFreq = 0.0
		filmGrain.LinesScale = 0.2
		filmGrain.LinesSpeed = ::Math.Vec2.Construct( -1.1, -6.7 )
		filmGrain.SmudgeFreq = 1.0
		filmGrain.SmudgeScale = 0.35
		filmGrain.SmudgeSpeed = ::Math.Vec2.Construct( 20.0, 3.0 )
		
		::GameApp.HudRoot.AddChild( this )
		SetAlpha( 0.0 )
	}
	
	function Show( show, player )
	{
		if( show )
		{
			SetAlpha( 0 )
			FadeIn( 0.2 )
		}
		else
			FadeOut( 0.2 )
			
		::GameApp.HudLayer( "viewport" + user.ViewportIndex.tostring( ) ).Invisible = show
		::GameApp.HudLayer( "hover" + user.ViewportIndex.tostring( ) ).Invisible = show
		::GameApp.HudLayer( "alwaysHide" ).Invisible = show
		player.SetFullScreenOverlayActive( show )
		
		if( ::GameApp.GameMode.IsSplitScreen )
			SetScissorRect( player.ComputeViewportRect( ) )
	}
	
	function SetAngle( angle )
	{
		if( rotating )
			reticle.SetAngle( angle )
	}
	
	function OnTick( dt )
	{
		if( ::GameApp.Paused( ) )
			return
			
		::AnimatingCanvas.OnTick( dt )
		timeCountdown -= dt
		
		if( GetAlpha( ) > 0 )
		{
			tickCounter += dt
			if( tickCounter > 1.0 )
			{
				PlaySound( "Play_HUD_Barrage_Clock" )
				tickCounter = 0
			}
		}
		
		if( timeCountdown < 0 )
			timeCountdown = 0
			
		timeDisplay.BakeLocString( ::LocString.ConstructTimeString( timeCountdown, true ), TEXT_ALIGN_CENTER )
	}
}

class B52BombDropOverlay extends BombDropOverlay
{
	// Display
	leftDial = null
	
	constructor( cppObj )
	{
		::BombDropOverlay.constructor( cppObj )
		local brightGreen = ::Math.Vec4.Construct( 0.643, 0.792, 0.035, 1.0 )
		
		timeDisplay.SetRgba( brightGreen )
		timeDisplay.SetPosition( 0, -267, 0.01 )
		
		local bg = ::Gui.TexturedQuad( )
		bg.SetTexture( "gui/textures/weapons/overlays/bombdrop_overlay_g.png" )
		bg.CenterPivot( )
		bg.SetPosition( 0, 0, 0 )
		AddChild( bg )
		
		// Special pieces
		local staticText = ::Gui.Text( )
		staticText.SetFontById( FONT_FANCY_MED )
		staticText.SetRgba( brightGreen )
		staticText.SetPosition( -370, -21, 0.01 )
		staticText.SetUniformScale( 0.8 )
		staticText.BakeCString( "51 : MK82" )
		AddChild( staticText )
		
		local staticReticle = ::Gui.TexturedQuad( )
		staticReticle.SetTexture( "gui/textures/weapons/overlays/b52_overlay_staticreticule_g.png" )
		staticReticle.CenterPivot( )
		staticReticle.SetPosition( 0, 0, 0.01 )
		AddChild( staticReticle )
		
		if( !::GameApp.GameMode.IsSplitScreen )
		{
			local rightDial = ::AnimatingCanvas( )
				local rightDialImage = ::Gui.TexturedQuad( )
				rightDialImage.SetTexture( "gui/textures/weapons/overlays/b52_overlay_dialright_g.png" )
				rightDialImage.SetPosition( 0, 0, 0 )
				rightDial.AddChild( rightDialImage )
			rightDial.SetPosition( 279, -253, 0.01 )
			rightDial.SetScissorRect( ::Math.Rect.Construct( ::Math.Vec2.Construct( 919, 106 ), ::Math.Vec2.Construct( 60, 503 ) ) )
			AddChild( rightDial )
			
			rightDial.AddAction( ::YMotionTween( -253, -253 - (rightDialImage.TextureDimensions( ).y - 503), 30.0, EasingTransition.Quadratic, EasingEquation.InOut ) )
			
			leftDial = ::AnimatingCanvas( )
				local leftDialImage = ::Gui.TexturedQuad( )
				leftDialImage.SetTexture( "gui/textures/weapons/overlays/b52_overlay_dialleft_g.png" )
				leftDial.AddChild( leftDialImage )
			leftDial.SetPosition( -388, 13, 0.01 )
			leftDial.SetScissorRect( ::Math.Rect.Construct( ::Math.Vec2.Construct( 268, 372 ), ::Math.Vec2.Construct( 128, 249 ) ) )
			AddChild( leftDial )
			
			SlideToRandom( )
		}
	}
	
	function SlideToRandom( )
	{
		if( leftDial )
		{
			local randomY = ::SubjectiveRand.Int( -(1024 - 249), 0 )
			local randomTime = ::SubjectiveRand.Float( 1.0, 10.0 )
			leftDial.AddAction( ::YMotionTween( leftDial.GetYPos( ), randomY, randomTime, EasingTransition.Quadratic, EasingEquation.InOut, null, function( canvas ) { SlideToRandom( ) }.bindenv(this) ) )
		}
	}
}

class NukeBombDropOverlay extends BombDropOverlay
{
	// Display
	rotateReticle = null
	
	constructor( cppObj )
	{
		::BombDropOverlay.constructor( cppObj )
		reticle.SetTexture( "gui/textures/weapons/overlays/bombdrop_adjustment_nuke_g.png" )
		reticle.CenterPivot( )
		reticle.SetPosition( 0, 0, 0.00 )
		rotating = false
		
		// TODO: Add "TIME: " text
		//timeDisplay.SetPosition( 0, -267, 0.01 )
		
		// TODO: Add "WIND: XYZ" text
		
		local staticReticle = ::Gui.TexturedQuad( )
		staticReticle.SetTexture( "gui/textures/weapons/overlays/bombdrop_reticle_nuke_g.png" )
		staticReticle.CenterPivot( )
		staticReticle.SetPosition( 0, 0, 0.02 )
		AddChild( staticReticle )
		
		rotateReticle = ::Gui.TexturedQuad( )
		rotateReticle.SetTexture( "gui/textures/weapons/overlays/bombdrop_radial_nuke_g.png" )
		rotateReticle.CenterPivot( )
		rotateReticle.SetPosition( 0, 0, 0.01 )
		AddChild( rotateReticle )
		
		local filmGrain = cppObj.PostEffectData.FilmGrainOverride
		filmGrain.Exposure = ::Math.Vec3.Construct( 0.50, 0.48, 0.00 )
		filmGrain.Saturation = ::Math.Vec3.Construct( 0.0, 0.0, 0.0 )
	}
	
	function OnTick( dt )
	{
		::BombDropOverlay.OnTick( dt )
		
		local rotateSpeed = MATH_PI_OVER_2
		rotateReticle.SetAngle( rotateReticle.GetAngle( ) + rotateSpeed * dt )
	}
}

class LaserTargetingOverlay extends BombDropOverlay
{
	constructor( cppObj )
	{
		::BombDropOverlay.constructor( cppObj )
		reticle.SetTexture( "gui/textures/weapons/overlays/laser_g.png" )
		reticle.CenterPivot( )
		reticle.SetPosition( 0, 0, 0.00 )
		rotating = false

		RemoveChild( buttonPrompt )
		buttonPrompt = ::ControllerButton( GAMEPAD_BUTTON_A, "Fire_Laser" )
		buttonPrompt.CenterPivot( )
		buttonPrompt.SetPosition( 0, 230, 0 )
		AddChild( buttonPrompt )
		
		local bg = ::Gui.TexturedQuad( )
		bg.SetTexture( "gui/textures/weapons/overlays/bombdrop_overlay_g.png" )
		bg.CenterPivot( )
		bg.SetPosition( 0, 0, 0.001 )
		AddChild( bg )
		
		// No green tint, full saturation
		local filmGrain = cppObj.PostEffectData.FilmGrainOverride
		filmGrain.Exposure = ::Math.Vec3.Construct( 1.0, 1.0, 1.0 )
		filmGrain.Saturation = ::Math.Vec3.Construct( 1.0, 1.0, 1.0 )
		
		// No "white-hot"
		local postEffectData = cppObj.PostEffectData
		postEffectData.UnitTint = ::Math.Vec4.Construct( 0.0, 0.0, 0.0, 0.0 )
	}
}
