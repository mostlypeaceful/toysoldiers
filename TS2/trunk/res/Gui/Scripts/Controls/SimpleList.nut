

class SimpleList extends Gui.CanvasFrame
{
	items = null
	VerticalPadding = null
	constructor( )
	{
		Gui.CanvasFrame.constructor( )
		items = [ ]
		VerticalPadding = 2
	}
	function Clear( )
	{
		foreach(i in items)
			i.DeleteSelf( )
		items = [ ]
	}
	function AddItem( i )
	{
		if( items.len( ) > 0 )
		{
			local prev = items.top( )
			local h = VerticalPadding + prev.LocalRect.Height
			local pos = prev.GetPosition( )
			pos.y += h
			i.SetPosition( pos )
		}
		else
			i.SetPosition( Math.Vec3.Construct( 0, 0, 0 ) )
	
		AddChild( i )
		items.push( i )
	}
	function ItemCount( )
		return items ? items.len( ) : 0
}
