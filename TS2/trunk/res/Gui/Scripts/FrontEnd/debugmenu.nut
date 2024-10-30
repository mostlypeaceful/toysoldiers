
sigimport "Gui/Scripts/FrontEnd/common.nut"
sigimport "gui/scripts/endgamescreens/trialbuygamescreen.nut"
sigimport "gui/scripts/loadscreens/levelloadtest.nut"

class FrontEndDebugMenu extends FrontEndMenuBase
{
	function FinalizeIconSetup( )
	{
		icons.push( FrontEndMenuEntry( "Single Player", null, ShowSinglePlayerMenu.bindenv(this), true ) )
		icons.push( FrontEndMenuEntry( "Co Op", null, ShowCoOpMenu.bindenv(this), true ) )
		icons.push( FrontEndMenuEntry( "Reload Front End", null, ReloadFrontEnd.bindenv(this), true ) )
		
		icons.push( FrontEndMenuEntry( "Buy Screen", null, function( )
		{
			local buyScreen = ::TrialBuyGameScreen( )
			buyScreen.backOutBehavior = function( ) { return true }
			return PushNextMenu( buyScreen )
		}.bindenv(this) , true ) )
		
		icons.push( FrontEndMenuEntry( "Load Screen Test", null, function( ) { return PushNextMenu( ::LevelLoadTestMenu( ) ) }.bindenv(this), true ) )
		VerticalMenu.FinalizeIconSetup( )
	}
	function ShowSinglePlayerMenu( )
	{
		nextMenu = FrontEndDebugLevelMenu( false )
		nextAction = VerticalMenuAction.PushMenu
		return true
	}
	function ShowCoOpMenu( )
	{
		nextMenu = FrontEndDebugLevelMenu( true )
		nextAction = VerticalMenuAction.PushMenu
		return true
	}
	function ReloadFrontEnd( )
	{
		GameApp.LoadFrontEnd( )
		nextAction = VerticalMenuAction.ExitStack
		return true
	}
}

class FrontEndDebugLevelMenu extends FrontEndMenuBase
{
	coOp = false
	constructor( _coOp )
	{
		FrontEndMenuBase.constructor( )
		coOp = _coOp
	}
	function FinalizeIconSetup( )
	{
		local numLevels = GameApp.DebugLevelCount
		SetIconCount( numLevels )
		for( local i = 0; i < numLevels; i += 1 )
		{
			local menu = this
			local f = function( ):(menu,i) { return menu.LoadLevelByIndex(i) }
			icons[ i ] = FrontEndMenuEntry( 
				GameApp.DebugLevelName( i ),
				null, 
				f,
				true )
		}
		VerticalMenu.FinalizeIconSetup( )
	}
	function LoadLevelByIndex( index )
	{
		return LoadLevel( MAP_TYPE_DEVSINGLEPLAYER, index )
	}
	function FillLoadLevelInfo( info )
	{
		if( coOp )
			info.GameMode.AddCoOpFlag( )
	}
	function LoadLevel( front, levelIndex )
	{
		GameApp.LoadLevel( front, levelIndex, FillLoadLevelInfo.bindenv( this ) )
		nextAction = VerticalMenuAction.ExitStack
		return true
	}
}
