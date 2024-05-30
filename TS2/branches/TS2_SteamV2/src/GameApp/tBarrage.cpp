#include "GameAppPch.hpp"
#include "tBarrage.hpp"
#include "tGameApp.hpp"

#include "Wwise_IDs.h"

using namespace Sig::Math;

namespace Sig
{
	void tBarrage::fAudioEvent( const tStringPtr& event, tPlayer* player ) const
	{
		if( event.fExists( ) )
		{
			const Audio::tSourcePtr& sourceUse = player ? player->fSoundSource( ) : tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( );
			sourceUse->fSetSwitch( tGameApp::cBarrageFactionSwitchGroup, mBarrageFactionSwitchValue );
			sourceUse->fHandleEvent( event );
		}
	}

	void tBarrage::fAudioState( tAudioState state, tPlayer* player  ) const
	{
		const Audio::tSourcePtr& sourceUse = player ? player->fSoundSource( ) : tGameApp::fInstance( ).fSoundSystem( )->fMasterSource( );
		switch( state )
		{
		case cCallIn:
			sourceUse->fSetSwitch( AK::SWITCHES::BARRAGE_STATE::GROUP, AK::SWITCHES::BARRAGE_STATE::SWITCH::CALL_IN );
			break;
		case cRolling:
			sourceUse->fSetSwitch( AK::SWITCHES::BARRAGE_STATE::GROUP, AK::SWITCHES::BARRAGE_STATE::SWITCH::ROLLING );
			break;
		case cEnd:
			sourceUse->fSetSwitch( AK::SWITCHES::BARRAGE_STATE::GROUP, AK::SWITCHES::BARRAGE_STATE::SWITCH::EXIT );
			break;
		}
	}

	void tBarrage::fSelected( tPlayer* player )
	{
		fAudioEvent( mAudioEventSelected, player );
	}

	void tBarrage::fBegin( tPlayer* player ) 
	{ 
		if( !mSkippedInto )
		{
			fAudioState( cCallIn, player );
			fAudioEvent( mAudioEventBegin, player );
		}
	}

	void tBarrage::fReset( tPlayer* player ) 
	{ 
		fAudioState( cEnd, player );
		fAudioEvent( mAudioEventEnd, player );
		mSkippedInto = false;
		mForTutorial = false;
	}

	b32 tBarrage::fKilledWithBarrageUnit( tPlayer* player, tUnitLogic* killer, tUnitLogic* killed )
	{ 
		fAudioEvent( mAudioEventKill, player );		
		if( mKillStatToIncrement != ~0 )
			player->fStats( ).fIncStat( mKillStatToIncrement, 1 );
		return false; 
	}

	void tBarragePtr::fBegin( tPlayer* player )
	{
		if( fIsCodeOwned( ) )
			fCodeObject( )->fBegin( player );
		else
		{
			Sqrat::Function f( fScriptObject( ), "Begin" );
			sigassert( !f.IsNull( ) );
			f.Execute( player );
		}
	}

	void tBarragePtr::fReset( tPlayer* player )
	{
		if( fIsCodeOwned( ) )
			fCodeObject( )->fReset( player );
		else
		{
			Sqrat::Function f( fScriptObject( ), "Reset" );
			sigassert( !f.IsNull( ) );
			f.Execute( player );
		}
	}

	f32 tBarragePtr::fProcessST( tPlayer* player, f32 dt )
	{
		tBarrage* cpp = fCodeObject( );
		if( cpp )
			return fClamp( cpp->fProcessST( player, dt ), 0.f, 1.f );
		else
			return 1.f; //done
	}

}


namespace Sig
{
	void tBarrage::fExportScriptInterface( tScriptVm& vm )
	{
		{
			Sqrat::Class<tBarrage, Sqrat::DefaultAllocator<tBarrage>> classDesc( vm.fSq( ) );

			classDesc
				.Var( _SC("DevName"),			&tBarrage::mDevName )
				.Var( _SC("Name"),				&tBarrage::mName )
				.Var( _SC("IconPath"),			&tBarrage::mIconPath )
				.Var( _SC("Duration"),			&tBarrage::mDuration )
				.Var( _SC("AudioEventSelected"),&tBarrage::mAudioEventSelected )
				.Var( _SC("AudioEventBegin"),	&tBarrage::mAudioEventBegin )
				.Var( _SC("AudioEventEnd"),		&tBarrage::mAudioEventEnd )
				.Var( _SC("AudioEventKill"),	&tBarrage::mAudioEventKill )
				.Var( _SC("UsingStatToIncrement"),	&tBarrage::mUsingStatToIncrement)
				.Var( _SC("KillStatToIncrement"),	&tBarrage::mKillStatToIncrement)
				.Func(_SC("Begin"),				&tBarrage::fBegin )
				.Func(_SC("Reset"),				&tBarrage::fReset )
				.Prop(_SC("BarrageUsable"),		&tBarrage::fBarrageUsable )
				;

			vm.fRootTable( ).Bind(_SC("Barrage"), classDesc );
		}
	}
}

