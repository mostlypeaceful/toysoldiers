// Screen space notifications for money and combos

// Requires
sigimport "gui/scripts/controls/animatingcanvas.nut"

sigexport function CanvasCreateScreenSpaceNotification( obj )
{
	return ScreenSpaceNotification( obj )
}

class ScreenSpaceNotificationItem extends AnimatingCanvas
{
	// Display
	text = null // Gui.Text
	multiplierText = null // Gui.Text
	
	// Data
	string = null
	multiplier = null
	timer = null
	container = null
	
	constructor( locString, color, cont )
	{
		::AnimatingCanvas.constructor( )
		
		string = locString
		multiplier = 1
		timer = 0
		container = cont.weakref( )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		text.SetRgba( color.x, color.y, color.z, 1.0 )
		text.BakeLocString( locString, TEXT_ALIGN_RIGHT )
		text.SetUniformScale( 0.7 )
		AddChild( text )
		
		multiplierText = ::Gui.Text( )
		multiplierText.SetFontById( FONT_FANCY_MED )
		multiplierText.SetRgba( 0.5, 0.5, 0.5, 1.0 )
		multiplierText.SetPosition( 4, 3, 0 )
		multiplierText.SetUniformScale( 0.5 )
		AddChild( multiplierText )
		UpdateMultiplier( )
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )

		if( timer > 2 )
		{
			// Fade out
			timer = -1
			AddAction( ::AlphaTween( 1.0, 0.0, 2.0, EasingTransition.Quadratic, EasingEquation.Out, null, function( canvas )
				{
					canvas.container.RemoveItem( canvas )
					canvas.DeleteSelf( )
				} ) )
		}
		else if( timer >= 0 )
		{
			timer += dt
		}
	}
	
	function UpdateMultiplier( )
	{
		if( multiplier > 1 )
			multiplierText.BakeCString( "(x" + multiplier.tostring( ) + ")", TEXT_ALIGN_LEFT )
		else
			multiplierText.BakeCString( " ", TEXT_ALIGN_LEFT )
	}
	
	function Add( )
	{
		multiplier++
		UpdateMultiplier( )
		timer = 0
		
		ClearActions( )
		SetAlpha( 1 )
	}
}

class SSNContainer extends AnimatingCanvas
{
	// Data
	items = null // array of ScreenSpaceNotificationItem objects
	
	constructor( )
	{
		::AnimatingCanvas.constructor( )
		
		items = [ ]
	}
	
	function AddItem( item )
	{
		item.SetYPos( 0 )
		//items.push( item )
		items.insert( 0, item )
		AddChild( item )
		Reposition( )
	}
	
	function RemoveItem( item )
	{
		foreach( i, otherItem in items )
		{
			if( otherItem.string == item )
			{
				items.remove( i )
				Reposition( )
				return
			}
		}
	}
	
	function Reposition( )
	{
		local spacing = 23
		
		foreach( i, item in items )
		{
			if( i > 4 )
			{
				item.DeleteSelf( )
				continue
			}
				
			local currentPos = item.GetYPos( )
			local goalPos = -(i * spacing)
			//item.AddAction( ::YMotionTween( currentPos, goalPos, 0.4 ) )
			item.SetYPos( goalPos )
		}
		
		if( items.len( ) > 5 )
			items = items.slice( 0, 5 )
	}
}

class ScreenSpaceNotification extends AnimatingCanvas
{
	// Data
	container = null // SSNContainer
	
	constructor( obj )
	{
		::AnimatingCanvas.constructor( )
		
		local vpRect = obj.User.ComputeViewportSafeRect( )
		SetPosition( vpRect.Center.x, vpRect.Center.y - 140, 0.25 )
		::GameApp.HudRoot.AddChild( this )
		
		container = ::SSNContainer( )
		AddChild( container )
	}
	
	function SpawnText( text, color )
	{
		// Check if any of the items match the text
		foreach( item in container.items )
		{
			if( item.string == text )
			{
				item.Add( )
				return
			}
		}

		local item = ::ScreenSpaceNotificationItem( text, color, container )
		container.AddItem( item )
	}
	
	function Clear( )
	{
		container.items.clear( )
		container.ClearChildren( )
	}
}