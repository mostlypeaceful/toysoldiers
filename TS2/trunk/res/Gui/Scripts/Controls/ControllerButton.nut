sigimport "gui/textures/gamepad/button_a_g.png"
sigimport "gui/textures/gamepad/button_b_g.png"
sigimport "gui/textures/gamepad/button_back_g.png"
sigimport "gui/textures/gamepad/button_bulletpoint_g.png"
sigimport "gui/textures/gamepad/button_lshoulder_g.png"
sigimport "gui/textures/gamepad/button_lstick_g.png"
sigimport "gui/textures/gamepad/button_lstickpress_g.png"
sigimport "gui/textures/gamepad/button_ltrigger_g.png"
sigimport "gui/textures/gamepad/button_rshoulder_g.png"
sigimport "gui/textures/gamepad/button_rstick_g.png"
sigimport "gui/textures/gamepad/button_rstickpress_g.png"
sigimport "gui/textures/gamepad/button_rtrigger_g.png"
sigimport "gui/textures/gamepad/button_start_g.png"
sigimport "gui/textures/gamepad/button_x_g.png"
sigimport "gui/textures/gamepad/button_y_g.png"
sigimport "gui/textures/gamepad/button_dpadleft_g.png"
sigimport "gui/textures/gamepad/button_dpadright_g.png"
sigimport "gui/textures/gamepad/button_dpadup_g.png"
sigimport "gui/textures/gamepad/button_dpaddown_g.png"
sigimport "gui/textures/gamepad/button_dpadleftright_g.png"
sigimport "gui/textures/gamepad/button_dpadupdown_g.png"

class ControllerButtonAlignment
{
	LEFT = 0
	RIGHT = 1
	TOP = 2
	BOTTOM = 3
}

function ControllerButtonImageFromId( buttonId )
{
	if( buttonId == ( GAMEPAD_BUTTON_DPAD_LEFT | GAMEPAD_BUTTON_DPAD_RIGHT ) )
		return "gui/textures/gamepad/button_dpadleftright_g.png"
	else if( buttonId == ( GAMEPAD_BUTTON_DPAD_UP | GAMEPAD_BUTTON_DPAD_DOWN ) )
		return "gui/textures/gamepad/button_dpadupdown_g.png"
		
	switch( buttonId )
	{
	case GAMEPAD_BUTTON_START:
		return "gui/textures/gamepad/button_start_g.png"
	case GAMEPAD_BUTTON_SELECT:
		return "gui/textures/gamepad/button_back_g.png"
	case GAMEPAD_BUTTON_A:
		return "gui/textures/gamepad/button_a_g.png"
	case GAMEPAD_BUTTON_B:
		return "gui/textures/gamepad/button_b_g.png"
	case GAMEPAD_BUTTON_X:
		return "gui/textures/gamepad/button_x_g.png"
	case GAMEPAD_BUTTON_Y:
		return "gui/textures/gamepad/button_y_g.png"
	case GAMEPAD_BUTTON_DPAD_RIGHT:
		return "gui/textures/gamepad/button_dpadright_g.png"
	case GAMEPAD_BUTTON_DPAD_UP:
		return "gui/textures/gamepad/button_dpadup_g.png"
	case GAMEPAD_BUTTON_DPAD_LEFT:
		return "gui/textures/gamepad/button_dpadleft_g.png"
	case GAMEPAD_BUTTON_DPAD_DOWN:
		return "gui/textures/gamepad/button_dpaddown_g.png"
	case GAMEPAD_BUTTON_LSHOULDER:
		return "gui/textures/gamepad/button_lshoulder_g.png"
	case GAMEPAD_BUTTON_LTHUMB:
		return "gui/textures/gamepad/button_lstickpress_g.png"
	case GAMEPAD_BUTTON_LTRIGGER:
		return "gui/textures/gamepad/button_ltrigger_g.png"
	case GAMEPAD_BUTTON_RSHOULDER:
		return "gui/textures/gamepad/button_rshoulder_g.png"
	case GAMEPAD_BUTTON_RTHUMB:
		return "gui/textures/gamepad/button_rstickpress_g.png"
	case GAMEPAD_BUTTON_RTRIGGER:
		return "gui/textures/gamepad/button_rtrigger_g.png"
	case GAMEPAD_BUTTON_LTHUMB_MINMAG:
		return "gui/textures/gamepad/button_lstick_g.png"
	case GAMEPAD_BUTTON_RTHUMB_MINMAG:
		return "gui/textures/gamepad/button_rstick_g.png"
	default:
		return "gui/textures/gamepad/button_bulletpoint_g.png"
	}
}

