// Rolling Menu (POORLY WRITTEN CODE)

sigvars Menu Behavior
@[ROLL_TIME] { "Roll Time", 0.2, [ 0.0:1.0 ], "Time in seconds that it takes to roll from one icon to the next" }

// Requires
sigimport "gui/scripts/controls/verticalmenu.nut"

// Resources
sigimport "gui/textures/frontend/locked_level_small_g.png"
sigimport "gui/textures/frontend/suspended_level_g.png"

// Icons
const LEVELSELECT_MENUICON_NONE = 0
const LEVELSELECT_MENUICON_LOCKED = 1
const LEVELSELECT_MENUICON_SUSPENDED = 2

class RollingMenuIcon extends VerticalMenuIcon
{
	// Data
	index = null
	
	// Constructor
	constructor( locTextID, selectCB, icon = LEVELSELECT_MENUICON_NONE )
	{
		::VerticalMenuIcon.constructor( null, selectCB )
		local inactiveColor = ::Math.Vec4.Construct( 0.6, 0.6, 0.6, 0.8 )
		SetActiveInactiveScale( ::Math.Vec2.Construct( 0.9, 0.9 ), ::Math.Vec2.Construct( 1.0, 1.0 ) )
		SetActiveInactiveRgba( inactiveColor, COLOR_CLEAN_WHITE )
		
		text = Gui.Text( )
		text.SetFontById( FONT_FANCY_MED )
		if( typeof locTextID == "string" )
			text.BakeLocString( ::GameApp.LocString( locTextID ), TEXT_ALIGN_LEFT )
		else
			text.BakeLocString( locTextID, TEXT_ALIGN_LEFT )
		text.SetPosition( 0, -text.Height * 0.5, 0 )
		AddChild( text )
		
		SetIcon( icon )

		if( image )
		{
			local imgSize = image.TextureDimensions( )
			image.SetPosition( -imgSize.x, -imgSize.y * 0.5 - 4, 0 )
		}	
	}
	
	function SetIcon( icon = LEVELSELECT_MENUICON_NONE )
	{
		if( !image )
		{
			image = ::Gui.TexturedQuad( )
			AddChild( image )
		}
			
		local iconPaths = [
			null,
			"gui/textures/frontend/locked_level_small_g.png",
			"gui/textures/frontend/suspended_level_g.png",
		]
		local colors = [ COLOR_CLEAN_WHITE, COLOR_LOCKED_GREEN, COLOR_SUSPENDED_BLUE ]
		
		local path = iconPaths[ icon ]
		if( path )
		{
			image.SetAlpha( 1 )
			image.SetTexture( path )
		}
		else
		{
			image.SetAlpha( 0 )
		}
		
		text.SetRgba( colors[ icon ] )
	}
	
	function OnHighlight( active )
	{
		isActive = active
	}
	
	function OnSelect( )
	{
		if( onSelectCallback && onSelectCallback( ) )
			return true
		return false
	}
	
	function OnTick( dt )
	{
		// Skip both VerticalMenuIcon's and BaseMenuIcon's OnTick
		// All the animation is controlled by the RollingMenu
		::Gui.CanvasFrame.OnTick( dt )
	}
	
}

enum RollingMenuState
{
	None = 0,
	Rolling = 1,
	Intro = 2,
}

class RollingMenu extends VerticalMenu
{
	// Data
	rolling_totalHeight = null // number, only valid after finalize
	rolling_selectable = null // bool
	rolling_visibleItemCount = null // number
	rolling_currentItemIndex = null // number [0:visibleItemCount-1]
	rolling_circular = null // bool
	rolling_state = null // RollingMenuState
	rolling_timer = null // number
	rolling_rollVelocity = null // number
	rolling_t = null // number, [0:n] n = number of items
	rolling_tGoal = null // number
	rolling_n = null // number, number of items, only valid after finalize
	
	// Statics
	static rolling_iconSpacing = 30.0
	
	constructor( backgroundTexture_ = null, visibleItemCount_ = 5, currentItemIndex_ = 1, circular_ = true )
	{
		::VerticalMenu.constructor( )
		
		rolling_visibleItemCount = visibleItemCount_
		rolling_currentItemIndex = currentItemIndex_
		rolling_circular = false
		rolling_state = RollingMenuState.None
		rolling_timer = 0.0
		rolling_rollVelocity = 0.0
		rolling_totalHeight = 0.0
		rolling_t = 0.0
		rolling_tGoal = 0.0
		rolling_n = 0
		rolling_selectable = true
	}
	
