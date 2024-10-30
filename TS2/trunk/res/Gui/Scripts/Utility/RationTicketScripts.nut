// Functions for ration tickets

/*<i name="Level1_RationTicket1_Name">
  <Text>Combined Arms</Text>
</i>
<i name="Level1_RationTicket1_Description">
  <Text>Use a machine gun turret to finish off 50 mortar-slowed infantry.</Text>
</i>
<i name="Level1_RationTicket1_Progress">
  <Text> slowed infantry killed.</Text>
</i>*/

function RationTicketLocID( levelIndex, rationTicketIndex )
{
	return "Level" + levelIndex.tostring( ) + "_RationTicket" + rationTicketIndex.tostring( )
}

function RationTicketNameLocString( levelIndex, rationTicketIndex )
{
	return ::GameApp.LocString( ::RationTicketLocID( levelIndex, rationTicketIndex ) + "_Name" ).Clone( )
}

function RationTicketDescLocString( levelIndex, rationTicketIndex )
{
	return ::GameApp.LocString( ::RationTicketLocID( levelIndex, rationTicketIndex ) + "_Description" ).Clone( )
}

function RationTicketFullDescLocString( levelIndex, rationTicketIndex )
{
	return ::GameApp.LocString( "LevelIntro_DecorationFormat" ).Replace( "name", ::RationTicketNameLocString( levelIndex, rationTicketIndex ) ).Replace( "desc", ::RationTicketDescLocString( levelIndex, rationTicketIndex ) )
}

function RationTicketProgressLocString( levelIndex, rationTicketIndex, value, max )
{
	return ::GameApp.LocString( "Decoration_ProgressFormat" ).Replace( "current", value ).Replace( "max", max ).Replace( "desc", ::GameApp.LocString( ::RationTicketLocID( levelIndex, rationTicketIndex ) + "_Progress" ) )
}

function RationTicketImagePath( levelIndex, rationTicketIndex )
{
	return "gui/textures/rationtickets/rationticket_lvl" + levelIndex.tostring( ) + "-" + rationTicketIndex.tostring( ) + "_g.png"
}

function GoldenRationTicketNameLocString( index )
{
	if( index == 0 )
		return ::GameApp.LocString( "Golden_RationTicket1_Name" )
	else
		return ::GameApp.LocString( "Golden_RationTicket11_Name" )
}

function GoldenRationTicketDescLocString( index )
{
	if( index == 0 )
		return ::GameApp.LocString( "Golden_RationTicket1_Description" )
	else
		return ::GameApp.LocString( "Golden_RationTicket11_Description" )
}

function GoldenRationTicketImagePath( index )
{
	if( index == 0 )
		return "gui/textures/rationtickets/goldenarcade1_g.png"
	else
		return "gui/textures/rationtickets/goldenarcade11_g.png"
}

// Decoration management
class DecorationData
{
	index = null
	progress = null
	max = null
	condition = null
	level = null
	
	constructor( index_, max_, condition_, level_ )
	{
		index = index_
		progress = 0
		max = max_
		condition = condition_
		level = level_
		
		if( max != null && level.RationTicketActive( index ) )
			level.RationTicketProgress( index, progress, max )
	}
	
	function Check( event )
	{
		if( !level.RationTicketActive( index ) )
			return
		
		if( condition && condition( event ) )
		{
			if( max != null )
				_inc( )
			else
				_award( )
		}
	}
	
	function Inc( )
	{
		if( level.RationTicketActive( index ) )
			_inc( )
	}
	
	function Award( )
	{
		if( level.RationTicketActive( index ) )
			_award( )
	}
	
	// Private
	function _inc( )
	{
		progress++
		level.RationTicketProgress( index, progress, max )
		if( progress >= max )
			level.AwardRationTicket( index )
	}
	
	function _award( )
	{
		level.AwardRationTicket( index )
	}
}