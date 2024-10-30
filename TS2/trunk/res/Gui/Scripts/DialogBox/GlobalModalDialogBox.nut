
// Requires
sigimport "gui/scripts/controls/basemenu.nut"
sigimport "Gui/Scripts/Controls/Gamepad.nut"
sigimport "gui/scripts/controls/controllerbutton.nut"
sigimport "gui/scripts/frontend/levelselect/profilebadge.nut"

LocalGlobalDialogBoxObject <- null
LocalGlobalDialogBoxQueue <- []

RemoteGlobalDialogBoxObject <- null
RemoteGlobalDialogBoxQueue <- []

function ResetGlobalDialogBoxSystem( )
{
	ClearGlobalDialog( ::LocalGlobalDialogBoxObject )
	ClearGlobalDialogBoxQueue( ::LocalGlobalDialogBoxQueue )
	::LocalGlobalDialogBoxObject = null
	
	ClearGlobalDialog( ::RemoteGlobalDialogBoxObject )
	ClearGlobalDialogBoxQueue( ::RemoteGlobalDialogBoxQueue )
	::RemoteGlobalDialogBoxObject = null
}

function ClearGlobalDialogBoxQueue( queue )
{
	foreach( box in queue )
		ClearGlobalDialog( box )
	
	queue.clear( )
}

function ClearGlobalDialog( box )
{
	if( box )
	{
		// Ensure we unpause the game
		if( box.didPause )
			::GameApp.Pause( false, null )
			
		box.ReleasePads( )
		box.DeleteSelf( )
	}
}

function ClearAllGlobalDialogBoxQueuesOfTextId( textId )
{
	::LocalGlobalDialogBoxQueue = ClearGlobalDialogBoxQueueOfTextId(
		::LocalGlobalDialogBoxQueue, textId )
		
	::RemoteGlobalDialogBoxQueue = ClearGlobalDialogBoxQueueOfTextId(
		::RemoteGlobalDialogBoxQueue, textId )
}

function ClearGlobalDialogBoxQueueOfTextId( queue, textId )
{
	local newQueue = []
	
	foreach( box in queue )
	{
		if( box.textId != textId )
			newQueue.push( box )
		else
			::ClearGlobalDialog( box )
	}
	
	queue.clear( )
	
	return newQueue;
}

class GlobalModalDialogBox extends AnimatingCanvas
{
	// Display
	controls = null
	fadeBackground = null
	gamerPicture = null
	gamerTag = null
	
	// Data
	allPadsList = null
	checkPadsList = null
	filteredPad = null
	checkAllPads = false
	isLocal = false
	
	onFadedOut = null
	onCanceled = null
	showCancel = false
	canceled = false
	onAPress = null
	onBPress = null
	noInput = null
	cancelExiting = null
	cancelButtonId = null
	
	wantsPaused = null
	didPause = null
	
	fadingOut = null
	textId = null
	
	// Statics
	static DialogFadeTime = 0.4

