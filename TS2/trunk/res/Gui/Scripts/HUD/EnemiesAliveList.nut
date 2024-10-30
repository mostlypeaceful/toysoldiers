 
sigexport function CanvasCreateEnemiesAliveList( waveList )
{
	return EnemiesAliveList( waveList )
}

class EnemyIcon extends AnimatingCanvas
{
	// Display
	image = null
	countText = null
	
	// Data
	unitID = null
	count = null
	textAlign = null
	maxAlpha = null

	constructor( unitID_, country, alt = false )
	{
		::AnimatingCanvas.constructor( )

		unitID = unitID_
		count = 0
		maxAlpha = 1.0
		textAlign = ( alt? TEXT_ALIGN_LEFT: TEXT_ALIGN_RIGHT )

		image = ::Gui.TexturedQuad( )
		image.SetTexture( ::GameApp.UnitWaveIconPath( unitID, country ) )
		image.SetUniformScale( 0.5 )
		image.SetPosition( -image.WorldRect.Width, 0, 0 )
		AddChild( image )

		countText = ::Gui.Text( )
		countText.SetFontById( FONT_SIMPLE_SMALL )
		countText.SetRgba( COLOR_CLEAN_WHITE )
		local yOffset = ( image.WorldRect.Width - countText.LineHeight ) * 0.5
		local spacingBetweenTextAndImage = 5
		countText.SetPosition( -( image.WorldRect.Width + spacingBetweenTextAndImage ), yOffset, 0 )
		AddChild( countText )

		if( alt )
		{
			image.SetPosition( 0, 0, 0 )
			countText.SetPosition( -countText.GetXPos( ), yOffset, 0 )
		}
		
		if( ::EnemiesAliveList.Debugging )
			::print( "AddIcon( id:" + unitID.tostring( ) + ")" )
		
		SetAlpha( 0 )
	}
	
	function FadeIn( )
	{
		ClearActions( )
		AddAction( ::AlphaTween( GetAlpha( ), maxAlpha, 0.5 * ( maxAlpha - GetAlpha( ) ) * maxAlpha ) )
	}
	
	function FadeOut( )
	{
		FadeOutAndDie( 0.5 )
	}
	
	function SetCount( count_ )
	{
		if( count != count_ )
		{
			if( ::EnemiesAliveList.Debugging )
				::print( "SetCount( id:" + unitID.tostring( ) + "-" + count.tostring( ) + ")" )
			count = count_
			countText.BakeCString( count.tostring( ), textAlign )
		}
	}
	
	function ImageWidth( )
	{
		return image.WorldRect.Width
	}
	
	function ImageHeight( )
	{
		return image.WorldRect.Height
	}
}

class EnemiesAliveList extends AnimatingCanvas
{
	enemyList = null
	icons = null
	isAlt = null // bool, to check if this is the alternate list
	text = null
	
	// Static
	static Debugging = false

