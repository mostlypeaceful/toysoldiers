#ifndef __tSigFxFile__
#define	__tSigFxFile__

#include "tSigFxSystem.hpp"
#include "FxEditor/tSigFxParticleSystem.hpp"

namespace Sig
{

	class tSigFxScene
	{
	public:
		tSigFxScene( tToolsGuiApp& guiApp );
		~tSigFxScene( );

		void fClearSystems( );
		void fAddSystem( FX::tParticleSystemPtr system );

		const f32 fLifetime( ) const { return mFxSystem->fLifetime( ); }
		void fSetSystemCurrentTime( const f32 time );

		const u32 fParticleCount( ) const { return mFxSystem->fParticleCount( ); }

	private:

		tToolsGuiApp& mGuiApp;
		FX::tSigFxSystem* mFxSystem;
	};

}

#endif //__tSigFxFile__