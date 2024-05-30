#include "GameAppPch.hpp"
#include "tCharacterAimAnimTrack.hpp"
#include "tUserControllableCharacterLogic.hpp"
#include "tKeyFrameAnimation.hpp"
#include "tAnimatedSkeleton.hpp"
#include "Logic/tPhysical.hpp"

namespace Sig
{

	namespace
	{
		static const f32 cBlendEpsilon = 0.0001f;


		enum tCharacterAimTracks
		{
			cCharacterAimHighAnim = 0,
			cCharacterAimLowAnim,
			cCharacterAimAnimCount
		};
	}

	tCharacterAimAnimTrack::tCharacterAimAnimTrack( 
		tBlendAnimTrack::tTrackList& tracksToBlend, const tCharacterAimAnimDesc& desc )
		: tBlendAnimTrack( tracksToBlend, desc.mBlendIn, desc.mBlendOut, desc.mTimeScale, desc.mBlendScale, 0.f, -1.f, tAnimTrack::cFlagPartial )
		, mCharacter( desc.mCharacter )
	{
		fSetTag( desc.mTag );
	}

	void tCharacterAimAnimTrack::fStepInternal( Math::tPRSXformf& refFrameDelta, tAnimatedSkeleton& animSkel, b32 forceFullBlend, f32 dt, f32 wrapSign )
	{
		sigassert( mSubTracks.fCount( ) == cCharacterAimAnimCount );

		fSetBlendScale( mCharacter->fGetAimBlendMain( ) );
		f32 l = mCharacter->fGetAimBlendDownUp( ) / 2.0f + 0.5f;

		mSubTracks[ cCharacterAimHighAnim ]->fSetBlendScale( 1.0f );
		mSubTracks[ cCharacterAimLowAnim ]->fSetBlendScale( l );

		tBlendAnimTrack::fStepInternal( refFrameDelta, animSkel, forceFullBlend, dt, wrapSign );
	}
}


namespace Sig
{
	namespace
	{
		static void fPushAnim( const tCharacterAimAnimDesc* desc, tAnimatedSkeleton* stack )
		{
			if( !stack->fHasTracksOfType<tCharacterAimAnimTrack>( ) )
			{
				tBlendAnimTrack::tTrackList trackList;
				trackList.fNewArray( cCharacterAimAnimCount );

				trackList[ cCharacterAimHighAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mHigh, 0.f, 0.f ) ) );
				trackList[ cCharacterAimLowAnim ] = tAnimTrackPtr( NEW tKeyFrameAnimTrack( tKeyFrameAnimDesc( desc->mLow, 0.f, 0.f ) ) );
				
				tAnimTrackPtr atp( NEW tCharacterAimAnimTrack( trackList, *desc ) );
				stack->fPushTrack( atp );
			}
		}
	}
	void tCharacterAimAnimDesc::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tCharacterAimAnimDesc,Sqrat::DefaultAllocator<tCharacterAimAnimDesc> > classDesc( vm.fSq( ) );
		classDesc
			.GlobalFunc(_SC("Push"), &fPushAnim)
			.Var(_SC("HighAnim"), &tCharacterAimAnimDesc::mHigh)
			.Var(_SC("LowAnim"), &tCharacterAimAnimDesc::mLow)

			.Var(_SC("BlendIn"), &tCharacterAimAnimDesc::mBlendIn)
			.Var(_SC("BlendOut"), &tCharacterAimAnimDesc::mBlendOut)
			.Var(_SC("TimeScale"), &tCharacterAimAnimDesc::mTimeScale)
			.Var(_SC("BlendScale"), &tCharacterAimAnimDesc::mBlendScale)
			.Var(_SC("Character"), &tCharacterAimAnimDesc::mCharacter)
			.Var(_SC("Tag"), &tCharacterAimAnimDesc::mTag)
			;

		vm.fNamespace(_SC("Anim")).Bind( _SC("CharacterAimTrack"), classDesc );
	}
}
