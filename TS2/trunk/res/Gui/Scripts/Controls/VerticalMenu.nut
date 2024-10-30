
sigimport "Gui/Scripts/Controls/BaseMenu.nut"
sigimport "Gui/Scripts/Controls/Gamepad.nut"


class VerticalMenuIcon extends BaseMenuIcon
{
	text = null
	pulseTimer = 0
	basePulseAlpha = 1
	absFunc = null
	cosFunc = null
	lerpFunc = null
	
	constructor( imagePath, selectCb )
	{
		::BaseMenuIcon.constructor( imagePath, selectCb )
		pulseTimer = 0
		basePulseAlpha = 1
		absFunc = ::Math.Abs
		cosFunc = ::Math.Cos
		lerpFunc = ::Math.Lerp
	}
	function Finalize( )
	{
		if( image )
		{
			local textHeight = text.LocalRect.Height
			local imageHeight = image.LocalRect.Height
			local diff = absFunc( imageHeight - textHeight )
			local textYOffset = 0.5 * diff - ( textHeight - text.Base )

			text.SetPosition( image.LocalRect.Width + 2, textYOffset, 0 )
		}
		else
			text.SetPosition( 0, -0.25 * text.LocalRect.Height, 0 )
			
		AddChild( text )
		ShiftPivot( Math.Vec2.Construct( 0, 0.5 * LocalRect.Height ) )
	}
	function OnHighlight( active  )
	{
		::BaseMenuIcon.OnHighlight( active )
		basePulseAlpha = GetAlpha( )
		pulseTimer = 0
	}
	function OnTick( dt )
	{
		::BaseMenuIcon.OnTick( dt )

		if( isActive )
		{
			local t = absFunc( cosFunc( pulseTimer ) )
			SetAlpha( basePulseAlpha * lerpFunc( 0.350, 1.125, t ) )
			pulseTimer += 3 * dt
		}
	}
}

enum VerticalMenuAction
{
	None		= 0,
	ExitStack	= 1,
	PushMenu	= 2,
	PopMenu		= 3,
}

class VerticalMenu extends BaseMenu
{
	vSpacing = 2
	finalized = false
	nextAction = VerticalMenuAction.None
	nextMenu = null
	triggerUserReset = false
	user = null
	inputDelay = null
	advancedInputDelay = null
	allowFastInput = null
	menuPositionOffset = null
	
	ForwardButtons = null
	ForwardUpButtons = null
	BackButtons = null
	AutoExit = null
	AutoAdvance = null
	AutoBackOut = null

	constructor( )
	{
		::BaseMenu.constructor( )
		AutoDelete = false
		fadeInTime = 0.5
		triggerUserReset = false
		inputDelay = 0.2
		advancedInputDelay = null
		allowFastInput = true
		menuPositionOffset = ::Math.Vec3.Construct( 0, 0, 0 )

		ForwardButtons = GAMEPAD_BUTTON_A
		ForwardUpButtons = 0
		BackButtons = GAMEPAD_BUTTON_B
		AutoExit = false
		AutoAdvance = false
		AutoBackOut = false
	}

	function FinalizeIconSetup( ) // assumes 'icons' array is filled with RadialMenuIcons
	{
		if( finalized || icons == null || icons.len( ) == 0 )
			return
		finalized = true
		
		//if( !user )
		//{
		//	::print("SCRIPT ERROR: VertMenu.FinalizeIconSetup has a NULL user!")
		//	DumpCallstack( )
		//	BreakPoint( )
		//}

		local yPos = menuPositionOffset.y
		foreach( i in icons )
		{
			i.user = user
			i.SetPosition( menuPositionOffset.x, yPos, menuPositionOffset.z + i.GetZPos( ) )
			AddChild( i )
			yPos += i.LocalRect.Height * i.scaleInactive.y + vSpacing
		}

		HighlightByIndex( -1 )
	}
	
	function RefinalizeIconSetup( )
	{
		finalized = false
		
		local selIndex = highlightIndex
		
		for( local i = 0; i < icons.len( ); ++i )
			icons[ i ].DeleteSelf( )
			
		icons.clear( )
		FinalizeIconSetup( )
		
		highlightIndex = -1
		HighlightByIndex( selIndex )
		
	}		
		