function ButtonIdFromImage( imagePath )
{
	local lowerImagePath = imagePath.tolower( )
	switch( lowerImagePath )
	{
	case "gui/textures/gamepad/button_start_g.png":
		return GAMEPAD_BUTTON_START
	case "gui/textures/gamepad/button_back_g.png":
		return GAMEPAD_BUTTON_SELECT
	case "gui/textures/gamepad/button_a_g.png":
		return GAMEPAD_BUTTON_A
	case "gui/textures/gamepad/button_b_g.png":
		return GAMEPAD_BUTTON_B
	case "gui/textures/gamepad/button_x_g.png":
		return GAMEPAD_BUTTON_X
	case "gui/textures/gamepad/button_y_g.png":
		return GAMEPAD_BUTTON_Y
	case "gui/textures/gamepad/button_dpadright_g.png":
		return GAMEPAD_BUTTON_DPAD_RIGHT
	case "gui/textures/gamepad/button_dpadup_g.png":
		return GAMEPAD_BUTTON_DPAD_UP
	case "gui/textures/gamepad/button_dpadleft_g.png":
		return GAMEPAD_BUTTON_DPAD_LEFT
	case "gui/textures/gamepad/button_dpaddown_g.png":
		return GAMEPAD_BUTTON_DPAD_DOWN
	case "gui/textures/gamepad/button_lshoulder_g.png":
		return GAMEPAD_BUTTON_LSHOULDER
	case "gui/textures/gamepad/button_lstickpress_g.png":
		return GAMEPAD_BUTTON_LTHUMB
	case "gui/textures/gamepad/button_ltrigger_g.png":
		return GAMEPAD_BUTTON_LTRIGGER
	case "gui/textures/gamepad/button_rshoulder_g.png":
		return GAMEPAD_BUTTON_RSHOULDER
	case "gui/textures/gamepad/button_rstickpress_g.png":
		return GAMEPAD_BUTTON_RTHUMB
	case "gui/textures/gamepad/button_rtrigger_g.png":
		return GAMEPAD_BUTTON_RTRIGGER
	case "gui/textures/gamepad/button_lstick_g.png":
		return GAMEPAD_BUTTON_LTHUMB_MINMAG
	case "gui/textures/gamepad/button_rstick_g.png":
		return GAMEPAD_BUTTON_RTHUMB_MINMAG
	default:
		return null
	}
}

function GetButtonId( buttonId )
{
	switch( typeof buttonId )
	{
		case "integer":
			return buttonId
		break
		
		case "string":
			return ::ButtonIdFromImage( buttonId )
		break
	}
	
	return null
}

ButtonOrderTable <- {
	[ GAMEPAD_BUTTON_LSHOULDER ] = 0,
	[ GAMEPAD_BUTTON_LTRIGGER ] = 1,
	[ GAMEPAD_BUTTON_LTHUMB_MINMAG ] = 2,
	[ GAMEPAD_BUTTON_LTHUMB ] = 3,
	[ GAMEPAD_BUTTON_SELECT ] = 4,
	[ GAMEPAD_BUTTON_Y ] = 5,
	[ GAMEPAD_BUTTON_X ] = 6,
	[ GAMEPAD_BUTTON_A ] = 7,
	[ GAMEPAD_BUTTON_B ] = 8,
	[ GAMEPAD_BUTTON_START ] = 9,
	[ GAMEPAD_BUTTON_RTHUMB ] = 10,
	[ GAMEPAD_BUTTON_RTHUMB_MINMAG ] = 11,
	[ GAMEPAD_BUTTON_RTRIGGER ] = 12,
	[ GAMEPAD_BUTTON_RSHOULDER ] = 13,
}