	constructor( _enemyList )
	{
		::AnimatingCanvas.constructor( )

		enemyList = _enemyList		
		icons = [ ]
		isAlt = false
		
		local vpRect = enemyList.User.ComputeViewportSafeRect( )
		local vpIndex = enemyList.User.ViewportIndex
		local verticalOffset = 190
		local netCoopOffset = 30 + 74
		
		local levelInfo = ::GameApp.CurrentLevelLoadInfo
		local listZ = 0.25
		
		// Split Screen Coop or Versus
		if( ( ::GameApp.GameMode.IsCoOp || ::GameApp.GameMode.IsVersus ) && ::GameApp.GameMode.IsSplitScreen )
		{
			CenterPivot( )
			if( vpIndex == 0 )
			{
				isAlt = true
				SetPosition( vpRect.Left, vpRect.Top + verticalOffset, listZ )
			}
			else
				SetPosition( vpRect.Right, vpRect.Top + verticalOffset, listZ ) 
		}
		// XBLA Versus
		else if( ::GameApp.GameMode.IsVersus && ::GameApp.GameMode.IsNet )
		{
			if( !enemyList.User.IsViewportVirtual )
				SetPosition( vpRect.Right, vpRect.Top + verticalOffset, listZ )
			else
			{
				isAlt = true
				SetPosition( vpRect.Left, vpRect.Top + verticalOffset, listZ )
			}
		}
		// XBLA Coop
		else if( ( ::GameApp.GameMode.IsNet && ::GameApp.GameMode.IsCoOp ) 
			  || ( !::GameApp.IsFullVersion && ( levelInfo.MapType == MAP_TYPE_MINIGAME || levelInfo.MapType == MAP_TYPE_SURVIVAL ) ) )
		{
			SetPosition( vpRect.Right, vpRect.Top + netCoopOffset, listZ )
		}
		// Single Player
		else
		{
			SetPosition( vpRect.Right, vpRect.Top + 5, listZ )
		}
		
		if( ::EnemiesAliveList.Debugging )
		{
			text = ::Gui.Text( )
			text.SetFontById( FONT_SIMPLE_SMALL )
			text.SetRgba( COLOR_CLEAN_WHITE )
			text.SetPosition( -200, 0, 0 )
			AddChild( text )
		}
		
		if( ::GameApp.CurrentLevelLoadInfo.MapType == MAP_TYPE_MINIGAME )
			Invisible = true

		::GameApp.HudLayer( "viewport" + enemyList.User.ViewportIndex.tostring( ) ).AddChild( this )
	}
	
	function UpdateDebugText( )
	{
		if( !::EnemiesAliveList.Debugging )
			return
			
		local outString = ( "icons (" + icons.len( ).tostring( ) + ")\n" )
		
		foreach( i, icon in icons )
			outString += ( "id: " + icon.unitID.tostring( ) + " count:" + icon.count.tostring( ) + "\n" )
		
		text.BakeBoxCString( 400, outString, TEXT_ALIGN_LEFT )
	}
	
	function AddIcon( unitID, country, count )
	{
		foreach( icon in icons )
		{
			if( icon.unitID == unitID )
			{
				icon.SetCount( count )
				return
			}
		}
		
		if( count <= 0 )
			return
		
		local ico = ::EnemyIcon( unitID, country, isAlt )
		ico.SetCount( count )
		ico.SetYPos( ( ico.ImageHeight( ) + 5 ) * icons.len( ) )
		icons.push( ico )
		AddChild( ico )
		PositionIcons( )
	}
	
	function RemoveIcon( index )
	{
		if( index >= icons.len( ) )
			return
		
		if( ::EnemiesAliveList.Debugging )
			::print( "RemoveIcon( index:" + index.tostring( ) + ")" )
		icons[ index ].SetCount( 0 )
		icons[ index ].FadeOutAndDie( )
		icons.remove( index )
		PositionIcons( )
		UpdateDebugText( )
	}
	
	function SetCount( unitID, country, count )
	{
		foreach( i, icon in icons )
		{
			if( icon.unitID == unitID )
			{
				if( count <= 0 )
					RemoveIcon( i )
				else
					icon.SetCount( count )
				UpdateDebugText( )
				return
			}
		}

		if( count > 0 )
			AddIcon( unitID, country, count )
		
		UpdateDebugText( )
	}
	
	function PositionIcons( )
	{
		if( icons.len( ) == 0 )
			return

		local height = icons[ 0 ].ImageHeight( ) + 5
		local y = 0

		foreach( i, icon in icons )
		{
			icon.maxAlpha = ::Math.Clamp( 1.0 - (i * 0.2), 0.4, 1.0 )
			icon.FadeIn( )
			
			if( icon.GetYPos( ) != y )
				icon.AddAction( ::YMotionTween( icon.GetYPos( ), y, 0.3, null, EasingEquation.InOut ) )
				
			y += height
		}
	}
}

