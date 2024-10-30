// Base hud for AC130 overlays

// Requires
sigimport "Gui/Scripts/weapons/gunweapon.nut"
sigimport "gui/scripts/weapons/barrage/ac130overlay.nut"

// Resources

class AC130_Base_HUD extends GunWeaponUI
{
	// Display
	overlay = null
	
	constructor( weaponUI, type, exposure )
	{
		// Jittery Reticle
		local ret = ::AutoAimReticle( )
		ret.size = ::Math.Vec2.Construct( 514, 376 )
		ret.autoAimBox.SetAlpha( 0 )
		ret.RemoveChild( ret.image )
		ret.image = ::JitteryReticle( "gui/textures/weapons/overlays/ac130_reticle_g.png" )
		ret.image.CenterPivot( )
		ret.image.SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		ret.AddChild( ret.image )
		
		::GunWeaponUI.constructor( weaponUI, ret )
		
		local alternateCamEffect = weaponUI.ScreenEffects( WEAPON_SCREEN_EFFECT_NORMAL )
		local alternateCam = alternateCamEffect.Data
		alternateCam.UnitTint = Math.Vec4.Construct( 10, 1.0, 1.0, 1.0 )
		
		local filmGrain = alternateCam.FilmGrainOverride
		filmGrain.TextureKey = "video_noise"
		filmGrain.Exposure = exposure
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
		
		// Overlay
		overlay = ::AC130Overlay( type )
		AddChild( overlay )
	}
	
	function SetViewportIndex( index )
	{
		overlay.SetViewportIndex( index )
		::GunWeaponUI.SetViewportIndex( index )
	}
	
	function UserControl( control, player )
	{
		::GunWeaponUI.UserControl( control, player )
		::GameApp.HudLayer( "viewport" + vpIndex.tostring( ) ).Invisible = control
		::GameApp.HudLayer( "hover" + vpIndex.tostring( ) ).Invisible = false // Don't hide these things
		::GameApp.HudLayer( "alwaysHide" ).Invisible = control
		player.SetFullScreenOverlayActive( control )
	}
	
	function AutoAimBoxSize( ) 
	{ 
		return ::Math.Vec2.Construct( 514, 376 )
	}
	
	function ReticleSize( )
	{
		return ::Math.Vec2.Construct( 32, 32 )
	}
}