function ButtonOrder( id )
{
	if( id == null )
		return 99
	else if( id in ::ButtonOrderTable )
		return ::ButtonOrderTable[ id ]
	else
		return 99
}

class ControllerButtonMultiButtonImage extends Gui.CanvasFrame
{
	constructor( imagePaths ) // imagePaths: array 
	{
		::Gui.CanvasFrame.constructor( )
		
		local pos = 0
		foreach( path in imagePaths )
		{
			if( type( path ) == "integer" )
			{
				path = ::ControllerButtonImageFromId( path )
			}
			
			local image = ::Gui.TexturedQuad( )
			image.SetTexture( path )
			image.SetPosition( Math.Vec3.Construct( pos, 0, 0 ) )
			AddChild( image )
			
			pos += image.WorldRect.Width
		}
	}
	
	function TextureDimensions( )
	{
		return Math.Vec2.Construct( WorldRect.Width, WorldRect.Height )
	}
}

class ControllerButtonComboButtonImage extends Gui.CanvasFrame
{
	constructor( imagePaths, comboSymbol = "Plus" ) // imagePaths: array 
	{
		::Gui.CanvasFrame.constructor( )
		
		if( typeof comboSymbol != "string" )
			comboSymbol = "Plus"
		
		local pos = 0
		foreach( i, path in imagePaths )
		{
			if( type( path ) == "integer" )
			{
				path = ::ControllerButtonImageFromId( path )
			}
			
			local image = ::Gui.TexturedQuad( )
			image.SetTexture( path )
			image.SetPosition( Math.Vec3.Construct( pos, 0, 0 ) )
			AddChild( image )
			
			pos += image.WorldRect.Width
			
			if( i != imagePaths.len( ) - 1 )
			{
				local text = ::Gui.Text( )
				text.SetFontById( FONT_SIMPLE_SMALL )
				text.SetRgba( COLOR_CLEAN_WHITE )
				text.BakeLocString( GameApp.LocString( comboSymbol ) )
				text.SetPosition( Math.Vec3.Construct( pos, 0, 0 ) )
	
			AddChild( text )
				
				pos += text.WorldRect.Width
			}
		}
	}
	
	function TextureDimensions( )
	{
		return Math.Vec2.Construct( WorldRect.Width, WorldRect.Height )
	}
}

class ControllerButton extends AnimatingCanvas
{
	// Display
	image = null
	text = null
	
	// Data
	alignment = 0
	textID = null
	order = null
	
	constructor( imagePath, textID_, align = null, fontID = null, comboButton = null )
	{
		::AnimatingCanvas.constructor( )
		
		text = ::Gui.Text( )
		text.SetRgba( COLOR_CLEAN_WHITE )
		AddChild( text )
		
		Set( imagePath, textID_, align, fontID, comboButton )
	}
	
	function Set( imagePath, textID_, align = null, fontID = null, comboButton = null )
	{
		if( align )
			alignment = align
		
		if( image )
		{
			image.DeleteSelf( )
			RemoveChild( image )
		}

		if( typeof imagePath == "integer" )
		{
			imagePath = ::ControllerButtonImageFromId( imagePath )
		}
		
		if( typeof imagePath == "string" )
		{
			image = ::Gui.TexturedQuad( )
			image.SetTexture( imagePath )
		}
		else if( typeof imagePath == "array" )
		{
			if( comboButton )
				image = ::ControllerButtonComboButtonImage( imagePath, comboButton )
			else
				image = ::ControllerButtonMultiButtonImage( imagePath )
		}
		AddChild( image )

		textID = textID_
		if( !fontID )
			fontID = FONT_SIMPLE_MED
		text.SetFontById( fontID )
		SetText( textID )
		
		order = ::ButtonOrder( ::GetButtonId( imagePath ) )

		Reposition( )
	}
	
	function _cmp( that )
	{
		local thisOrder = order
		local thatOrder = that.order
		
		if( thisOrder != null && thatOrder != null )
		{
			if( thisOrder > thatOrder )
				return 1
			else if( thisOrder < thatOrder )
				return -1
			else
				return 0
		}
		else if( thatOrder != null )
			return -1
		else
			return -1
	}
	
