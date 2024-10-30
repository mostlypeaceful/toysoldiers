
sigvars Menus
@[StandardMenuFont] { "Standard Menu Font", FONT_FANCY_LARGE, [ FONT_FANCY_SMALL, FONT_FANCY_MED, FONT_FANCY_LARGE ] }
@[StandardMenuPosX] { "Standard Menu Pos: X", 460, [ 1:1280 ] }
@[StandardMenuPosY] { "Standard Menu Pos: Y", 320, [ 1:720 ] }
@[TSLogoPosX] { "TS Logo Pos: X", 240, [ 1:1280 ] }
@[TSLogoPosY] { "TS Logo Pos: Y", 120, [ 1:720 ] }

// Requires
sigimport "Gui/Scripts/Controls/VerticalMenu.nut"
sigimport "gui/scripts/frontend/levelselect/profilebadge.nut"

function FrontEndDoNothing( ) return false

class FrontEndMenuEntry extends VerticalMenuIcon
{
	descriptor = null
	unlockedSelectCB = null
	
	constructor( textString, desc, selectCb, bakeCString = false )
	{
		::VerticalMenuIcon.constructor( null, selectCb )
		descriptor = desc
		unlockedSelectCB = selectCb

		text = ::Gui.Text( )
		text.SetFontById( @[StandardMenuFont] )
		
		BakeText( textString, bakeCString )

		Finalize( )

		SetActiveInactiveScale( ::Math.Vec2.Construct( 0.6, 0.6 ), ::Math.Vec2.Construct( 0.9, 0.9 ) )
		SetActiveInactiveRgba( ::Math.Vec4.Construct( 0.6, 0.6, 0.6, 0.8 ), COLOR_CLEAN_WHITE )
		
		if( ::GameApp.IsAsianLanguage( ) )
			SetActiveInactiveScale( ::Math.Vec2.Construct( 0.8, 0.8 ), ::Math.Vec2.Construct( 1.0, 1.0 ) )
	}
	
	function SetLocked( locked )
	{
		if( locked )
		{
			if( !image )
				image = ::Gui.TexturedQuad( )
			image.SetTexture( "gui/textures/frontend/locked_level_small_g.png" )
			image.SetAlpha( 1 )
			local imgSize = image.TextureDimensions( )
			image.SetPosition( -imgSize.x, -imgSize.y, 0 )
			AddChild( image )
			
			text.SetRgba( COLOR_LOCKED_GREEN )
			
			onSelectCallback = null
		}
		else
		{
			if( image )
				image.SetAlpha( 0 )
			text.SetRgba( COLOR_CLEAN_WHITE )
			
			onSelectCallback = unlockedSelectCB
		}
	}
	
	function Reset( textString, desc, selectCb )
	{
		BakeText( textString )
		descriptor = desc
		onSelectCallback = selectCb
	}
	
	function BakeText( textString, bakeCString = false )
	{
		if( bakeCString )
			text.BakeCString( textString, TEXT_ALIGN_LEFT )
		else
		{
			if( typeof textString == "string" )
				text.BakeLocString( ::GameApp.LocString( textString ), TEXT_ALIGN_LEFT )
			else if( typeof textString == "instance" )
				text.BakeLocString( textString, TEXT_ALIGN_LEFT )
		}
	}
}
	
class FrontEndMenuBase extends VerticalMenu
{
	// Display
	gameLogo = null
	profileName = null
	menuName = null
	controls = null // ControllerButtonContainer
	secondaryControls = null // ControllerButtonContainer
	descriptor = null
	
	// Statics
	static buttonSpacing = 24
	