	constructor( _textId, user, acceptText = "Ok", cancelText = null, cancelButtonId_ = GAMEPAD_BUTTON_B )
	{
		::AnimatingCanvas.constructor( )
		
		textId = _textId
		fadingOut = false
		canceled = false
		//DumpCallstack( )
	
		checkAllPads = ( user == null )
		isLocal = ( !user || user.IsLocal )
		
		filteredPad = null
		allPadsList = []
		checkPadsList = []
		
		if( isLocal )
		{
			local userCount = ::GameApp.LocalUserCount
			for( local u = 0; u < userCount; ++u )
			{
				local locUser = ::GameApp.GetLocalUser( u )
					
				local pad = ::FilteredGamepad( locUser, true, true )
				allPadsList.push( pad )
				
				// If its a referenced user then we can check the pad
				if( ::GameApp.WhichPlayer( locUser ) != ~0 )
					checkPadsList.push( pad )
				
				// If this is the specified user, then set the pad
				if( user && locUser.LocalHwIndex == user.LocalHwIndex )
					filteredPad = pad
			}
		}
		else
		{					
			local pad = ::FilteredGamepad( user, true, true )
			allPadsList.push( pad )
			checkPadsList.push( pad )
			filteredPad = pad
		}
		
		onAPress = null
		onBPress = null
		onFadedOut = null
		onCanceled = null
		
		showCancel = (cancelText != null)
		noInput = true
		cancelExiting = false
		wantsPaused = false
		didPause = false
		cancelButtonId = cancelButtonId_
		
		local vpRect = ::GameApp.ComputeScreenSafeRect( )
		
		fadeBackground = ::Gui.ColoredQuad( )
		fadeBackground.SetRgba( 0.0, 0.0, 0.0, 0.8 )
		fadeBackground.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fadeBackground.SetPosition( -vpRect.Center.x, -vpRect.Center.y, 0.02 )
		AddChild( fadeBackground )

		local displayText = null
		if( textId != null )
		{
			displayText = ::Gui.Text( )
			displayText.SetFontById( FONT_FANCY_MED )
			displayText.BakeBoxLocString( 1000, GameApp.LocString( textId ), TEXT_ALIGN_CENTER )
			displayText.SetPosition( Math.Vec3.Construct( 0, -0.5 * displayText.LocalRect.Height, 0 ) )
			displayText.SetRgba( COLOR_CLEAN_WHITE )
			AddChild( displayText )
		}
		
		/*badge = ::ProfileBadge( )
		badge.SetUser( ::GameApp.FrontEndPlayer.User, user )
		badge.EnableControl( false )
		badge.SetPosition( -badge.size.x / 2, ( ( displayText )? -displayText.LocalRect.Height : 0 ) - badge.size.y, 0 )
		AddChild( badge )*/
		
		local startX = -130
		local startY = ( ( displayText )? -displayText.LocalRect.Height : 0 ) - 72
		
		// Player Image
		gamerPicture = null
		if( user )
		{
			gamerPicture = ::Gui.GamerPictureQuad( )
			gamerPicture.SetPosition( startX, startY, 0 )
			gamerPicture.SetTexture( ::GameApp.FrontEndPlayer.User, user, false )
			AddChild( gamerPicture )
		}
		
		// Player Text
		gamerTag = null
		if( user )
		{
			gamerTag = ::Gui.Text( )
			gamerTag.SetFontById( FONT_FANCY_MED )
			gamerTag.SetRgba( COLOR_CLEAN_WHITE )
			gamerTag.BakeLocString( user.GamerTag, TEXT_ALIGN_LEFT )
			gamerTag.SetPosition( startX + 74, startY + 32 - gamerTag.Height * 0.5, 0 )
			AddChild( gamerTag )
		}
		
		local buttonHeight = ( ( displayText )? displayText.LocalRect.Height: 0 )
		
		controls = ::ControllerButtonContainer( FONT_SIMPLE_MED, 24 )
		controls.AddControl( GAMEPAD_BUTTON_A, acceptText )
		if( showCancel )
			controls.AddControl( cancelButtonId_, cancelText )
		controls.SetPosition( -controls.LocalRect.Width * 0.5, buttonHeight, 0 )
		AddChild( controls )
		
		SetPosition( vpRect.Center.x, vpRect.Center.y, 0.0 )
		
		SetAlpha( 0 )
		DoFadeIn( )
		
		if( isLocal )
			SetupLocalBox( )
		else
			SetupRemoteBox( )
	}
	
	function SetupLocalBox( )
	{
		::LocalGlobalDialogBoxObject = AddToGlobalSystem( 
			::LocalGlobalDialogBoxObject, ::LocalGlobalDialogBoxQueue )
	}
	
	function SetupRemoteBox( )
	{
		Invisible = true
		::RemoteGlobalDialogBoxObject = AddToGlobalSystem( 
			::RemoteGlobalDialogBoxObject, ::RemoteGlobalDialogBoxQueue )
	}
	
	function AddToGlobalSystem( obj, queue )
	{
		// Push the current box onto the queue
		if( obj && !obj.fadingOut )
		{	
			queue.push( obj )
			::GameApp.WarningCanvas.RemoveChild( obj )
		}
		
		// Set myself as the current
		::GameApp.WarningCanvas.AddChild( this )
		return this;
	}
	
	function ShutdownLocalBox( )
	{
		::LocalGlobalDialogBoxObject = 
			RemoveFromGlobalSystem( 
				::LocalGlobalDialogBoxObject,
				::LocalGlobalDialogBoxQueue )
	}
	
	function ShutdownRemoteBox( )
	{
		::RemoteGlobalDialogBoxObject = 
			RemoveFromGlobalSystem( 
				::RemoteGlobalDialogBoxObject,
				::RemoteGlobalDialogBoxQueue )
	}
	
	function RemoveFromGlobalSystem( obj, queue )
	{
		// Pop the stack of errors upward
		if( obj == this )
		{
			if( queue.len( ) > 0 )
			{
				local next = queue[ queue.len( ) - 1 ]
				queue.pop( )
			
				next.DoFadeIn( )
				::GameApp.WarningCanvas.AddChild( next )
				
				return next
			}
			else
				return null
		}
		
		return obj
	}
	
	function DoFadeIn( )
	{
		FadeInAnd( DialogFadeTime, function( canvas ) { canvas.noInput = false } )
		
		if( wantsPaused )
			DoPause( )
	}
	
	function DoPause( )
	{
		if( !::GameApp.GameMode.IsFrontEnd && !::GameApp.Paused( ) )
		{
			::GameApp.Pause( true, null )
			didPause = true
		}
		
		wantsPaused = true
	}
	
	function ReleasePads( )
	{
		if( allPadsList != null )
		{
			foreach( pad in allPadsList )
				pad.Release( )
			allPadsList = null
		}
		
		checkPadsList = null
		filteredPad = null
	}
	