	function GetSize( )
	{
		local math = ::Math
		local minFunc = math.Min
		local maxFunc = math.Max
		local vec2Construct = math.Vec2.Construct
		
		local min = vec2Construct( )
		local max = vec2Construct( )
		local imgPos = image.GetPosition( )
		local textPos = text.GetPosition( )
		local imgSize = image.WorldRect;
		local textSize = text.WorldRect;
		
		min.x = minFunc( imgPos.x, textPos.x )
		min.y = minFunc( imgPos.y, textPos.y )
		max.x = maxFunc( imgPos.x + imgSize.Width, textPos.x + textSize.Width )
		max.y = maxFunc( imgPos.y + imgSize.Height, textPos.y + textSize.Height )
		
		return math.Rect.Construct( vec2Construct( 0.0, 0.0 ), vec2Construct( max.x - min.x, max.y - min.y ) )
	}
	
	function SetAlignment( align )
	{
		if( alignment == align )
			return
		
		alignment = align
		Reposition( )
		SetText( textID )
	}
	
	function Reposition( )
	{
		local imageOffset = Math.Vec3.Construct( 0, 0, 0 )
		local textOffset = Math.Vec3.Construct( 0, 0, 0 )
		
		local buttonTextSpacing = 4
		local imageDims = image.TextureDimensions( )
		local imageScale = image.GetScale( )
		imageDims.x = imageDims.x * imageScale.x
		imageDims.y = imageDims.y * imageScale.y
		
		switch( alignment )
		{
		case ControllerButtonAlignment.LEFT:
			imageOffset.y = -imageDims.y * 0.5
			textOffset.x = imageDims.x + buttonTextSpacing
			textOffset.y = -text.LineHeight * 0.5
			break;
		case ControllerButtonAlignment.RIGHT:
			imageOffset.x = -imageDims.x
			imageOffset.y = -imageDims.y * 0.5
			textOffset.x = -( imageDims.x + buttonTextSpacing )
			textOffset.y = -text.LineHeight * 0.5
			break;
		case ControllerButtonAlignment.TOP:
			imageOffset.x = -imageDims.x * 0.5
			textOffset.y = imageDims.y * 0.5 + buttonTextSpacing + 10 // Different spacing for the top alignment
			break;
		case ControllerButtonAlignment.BOTTOM:
			imageOffset.x = -imageDims.x * 0.5
			imageOffset.y = -imageDims.y
			textOffset.y = -( imageDims.y + buttonTextSpacing + text.LineHeight )
			break;
		default:
			alignment = ControllerButtonAlignment.LEFT
			imageOffset.y = -imageDims.y * 0.5
			textOffset.x = imageDims.x + buttonTextSpacing
			textOffset.y = -text.LineHeight * 0.5
			break;
		}

		image.SetPosition( imageOffset )
		text.SetPosition( textOffset )
	}
	
	function SetText( string )
	{
		textID = string
		switch( alignment )
		{
		case ControllerButtonAlignment.LEFT:
			BakeString( textID, TEXT_ALIGN_LEFT )
			break;
		case ControllerButtonAlignment.RIGHT:
			BakeString( textID, TEXT_ALIGN_RIGHT )
			break;
		case ControllerButtonAlignment.TOP:
		case ControllerButtonAlignment.BOTTOM:
			BakeString( textID, TEXT_ALIGN_CENTER )
			break;
		default:
			alignment = ControllerButtonAlignment.LEFT
			BakeString( textID, TEXT_ALIGN_LEFT )
			break;
		}
	}
	
	function BakeString( string, align )
	{
		if( string == null )
			text.BakeCString( "", align )
		else
		{
			local str = null
			if( typeof string == "string" )
				str = ::GameApp.LocString( string )
			else if( typeof string == "instance" )
				str = string
				
			if( str )
				text.BakeLocString( str, align )
		}
	}
	
	function ConstrainText( availableTextSpace )
	{
		text.Compact( availableTextSpace )
	}
}
