
// Requires
sigimport "gui/scripts/controls/radialmenu.nut"
sigimport "gui/scripts/hud/warningdialog.nut"

sigexport function CanvasCreateRadialMenu( radialMenu )
{
	return TurretOptionsRadialMenu( )
}

class TurretOptionsIcon extends RadialMenuIcon
{
	constructor( imagePath, selectCb )
	{
		::RadialMenuIcon.constructor( imagePath, selectCb )
		SetActiveInactiveScale( Math.Vec2.Construct( 0.5, 0.5 ), Math.Vec2.Construct( 1.0, 1.0 ) )
	}
}

class TurretOptionsRadialMenu extends RadialMenu
{
	unit = null
	player = null

	function DefaultSetup( params )
	{
		unit = params.Unit
		player = params.Player
		audioSource = player.AudioSource
		
		SetRadius( 66 )
		SetIconCount( 4 )
		
		local level = GameApp.CurrentLevel

		if( level.TutDisableUse  )
		{
			icons[ 0 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/use_unit_nouse_g.png", DisabledCallback.bindenv(this) )
			icons[ 0 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Use_Locked" )
			}
		}
		else if( ::GameApp.GetDifficulty( ) == DIFFICULTY_GENERAL  )
		{
			icons[ 0 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/use_unit_nouse_g.png", DisabledCallback.bindenv(this) )
			icons[ 0 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "General_NoUse" )
			}
		}
		else
		{
			icons[ 0 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/use_unit_g.png", UseTurretCallback.bindenv(this) )
			icons[ 0 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Use_Unit" )
			}
		}

		if( level.TutDisableUpgrade )
		{
			icons[ 1 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/upgrade_unit_locked_g.png", DisabledCallback.bindenv(this) )
			icons[ 1 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Upgrade_Locked" )
			}
		}
		else if( unit.UpgradeMaxed( player ) )
		{
			icons[ 1 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/upgrade_unit_maxed_g.png", UpgradeTurretCallback.bindenv(this) )
			icons[ 1 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Upgrade_Maxed" )
			}
		}
		else if( unit.UpgradeLocked( player ) )
		{
			icons[ 1 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/upgrade_unit_locked_g.png", UpgradeTurretCallback.bindenv(this) )
			icons[ 1 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Upgrade_Locked" )
			}
		}
		else
		{
			icons[ 1 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/upgrade_unit_g.png", UpgradeTurretCallback.bindenv(this) )
			icons[ 1 ].displayStringCallback = function( unit, player )
			{
				local out = ::GameApp.LocString( "Upgrade_Unit" ).Replace( "money", ::LocString.ConstructMoneyString( unit.UpgradeCost.tostring( ) ) )
				local locBase = ( player.Country == COUNTRY_USA? "USA_": "USSR_" ) + unit.UpgradeID
				return out % "\n" % ::GameApp.LocString( locBase ) % "\n\n" % ::GameApp.LocString( locBase + "_Class" ) % "\n" % ::GameApp.LocString( locBase + "_PurchaseDescription" )
			}
		}

		if( level.TutDisableRepair )
		{
			icons[ 2 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/repair_unit_locked_g.png", DisabledCallback.bindenv(this) )
			icons[ 2 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Repair_Disabled" )
			}
		}
		else
		{
			icons[ 2 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/repair_unit_g.png", RepairTurretCallback.bindenv(this) )
			icons[ 2 ].displayStringCallback = function( unit, player )
			{
				return::GameApp.LocString( "Repair_Unit" ).Replace( "money", ::LocString.ConstructMoneyString( unit.RepairCost.tostring( ) ) )
			}
		}
			
		if( level.TutDisableSell )
		{
			icons[ 3 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/sell_unit_locked_g.png", DisabledCallback.bindenv(this) )
			icons[ 3 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Sell_Disabled" )
			}
		}
		else
		{
			icons[ 3 ] = ::TurretOptionsIcon( "gui/textures/radialmenus/sell_unit_g.png", SellTurretCallback.bindenv(this) )
			icons[ 3 ].displayStringCallback = function( unit, player )
			{
				return ::GameApp.LocString( "Sell_Unit" ).Replace( "money", ::LocString.ConstructMoneyString( unit.SellValue.tostring( ) ) )
			}
		}
			
		FinalizeIconSetup( )
	}
	
	function HighlightByAngle( angle, magnitude )
	{
		local retValue = ::RadialMenu.HighlightByAngle( angle, magnitude )
		if( highlightIndex < 0 ) return retValue
		
		if( retValue )
		{
			local outString = icons[ highlightIndex ].DisplayString( unit, player )

			if( outString )
				displayText.BakeBoxLocString( 400, outString, TEXT_ALIGN_CENTER )
			else
				displayText.BakeCString( "", TEXT_ALIGN_CENTER )
		}
			
		return retValue
	}

	function TryHotKeys( gamepad )
	{
		local iconIndex = -1
		if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_UP ) )
			iconIndex = 0
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_RIGHT ) )
			iconIndex = 1
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_DOWN ) )
			iconIndex = 2
		else if( gamepad.ButtonDown( GAMEPAD_BUTTON_DPAD_LEFT ) )
			iconIndex = 3

		if( iconIndex >= 0 )
			return icons[ iconIndex ].onSelectCallback( )

		return false
	}

	function UseTurretCallback( )
	{
		return unit.TryToUse( player )
	}

	function UpgradeTurretCallback( )
	{
		return unit.TryToUpgrade( player )
	}

	function RepairTurretCallback( )
	{
		return unit.TryToRepair( player )
	}

	function SellTurretCallback( )
	{
		// Warn Player about selling
		local dialog = ::WarningDialog( "Sell_Confirm", player )
		::GameApp.CurrentLevel.AddSellDialog( player.User, dialog )
		unit.DeleteCallback = function( ):( dialog ) { dialog.Close( ) }
		dialog.onOk = function( ):( unit, player ) { if( unit && player ) unit.TryToSell( player ) }
		return true
	}

	function DisabledCallback( )
	{
		return false
	}
	
	function OnTick( dt )
	{
		::RadialMenu.OnTick( dt )
		
		if( ::GameApp.CurrentLevel.TurretMenuChanged )
		{
			// reconstruct
			DefaultSetup( { Unit = unit, Player = player } )
		}
	}

}
