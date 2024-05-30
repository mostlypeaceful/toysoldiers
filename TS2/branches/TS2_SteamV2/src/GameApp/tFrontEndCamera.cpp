#include "GameAppPch.hpp"
#include "tFrontEndCamera.hpp"
#include "tPlayer.hpp"

namespace Sig
{
	tFrontEndCamera::tFrontEndCamera( tPlayer& player )
		: tCameraController( player.fUser( )->fViewport( ) )
		, mPlayer( player )
	{
		fAddUser( player.fUser( ) );
	}

	void tFrontEndCamera::fOnTick( f32 dt )
	{

	}
}

