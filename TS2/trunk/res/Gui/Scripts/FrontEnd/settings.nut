// Allows the user to change their settings

// Requires
sigimport "gui/scripts/frontend/staticscreens/staticscreen.nut"
sigimport "gui/scripts/controls/animatingcanvas.nut"
sigimport "gui/scripts/controls/slider.nut"

// Resources
sigimport "gui/textures/frontend/settings/slider_background_g.png"
sigimport "gui/textures/frontend/settings/slider_mark_g.png"
sigimport "gui/textures/frontend/settings/slider_gradient_g.png"
sigimport "gui/textures/frontend/settings/brightness_test_d.png"
sigimport "gui/textures/frontend/settings/frame_top_g.png"
sigimport "gui/textures/frontend/settings/frame_middle_g.png"
sigimport "gui/textures/frontend/settings/frame_bottom_g.png"

class SettingItemBase extends BaseMenuIcon
{
	// Data
	onChange = null // Function
	defaultValue = null
	
	constructor( def, changeCB )
	{
		::BaseMenuIcon.constructor( null, null )
		SetActiveInactiveScale( ::Math.Vec2.Construct( 1.0, 1.0 ), ::Math.Vec2.Construct( 1.0, 1.0 ) )
		defaultValue = def
		onChange = changeCB
	}
	
	function CurrentIndex( )
	{
		return -1
	}
	
	function RestoreDefault( )
	{
		return false
	}
}

class SettingItem_Choice extends SettingItemBase
{
	// Display
	nameText = null // Gui.Text
	textChoice = null // TextChoice
	
	constructor( name, options, initial = 0, def = 0, changeCB = null, spacing = null, choiceWidth = null, looping = null, audioSource = null )
	{
		::SettingItemBase.constructor( def, changeCB )

		spacing = ( ( spacing == null )? 300: spacing )
		choiceWidth = ( ( choiceWidth == null )? 150: choiceWidth )
		
		nameText = ::Gui.Text( )
		nameText.SetFontById( FONT_SIMPLE_SMALL )
		nameText.SetRgba( ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ) )
		nameText.BakeLocString( ::GameApp.LocString( name ) )
		nameText.SetPosition( 0, 0, 0 )
		AddChild( nameText )
		
		textChoice = ::TextChoice( options, initial, -1, true, choiceWidth, looping )
		textChoice.audioSource = audioSource
		textChoice.SetPosition( spacing, nameText.Height * 0.5, 0 )
		AddChild( textChoice )
		
		OnHighlight( false )
	}
	
	function OnHighlight( active )
	{
		// Set Color
		nameText.SetRgba( active? ::Math.Vec4.Construct( 1.0, 1.0, 1.0, 1.0 ): ::Math.Vec4.Construct( 0.5, 0.5, 0.5, 1.0 ) )
		textChoice.ShowArrows( active )
	}
	
	function ChangeHorizontalHighlight( delta )
	{
		local success = textChoice.Shift( delta )
		if( success && onChange )
			onChange( textChoice.currentIndex )
		return success
	}
	
	function SetIndex( index )
	{
		local success = textChoice.GoToIndex( index )
		if( success && onChange )
			onChange( textChoice.currentIndex )
		return success
	}
	
	function CurrentIndex( )
	{
		return textChoice.currentIndex
	}
	
	function RestoreDefault( )
	{
		if( defaultValue != textChoice.currentIndex )
		{
			textChoice.GoToIndexInstant( defaultValue )
			return true
		}
		return false
	}
	
	function SetLooping( looping )
	{
		textChoice.SetLooping( looping )
	}
	
	function SetMultiline( multiline )
	{
		textChoice.SetYPos( nameText.Height * 0.5 + ( ( multiline )? nameText.LineHeight: 0 ) )
	}
}

class SettingItem_Slider extends SettingItemBase
{
	// Display
	nameText = null
	valueSlider = null
	
	// Data
	currentValue = null
	
	// Statics
	static modifySpeed = 0.025
	
