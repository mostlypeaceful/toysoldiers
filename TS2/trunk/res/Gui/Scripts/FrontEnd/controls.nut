// Static controls screen

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/frontend/settings.nut"

// Resources
sigimport "gui/textures/score/score_decoration_g.png"

// Global Data
ControlsButtonPositionTable <- { // Positions relative to the controller image position
	[ GAMEPAD_BUTTON_START ] 		= ::Math.Vec3.Construct( 196, 239, 0 ),
	[ GAMEPAD_BUTTON_SELECT ] 		= ::Math.Vec3.Construct( 115, 240, 0 ),
	[ GAMEPAD_BUTTON_A ] 			= ::Math.Vec3.Construct( 255, 263, 0 ),
	[ GAMEPAD_BUTTON_B ] 			= ::Math.Vec3.Construct( 279, 239, 0 ),
	[ GAMEPAD_BUTTON_X ] 			= ::Math.Vec3.Construct( 232, 238, 0 ),
	[ GAMEPAD_BUTTON_Y ] 			= ::Math.Vec3.Construct( 254, 210, 0 ),
	[ GAMEPAD_BUTTON_DPAD_RIGHT ] 	= ::Math.Vec3.Construct( 125, 293, 0 ),
	[ GAMEPAD_BUTTON_DPAD_UP ] 		= ::Math.Vec3.Construct( 107, 273, 0 ),
	[ GAMEPAD_BUTTON_DPAD_LEFT ] 	= ::Math.Vec3.Construct( 89, 292, 0 ),
	[ GAMEPAD_BUTTON_DPAD_DOWN ] 	= ::Math.Vec3.Construct( 107, 309, 0 ),
	[ GAMEPAD_BUTTON_LSHOULDER ] 	= ::Math.Vec3.Construct( 39, 64, 0 ),
	[ GAMEPAD_BUTTON_LTHUMB ] 		= ::Math.Vec3.Construct( 42, 234, 0 ),
	[ GAMEPAD_BUTTON_LTRIGGER ] 	= ::Math.Vec3.Construct( 65, 19, 0 ),
	[ GAMEPAD_BUTTON_RSHOULDER ] 	= ::Math.Vec3.Construct( 272, 64, 0 ),
	[ GAMEPAD_BUTTON_RTHUMB ] 		= ::Math.Vec3.Construct( 220, 295, 0 ),
	[ GAMEPAD_BUTTON_RTRIGGER ] 	= ::Math.Vec3.Construct( 247, 19, 0 )
}

ControlsButtonTextPositionTable <- { // Positions relative to the controller image position
	[ GAMEPAD_BUTTON_START ] 		= ::Math.Vec3.Construct( 332, 330, 0 ),
	[ GAMEPAD_BUTTON_SELECT ] 		= ::Math.Vec3.Construct( -20, 270, 0 ),
	[ GAMEPAD_BUTTON_A ] 			= ::Math.Vec3.Construct( 332, 285, 0 ),
	[ GAMEPAD_BUTTON_B ] 			= ::Math.Vec3.Construct( 332, 240, 0 ),
	[ GAMEPAD_BUTTON_X ] 			= ::Math.Vec3.Construct( 332, 180, 0 ),
	[ GAMEPAD_BUTTON_Y ] 			= ::Math.Vec3.Construct( 332, 145, 0 ),
	[ GAMEPAD_BUTTON_DPAD_RIGHT ] 	= ::Math.Vec3.Construct( -20, 370, 0 ),
	[ GAMEPAD_BUTTON_DPAD_UP ] 		= ::Math.Vec3.Construct( -20, 310, 0 ),
	[ GAMEPAD_BUTTON_DPAD_LEFT ] 	= ::Math.Vec3.Construct( -20, 340, 0 ),
	[ GAMEPAD_BUTTON_DPAD_DOWN ] 	= ::Math.Vec3.Construct( -20, 400, 0 ),
	[ GAMEPAD_BUTTON_LSHOULDER ] 	= ::Math.Vec3.Construct( -20, 65, 0 ),
	[ GAMEPAD_BUTTON_LTHUMB ] 		= ::Math.Vec3.Construct( -20, 200, 0 ),
	[ GAMEPAD_BUTTON_LTRIGGER ] 	= ::Math.Vec3.Construct( -20, -5, 0 ),
	[ GAMEPAD_BUTTON_RSHOULDER ] 	= ::Math.Vec3.Construct( 332, 65, 0 ),
	[ GAMEPAD_BUTTON_RTHUMB ] 		= ::Math.Vec3.Construct( 332, 375, 0 ),
	[ GAMEPAD_BUTTON_RTRIGGER ] 	= ::Math.Vec3.Construct( 332, -5, 0 )
}