	function ClearGamerTagDisplay( )
	{
		if( gamerPicture )
			RemoveChild( gamerPicture )
			
		if( gamerTag )
			RemoveChild( gamerTag )
	}
	
	function HandleCanvasEvent( event )
	{
		//if( event.Id == SESSION_INVITE_ACCEPTED )
		//{
		//	::print( "Dialog deleted because of invite accepted" )
		//	onCanceled = null
		//	onFadedOut = null
		//	FadeOutAnd( 0.5, FadeOutProc.bindenv( this ) )
		//	return true
		//}
		return false
	}
	
	function TestButton( button )
	{
		if( !checkAllPads )
		{
			//::print( "Checking 1 input: " + ( filteredPad.IsNull( ) ? "null gamepad" : "valid gamepad" ) )
			return filteredPad.Get( ).ButtonDown( button )
		}
		else
		{
			foreach( pad in checkPadsList )
			{
				//::print( "Checking multi input: " + ( pad.IsNull( ) ? "null gamepad" : "valid gamepad" ) )
				if( pad.Get( ).ButtonDown( button ) )
					return true
			}
			
			return false
		}
	}
	
	function OnTick( dt )
	{
		::AnimatingCanvas.OnTick( dt )
		
		if( allPadsList && !noInput )
		{
			local buttonDown = TestButton( GAMEPAD_BUTTON_A )
				
			if( buttonDown )
			{
				::GameApp.AudioEvent( "Play_UI_Select_Forward" )
				if( onAPress )
					onAPress( )
					
				noInput = true
			}
				
			if( showCancel && !buttonDown )
			{
				buttonDown = TestButton( cancelButtonId )
				
				if( buttonDown )
				{
					::GameApp.AudioEvent( "Play_UI_Select_Backward" )
					if( onBPress )
						onBPress( )
					canceled = true
					noInput = true
				}
			}
			
			if( cancelExiting )
			{
				cancelExiting = false
				canceled = false
				noInput = false
				buttonDown = false
			}
			else if( buttonDown )
			{
				DoFadeOut( )
			}
		}
	}
	
	function DoFadeOut( )
	{		
		if( didPause )
			::GameApp.Pause( false, null )
		fadingOut = true
		FadeOutAnd( DialogFadeTime, FadeOutProc.bindenv( this ) )
	}
	
	function FadeOutProc( canvas )
	{
		ReleasePads( )
		DeleteSelf( )
				
		if( canceled )
		{
			if( onCanceled )
				onCanceled( )
		}
		else
		{
			if( onFadedOut )
				onFadedOut( )
		}
		
		// Fix up the global system withou our death
		if( isLocal )
			ShutdownLocalBox( )
		else
			ShutdownRemoteBox( )
	}
}

class ModalInfoBox extends GlobalModalDialogBox
{
	constructor( textId, user, acceptText = "Ok" )
	{
		::GlobalModalDialogBox.constructor( textId, user, acceptText )
	}
}

class ModalConfirmationBox extends GlobalModalDialogBox
{
	constructor( textId, user, acceptText = "Ok", cancelText = "Cancel", cancelButton = GAMEPAD_BUTTON_B )
	{
		::GlobalModalDialogBox.constructor( textId, user, acceptText, cancelText, cancelButton )
	}
}

class ModalErrorBox extends GlobalModalDialogBox
{
	constructor( textId, user, acceptText = "Ok" )
	{
		::GlobalModalDialogBox.constructor( textId, user, acceptText )
		
		// Error boxes accept any users input
		checkAllPads = true
	}
}

function CanvasCreateModalDialogBox( locId, user, data, pause, acceptAny )
{
	local dialog = null
	if( data.CanCancel )
		dialog = ::ModalConfirmationBox( locId, user )
	else
		dialog = ::ModalErrorBox( locId, user )
	
	if( acceptAny )
	{
		// accept input from any physical user. circumvents the internal "all users" system in the dialog. 
		// this one really means it. for super bad app-start errors when no user may be logged in.
		dialog.checkAllPads = true
		
		local userCount = ::GameApp.LocalUserCount
		for( local u = 0; u < userCount; ++u )
		{
			local locUser = ::GameApp.GetLocalUser( u )
			local pad = ::FilteredGamepad( locUser, true, true )
			dialog.checkPadsList.push( pad )
			dialog.allPadsList.push( pad )
		}
	}
	
	if( data.CanCancel )
		dialog.onBPress = function( ):( data, user, dialog ) { if( !::GameApp.ErrorDialogConfirm( data.Value, 0, user ) ) dialog.cancelExiting = true }
		
	dialog.onAPress = function( ):( data, user, dialog ) { if( !::GameApp.ErrorDialogConfirm( data.Value, 1, user ) ) dialog.cancelExiting = true }
	
	if( pause )
		dialog.DoPause( )
}
