// Mode Names

SurvivalModeNames <- [
	::GameApp.LocString( "ChallengeMode_Survival" ),
	::GameApp.LocString( "ChallengeMode_Lockdown" ),
	::GameApp.LocString( "ChallengeMode_Hardcore" ),
	::GameApp.LocString( "ChallengeMode_Trauma" ),
	::GameApp.LocString( "ChallengeMode_Lockcore" ),
]

SurvivalModeDesc <- [
	"Survival_SurvivalDesc",
	"Survival_LockdownDesc",
	"Survival_HardcoreDesc",
	"Survival_TraumaDesc",
	"Survival_HardLockDesc",
]

function GetSurvivalModeName( mode )
{
	if( mode in ::SurvivalModeNames )
		return ::SurvivalModeNames[ mode ]
	else
		return null
}

function GetSurvivalModeDesc( mode )
{
	if( mode in ::SurvivalModeDesc )
		return ::SurvivalModeDesc[ mode ]
	else
		return null
}

DifficultyNames <- [
	::GameApp.LocString( "Difficulty_Casual" ),
	::GameApp.LocString( "Difficulty_Normal" ),
	::GameApp.LocString( "Difficulty_Hard" ),
	::GameApp.LocString( "Difficulty_Elite" ),
	::GameApp.LocString( "Difficulty_General" )
]

function GetDifficultyName( diff )
{
	if( diff in ::DifficultyNames )
		return ::DifficultyNames[ diff ]
	else
		return null
}

function ModeCount( mapType )
{
	switch( mapType )
	{
		case MAP_TYPE_CAMPAIGN:
		return DIFFICULTY_COUNT
		
		case MAP_TYPE_SURVIVAL:
		return CHALLENGE_MODE_COUNT
		
		case MAP_TYPE_MINIGAME:
		case MAP_TYPE_HEADTOHEAD:
		return 1
	}
}