ControlsButtonTextAlignmentTable <- {
	[ GAMEPAD_BUTTON_START ] 		= TEXT_ALIGN_LEFT,
	[ GAMEPAD_BUTTON_SELECT ] 		= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_A ] 			= TEXT_ALIGN_LEFT,
	[ GAMEPAD_BUTTON_B ] 			= TEXT_ALIGN_LEFT,
	[ GAMEPAD_BUTTON_X ] 			= TEXT_ALIGN_LEFT,
	[ GAMEPAD_BUTTON_Y ] 			= TEXT_ALIGN_LEFT,
	[ GAMEPAD_BUTTON_DPAD_RIGHT ] 	= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_DPAD_UP ] 		= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_DPAD_LEFT ] 	= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_DPAD_DOWN ] 	= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_LSHOULDER ] 	= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_LTHUMB ] 		= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_LTRIGGER ] 	= TEXT_ALIGN_RIGHT,
	[ GAMEPAD_BUTTON_RSHOULDER ] 	= TEXT_ALIGN_LEFT,
	[ GAMEPAD_BUTTON_RTHUMB ] 		= TEXT_ALIGN_LEFT,
	[ GAMEPAD_BUTTON_RTRIGGER ] 	= TEXT_ALIGN_LEFT
}

ControlsLabelLocStringTable <- {
	[ CONTROLS_CAMERA ] = "Controls_Camera",
	[ CONTROLS_TURRETS ] = "Controls_Turrets",
	[ CONTROLS_SHELLCAM ] = "Controls_ShellCam",
	[ CONTROLS_PLANES ] = "Controls_Planes",
	[ CONTROLS_VEHICLES ] = "Controls_Vehicles",
	[ CONTROLS_CHARACTER ] = "Controls_Character",
}

ControlsSettingLocStringTable <- {
	[ CONTROLS_SETTING_NORMAL ] = "Controls_Normal",
	[ CONTROLS_SETTING_SOUTHPAW ] = "Controls_Southpaw",
	[ CONTROLS_SETTING_INVERTED ] = "Controls_Inverted",
	[ CONTROLS_SETTING_NORMALSOUTHPAW ] = "Controls_NormalSouthpaw",
	[ CONTROLS_SETTING_INVERTEDSOUTHPAW ] = "Controls_InvertedSouthpaw"
}

ControlsOptionsTable <- {
	[ CONTROLS_CAMERA ] = [ CONTROLS_SETTING_NORMAL, CONTROLS_SETTING_INVERTED, CONTROLS_SETTING_NORMALSOUTHPAW, CONTROLS_SETTING_INVERTEDSOUTHPAW ],
	[ CONTROLS_TURRETS ] = [ CONTROLS_SETTING_NORMAL, CONTROLS_SETTING_INVERTED, CONTROLS_SETTING_NORMALSOUTHPAW, CONTROLS_SETTING_INVERTEDSOUTHPAW ],
	[ CONTROLS_PLANES ] = [ CONTROLS_SETTING_NORMAL, CONTROLS_SETTING_INVERTED, CONTROLS_SETTING_NORMALSOUTHPAW, CONTROLS_SETTING_INVERTEDSOUTHPAW ],
	[ CONTROLS_SHELLCAM ] = [ CONTROLS_SETTING_NORMAL, CONTROLS_SETTING_INVERTED, CONTROLS_SETTING_NORMALSOUTHPAW, CONTROLS_SETTING_INVERTEDSOUTHPAW ],
	[ CONTROLS_VEHICLES ] = [ CONTROLS_SETTING_NORMAL, CONTROLS_SETTING_INVERTED, CONTROLS_SETTING_NORMALSOUTHPAW, CONTROLS_SETTING_INVERTEDSOUTHPAW ],
	[ CONTROLS_CHARACTER ] = [ CONTROLS_SETTING_NORMAL, CONTROLS_SETTING_INVERTED, CONTROLS_SETTING_NORMALSOUTHPAW, CONTROLS_SETTING_INVERTEDSOUTHPAW ],
}

