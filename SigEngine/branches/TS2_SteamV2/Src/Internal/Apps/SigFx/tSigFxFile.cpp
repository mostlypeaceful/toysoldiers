#include "SigFxPch.hpp"
#include "tSigFxFile.hpp"

namespace Sig
{

	tSigFxScene::tSigFxScene( tToolsGuiApp& guiApp )
		: mGuiApp( guiApp )
	{
		mFxSystem = new Effects::tSigFxSystem( );
		mGuiApp.fSceneGraph( )->fAddToRoot( *mFxSystem );
	}

	tSigFxScene::~tSigFxScene( )
	{
		mGuiApp.fSceneGraph( )->fRemoveFromRoot( *mFxSystem );
	}

	void tSigFxScene::fClearSystems( )
	{
		mFxSystem->fClearSystems( );
	}

	void tSigFxScene::fAddSystem( Effects::tParticleSystemPtr system )
	{
		mFxSystem->fAddParticleSystem( system );
	}

	void tSigFxScene::fSetSystemCurrentTime( const f32 time )
	{
		mFxSystem->fSetCurrentTime( time );
	}

	
}