	function ChangeHighlight( indexDelta )
	{
		local newHighlightIndex = highlightIndex + indexDelta
		if( newHighlightIndex >= icons.len( ) )
			newHighlightIndex = 0
		else if( newHighlightIndex < 0 )
			newHighlightIndex = icons.len( ) - 1
		return HighlightByIndex( newHighlightIndex )
	}
	function ChangeHorizontalHighlight( delta )
	{
		if( highlightIndex >= 0 && highlightIndex < icons.len( ) )
			return icons[ highlightIndex ].ChangeHorizontalHighlight( delta )
		else
			return false
	}
	function PushNextMenu( nextMenuInstance )
	{
		nextMenu = nextMenuInstance
		nextAction = VerticalMenuAction.PushMenu
		nextMenu.audioSource = audioSource
		return true
	}
	function PushNextMenuFromType( nextMenuType, ... )
	{
		local args = [ ]
		
		local obj = nextMenuType.instance( )
		args.push( obj )
		
		for( local i = 0; i < vargc; ++i )
			args.push( vargv[ i ] )
		
		obj.constructor.acall( args )
		return PushNextMenu( obj )
	}
	function VerticalMenuFadeIn( verticalMenuStack )
	{
		::BaseMenu.FadeIn( )
	}
	function QueryUser( )
	{
		// Do nothing, since its up to the application to respond
		return null
	}
	function HandleInput( gamepad )
	{
	}
	function SetUser( _user )
	{
		user = _user
	}
	function IgnoreSignOutEvent( )
	{
		return false
	}
}

class VerticalMenuStack extends Gui.CanvasFrame
{
	menus = null
	filteredGamepad = null
	minStackCount = 1
	user = null
	inputTime = null
	previousGamepadDir = null
	inputEnabled = null
	continuousInputTimer = null
	audioSource = null

