
// Resources
sigimport "gui/textures/misc/async_small_g.png"

enum AsyncStatusImageLocation
{
	Left, Right
}

class AsyncStatusSmall extends AnimatingCanvas
{
	// Statics
	rotateVelocity = MATH_2_PI
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		
		local image = ::Gui.TexturedQuad( )
		image.SetTexture( "gui/textures/misc/async_small_g.png" )
		image.SetPosition( -8, -8, 0 )
		AddChild( image )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		Rotate( rotateVelocity * dt )
	}
}

class AsyncStatus extends AnimatingCanvas
{
	loadingTimer = 0
	loadingText = null
	loadingImage = null
	imageLoc = null

	constructor( loadingTextId = "Loading", imagePath = "Gui/Textures/Misc/loading_g.png", pos = ::Math.Vec3.Construct( 90, 620, 0 ), imageLoc_ = AsyncStatusImageLocation.Right, fontId = FONT_FANCY_MED )
	{
		::AnimatingCanvas.constructor( )

		loadingTimer = 0
		imageLoc = imageLoc_

		if( loadingTextId )
		{
			loadingText = Gui.Text( )
			loadingText.SetFontById( fontId )
			loadingText.SetRgba( COLOR_CLEAN_WHITE )
			loadingText.BakeLocString( GameApp.LocString( loadingTextId ), TEXT_ALIGN_LEFT )
			AddChild( loadingText )
		}

		loadingImage = Gui.TexturedQuad( )
		loadingImage.SetTexture( imagePath )
		loadingImage.CenterPivot( )
		AddChild( loadingImage )

		Reposition( )

		SetPosition( pos )
	}
	
	function OnTick( dt )
	{
		if( loadingText )
		{
			local t = ::Math.Abs( ::Math.Sin( loadingTimer ) )
			loadingText.SetAlpha( ::Math.Lerp( 1, 0.125, t ) )
			loadingTimer += 2 * dt
		}

		loadingImage.Rotate( 4 * dt )

		::AnimatingCanvas.OnTick( dt )
	}
	
	function ChangeText( newTextId, width = null )
	{
		if( loadingText )
		{
			local str = newTextId
			if( typeof newTextId == "string" )
				str = ::GameApp.LocString( newTextId )
			
			if( width != null )
				loadingText.BakeBoxLocString( width, str, TEXT_ALIGN_LEFT )
			else
				loadingText.BakeLocString( str, TEXT_ALIGN_LEFT )
			
			Reposition( )
		}
	}
	
	function Reposition( )
	{
		if( loadingText )
		{
			switch( imageLoc )
			{
				case AsyncStatusImageLocation.Right:
					loadingText.SetPosition( 0, -loadingText.Height * 0.5, 0 )
					loadingImage.SetPosition( loadingText.Width + 4 + loadingImage.LocalRect.Width * 0.5, 0, 0 )
				break
					
				case AsyncStatusImageLocation.Left:
					loadingText.SetPosition( loadingImage.LocalRect.Width * 0.5 + 4, -loadingText.Height * 0.5, 0 )
					loadingImage.SetPosition( 0, 0, 0 )
				break
			}
		}
		else
		{
			loadingImage.SetPosition( 0, 0, 0 )
		}
	}
}