ControlsMasterTable <- {
	[ CONTROLS_CAMERA ] = {
		[ CONTROLS_SETTING_NORMAL ] = {
			[ GAMEPAD_BUTTON_START ] 		= "Controls_Pause",
			[ GAMEPAD_BUTTON_SELECT ] 		= "Controls_Rewind",
			[ GAMEPAD_BUTTON_A ] 			= "Controls_Select",
			[ GAMEPAD_BUTTON_B ] 			= "Controls_Cancel",
			[ GAMEPAD_BUTTON_X ] 			= [ "Controls_LaunchWave", "Controls_OffensiveWaveLaunch" ],
			[ GAMEPAD_BUTTON_Y ] 			= "Controls_UseBarrage",
			[ GAMEPAD_BUTTON_DPAD_RIGHT ] 	= "Controls_QuickUpgrade",
			[ GAMEPAD_BUTTON_DPAD_UP ] 		= "Controls_QuickUse",
			[ GAMEPAD_BUTTON_DPAD_LEFT ] 	= "Controls_QuickSell",
			[ GAMEPAD_BUTTON_DPAD_DOWN ] 	= "Controls_QuickRepair",
			[ GAMEPAD_BUTTON_LSHOULDER ] 	= "Controls_RotateUnitLeft",
			[ GAMEPAD_BUTTON_LTHUMB ] 		= [ "Controls_MoveCursor", "Controls_BarbedWire" ],
			[ GAMEPAD_BUTTON_LTRIGGER ] 	= "Controls_FastCamera",
			[ GAMEPAD_BUTTON_RSHOULDER ] 	= "Controls_RotateUnitRight",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= [ "Controls_RotateZoom", "Controls_StrategicCam" ],
			[ GAMEPAD_BUTTON_RTRIGGER ] 	= "Controls_TurretPlacementMenu"
		},
		[ CONTROLS_SETTING_SOUTHPAW ] = {
			[ GAMEPAD_BUTTON_LTHUMB ] 		= [ "Controls_RotateZoom", "Controls_StrategicCam" ],
			[ GAMEPAD_BUTTON_RTHUMB ] 		= [ "Controls_MoveCursor", "Controls_BarbedWire" ],
		}
	},
	[ CONTROLS_TURRETS ] = {
		[ CONTROLS_SETTING_NORMAL ] = {
			[ GAMEPAD_BUTTON_B ] 			= "Controls_ExitTurret",
			[ GAMEPAD_BUTTON_DPAD_LEFT ] 	= "Controls_SwitchTurrets",
			[ GAMEPAD_BUTTON_LSHOULDER ] 	= "Controls_RotateUnitLeft",
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_ChangeCameraAngle",
			[ GAMEPAD_BUTTON_LTRIGGER ] 	= [ "Controls_Zoom", "Controls_FireSecondary" ],
			[ GAMEPAD_BUTTON_RSHOULDER ] 	= "Controls_RotateUnitRight",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTRIGGER ] 	= [ "Controls_ShellCamLabel", "Controls_FireMain" ]
		},
		[ CONTROLS_SETTING_SOUTHPAW ] = {
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_ChangeCameraAngle",
		}
	},
	[ CONTROLS_PLANES ] = {
		[ CONTROLS_SETTING_NORMAL ] = {
			[ GAMEPAD_BUTTON_B ] 			= "Controls_ExitVehicle",
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_PlaneSpeed",
			[ GAMEPAD_BUTTON_LTRIGGER ] 	= [ "Controls_AdvancedTargeting", "Controls_LaunchMissiles" ],
			[ GAMEPAD_BUTTON_RTHUMB ] 		= [ "Controls_SteerAim", "Controls_BombingCamera" ],
			[ GAMEPAD_BUTTON_RTRIGGER ] 	= [ "Controls_DropBomb", "Controls_FireGuns" ], 
		},
		[ CONTROLS_SETTING_SOUTHPAW ] = {
			[ GAMEPAD_BUTTON_LTHUMB ] 		= [ "Controls_SteerAim", "Controls_BombingCamera" ],
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_PlaneSpeed",
		}
	},
	[ CONTROLS_VEHICLES ] = {
		[ CONTROLS_SETTING_NORMAL ] = {
			[ GAMEPAD_BUTTON_B ] 			= "Controls_ExitVehicle",
			[ GAMEPAD_BUTTON_LSHOULDER ] 	= "Controls_AltitudeDown",
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_Move",
			[ GAMEPAD_BUTTON_LTRIGGER ] 	= "Controls_FireSecondary",
			[ GAMEPAD_BUTTON_RSHOULDER ] 	= "Controls_AltitudeUp",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTRIGGER ] 	= "Controls_FireMain"
		},
		[ CONTROLS_SETTING_SOUTHPAW ] = {
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_Move",
		}
	},
	[ CONTROLS_SHELLCAM ] = {
		[ CONTROLS_SETTING_NORMAL ] = {
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_PlaneSpeed",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTRIGGER ] 	= "Controls_ExitShellCam"
		},
		[ CONTROLS_SETTING_SOUTHPAW ] = {
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_PlaneSpeed",
		}
	},
	[ CONTROLS_CHARACTER ] = {
		[ CONTROLS_SETTING_NORMAL ] = {
			[ GAMEPAD_BUTTON_A ] 			= "Controls_Jump",
			[ GAMEPAD_BUTTON_B ] 			= "Controls_ExitUnit",
			[ GAMEPAD_BUTTON_LTHUMB ] 		= [ "Controls_Move", "Controls_Sprint" ],
			[ GAMEPAD_BUTTON_LTRIGGER ] 	= "Controls_FireSecondary",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTRIGGER ] 	= "Controls_FireMain"
		},
		[ CONTROLS_SETTING_SOUTHPAW ] = {
			[ GAMEPAD_BUTTON_LTHUMB ] 		= "Controls_Aim",
			[ GAMEPAD_BUTTON_RTHUMB ] 		= [ "Controls_Move", "Controls_Sprint" ],
		}
	},
}

