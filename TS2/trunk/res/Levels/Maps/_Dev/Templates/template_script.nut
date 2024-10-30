sigimport "Levels/Scripts/Common/game_standard.nut"

sigexport function EntityOnCreate( entity )
{
	RegisterLevelLogic( entity, LevelLogic_MODE_LEVELNUM( ) ) // RENAME to current level (e.g. LevelLogic_Campaign_lvl01)
}

class LevelLogic_MODE_LEVELNUM extends GameStandardLevelLogic // RENAME to current level (e.g. LevelLogic_Campaign_lvl01)
{
	// Declare wavelist variables
	// wave_variable_01 = null
	// wave_variable_02 = null
	// ...
	
	function OnSpawn( )
	{
		::GameStandardLevelLogic.OnSpawn( )
		
		// Initialize wavelist variables
		//wave_variable_01 = AddWaveList( "WaveListNameFromLevelSpreadsheet" )
		//wave_variable_02 = AddWaveList( "AnotherWaveListName" )
				
		// You may activate waves here that will be started immediately on level start (before the fly-in)
		
		// Begin the level intro
		// The first parameter is the camera path for the fly-in, the second is an array of the wave variables that will be activated after the fly-in
		// If you set the first parameter as null, then no fly-in occurs and the wave is activiated immediately
		// If you set the second parameter as null or an empty array, then no waves are activated at the end of the fly-in
		LevelIntro2P( "nis_intro_campath", "nis_intro_campath", [ wave_variable_01, wave_variable_02 ] )
		
		// See game_standard.nut for more examples of working with wavelists and victory conditions in script		
	}
}

// Other things to do:
// - Set up three localized strings in text.locml (where ## is the level number)
//   - CampaignName_Level##_Global_Intro
//       This is for the introductory text, usually <Map Name>, <Location> <Year>
//   - CampaignName_Level##_Global_Objective
//       This is the simple objective text for the level
//   - CampaignName_Level#_Global_RationTicketRequirement
//       This is what needs to be done by the player to earn a ration ticket
