#include "BasePch.hpp"
#include "tListener.hpp"
#include "tSystem.hpp"
#include "tGameAppBase.hpp"
#include "tAudioLogic.hpp"

#ifdef sig_use_wwise
#	include <AK/SoundEngine/Common/AkSoundEngine.h>
#endif//sig_use_wwise

namespace Sig { namespace Audio
{

	tListener::tListener( const char *debugName )
		: mLastXForm( Math::tMat3f::cIdentity )
	{
        if ( tGameAppBase::fInstance( ).fSoundSystem( ) )
            tGameAppBase::fInstance( ).fSoundSystem( )->fRegisterListener( this );
	}

	tListener::~tListener( )
	{
		if( tGameAppBase::fInstance( ).fSoundSystem( ) )
			tGameAppBase::fInstance( ).fSoundSystem( )->fUnRegisterListener( this );
	}

	void tListener::fPushVolume( tAudioLogic& volume )
	{
		sigassert( mVolumeStack.fIndexOf( volume.fOwnerEntity( ) ) == ~0 && "Volume already in volume stack." );
		mVolumeStack.fPushBack( tEntityPtr( volume.fOwnerEntity( ) ) );
	}

	void tListener::fPopVolume( tAudioLogic& volume )
	{
		u32 index = mVolumeStack.fIndexOf( volume.fOwnerEntity( ) );
		if( index != ~0 )
		{
			mVolumeStack.fEraseOrdered( index );
			if( index == mVolumeStack.fCount( ) && index > 0 )
				mVolumeStack.fBack( )->fLogicDerived<tAudioLogic>( )->fBackOnTop( );
		}
	}

	void tListener::fClearVolumeStack( )
	{
		mVolumeStack.fSetCount( 0 );
	}

	void tListener::fSetTransform( const Math::tMat3f& xform, u32 index )
	{
		if_devmenu( if( tSystem::fGlobalDisable( ) ) return; )

		mLastXForm = xform;
		if_wwise( AKRESULT result = AK::SoundEngine::SetListenerPosition( fSigToAKListenerPosition( xform ), index ) );
		if_wwise( sigassert( result == AK_Success ) );
	}

}
}


