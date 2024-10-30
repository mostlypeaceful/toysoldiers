// Radial Progress Menu

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

class RadialProgressMeter extends AnimatingCanvas
{
	// Display
	images = null // array of Gui.MutableTexturedQuad objects
	
	// Data
	radialDirection = null // DIRECTION_COUNTERCLOCKWISE or DIRECTION_CLOCKWISE
	size = null // Rect, size of each image
	uvs = null // array of uv coordinates
	
	constructor( texture, direction = DIRECTION_COUNTERCLOCKWISE )
	{
		::AnimatingCanvas.constructor( )
		
		images = [ ]
		radialDirection = direction
		
		// Create the images
		for( local i = 0; i < 4; ++i )
		{
			local image = ::Gui.MutableTexturedQuad( )
			image.SetTexture( texture )
			
			images.push( image )
			AddChild( image )
		}
		
		size = ::Math.Rect.Construct( ::Math.Vec2.Construct( 0, 0 ), images[ 0 ].TextureDimensions( ) * 0.5 )
		
		// Position them in a square
		images[ 0 ].SetPosition( ::Math.Vec3.Construct( size.Width, size.Height, 0 ) )
		images[ 1 ].SetPosition( ::Math.Vec3.Construct( 0, size.Height, 0 ) )
		images[ 2 ].SetPosition( ::Math.Vec3.Construct( 0, 0, 0 ) )
		images[ 3 ].SetPosition( ::Math.Vec3.Construct( size.Width, 0, 0 ) )
		
		// Set up the proper UV coords
		uvs = [ ]
		uvs.push( ::Math.Rect.Construct( ::Math.Vec2.Construct( 0.5, 0.5 ), ::Math.Vec2.Construct( 0.5, 0.5 ) ) )
		uvs.push( ::Math.Rect.Construct( ::Math.Vec2.Construct( 0, 0.5 ), ::Math.Vec2.Construct( 0.5, 0.5 ) ) )
		uvs.push( ::Math.Rect.Construct( ::Math.Vec2.Construct( 0, 0 ), ::Math.Vec2.Construct( 0.5, 0.5 ) ) )
		uvs.push( ::Math.Rect.Construct( ::Math.Vec2.Construct( 0.5, 0 ), ::Math.Vec2.Construct( 0.5, 0.5 ) ) )
		
		// Set them all the same size
		foreach( i, image in images )
		{
			image.SetRect( ::Math.Vec2.Construct( 0, 0 ), size.WidthHeight )
			image.SetTextureRect( uvs[ i ].TopLeft, uvs[ i ].WidthHeight )
		}
	}
	
	function SetPercent( percent )
	{
		local angle = MATH_2_PI * percent
		
		images[ 0 ].SetAngleLayout( 0, angle, uvs[ 0 ], size, radialDirection )
		images[ 1 ].SetAngleLayout( 1, angle, uvs[ 1 ], size, radialDirection )
		images[ 2 ].SetAngleLayout( 3, angle, uvs[ 2 ], size, radialDirection )
		images[ 3 ].SetAngleLayout( 2, angle, uvs[ 3 ], size, radialDirection )
	}
	
	function SetDirection( dir )
	{
		radialDirection = dir
	}
}