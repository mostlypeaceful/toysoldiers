sigimport "gui/scripts/weapons/expandingreticle.nut"
sigimport "Gui/Scripts/Controls/ControllerButton.nut"
sigimport "Gui/Scripts/weapons/weaponbase.nut"

sigexport function CanvasCreateWeaponUI( weaponUI )
{
	return GunWeaponUI( )
}
 
class GunWeaponUI extends WeaponBase
{
	// Display
	scopeImage = null // Gui.TexturedQuad

	constructor( owner_, reticle_ = null )
	{
		if( reticle_ )
			::WeaponBase.constructor( owner_, reticle_ )
		else
			::WeaponBase.constructor( owner_, ::ExpandingReticle( ) )
		
		scopeImage = null
	}

	function SetCenterPos( pos, safeRect )
	{
		if( scopeImage )
			scopeImage.SetPosition( pos )
			
		::WeaponBase.SetCenterPos( pos, safeRect )
	}

	function SetScopeBlend( blend )
	{
		//0.f no scope, 1.f fully blended into scope
		if( scopeImage )
		{
			scopeImage.SetAlpha( blend )
			reticle.SetAlpha( 1.0 - blend )
		}
	}
	
	function SetScope( path ) // For derived classes' constructors
	{
		local scopeSize = ::Math.Vec2.Construct( 1280, 720 )
		if( ::GameApp.GameMode.IsSplitScreen && !::GameApp.SingleScreenCoop )
			scopeSize = ::Math.Vec2.Construct( 1280 / 2, 720 )
		scopeImage = CreateScope( path, scopeSize )
		
		local ratio = scopeSize.x / scopeSize.y
		local textureSize = ::Math.Vec2.Construct( ratio, 1.0 )
		scopeImage.SetTextureRect( ::Math.Vec2.Construct( (ratio-1) / -2.0, 0 ), textureSize )
		
		scopeImage.SetPosition( 0, 0, 0.45 )
		
		AddChild( scopeImage )
		scopeImage.SetAlpha( 0 )
	}
	
	function CreateScope( path, size )
	{	
		local newOne = ::Gui.TexturedQuad( )
		newOne.SetTexture( path )
		newOne.CenterPivot( )
		
		local dims = newOne.TextureDimensions( )
		local scale = ::Math.Vec2.Construct( size.x / dims.x, size.y / dims.y ) 
		newOne.SetScale( scale )
		
		return newOne
	}	
	
	function UserControl( userControlled, player )
	{
		if( scopeImage )
			scopeImage.SetAlpha( 0.0 )
		::WeaponBase.UserControl( userControlled, player )
	}
}
