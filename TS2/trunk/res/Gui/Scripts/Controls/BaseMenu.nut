sigimport "gui/scripts/controls/animatingcanvas.nut"

class BaseMenuIcon extends AnimatingCanvas
{
	image = null
	onSelectCallback = null
	isActive = false
	scaleInactive = null
	scaleActive = null
	rgbaInactive = null
	rgbaActive = null
	transitionTime = null
	user = null

	constructor( imagePath, selectCb )
	{
		::AnimatingCanvas.constructor( )
		IgnoreBoundsChange = 0

		onSelectCallback = selectCb

		transitionTime = 0.2
		isActive = false

		scaleInactive = Math.Vec2.Construct( 0.5, 0.5 )
		scaleActive = Math.Vec2.Construct( 1.0, 1.0 )
		rgbaInactive = Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 )
		rgbaActive = Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 )

		if( imagePath )
		{
			image = ::Gui.TexturedQuad( )
			image.SetTexture( imagePath )
			AddChild( image )
		}
		
		SetZPos( 0.001 )
	}
	function SetActiveInactiveScale( inactive, active )
	{
		scaleInactive = inactive
		scaleActive = active
		SetScale( scaleInactive )
	}
	function SetActiveInactiveRgba( inactive, active )
	{
		rgbaInactive = inactive
		rgbaActive = active
		SetRgba( rgbaInactive )
	}
	function OnHighlight( active )
	{
		if( active )
		{
			ClearActions( )
			//SetScale( scaleActive )
			AddAction( ::ScaleTween( GetScale( ), scaleActive, transitionTime ) )
			//SetRgba( rgbaActive )
			AddAction( ::RgbaTween( GetRgba( ), rgbaActive, transitionTime ) )
			SetZPos( 0.000 )
		}
		else
		{
			ClearActions( )
			//SetScale( scaleInactive )
			AddAction( ::ScaleTween( GetScale( ), scaleInactive, transitionTime ) )
			//SetRgba( rgbaInactive )
			AddAction( ::RgbaTween( GetRgba( ), rgbaInactive, transitionTime ) )
			SetZPos( 0.001 )
		}

		isActive = active
	}
	function ChangeHorizontalHighlight( delta )
	{
		return false
	}
	function OnSelect( )
	{
		if( onSelectCallback && onSelectCallback( ) )
			return true
		return false
	}
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
	}
}

class BaseMenu extends AnimatingCanvas
{
	icons = null
	timer = 0
	fadeDelta = 0
	highlightIndex = -1
	AutoDelete = true
	fadeInSpeed = 4.0
	fadeOutSpeed = 2.0
	fadeInTime = 0.25
	fadeOutTime = 0.5
	inputHook = null
	backingOut = null
	
	// Sounds
	scrollSound = null
	forwardSound = null
	backSound = null
	errorSound = null

	constructor( )
	{
		::AnimatingCanvas.constructor( )
		IgnoreBoundsChange = 1
		highlightIndex = -1
		icons = [ ]
		SetAlpha( 0 )
		inputHook = false
		backingOut = false
		
		scrollSound = "Play_UI_Scroll"
		forwardSound = "Play_UI_Select_Forward"
		backSound = "Play_UI_Select_Backward"
		errorSound = "Play_HUD_WeaponMenu_Error"
	}
	function SetIconCount( count )
	{
		icons = array( count, null )
	}
	function FadeIn( )
	{
		ClearActionsOfType( "AlphaTween" )
		AddAction( ::AlphaTween( GetAlpha( ), 1.0, fadeInTime, null, null, null, function( canvas ) { canvas.OnFadeIn( ) } ) )
	}
	function FadeOut( )
	{
		ClearActionsOfType( "AlphaTween" )
		AddAction( ::AlphaTween( GetAlpha( ), 0.0, fadeOutTime, null, null, null, function( canvas ) { canvas.OnFadeOut( ) } ) )
	}
	function OnBackOut( )
	{
		backingOut = true
		// Play sound
		PlaySound( backSound )
		return true
	}
	function UnloadResources( )
	{
		// To be filled in by base classes
	}
	function HighlightByIndex( newHighlightIndex )
	{
		if( !icons || icons.len( ) == 0 )
			return false
		if( highlightIndex == -1 && newHighlightIndex == -1 )
			newHighlightIndex = 0

		if( newHighlightIndex != -1 && newHighlightIndex != highlightIndex )
		{
			OnHighlightChange( highlightIndex, newHighlightIndex )

			if( highlightIndex != -1 )
				icons[ highlightIndex ].OnHighlight( false )

			highlightIndex = newHighlightIndex

			if( highlightIndex != -1 )
				icons[ highlightIndex ].OnHighlight( true )

			return true
		}

		return false
	}
	function OnHighlightChange( oldHighlightIndex, newHighlightIndex )
	{
		if( oldHighlightIndex != -1 && newHighlightIndex != -1 )
			PlaySound( scrollSound )
	}
	function SelectActiveIcon( )
	{
		if( highlightIndex != -1 )
		{
			if( icons[ highlightIndex ].OnSelect( ) )
			{
				PlaySound( forwardSound )
				return true
			}
		}

		PlaySound( errorSound )
		return false
	}
	function OnFadeIn( ) { }
	function OnFadeOut( )
	{
		if( backingOut )
		{
			UnloadResources( )
			backingOut = false
		}
		
		if( AutoDelete )
		{
			ClearChildren( )
			DeleteSelf( )
		}
	}
	function ClearChildren( )
	{
		foreach( i in icons )
			i.onSelectCallback = null
		icons = [ ]
		Gui.CanvasFrame.ClearChildren( )
	}
	
	function PlaySound( sound )
	{
		 if( sound != null && !ParentIsInvisible( ) )
		 {
			if( audioSource != null )
			{
				audioSource.AudioEvent( sound )
			}
			else
				::GameApp.AudioEvent( sound )
		 }
	}
}