	constructor( name, initial = 0, def = 0, changeCB = null )
	{
		::SettingItemBase.constructor( def, changeCB )
		currentValue = initial

		valueSlider = ::Slider( "gui/textures/frontend/settings/slider_background_g.png", "gui/textures/frontend/settings/slider_gradient_g.png", "gui/textures/frontend/settings/slider_mark_g.png" )
		valueSlider.SetPosition( 0, 0, 0 )
		valueSlider.SetRgba( ::Math.Vec4.Construct( 0.5, 0.5, 0.5, 1.0 ) )
		AddChild( valueSlider )
		
		nameText = ::Gui.Text( )
		nameText.SetFontById( FONT_SIMPLE_SMALL )
		nameText.SetRgba( ::Math.Vec4.Construct( 0.5, 0.5, 0.5, 1.0 ) )
		nameText.BakeLocString( ::GameApp.LocString( name ), TEXT_ALIGN_LEFT )
		nameText.SetPosition( 0, valueSlider.LocalRect.Height, 0 )
		AddChild( nameText )
		
		UpdateSlider( )
	}
	
	function OnHighlight( active )
	{
		// Set Color
		nameText.SetRgba( active? COLOR_CLEAN_WHITE: ::Math.Vec4.Construct( 0.5, 0.5, 0.5, 1.0 ) )
		valueSlider.SetRgba( active? COLOR_CLEAN_WHITE: ::Math.Vec4.Construct( 0.5, 0.5, 0.5, 1.0 ) )
	}
	
	function UpdateSlider( )
	{
		//valueSlider.BakeCString( ::StringUtil.FloatToString( currentValue, 2 ), TEXT_ALIGN_CENTER )
		valueSlider.SetPercent( currentValue )
	}
	
	function Modify( delta ) // delta is [0:1] and expected to be modified
	{
		delta *= modifySpeed
		local prevValue = currentValue
		currentValue = ::Math.Clamp( currentValue + delta, 0.0, 1.0 )
		UpdateSlider( )
		
		if( onChange && prevValue != currentValue )
			onChange( currentValue )
	}
	
	function CurrentIndex( )
	{
		return currentValue
	}
	
	function RestoreDefault( )
	{
		currentValue = defaultValue
		UpdateSlider( )
		return true
	}
}

class BaseSettingsMenu extends FrontEndStaticScreen
{
	// Data
	hasChanged = null
	currentSettings = null
	dialog = null
	
	constructor( )
	{
		::FrontEndStaticScreen.constructor( )
		secondaryControls.AddControl( "gui/textures/gamepad/button_x_g.png", "Menus_RestoreDefaults" )
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
		inputHook = true
		hasChanged = false
		dialog = null
		currentSettings = { }
	}
	
	function AddSetting( key, settingItem )
	{
		settingItem.onChange = function( index ):(key)
		{
			OnChanged( key, index )
			hasChanged = true
		}.bindenv( this )
		
		AddChild( settingItem )
		icons.push( settingItem )
		currentSettings[ key ] <- settingItem
	}
	
	function Setting( key )
	{
		if( key in currentSettings )
			return currentSettings[ key ].CurrentIndex( )
		else
			return null
	}
	
	function AddSettingChoice( key, name, options, initial = 0, def = 0, spacing = null, choiceWidth = null )
	{
		local settingItem = ::SettingItem_Choice( name, options, initial, def, null, spacing, choiceWidth, audioSource )
		AddSetting( key, settingItem )
		return settingItem
	}
	
	function HandleInput( gamepad )
	{
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_X ) )
		{
			PlaySound( "Play_UI_Select_Forward" )
			RestoreDefaults( )
			return true
		}
		return false
	}
	
	function RestoreDefaults( )
	{
		local changed = false
		foreach( key, item in currentSettings )
		{
			local defaultRestored = item.RestoreDefault( )
			if( defaultRestored )
				OnChanged( key, item.CurrentIndex( ) )
			changed = ( defaultRestored || changed ) 
		}
		hasChanged = hasChanged || changed
	}
	
	function OnBackOut( )
	{
		PlaySound( "Play_UI_Select_Backward" )
		
		if( hasChanged )
		{
			if( !dialog )
			{
				// Show Dialog
				dialog = ::ModalConfirmationBox( 
					"Menus_SettingsWarning", 
					user, 
					"Menus_SettingsWarningSave", 
					"Menus_SettingsWarningCancel" )
				
				dialog.onBPress = function( )
				{
					CancelSettings( )
					AutoBackOut = true
					hasChanged = false
				}.bindenv( this )
				
				dialog.onAPress = function( )
				{
					SaveSettings( )
					hasChanged = false
					AutoBackOut = true
				}.bindenv( this )
			}
			
			return false
		}
		else
		{
			return true
		}
	}
	
	function SaveSettings( )
	{
		hasChanged = false
	}
	
	function CancelSettings( )
	{
	}
	
	function OnChanged( key, value )
	{
	}
}

