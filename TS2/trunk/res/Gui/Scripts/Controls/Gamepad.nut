
/* Buttons exposed from tGamepad.cpp

	GAMEPAD_BUTTON_START
	GAMEPAD_BUTTON_SELECT
	GAMEPAD_BUTTON_A
	GAMEPAD_BUTTON_B
	GAMEPAD_BUTTON_X
	GAMEPAD_BUTTON_Y
	GAMEPAD_BUTTON_DPAD_RIGHT
	GAMEPAD_BUTTON_DPAD_UP
	GAMEPAD_BUTTON_DPAD_LEFT
	GAMEPAD_BUTTON_DPAD_DOWN
	GAMEPAD_BUTTON_LSHOULDER
	GAMEPAD_BUTTON_LTHUMB
	GAMEPAD_BUTTON_LTRIGGER
	GAMEPAD_BUTTON_RSHOULDER
	GAMEPAD_BUTTON_RTHUMB
	GAMEPAD_BUTTON_RTRIGGER
	
*/

/* Functions exposed from tGamepad.cpp

	LeftStick 	- tVec2f
	RightStick 	- tVec2f
	ButtonDown 	- takes button returns b32
	ButtonUp 	- takes button returns b32
	ButtonHeld 	- takes button returns b32

*/

class FilteredGamepad
{
	user = null
	filterID = 0
	doFilter = true
	warningFilter = false
	
	constructor( _user, _doFilter = true, _warningFilter = false )
	{
		doFilter = _doFilter
		warningFilter = _warningFilter
		user = null
		SetUser( _user )
	}
	function SetUser( _user )
	{
		if( user )
			Release( )
		
		if( _user )
		{
			user = _user
			if( doFilter )
			{
				filterID = warningFilter ? user.IncWarningInputFilterLevel( ) : user.IncInputFilterLevel( )
			}
			else
				filterID = user.InputFilterLevel
		}
	}
	function Release( )
	{
		if( user )
		{
			if( doFilter )
				user.DecInputFilterLevel( filterID )
			user = null
		}
	}
	function IsNull( ) 
	{
		return user == null
	}
	function Get( )
	{
		if( user )
			return user.FilteredGamepad( filterID )
		else
			return Input.Gamepad.NullGamepad( )
	}
}

class UnFilteredGamepad
{
	user = null
	filterID = 0
	constructor( _user )
	{
		user = _user
	}
	function Release( )
	{
		if( filterID != 99999 )
		{
			filterID = 99999
		}
	}
	function IsNull( ) 
	{
		return user == null
	}
	function Get( )
	{
		return user.RawGamepad( filterID )
	}
	function SetUser( _user )
	{
		user = _user
	}
}

