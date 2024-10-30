
class FlashingText extends Gui.Text
{	
	time = 0
	show = false
	
	flashSpeed = null
	fadeOut = null
	flashCount = -1 
	highLow = false //used to count loops
	
	constructor( locID, speed, fade, alignment = TEXT_ALIGN_LEFT )
	{
		time = 0
		show = false
		
		flashSpeed = speed
		fadeOut = fade
		flashCount = -1
		highLow = false
	
		Gui.Text.constructor()
		SetFontById( FONT_FANCY_MED )
		SetRgba( COLOR_CLEAN_WHITE )
		BakeLocString( GameApp.LocString( locID ), alignment )
	}
	
	function OnTick( dt )
	{
		::Gui.Text.OnTick( dt )
		if( show )
		{
			time += dt
			local alpha = (Math.Sin( time * flashSpeed ) + 1) * 0.5
			
			if( flashCount > 0 )
			{
				if( !highLow && alpha > 0.75 )
				{
					highLow = true
					flashCount -= 1
					if( flashCount == 0 )
						Show( false )
				}
				else if( highLow && alpha < 0.75 )
					highLow = false
			}
			
			SetAlpha( alpha )
		}
		else
		{
			SetAlpha( GetAlpha( ) * fadeOut )
		}
	}
	
	function Show( visible )
	{
		if( !show ) time = 0
		show = visible
	}
	
	function ShowCount( count )
	{
		Show( true )
		flashCount = count
	}
}