class FrontEndSettingsMenu extends BaseSettingsMenu
{	
	// Data
	topHighlight = null
	player = null
	
	// Statics 
	static sliderSettings = 3
	static topSettings = 4
	
	function FinalizeIconSetup( )
	{
		SetTitle( "Menus_Settings" )
		menuPositionOffset = ::Math.Vec3.Construct( 300, 150, 0 )
		secondaryControls.Clear( )
		secondaryControls.AddControl( "gui/textures/gamepad/button_x_g.png", "Menus_RestoreDefaults" )
		controls.Clear( )
		controls.AddControl( "gui/textures/gamepad/button_lstick_g.png", "Menus_ChangeSetting" )
		controls.AddControl( "gui/textures/gamepad/button_b_g.png", "Menus_Back" )
		
		if( user )
			player = ::GameApp.GetPlayerByUser( user )
		else
			player = ::GameApp.FrontEndPlayer
			
		audioSource = player.AudioSource
		
		// Fade out bg
		local fade = ::Gui.ColoredQuad( )
		fade.SetRgba( 0.0, 0.0, 0.0, 0.5 )
		fade.SetRect( ::Math.Vec2.Construct( 1280, 720 ) )
		fade.SetPosition( 0, 0, 0.06 )
		AddChild( fade )
		
		// Frame
		local rect = player.User.ComputeScreenSafeRect( )
		local framePos = ::Math.Vec3.Construct( rect.Left, rect.Center.y - 200, 0.05 )
		local frameTop = ::Gui.TexturedQuad( )
		frameTop.SetTexture( "gui/textures/frontend/settings/frame_top_g.png" )
		frameTop.SetPosition( framePos )
		AddChild( frameTop )
		
		local frameMiddle = ::Gui.TexturedQuad( )
		frameMiddle.SetTexture( "gui/textures/frontend/settings/frame_middle_g.png" )
		frameMiddle.SetPosition( framePos.x + 346, framePos.y + 16, framePos.z )
		AddChild( frameMiddle )
		
		local frameBottom = ::Gui.TexturedQuad( )
		frameBottom.SetTexture( "gui/textures/frontend/settings/frame_bottom_g.png" )
		frameBottom.SetPosition( framePos.x, framePos.y + 468 - 16, framePos.z )
		AddChild( frameBottom )
		
		local topY = rect.Center.y - 200 + 40 - 4
		local leftX = rect.Left + 20
		local spacing = 80
		
		local profile = player.GetUserProfile( )
		local music = profile.MusicVolume
		local sfx = profile.SfxVolume
		local brightness = profile.Brightness
		local vibe = profile.GetSetting( SETTINGS_DISABLE_CONTROLLER_VIBE )
		local listeningMode = profile.ListeningMode
		
		// Music
		local musicSlider = ::SettingItem_Slider( "Settings_MusicVolume", music, 1.0 )
		AddSetting( "Settings_MusicVolume", musicSlider )
		musicSlider.SetPosition( leftX, topY, 0 )
		
		// Sound Effects
		local sfxSlider = ::SettingItem_Slider( "Settings_SoundEffectsVolume", sfx, 1.0 )
		AddSetting( "Settings_SoundEffectsVolume", sfxSlider )
		sfxSlider.SetPosition( leftX, topY + spacing, 0 )
		
		// Brightness
		local brightnessSlider = ::SettingItem_Slider( "Settings_Brightness", brightness, 0.3 )
		AddSetting( "Settings_Brightness", brightnessSlider )
		brightnessSlider.SetPosition( leftX, topY + 2 * spacing, 0 )
		
		local wideLanguage = ::GameApp.IsWideLanguage( )
		
		// Vibration
		local vibrationChoice = ::SettingItem_Choice( "Settings_Vibration", 
			[ "CustomMatch_Yes", "CustomMatch_No" ], vibe, 0, 
			null, 245, 120, audioSource )
		vibrationChoice.SetMultiline( wideLanguage )
		AddSetting( "Settings_Vibration", vibrationChoice )
		vibrationChoice.SetPosition( leftX, topY + 3 * spacing + 20, 0 )
		
		// Sound Options
		local filmGrainChoice = ::SettingItem_Choice( "Settings_ListeningMode", 
			[ "Settings_ListeningMode_Small", "Settings_ListeningMode_Medium", "Settings_ListeningMode_Large" ], listeningMode, 1, 
			null, 245, 120, audioSource )
		filmGrainChoice.SetMultiline( wideLanguage )
		AddSetting( "Settings_ListeningMode", filmGrainChoice )
		filmGrainChoice.SetPosition( leftX, topY + 4 * spacing, 0 )
		
		// Brightness Settings Image
		local brightnessImage = ::Gui.TexturedQuad( )
		brightnessImage.SetTexture( "gui/textures/frontend/settings/brightness_test_d.png" )
		brightnessImage.SetPosition( leftX + 365, topY, 0 )
		AddChild( brightnessImage )

		finalized = true
		HighlightByIndex( -1 )
		ChangeHighlight( 0 )
	}
	