class ControlsDisplayItem extends AnimatingCanvas
{
	// Display
	text = null
	line = null
	line2 = null
	
	// Data
	button = null
	align = null
	lineColor = null
	
	// Statics
	static boxWidth = 300
	
	constructor( button_ )
	{
		::AnimatingCanvas.constructor( )
		button = button_
		align = ::ControlsButtonTextAlignmentTable[ button ]
		lineColor = COLOR_CLEAN_WHITE//::Math.Vec4.Construct( 0, 0, 0, 1 )
		
		// Create Text
		text = ::Gui.Text( )
		text.SetFontById( FONT_SIMPLE_SMALL )
		text.SetRgba( COLOR_CLEAN_WHITE )
		if( align == TEXT_ALIGN_RIGHT )
			text.SetPosition( -boxWidth, 0, 0 )
		AddChild( text )
		
		// Create lines
		line = ::Gui.LineList( )
		AddChild( line )
		
		line2 = ::Gui.LineList( )
		AddChild( line2 )
	}
	
	function SetText( mode, setting )
	{
		local hide = false
		
		// Set Text
		if( mode in ::ControlsMasterTable )
		{
			local modeTable = ::ControlsMasterTable[ mode ]
			local settingTable = modeTable[ setting ]
			if( !(button in settingTable) )
				settingTable = modeTable[ CONTROLS_SETTING_NORMAL ]
			
			if( button in settingTable )
			{
				local key = settingTable[ button ]
				local str = null
				if( typeof key == "string" )
					str = ::GameApp.LocString( key )
				else if( typeof key == "array" )
				{
					foreach( i, k in key )
					{
						if( i == 0 )
							str = ::GameApp.LocString( k ).Clone( )
						else
							str.JoinWithLocString( ::GameApp.LocString( k ) )
						if( i != key.len( ) - 1 )
							str.JoinWithCString( "\n" )
					}
				}
				
				text.BakeBoxLocString( boxWidth, str, align )
			}
			else
				hide = true
		}
		else
			hide = true
		
		// Set Line
		local end = ::ControlsButtonPositionTable[ button ] - ::ControlsButtonTextPositionTable[ button ]
		local lineMag = 4
		local lineX = ( ( align == TEXT_ALIGN_LEFT )? -lineMag: lineMag )
		local start = ::Math.Vec3.Construct( lineX, text.Height * 0.5, 0 )
		line.Line( start, end, lineColor )
		
		line2.Line( ::Math.Vec3.Construct( lineX, lineMag + 2, 0 ), ::Math.Vec3.Construct( lineX, text.Height - lineMag + 1, 0 ), lineColor )
		
		if( hide )
			Invisible = true
		else
			Invisible = false
	}
}

