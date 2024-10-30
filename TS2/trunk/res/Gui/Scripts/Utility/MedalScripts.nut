// Scripts for medals

const MEDAL_SIZE_XL = 0
const MEDAL_SIZE_LARGE = 1
const MEDAL_SIZE_FRIEND = 2
const MEDAL_SIZE_SMALL = 3

function VictoryScreenMedalImagePath( medalRank, size = MEDAL_SIZE_LARGE, overall = false )
{
	local sizeSuffixes = {
		[ MEDAL_SIZE_XL ] = "_xl",
		[ MEDAL_SIZE_LARGE ] = "_lg",
		[ MEDAL_SIZE_FRIEND ] = "_med",
		[ MEDAL_SIZE_SMALL ] = "_sm",
	}

	local medalImages = {
		[ MEDAL_RANK_BRONZE ] = "gui/textures/endgamescreens/medals/bronze",
		[ MEDAL_RANK_SILVER ] = "gui/textures/endgamescreens/medals/silver",
		[ MEDAL_RANK_GOLD ] = "gui/textures/endgamescreens/medals/gold",
		[ MEDAL_RANK_PLATINUM ] = "gui/textures/endgamescreens/medals/platinum"
	}
	
	local overallTag = ( ( overall )? "_overall": "" )
	
	if( medalRank in medalImages )
		return medalImages[ medalRank ] + overallTag + sizeSuffixes[ size ] + "_g.png"
	else
		return null
}