	function HandleInput( gamepad )
	{
		local deadZone = 0.5
		local stick = gamepad.LeftStick
		local offset = 0
		if( ::Math.Abs( stick.x ) > deadZone )
			offset = stick.x
		
		if( offset != 0 )
		{
			if( highlightIndex < sliderSettings )
				icons[ highlightIndex ].Modify( offset )
		}
		
		return ::BaseSettingsMenu.HandleInput( gamepad )
	}
	
	function ChangeHighlight( indexDelta )
	{
		::BaseSettingsMenu.ChangeHighlight( indexDelta )

		if( highlightIndex < topSettings )
		{
			local desc = [ "Settings_Music_Desc", "Settings_Sfx_Desc", "Settings_Brightness_Desc", "Settings_Vibration_Desc" ]
			SetDescriptorText( desc[ highlightIndex ] )
		}
		else
			OnListenerChanged( )
	}
	
	function OnListenerChanged( value = null )
	{
		if( value == null )
			value = Setting( "Settings_ListeningMode" )
			
		local listenerModeDesc = [
			"Settings_ListeningMode_Small_Desc",
			"Settings_ListeningMode_Medium_Desc",
			"Settings_ListeningMode_Large_Desc"
		]
		SetDescriptorText( listenerModeDesc[ value ] )
	}
	
	function OnChanged( key, value )
	{
		// Dirty regardless
		hasChanged = true
		
		//... but not actually change settings from remote users
		if( !player.User.IsLocal )
			return
			
		switch( key )
		{
		case "Settings_MusicVolume":
			::GameApp.SetMusicVolume( value )
			break
		
		case "Settings_SoundEffectsVolume":
			::GameApp.SetSfxVolume( value )
			break
		
		case "Settings_Brightness":
			::GameApp.SetBrightness( value )
			break
			
		case "Settings_Vibration":
			if( value == 0 )
				player.User.RawGamepad( ).Buzz( 0.5 )
			break
			
		case "Settings_ListeningMode":
			OnListenerChanged( value )
			::GameApp.SetListeningMode( value )
			break
		}
	}
	
	function SaveSettings( )
	{
		local profile = player.GetUserProfile( )
		
		profile.MusicVolume = Setting( "Settings_MusicVolume" )
		profile.SfxVolume = Setting( "Settings_SoundEffectsVolume" )
		profile.Brightness = Setting( "Settings_Brightness" )
		profile.SetSetting( SETTINGS_DISABLE_CONTROLLER_VIBE, Setting( "Settings_Vibration" ) )
		profile.ListeningMode = Setting( "Settings_ListeningMode" )
		
		player.SaveProfile( )
		::BaseSettingsMenu.SaveSettings( )
	}
	
	function CancelSettings( )
	{
		local profile = player.GetUserProfile( )
		::GameApp.SetMusicVolume( profile.MusicVolume )
		::GameApp.SetSfxVolume( profile.SfxVolume )
		::GameApp.SetBrightness( profile.Brightness )
		// Nothing to do for vibration
		::GameApp.SetListeningMode( profile.ListeningMode )
	}
}