class FrontEndControlsMenu extends BaseSettingsMenu
{
	// Display
	labels = null
	controllerImage = null
	player = null
	
	function FinalizeIconSetup( )
	{
		SetTitle( "Menus_Controls" )
		
		secondaryControls.Clear( )
		secondaryControls.AddControl( "gui/textures/gamepad/button_x_g.png", "Menus_RestoreDefaults" )
		controls.Clear( )
		controls.AddControl( "gui/textures/gamepad/button_lstick_g.png", "Menus_ChangeSetting" )
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
				
		local rect = ::GameApp.FrontEndPlayer.User.ComputeScreenSafeRect( )
		local wideLanguage = ::GameApp.IsWideLanguage( )
		
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		// Controller Image
		local pos = ::Math.Vec3.Construct( rect.Center.x, rect.Top + 100, 0.01 )
		controllerImage = ::Gui.AsyncTexturedQuad( )
		controllerImage.SetPosition( pos )
		controllerImage.SetTexture( "gui/textures/frontend/controls/controller_g.png" )
		AddChild( controllerImage )
		
		// Control decorations
		menuPositionOffset = ::Math.Vec3.Construct( 100, rect.Top + ( wideLanguage ? 200 : 100 ), 0 )
		local dec1 = ::Gui.TexturedQuad( )
		dec1.SetTexture( "gui/textures/score/score_decoration_g.png" )
		dec1.SetPosition( menuPositionOffset.x, menuPositionOffset.y - 4, 0 )
		AddChild( dec1 )
		
		local dec2 = ::Gui.TexturedQuad( )
		dec2.SetTexture( "gui/textures/score/score_decoration_g.png" )
		dec2.SetPosition( menuPositionOffset.x, menuPositionOffset.y + ( wideLanguage ? 306 : 163 ), 0 )
		AddChild( dec2 )
		
		// Control Options
		if( user )
			player = ::GameApp.GetPlayerByUser( user )
		else
			player = ::GameApp.FrontEndPlayer
		local profile = player.GetUserProfile( )
		local current = {
			[ CONTROLS_CAMERA ] = GetUserStartingIndex( CONTROLS_CAMERA, profile ),
			[ CONTROLS_TURRETS ] = GetUserStartingIndex( CONTROLS_TURRETS, profile ),
			[ CONTROLS_SHELLCAM ] = GetUserStartingIndex( CONTROLS_SHELLCAM, profile ),
			[ CONTROLS_PLANES ] = GetUserStartingIndex( CONTROLS_PLANES, profile ),
			[ CONTROLS_VEHICLES ] = GetUserStartingIndex( CONTROLS_VEHICLES, profile ),
			[ CONTROLS_CHARACTER ] = GetUserStartingIndex( CONTROLS_CHARACTER, profile ),
		}
		local yInvertedDefault = ( user.YAxisInvertedDefault? 1 : 0 )
		local southpawDefault = ( user.SouthpawDefault? 1 : 0 )
		local defaults = {
			[ CONTROLS_CAMERA ] = SettingToOptionIndex( CONTROLS_CAMERA, FlagsToSetting( yInvertedDefault, southpawDefault ) ),
			[ CONTROLS_TURRETS ] = SettingToOptionIndex( CONTROLS_TURRETS, FlagsToSetting( yInvertedDefault, southpawDefault ) ),
			[ CONTROLS_SHELLCAM ] = SettingToOptionIndex( CONTROLS_SHELLCAM, FlagsToSetting( yInvertedDefault, southpawDefault ) ),
			[ CONTROLS_PLANES ] = SettingToOptionIndex( CONTROLS_PLANES, FlagsToSetting( yInvertedDefault, southpawDefault ) ),
			[ CONTROLS_VEHICLES ] = SettingToOptionIndex( CONTROLS_VEHICLES, FlagsToSetting( yInvertedDefault, southpawDefault ) ),
			[ CONTROLS_CHARACTER ] = SettingToOptionIndex( CONTROLS_CHARACTER, FlagsToSetting( yInvertedDefault, southpawDefault ) ),
		}
		
		foreach( key, value in ::ControlsOptionsTable )
		{
			local spacing = ( wideLanguage ? 170 : 190 )
			local valueWidth = ( wideLanguage ? 210 : 180 )
			local options = [ ]
			foreach( option in value )
				options.push( ::ControlsSettingLocStringTable[ option ] )
			local settingItem = AddSettingChoice( key, ::ControlsLabelLocStringTable[ key ], options, current[ key ], defaults[ key ], spacing, valueWidth )
			settingItem.SetMultiline( wideLanguage )
		}
		
		// Control Labels
		labels = [ ]
		foreach( button, offset in ::ControlsButtonPositionTable )
		{
			local textPos = ::ControlsButtonTextPositionTable[ button ]
			local label = ::ControlsDisplayItem( button )
			label.SetPosition( pos + textPos )
			label.SetZPos( 0 )
			AddChild( label )
			labels.push( label )
		}
		
		SetLabels( 0, current[ 0 ] )

		::BaseSettingsMenu.FinalizeIconSetup( )
		
		ChangeHighlight( 0 )
	}
	
