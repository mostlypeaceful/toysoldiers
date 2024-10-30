// Map Preview for the rewind screen

// Resources
sigimport "gui/textures/rewind/unknown_preview_g.png"
sigimport "gui/textures/rewind/upgrade_star_g.png"
sigimport "gui/textures/rewind/turrets_aa_g.png"
sigimport "gui/textures/rewind/turrets_at_g.png"
sigimport "gui/textures/rewind/turrets_flame_g.png"
sigimport "gui/textures/rewind/turrets_howitzer_g.png"
sigimport "gui/textures/rewind/turrets_mg_g.png"
sigimport "gui/textures/rewind/turrets_mortar_g.png"
sigimport "gui/textures/rewind/turrets_aa_red_g.png"
sigimport "gui/textures/rewind/turrets_at_red_g.png"
sigimport "gui/textures/rewind/turrets_flame_red_g.png"
sigimport "gui/textures/rewind/turrets_howitzer_red_g.png"
sigimport "gui/textures/rewind/turrets_mg_red_g.png"
sigimport "gui/textures/rewind/turrets_mortar_red_g.png"
sigimport "gui/textures/rewind/unknown_preview_red_g.png"

RewindPreviewIconTurretData <- {
	[ UNIT_ID_TURRET_MG_01 ] = { t = "mg", quality = 1 },
	[ UNIT_ID_TURRET_MG_02 ] = { t = "mg", quality = 2 },
	[ UNIT_ID_TURRET_MG_03 ] = { t = "mg", quality = 3 },
	[ UNIT_ID_TURRET_FLAME_01 ] = { t = "flame", quality = 1 },
	[ UNIT_ID_TURRET_FLAME_02 ] = { t = "flame", quality = 2 },
	[ UNIT_ID_TURRET_FLAME_03 ] = { t = "flame", quality = 3 },
	[ UNIT_ID_TURRET_ARTY_01 ] = { t = "arty", quality = 1 },
	[ UNIT_ID_TURRET_ARTY_02 ] = { t = "arty", quality = 2 },
	[ UNIT_ID_TURRET_ARTY_03 ] = { t = "arty", quality = 3 },
	[ UNIT_ID_TURRET_AA_01 ] = { t = "aa", quality = 1 },
	[ UNIT_ID_TURRET_AA_02 ] = { t = "aa", quality = 2 },
	[ UNIT_ID_TURRET_AA_03 ] = { t = "aa", quality = 3 },
	[ UNIT_ID_TURRET_AT_01 ] = { t = "at", quality = 1 },
	[ UNIT_ID_TURRET_AT_02 ] = { t = "at", quality = 2 },
	[ UNIT_ID_TURRET_AT_03 ] = { t = "at", quality = 3 },
	[ UNIT_ID_TURRET_MORTAR_01 ] = { t = "mortar", quality = 1 },
	[ UNIT_ID_TURRET_MORTAR_02 ] = { t = "mortar", quality = 2 },
	[ UNIT_ID_TURRET_MORTAR_03 ] = { t = "mortar", quality = 3 },
}

RewindPreviewIconTurretImages <- {
	aa = "gui/textures/rewind/turrets_aa",
	at = "gui/textures/rewind/turrets_at",
	flame = "gui/textures/rewind/turrets_flame",
	arty = "gui/textures/rewind/turrets_howitzer",
	mg = "gui/textures/rewind/turrets_mg",
	mortar = "gui/textures/rewind/turrets_mortar",
}

function GetRewindPreviewIconTurretImage( type, country )
{
	local suffix = ( ( country == COUNTRY_USSR )? "_red_g.png": "_g.png" )
	local fileBase = ( ( type in ::RewindPreviewIconTurretImages )? ::RewindPreviewIconTurretImages[ type ]: "gui/textures/rewind/unknown_preview" )
	return fileBase + suffix
}

class RewindPreviewIcon extends AnimatingCanvas
{
	// Data
	type = null
	quality = null
	country = null
	
	// Consts
	static height = 32
	static starWidth = 13
	
	constructor( unitID, country_ )
	{
		::AnimatingCanvas.constructor( )
		
		local icon = ::Gui.TexturedQuad( )
		icon.SetTexture( "gui/textures/rewind/unknown_preview_g.png" )
		
		if( unitID in RewindPreviewIconTurretData )
		{
			local data = ::RewindPreviewIconTurretData[ unitID ]
			type = data.t
			quality = data.quality
			country = country_
			
			icon.SetTexture( ::GetRewindPreviewIconTurretImage( type, country ) )
		}

		icon.CenterPivot( )
		icon.SetPosition( 0, 0, 0.001 )
		AddChild( icon )
		
		if( quality != null && quality > 0 && quality <= 3 )
		{
			for( local i = 0; i < quality; ++i )
			{
				local star = ::Gui.TexturedQuad( )
				star.SetTexture( "gui/textures/rewind/upgrade_star_g.png" )
				star.CenterPivot( )
				star.SetPosition( (-((quality - 1) * 0.5) + i) * starWidth, height / 2 - 2, 0 )
				AddChild( star )
			}
		}
	}
}

const TURRET_ALIGN_LEFT = 0
const TURRET_ALIGN_RIGHT = 1

class TurretIconDisplay extends AnimatingCanvas
{
	// Display
	turretIcons = null
	
	// Data
	country = null
	width = null
	align = null
	
	// Statics
	static iconWidth = 50
	static iconHeight = 42
	
	constructor( country_ = COUNTRY_USA, width_ = 4, align_ = TURRET_ALIGN_LEFT )
	{
		::AnimatingCanvas.constructor( )
		turretIcons = [ ]
		country = country_
		width = width_
		align = align_
	}
	
	function AddIcon( unitId )
	{
		local icon = ::RewindPreviewIcon( unitId, country )
		icon.SetAlpha( 0 )
		icon.FadeIn( 0.1 )
		AddChild( icon )
		turretIcons.push( icon )
		
		local index = turretIcons.len( ) - 1
		local dir = ( ( align == TURRET_ALIGN_LEFT ) ? 1 : -1 )
		icon.SetPosition( dir * (index % width) * iconWidth, (index / width).tointeger( ) * iconHeight, 0 )
	}
	
	function Clear( )
	{
		foreach( icon in turretIcons )
			icon.FadeOutAndDie( 0.1 )
		turretIcons.clear( )
	}
}
