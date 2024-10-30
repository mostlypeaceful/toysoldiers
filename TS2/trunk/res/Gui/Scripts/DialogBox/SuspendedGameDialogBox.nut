// Suspended game dialog box

// Requires
sigimport "gui/scripts/dialogbox/globalmodaldialogbox.nut"

class SuspendedGameDialogBox extends ModalConfirmationBox
{
	reallyCanceled = null
	
	constructor( user )
	{
		::ModalConfirmationBox.constructor( 
			"LevelSelect_SuspendedNotice", 
			user, 
			"LevelSelect_SuspendedContinue", 
			"LevelSelect_SuspendedRestart", 
			GAMEPAD_BUTTON_X )
		
		// Setup B Cancel button
		controls.AddControl( GAMEPAD_BUTTON_B, "Menus_Back" )
		controls.SetXPos( -controls.Size.x * 0.5 )
		
		reallyCanceled = false
	}
	
	function OnTick( dt )
	{
		::ModalConfirmationBox.OnTick( dt )
		
		if( allPadsList && !noInput )
		{
			local buttonDown = TestButton( GAMEPAD_BUTTON_B )
			if( buttonDown )
			{
				::GameApp.AudioEvent( "Play_UI_Select_Backward" )
				reallyCanceled = true
				noInput = true
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
				FadeOutAnd( 0.5, FadeOutProc.bindenv( this ) )
			}
		}
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
		else if( !reallyCanceled )
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