	function OnChanged( key, index )
	{
		SetLabels( key, index )
	}
	
	function ChangeHighlight( indexDelta )
	{
		::BaseSettingsMenu.ChangeHighlight( indexDelta )
		SetLabels( highlightIndex, icons[ highlightIndex ].CurrentIndex( ) )
		local desc = [ "Controls_Cursor_Desc", "Controls_Turrets_Desc", "Controls_ShellCam_Desc", "Controls_Fighters_Desc", "Controls_Vehicles_Desc", "Controls_Soldiers_Desc" ]
		SetDescriptorText( desc[ highlightIndex ] )
	}
	
	function SetLabels( mode, setting )
	{
		local trueSetting = null
		if( ::ControlsOptionsTable[ mode ][ setting ] == CONTROLS_SETTING_NORMAL || ::ControlsOptionsTable[ mode ][ setting ] == CONTROLS_SETTING_INVERTED )
			trueSetting = CONTROLS_SETTING_NORMAL
		else
			trueSetting = CONTROLS_SETTING_SOUTHPAW
			
		foreach( label in labels )
			label.SetText( mode, trueSetting )
	}
	
	function UnloadResources( )
	{
		RemoveChild( controllerImage )
		controllerImage.Unload( )
	}
	
	function SaveSettings( )
	{
		local profile = player.GetUserProfile( )
		
		foreach( mode, option in currentSettings )
		{
			local setting = option.CurrentIndex( )
			local southpaw = ( ( setting >= 2 )? 1: 0 )
			local inverted = ( ( setting == 1 || setting == 3 )? 1: 0 )
			
			local flag1 = SettingToFlags( mode, 0 )
			local flag2 = SettingToFlags( mode, 1 )
			profile.SetSetting( flag1, inverted )
			profile.SetSetting( flag2, southpaw )
		}
		
		player.SaveProfile( )
	}
	
	function SettingToFlags( mode, flag ) // inversion == 0, southpaw = 1
	{
		return mode * 2 + flag
	}
	
	function FlagsToSetting( inversion, southpaw )
	{
		if( inversion == 1 )
			return ( ( southpaw == 1 )? CONTROLS_SETTING_INVERTEDSOUTHPAW: CONTROLS_SETTING_INVERTED )
		else
			return ( ( southpaw == 1 )? CONTROLS_SETTING_NORMALSOUTHPAW: CONTROLS_SETTING_NORMAL )
	}
	
	function SettingToOptionIndex( mode, setting )
	{
		foreach( i, s in ::ControlsOptionsTable[ mode ] )
		{
			if( s == setting )
				return i
		}
		return null
	}
	
	function GetUserStartingIndex( mode, profile )
	{
		return SettingToOptionIndex( mode, FlagsToSetting( profile.GetSetting( SettingToFlags( mode, 0 ) ), profile.GetSetting( SettingToFlags( mode, 1 ) ) ) )
	}
}