	constructor( imgPath = "gui/textures/frontend/logo_ts2_g.png", userOverride = null )
	{
		::VerticalMenu.constructor( )
		SetPosition( 0, 0, 0.5 )
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		menuPositionOffset = ::Math.Vec3.Construct( vpRect.Left + @[StandardMenuPosX], vpRect.Top + @[StandardMenuPosY], 0 )
		
		if( imgPath )
		{
			gameLogo = Gui.TexturedQuad( )
			gameLogo.SetTexture( imgPath )
			gameLogo.SetPosition( @[TSLogoPosX], @[TSLogoPosY], 0.3 )
			AddChild( gameLogo )
		}

		profileName = ::ProfileBadge( )
		if( userOverride != null && !is_null( ::GameApp.FrontEndPlayer.User ) )
			profileName.SetUser( ::GameApp.FrontEndPlayer.User, userOverride )
		else if( !is_null( ::GameApp.FrontEndPlayer.User ) )
			profileName.SetUser( ::GameApp.FrontEndPlayer.User, ::GameApp.FrontEndPlayer.User )
		else
			profileBadge.SetInactive( )
		profileName.EnableControl( false )
		profileName.SetPosition( vpRect.Left, vpRect.Top, 0 )
		AddChild( profileName )
		
		menuName = Gui.Text( )
		menuName.SetFontById( FONT_SIMPLE_MED )
		menuName.SetRgba( COLOR_CLEAN_WHITE )
		menuName.SetPosition( vpRect.Right, vpRect.Top, 0 )
		AddChild( menuName )
		
		// Description Text
		descriptor = Gui.Text( )
		descriptor.SetFontById( FONT_SIMPLE_SMALL )
		descriptor.SetRgba( COLOR_CLEAN_WHITE )
		descriptor.SetPosition( vpRect.Right, vpRect.Bottom - descriptor.LineHeight, 0 )
		AddChild( descriptor )
		
		// Controls
		controls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL, 20 )
		controls.SetPosition( vpRect.Left, vpRect.Bottom - 12, 0 )
		AddChild( controls )
		
		secondaryControls = ::ControllerButtonContainer( FONT_SIMPLE_SMALL, 15 )
		secondaryControls.SetPosition( controls.GetXPos( ), controls.GetYPos( ) - 26, 0 )
		AddChild( secondaryControls )
		
		SetControls( )
	}
	
	function SetControls( )
	{
		controls.Clear( )
		controls.AddControl( GAMEPAD_BUTTON_A, "Menus_Select" )
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
	}
	
	function HighlightByIndex( i )
	{
		local prev = highlightIndex
		::VerticalMenu.HighlightByIndex( i )
		if( ( highlightIndex != prev ) && ( highlightIndex in icons ) && ( icons[ highlightIndex ].descriptor != null ) )
			SetDescriptorText( icons[ highlightIndex ].descriptor )
		else
			descriptor.SetAlpha( 0 )
	}
	
	function SetMenuNameText( textId, alreadyLocStringd = false )
	{
		if( alreadyLocStringd )
			menuName.BakeLocString( textId, TEXT_ALIGN_RIGHT )
		else
			menuName.BakeLocString( ::GameApp.LocString( textId ), TEXT_ALIGN_RIGHT )
	}
	
	function SetDescriptorText( textId )
	{
		descriptor.SetAlpha( 1 )
		if( typeof textId == "string" )
			descriptor.BakeLocString( ::GameApp.LocString( textId ), TEXT_ALIGN_RIGHT )
		else if( typeof textId == "instance" )
			descriptor.BakeLocString( textId, TEXT_ALIGN_RIGHT )
		else
			descriptor.SetAlpha( 0 )
	}
}

FadeToBlackCanvas <- null

function FadeThroughBlack( inTime = 1.0, holdTime = 0.0, outTime = 1.0 )
{
	if( ::FadeToBlackCanvas )
	{
		::FadeToBlackCanvas.ClearActions( )
	}
	else
	{
		::FadeToBlackCanvas = ::AnimatingCanvas( )
			local black = ::Gui.ColoredQuad( )
			black.SetRgba( 0, 0, 0, 1 )
			black.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
			::FadeToBlackCanvas.AddChild( black )
		::FadeToBlackCanvas.SetPosition( 0, 0, 0 )
		::GameApp.WarningCanvas.AddChild( ::FadeToBlackCanvas )
	}
	
	::FadeToBlackCanvas.SetAlpha( 0 )
	::FadeToBlackCanvas.AddAction( ::AlphaTween( 0.0, 1.0, inTime ) )
	::FadeToBlackCanvas.AddDelayedAction( inTime + holdTime, ::AlphaTween( 1.0, 0.0, outTime, null, null, null, function( canvas )
	{
		::FadeToBlackCanvas.DeleteSelf( )
		::FadeToBlackCanvas = null
	} ) )
}
