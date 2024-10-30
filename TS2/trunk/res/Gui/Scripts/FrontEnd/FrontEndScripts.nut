// Restart Functions

enum FrontEndRootMenuStartMode
{
	Normal,
	WarnUserSignedOut,
	FromInvite
}

FrontEndStartMenuData <- null

function ClearFrontEndRestartData( )
{
	::FrontEndStartMenuData = null
}

function ClearFrontEndMultiplayerRestartData( )
{
	if( ::FrontEndStartMenuData )
	{
		::FrontEndStartMenuData.UserId = null
		::FrontEndStartMenuData.IsClient = null
	}
}

function SetFrontEndRankedMatchRestart( )
{
	::FrontEndStartMenuData = {
		Type = "RankedMatch",
		MapType = null,
		Dlc = null,
		UserId = null,
		IsClient = null,
	}
}

function SetFrontEndDisplayCaseRestart( )
{
	::FrontEndStartMenuData = {
		Type = "DisplayCase",
		MapType = null,
		Dlc = null,
		UserId = null,
		IsClient = null,
	}
}

function SetFrontEndMainMenuRestart( )
{
	::FrontEndStartMenuData = {
		Type = "MainMenu",
		MapType = null,
		Dlc = null,
		UserId = null,
		IsClient = null,
	}
}

function SetFrontEndLevelSelectRestart( mapType, dlc = null, userId = null, isClient = null )
{
	::FrontEndStartMenuData = {
		Type = "LevelSelect",
		MapType = mapType, 
		Dlc = dlc,
		UserId = userId,
		IsClient = isClient,
	}
}