// Achievement Image Scripts

function AchievementImagePath( index )
{
	local imageIndex = ::Math.Clamp( index, 0, ACHIEVEMENTS_COUNT - 1 ) + 1
	return "gui/textures/achievements/ach_" + ( ( imageIndex < 10 )? "0": "" ) + imageIndex.tostring( ) + "_g.png"
}

function AchievementName( index )
{
	index = ::Math.Clamp( index, 0, ACHIEVEMENTS_COUNT - 1 )
	return "Achievement" + index.tostring( ) + "Name"
}

function AchievementDesc( index )
{
	index = ::Math.Clamp( index, 0, ACHIEVEMENTS_COUNT - 1 )
	return "Achievement" + index.tostring( ) + "Desc"
}

function AchievementBuyText( index )
{
	index = ::Math.Clamp( index, 0, ACHIEVEMENTS_COUNT - 1 )
	return "Achievement" + index.tostring( ) + "BuyText"
}

class AchievementImage extends AnimatingCanvas
{
	static Width = 64
	static Height = 64
	
	constructor( index )
	{
		::AnimatingCanvas.constructor( )
		
		local image = ::Gui.AsyncTexturedQuad( )
		image.SetTexture( ::AchievementImagePath( index ) )
		AddChild( image )
	}
}

function AvatarAwardImagePath( index )
{
	index = ::Math.Clamp( index, 0, AVATAR_AWARDS_COUNT - 1 )
	return "gui/textures/avatarawards/avatar_" + index.tostring( ) + "_g.png"
}

function AvatarAwardName( index )
{
	index = ::Math.Clamp( index, 0, AVATAR_AWARDS_COUNT - 1 )
	return "AvatarAward" + index.tostring( ) + "Name"
}

function AvatarAwardDesc( index )
{
	index = ::Math.Clamp( index, 0, AVATAR_AWARDS_COUNT - 1 )
	return "AvatarAward" + index.tostring( ) + "Desc"
}

function AvatarAwardBuyText( index )
{
	index = ::Math.Clamp( index, 0, AVATAR_AWARDS_COUNT - 1 )
	return "AvatarAward" + index.tostring( ) + "BuyText"
}

class AvatarAwardImage extends AnimatingCanvas
{
	static Width = 64
	static Height = 64
	
	constructor( index )
	{
		::AnimatingCanvas.constructor( )
		
		local image = ::Gui.AsyncTexturedQuad( )
		image.SetTexture( ::AvatarAwardImagePath( index ) )
		AddChild( image )
	}
}