	function FinalizeIconSetup( )
	{
		if( finalized || icons == null || icons.len( ) == 0 )
			return
		finalized = true
		
		foreach( i, icon in icons )
		{
			icon.index = i
			icon.SetPosition( 0, i * rolling_iconSpacing, 0.0 )
			AddChild( icon )
		}
		
		rolling_n = icons.len( )
		rolling_totalHeight = rolling_iconSpacing * rolling_n
		
		::VerticalMenu.HighlightByIndex( 0 )
		rolling_state = RollingMenuState.Intro
	}
	
	// Add an item to the set of list items
	function AddItem( item )
	{
		icons.push( item )
	}

	function ChangeHighlight( indexDelta )
	{
		if( rolling_state == RollingMenuState.Rolling )
			return
		
		local i = highlightIndex + indexDelta
		
		if( i >= rolling_n )
		{
			i = ( ( rolling_circular )? 0: rolling_n - 1 )
		}
		else if( i < 0 )
		{
			i = ( ( rolling_circular )? rolling_n - 1: 0 )
		}
		
		return HighlightByIndex( i )
	}
	
	function InstantHighlight( index )
	{
		HighlightByIndex( index )
		rolling_t = rolling_tGoal
		rolling_timer = -1
	}
	
	function HighlightByIndex( toIndex )
	{
		// If there is one or fewer objects or we aren't actually changing, don't do anything
		if( rolling_n > 1 || toIndex != highlightIndex )
		{
			rolling_tGoal = toIndex
			rolling_rollVelocity = (rolling_tGoal - rolling_t) / ( @[ROLL_TIME] )
			rolling_timer = ( @[ROLL_TIME] )
			rolling_state = RollingMenuState.Rolling
		}
		
		return VerticalMenu.HighlightByIndex( toIndex )
	}
	
	function UpdateRolling( dt )
	{
		// Change t
		rolling_t += rolling_rollVelocity * dt
		rolling_timer -= dt
		
		if( rolling_timer < 0 )
		{
			rolling_t = rolling_tGoal
			rolling_state = RollingMenuState.None
		}
	}
	
	function OnTick( dt )
	{
		switch( rolling_state )
		{
			case RollingMenuState.Intro:
			{
				::VerticalMenu.OnTick( dt )
				UpdateRolling( dt )
			}
			break;
			
			case RollingMenuState.None:
			{
				::VerticalMenu.OnTick( dt )
			}
			break;
			
			case RollingMenuState.Rolling:
			{				
				UpdateRolling( dt )
				::AnimatingCanvas.OnTick( dt )
			}
			break;
		}
		
		// Set all icons to have the right properties for their t-offset
		local i = 0
		local tOffset = 0.0
		local tOffsetFixed = 0.0
		local iOffset = 0.0
		
		foreach( icon in icons )
		{
			tOffset = icon.index - rolling_t
			
			// Set position
			local x = menuPositionOffset.x
			local y = tOffset * rolling_iconSpacing + menuPositionOffset.y
			icon.SetPosition( x, y, icon.GetZPos( ) )
			
			// Set Alpha
			icon.SetAlphaClamp( -::Math.Abs( 0.25 * tOffset ) + 1.0 )
			
			// Set Color
			local aColor = icon.rgbaActive
			local iColor = icon.rgbaInactive
			local r0 = iColor.x
			local r1 = aColor.x
			local g0 = iColor.y
			local g1 = aColor.y
			local b0 = iColor.z
			local b1 = aColor.z
			local tt = ::Math.Clamp( -::Math.Abs( tOffset ) + 1.0, 0.0, 1.0 )
			icon.SetRgb( r0 + tt * ( r1 - r0 ), g0 + tt * ( g1 - g0 ), b0 + tt * ( b1 - b0 ) )
			
			local sI = icon.scaleInactive
			local sA = icon.scaleActive
			icon.SetScale( sI.x + tt * ( sA.x - sI.x ), sI.y + tt * ( sA.y - sI.y ) )
		}
	}
}