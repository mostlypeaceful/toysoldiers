
// Requires
sigimport "Gui/Scripts/Controls/FadeInFadeOutTexture.nut"

// Resources
sigimport "gui/textures/cursor/cursor_nocash_g.png"
sigimport "gui/textures/cursor/cursor_noupgrade_g.png"
sigimport "gui/textures/cursor/cursor_norepair_g.png"

sigexport function CanvasCreateRtsCursorUI( cppObject )
{
	return RtsCursorUI( )
}

class NoMoneyIcon extends FadeInFadeOutTexture
{
	// Display
	text = null
	
	constructor( )
	{
		FadeInFadeOutTexture.constructor( "gui/textures/cursor/cursor_nocash_g.png" )
		
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeCString( "" )
		text.SetPosition( ::Math.Vec3.Construct( 0, 24, 0 ) )
		AddChild( text )
		
		CenterPivot( )
	}
	
	function SetCost( cost )
	{
		if( cost )
		{
			local costString = GameApp.LocString( "Cost" ).Replace( "money", ::LocString.ConstructMoneyString( cost.tostring( ) ) )			
			text.BakeLocString( costString, TEXT_ALIGN_CENTER )
		}
		else
		{
			text.BakeCString( "" )
		}
	}
}

class NoUpgradeIcon extends FadeInFadeOutTexture
{
	constructor( )
	{
		FadeInFadeOutTexture.constructor( "gui/textures/cursor/cursor_noupgrade_g.png" )
		CenterPivot( )
	}
}

class NoRepairIcon extends FadeInFadeOutTexture
{
	constructor( )
	{
		FadeInFadeOutTexture.constructor( "gui/textures/cursor/cursor_norepair_g.png" )
		CenterPivot( )
	}
}

class RtsCursorUnitText extends Gui.CanvasFrame
{
	fadeInSpeed = 4.0
	fadeOutSpeed = 0
	fadeDelta = 0

	hoverText = null
	
	constructor( )
	{
		Gui.CanvasFrame.constructor( )

		fadeInSpeed = 4.0
		fadeOutSpeed = -2.0
		fadeDelta = fadeOutSpeed

		hoverText = ::Gui.Text( )
		hoverText.SetFontById( FONT_SIMPLE_SMALL )
		AddChild( hoverText )	

		SetAlpha( 0 )
	}
	function OnTick( dt )
	{
		local newAlpha = GetAlpha( ) + fadeDelta * dt
		SetAlphaClamp( newAlpha )

		::Gui.CanvasFrame.OnTick( dt )
	}
	function SetHoverInfo( text )
	{
		hoverText.BakeBoxLocString( 400, text, TEXT_ALIGN_CENTER )
	}
	function SetVisibility( visible )
	{
		if( visible )
			FadeIn( )
		else
			Hide( )
	}
	function FadeIn( )
	{
		fadeDelta = fadeInSpeed
	}
	function FadeOut( )
	{
		fadeDelta = fadeOutSpeed
	}
	function Hide( )
	{
		fadeDelta = fadeOutSpeed
		SetAlpha( 0 )
	}
}

class RtsCursorUI extends Gui.CanvasFrame
{
	noMoney = null
	noUpgrade = null // NoUpgradeIcon
	unitInfo = null
	
	noRepair = null
	purchasePlatformText = null

	constructor( )
	{
		::Gui.CanvasFrame.constructor( )

		noRepair = ::NoRepairIcon( )
		noRepair.inDelta = 4.0
		noRepair.outDelta = -3.0
		AddChild( noRepair )

		noMoney = ::NoMoneyIcon( )
		noMoney.inDelta = 4.0
		noMoney.outDelta = -3.0
		AddChild( noMoney )	
		
		noUpgrade = ::NoUpgradeIcon( )
		noUpgrade.inDelta = 4.0
		noUpgrade.outDelta = -3.0
		AddChild( noUpgrade )
		
		unitInfo = ::RtsCursorUnitText( )
		unitInfo.SetPosition( Math.Vec3.Construct( 0, 115, 0.1 ) )
		AddChild( unitInfo )

		purchasePlatformText = ::AnimatingCanvas( )
		AddChild( purchasePlatformText )
		
		local text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		text.BakeBoxLocString( 400, GameApp.LocString( "capture_platform" ), TEXT_ALIGN_CENTER )
		
		purchasePlatformText.AddChild( text )
		purchasePlatformText.SetAlpha( 0 )		
	}
	function ShowCapturePlatform( show )
	{
		if( show )
			purchasePlatformText.FadeIn( )	
		else
			purchasePlatformText.FadeOut( )	
	}
	function SetNoMoney( show, force, cost = null )
	{
		if( show )
		{
			if( cost )
				noMoney.SetCost( cost )
			
			noMoney.SetTime( 1.0, force )
			noRepair.ForceHide( )
			noUpgrade.ForceHide( )
		}
		else
		{
			noMoney.FadeOut( )
		}
	}
	function SetNoUpgrade( show, force )
	{
		if( show )
		{
			noUpgrade.SetTime( 1.0, force )
			noRepair.ForceHide( )
			noMoney.ForceHide( )
		}
		else
			noUpgrade.FadeOut( )
	}
	function SetNoRepair( show, force )
	{
		if( show )
		{
			noRepair.SetTime( 1.0, force )
			noMoney.ForceHide( )
			noUpgrade.ForceHide( )
		}
		else
			noRepair.FadeOut( )
	}
	function ShowUnitInfo( text )
	{
		unitInfo.SetHoverInfo( text )
		unitInfo.FadeIn( )
	}
	function HideUnitInfo( )
	{
		unitInfo.FadeOut( )
	}
	
}
