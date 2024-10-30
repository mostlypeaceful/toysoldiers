// This is for things with machine guns

// Sigvars
sigvars Reticle
@[Reticle_Size] { "Size", 16, [ 0.0:1000.0 ], "TODO COMMENT" }

sigvars Spread
@[Max_Spread_Dist] { "Max Distance", 8, [ 0.0:1000.0 ], "TODO COMMENT" }
@[Spread_Blend] { "Blend Lerp", 1.0, [ 0.0:1.0 ], "TODO COMMENT" }

// Requires
sigimport "gui/scripts/weapons/reticle.nut"

// Resources
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circularTR_g.png"
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circularTL_g.png"
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circularBL_g.png"
sigimport "Gui/Textures/Weapons/MachineGun/reticle_circularBR_g.png"

class ExpandingReticle extends Reticle
{
	// Setup parametsr
	paths = [ ]
	reticleSize = 0
	maxSpread = 0
	spreadBlend = 0
		
	// running parameters
	distance = 0
	targetDistance = 0
	expandStarts = [ ]
	expandVectors = [ ]
	quads = [ ]
	
	constructor( size = null, spread = null, blend = null, images = null )
	{
		::Reticle.constructor( )
		
		distance = 0
		targetDistance = 0
		expandStarts = [ ]
		expandVectors = [ ]
		quads = [ ]
		
		if( size )
			reticleSize = size
		else
			reticleSize = @[Reticle_Size]
			
		if( spread )
			maxSpread = spread
		else
			maxSpread = @[Max_Spread_Dist]
			
		if( blend )
			spreadBlend = blend
		else
			spreadBlend = @[Spread_Blend]
			
		if( images && images.len( ) >= 4 )
			paths = [ images[ 0 ], images[ 1 ], images[ 2 ], images[ 3 ] ]
		else
			paths = [ "Gui/Textures/Weapons/MachineGun/reticle_circularTR_g.png"
					, "Gui/Textures/Weapons/MachineGun/reticle_circularTL_g.png"
					, "Gui/Textures/Weapons/MachineGun/reticle_circularBL_g.png"
					, "Gui/Textures/Weapons/MachineGun/reticle_circularBR_g.png" ]
		
		local size = reticleSize
		local halfSize = size * 0.5
		
		expandVectors.push( Math.Vec2.Construct( 1, -1 ) )
		expandVectors.push( Math.Vec2.Construct( -1, -1 ) )
		expandVectors.push( Math.Vec2.Construct( -1, 1 ) )
		expandVectors.push( Math.Vec2.Construct( 1, 1 ) )
		
		expandStarts.push( expandVectors[ 0 ] * halfSize )
		expandStarts.push( expandVectors[ 1 ] * halfSize )
		expandStarts.push( expandVectors[ 2 ] * halfSize )
		expandStarts.push( expandVectors[ 3 ] * halfSize )
		
		local sizeVec = Math.Vec2.Construct( size, size )
		
		for( local p = 0; p < paths.len( ); p++ )
		{
			quads.push( CreateQuad( paths[ p ], sizeVec ) )
			AddChild( quads[ p ] )
		}
			
		OnTick( 0 )
	}

	function OnTick( dt )
	{	
		distance = Math.Lerp( distance, targetDistance, spreadBlend )
		
		for( local a=0; a<quads.len( ); a+=1 )
		{
			local pos = expandStarts[ a ] + expandVectors[ a ] * distance;
			quads[ a ].SetPosition( Math.Vec3.Construct( pos.x, pos.y, 0.0 ) )
		}
		
		Reticle.OnTick( dt )
	}
	
	function CreateQuad( path, size )
	{	
		local newOne = ::Gui.TexturedQuad( )
		newOne.SetTexture( path )
		newOne.CenterPivot( )
		
		local dims = newOne.TextureDimensions( )
		local scale = Math.Vec2.Construct( size.x / dims.x, size.y / dims.y ) 
		newOne.SetScale( scale )
		
		return newOne
	}
	
	function SetReticleSpread( spread ) // 0 to 1
	{
		targetDistance = 0.0
		if( spread > 0.5 )
		{
			spread *= 2.0
			spread -= 1.0
			
			targetDistance = maxSpread * (spread*spread)
		}
	}
}