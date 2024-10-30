
const PROGRESSBAR_MODE_SCALE = 0
const PROGRESSBAR_MODE_TEXTURE = 1

class ProgressBar extends AnimatingCanvas
{
	background = null
	meter = null
	meterPercent = 0
	mode = null

	constructor( backGroundTexture, foreGroundTexture )
	{
		::AnimatingCanvas.constructor( )

		meterPercent = 0
		mode = PROGRESSBAR_MODE_SCALE

		background = ::Gui.TexturedQuad( )
		background.SetTexture( backGroundTexture )
		background.SetPosition( ::Math.Vec3.Construct( 0, 0, 0.05 ) )
		AddChild( background )

		meter = ::Gui.TexturedQuad( )
		meter.SetTexture( foreGroundTexture )

		SetMeterVertical( 1.0 )
		AddChild( meter )
		
		IgnoreBoundsChange = 1
	}
	function SetMode( mode_ )
	{
		mode = mode_
	}
	function SetMeterHorizontal( percent )
	{
		meterPercent = percent
		switch( mode )
		{
		case PROGRESSBAR_MODE_TEXTURE:
			local texSize = meter.TextureDimensions( )
			meter.SetRect( ::Math.Vec2.Construct( meterPercent * texSize.x, texSize.y ) )
			meter.SetTextureRect( ::Math.Vec2.Construct( 0, 0 ), Math.Vec2.Construct( meterPercent, 1 ) )
			break;
		case PROGRESSBAR_MODE_SCALE:
		default:
			meter.SetScale( ::Math.Vec2.Construct( meterPercent, 1 ) )
			break;
		}
	}
	function SetMeterVertical( percent )
	{
		meterPercent = percent
		switch( mode )
		{
		case PROGRESSBAR_MODE_TEXTURE:
			local texSize = meter.TextureDimensions( )
			meter.SetRect( ::Math.Vec2.Construct( texSize.x, meterPercent * texSize.y ) )
			meter.SetTextureRect( ::Math.Vec2.Construct( 0, 0 ), Math.Vec2.Construct( 1, meterPercent ) )
			break;
		case PROGRESSBAR_MODE_SCALE:
		default:
			meter.SetScale( ::Math.Vec2.Construct( 1, meterPercent ) )
			break;
		}
		
		local backgroundDims = Size( )
		local meterDims = meter.TextureDimensions( )
		local border = (backgroundDims - meterDims) * 0.5;
		
		local positionY = backgroundDims.y - meterDims.y * meterPercent - border.y
		
		meter.SetPosition( Math.Vec3.Construct( border.x, positionY, 0.01 ) )
	}
	function SetMeterColor( color )
	{
		meter.SetRgba( color )
	}
	function SetMeterAlpha( alpha )
	{
		meter.SetAlpha( alpha )
	}
	function Size( )
	{
		return background.TextureDimensions( )
	}
	function MeterSize( )
	{
		return meter.TextureDimensions( )
	}
}