	constructor( user_, filterInput = true )
	{
		::Gui.CanvasFrame.constructor( )
		
		user = user_
		if( !user )
		{
			::print("SCRIPT ERROR: VerticalMenuStack.ctor called with a NULL user!")
			DumpCallstack( )
			BreakPoint( )
		}
		audioSource = ::GameApp.GetPlayerByUser( user ).AudioSource
		menus = [ ]
		filteredGamepad = ::FilteredGamepad( user, filterInput )
		minStackCount = 1
		inputTime = 0
		previousGamepadDir = 0
		inputEnabled = true
		continuousInputTimer = 0
	}
	function IsEmpty( )
	{
		return menus.len( ) == 0
	}
	function MenuCount( )
	{
		return menus.len( )
	}
	function ClearUser( )
	{
		filteredGamepad.Release( )
	}
	function SetUser( newUser )
	{
		if( !newUser )
		{
			::print("SCRIPT ERROR: VerticalMenuStack.SetUser called with a NULL user!")
			DumpCallstack( )
			BreakPoint( )
		}
		user = newUser
		filteredGamepad.SetUser( user )
	}
	function PushMenu( newMenu )
	{
		if( !user )
		{
			::print("SCRIPT ERROR: VerticalMenuStack.PushMenu called with a NULL user!")
			DumpCallstack( )
			BreakPoint( )
		}
		
		local curMenu = CurrentMenu( )
		if( curMenu )
			curMenu.FadeOut( )
		menus.push( newMenu )
		AddChild( newMenu )
		newMenu.SetUser( user )
		newMenu.audioSource = audioSource
		newMenu.icons = [ ]
		newMenu.FinalizeIconSetup( )
		newMenu.VerticalMenuFadeIn( this )
	}
	function PopMenu( )
	{
		if( menus.len( ) <= minStackCount )
			return

		local top = menus.top( )		
		if( !top.OnBackOut( ) )
			return // menu can't back out yet
		
		top.AcceptsCanvasEvents = false
		top.AutoDelete = true
		top.FadeOut( )
		if( menus && menus.len( ) > 0 )
			menus.pop( )

		local curMenu = CurrentMenu( )
		if( curMenu )
			curMenu.VerticalMenuFadeIn( this )
		else if( filteredGamepad )
			filteredGamepad.Release( )
	}
	function ExitStack( )
	{
		local curMenu = CurrentMenu( )
		for( local i = 0; i < menus.len( ); ++i )
		{
			menus[ i ].AcceptsCanvasEvents = false
			menus[ i ].FadeOut( )
		}
		menus = [ ]
		filteredGamepad.Release( )
		inputEnabled = false
	}
	function CurrentMenu( )
	{
		if( menus.len( ) == 0 )
			return null
		local curMenu = menus.top( )
		if( !curMenu.Parent )
		{
			menus.pop( )
			curMenu = CurrentMenu( )
			if( curMenu )
				curMenu.VerticalMenuFadeIn( this )
		}
		return curMenu
	}
	function DeleteSelf( )
	{
		foreach( i in menus )
			i.ClearChildren( )
		menus = [ ]
		if( filteredGamepad )
			filteredGamepad.Release( )
		Gui.CanvasFrame.DeleteSelf( )
	}
	function OnTick( dt )
	{
		::Gui.CanvasFrame.OnTick( dt )

		local curMenu = CurrentMenu( )
		
		if( curMenu && curMenu.triggerUserReset )
		{
			ClearUser( )
			curMenu.triggerUserReset = false
		}

		// If we don't have a gamepad yet then try to query for one
		if( curMenu && filteredGamepad.IsNull( ) )
		{
			local queryUser = curMenu.QueryUser( )
			if( queryUser )
				SetUser( queryUser )
		}
		
		local gamepad = filteredGamepad.Get( )
		
		// Additional input
		if( curMenu && curMenu.inputHook )
			curMenu.HandleInput( gamepad )
		
		if( curMenu && inputEnabled )
		{
			if( curMenu.AutoExit )
			{
				ExitStack( )
			}
			else if( curMenu.AutoBackOut || gamepad.ButtonDown( curMenu.BackButtons ) )
			{
				curMenu.AutoBackOut = false // reset auto backout flag
				PopMenu( )
			}
			else if( curMenu.AutoAdvance || ((gamepad.ButtonDown( curMenu.ForwardButtons ) || gamepad.ButtonUp( curMenu.ForwardUpButtons ))) )
			{
				curMenu.AutoAdvance = false // reset auto advance flag
				if( curMenu.SelectActiveIcon( ) && curMenu.nextAction )
				{
					switch( curMenu.nextAction )
					{
						case VerticalMenuAction.ExitStack:
							ExitStack( )
						break
						
						case VerticalMenuAction.PushMenu:
							if( curMenu.nextMenu )
							{
								PushMenu( curMenu.nextMenu )
								curMenu.nextMenu = null
							}
						break
						
						case VerticalMenuAction.PopMenu:
							PopMenu( )
						break
					}

					curMenu.nextAction = VerticalMenuAction.None
				}
			}
			else
			{
				local highlightDir = GetGamepadDirection( curMenu, gamepad, dt )
				switch( highlightDir )
				{
					case 1:
						curMenu.ChangeHighlight( -1 )
					break
					
					case 2:
						curMenu.ChangeHighlight( 1 )
					break
					
					case 3:
						curMenu.ChangeHorizontalHighlight( -1 )
					break
					
					case 4:
						curMenu.ChangeHorizontalHighlight( 1 )
					break
				}
			}
		}

		inputTime += dt
	}
	function GetGamepadDirection( curMenu, gamepad, dt ) // 0: nothing, 1/2: up/down, 3/4:left/right
	{
		local absFunc = ::Math.Abs
		
		// Check up/down
		local dir = 0
		local upDownDeadZone = 0.3
		local leftStick = gamepad.LeftStick
		if( ( leftStick.y < -upDownDeadZone && absFunc( leftStick.x ) < absFunc( leftStick.y ) ) || gamepad.ButtonHeld( GAMEPAD_BUTTON_DPAD_DOWN ) )
			dir = 2
		else if( ( leftStick.y > upDownDeadZone && absFunc( leftStick.x ) < absFunc( leftStick.y ) ) || gamepad.ButtonHeld( GAMEPAD_BUTTON_DPAD_UP ) )
			dir = 1
			
		// Check left/right
		local leftRightDeadZone = 0.5
		if( ( leftStick.x < -leftRightDeadZone && absFunc( leftStick.y ) < absFunc( leftStick.x ) ) || gamepad.ButtonHeld( GAMEPAD_BUTTON_DPAD_LEFT ) )
			dir = 3
		else if( ( leftStick.x > leftRightDeadZone && absFunc( leftStick.y ) < absFunc( leftStick.x ) ) || gamepad.ButtonHeld( GAMEPAD_BUTTON_DPAD_RIGHT ) )
			dir = 4
			
		local currentDelay = curMenu.inputDelay
		local advancedDelay = curMenu.advancedInputDelay
		if( advancedDelay != null )
		{
			if( previousGamepadDir == dir )
				continuousInputTimer += dt
			else
				continuousInputTimer = 0
			
			if( typeof advancedDelay == "table" )
			{
				foreach( time, delay in advancedDelay )
				{
					if( continuousInputTimer > time )
						currentDelay = delay
				}
			}
			else
			{
				::print( "Tried to use advancedInputDelay but did not set it as a table" )
				advancedInputDelay = null
			}
		}
		
		if( dir != 0 && ( inputTime > currentDelay || ( previousGamepadDir != dir && curMenu.allowFastInput ) ) )
		{
			inputTime = 0
			previousGamepadDir = dir
			return dir
		}
		
		previousGamepadDir = dir
		return 0
	}
}

