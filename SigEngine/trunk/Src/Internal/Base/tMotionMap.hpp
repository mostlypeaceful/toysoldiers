#ifndef __tMotionMap__
#define __tMotionMap__
#include "tAnimatedSkeleton.hpp"

// All for the new momap -_-
#include "tMotionMapFile.hpp"
#include "tBlendAnimTrack.hpp"
#include "tKeyFrameAnimTrack.hpp"
#include "tAnimPackFile.hpp"
#include "tAniMapFile.hpp"
namespace Sig { class tAniMapFile; }
//

namespace Sig { namespace Anim
{
	class base_export tMotionMap
	{
		tAnimatedSkeletonPtr mStack;
		tLogic *mLogic;
	public:
		tMotionMap( ) : mLogic( NULL ) { }
		virtual ~tMotionMap( ) { }

		void fSetLogic( tLogic* logic ) { mLogic = logic; }
		Sqrat::Object fGetLogicForScript( ) { sigassert( mLogic && "Forgot to call tAnimatable::fSetLogic( ) in your logic constructor." ); return mLogic->fOwnerEntity( )->fScriptLogicObject( ); }
		tAnimatedSkeleton* fAnimationStack( ) const;
		void fSetAnimationStack( const tAnimatedSkeletonPtr& stack ) { mStack = stack; }
	public:
		static void fExportScriptInterface( tScriptVm& vm );
	};

	///
	/// \brief Wrapper around base motion map type, providing glue between script and code.
	class base_export tScriptMotionMap : public tScriptObjectPtr<tMotionMap>
	{
	public:
		tScriptMotionMap( ) { }
		explicit tScriptMotionMap( const Sqrat::Object& o ) : tScriptObjectPtr<tMotionMap>( o ) { }
		tMotionMap* fMotionMap( ) const { return fCodeObject( ); }
		Sqrat::Function fMapState( const char* stateName );
	};


	class base_export tBlendReference
	{
	public:
		virtual ~tBlendReference( ) { }

		virtual void fAnimEnded( ) { }
	};

	// This object, created from a def, will keep the blend information up to date
	//  building out the skeleton as necessary.
	class base_export tSigAnimMoMap : public tRefCounter
	{
	public:
		tSigAnimMoMap( const tMotionMapFile& def, const tAniMapFile& animap, tAnimatedSkeleton& skeleton )
		{
			mAnimDatas.fSetCapacity( def.mNumAnimTracks );
			mBlendTracks.fSetCapacity( def.mNumBlendTracks );
			mBlendDatas.fSetCapacity( def.mNumBlendTracks );
			mContextData.fSetCapacity( animap.mContexts.fCount( ) );

			for( u32 i = 0; i < animap.mContexts.fCount( ); ++i )
				mContextData.fPushBack( tContextData( animap.mContexts[ i ].mEnumTypeValue, &mContextData ) );

			skeleton.fClearTracks( );
			fBuildTree( def.mRoot, animap, skeleton, NULL, false );
			fInitialize( );
		}

		class tBlendData;

		// This is only public because of the c++ standard, it should not be used outside of this class!
		struct base_export tBlendTrack
		{
			tBlendTrack( ) : mBlendDef( NULL ) { }
			tBlendTrack( tBlendAnimTrack* track, tMotionMapFile::tBlendTrackData* def, tBlendData* owner, tBlendTrack* parent );

			b32 operator == ( const tStringPtr& name ) const { return mBlendDef->mName->fGetStringPtr( ) == name; }

			void fSetBlend( f32 blend );
			void fSetTimeScale( f32 scale );

			void fEnableDigitalBlend( b32 enable );

			void fAnimEnded( );

			void fStep( f32 dt );

		private:
			tRefCounterPtr<tBlendAnimTrack> mTrack;
			const tMotionMapFile::tBlendTrackData* mBlendDef;

			b8 mDigitalBlendEnabled;
			b8 mThresholdTriggered;
			b8 mDigitalDirty;
			b8 pad1;

			f32 mCurrentBlend;

			b32 mAVisible;
			b32 mBVisible;

			tBlendData* mOwner;
			tBlendTrack* mParent;

			void fTrackVisibilityChanged( u32 logicalIndex, b32 visible );
			void fApplyBlend( );
		};

		class base_export tBlendData
		{
		public:
			tBlendData( ) { }
			tBlendData( const tStringPtr& name, b32 digital, b32 oneShot );
			b32 operator == ( const tStringPtr& name ) const { return mName == name; }

			void fAddTrack( tBlendTrack* track );

			// dynamic control
			void fSetBlend( f32 blend );
			f32 fBlend( ) const { return mCurrentBlendValue; }
			void fSetBlendControl( f32* control ) { mBlendControl = control; }

			void fSetTimeScale( f32 scale );
			void fSetTimeScaleControl( f32* control ) { mTimeScaleControl = control; }

			// digital control, while there are any outstanding references, blend will be at 1.0.
			void fAddReference( tBlendReference* ref );
			void fRemoveReference( tBlendReference* ref );

			void fStep( f32 dt );

			void fAnimEnded( );

			b32 fOneShot( ) const { return mOneShot; }

		private:
			tStringPtr mName;
			f32 mCurrentBlendValue;

			b8 mDigital;
			b8 mOneShot;
			b8 pad0;
			b8 pad1;

			f32* mBlendControl; //if this is set, it will be used to drive the current blend.
			f32* mTimeScaleControl;

			tGrowableArray<tBlendTrack*> mBlends;
			tGrowableArray<tBlendReference*> mReferences;

			void fInformRefsAnimEnded( );
		};

		class tContextData;

		struct tEvalutateData
		{
			b32 mFirstRun;

			tEvalutateData( b32 firstRun )
				: mFirstRun( firstRun )
			{ }
		};

		class base_export tAnimData
		{
		public:
			tAnimData( ) 
			{ }

			tAnimData( const tAniMapFile::tMapping* map, const tMotionMapFile::tAnimTrackData* data, Anim::tBlendAnimTrack* owner, u32 slot, tBlendTrack* parentBlendTrack, b32 oneShot )
				: mAnimMap( map )
				, mData( data )
				, mOwner( owner )
				, mOwnerSlot( slot )
				, mParentBlendTrack( parentBlendTrack )
				, mOneShot( oneShot )
				, mCurrentAnim( NULL )
			{ }

			void fEvaluateContext( const tGrowableArray< tContextData >& context, tEvalutateData& data );

			void fStep( f32 dt );

			tAnimTrackPtr& fTrackPtr( );

		private:
			const tAniMapFile::tMapping* mAnimMap;
			const tMotionMapFile::tAnimTrackData* mData;
			Anim::tBlendAnimTrack* mOwner;
			tAnimatedSkeleton* mSkeleton;
			tBlendTrack* mParentBlendTrack;
			b32 mOneShot;
			u32 mOwnerSlot;

			const tAniMapFile::tAnimRef* mCurrentAnim;
			tRefCounterPtr<tKeyFrameAnimTrack> mAnimTrack;
		};

		class base_export tContextData
		{
		public:
			tContextData( )
			{ }

			tContextData( u32 enumTypeVal, const tGrowableArray<tContextData>* contextDataArray )
				: mEnumTypeValue( enumTypeVal )
				, mCurrentValue( ~0 )
				, mContextDataArray( contextDataArray )
			{ }

			b32 operator == ( u32 enumTypeVal ) const { return mEnumTypeValue == enumTypeVal; }

			void fSetCurrentValue( u32 value );
			u32 fCurrentValue( ) const { return mCurrentValue; }

			void fAddAnimData( tAnimData* data );

			u32 fEnumTypeValue( ) const { return mEnumTypeValue; }

		private:
			u32 mEnumTypeValue;	// GameFlags::cEnum_SomeEnum
			u32 mCurrentValue;	// GameFlags::cEnum_SomeEnum_SomeValue

			tGrowableArray<tAnimData*> mAnims;

			// kinda ghettooo, this array is the mContextData array on tSigAnimMotionMap. so that when the value changes, evalutate can be called and pass it along.
			const tGrowableArray<tContextData>* mContextDataArray;
		};

		tBlendData* fFindBlendData( const tStringPtr& name ) { return mBlendDatas.fFind( name ); }
		tContextData* fFindContextData( u32 enumTypeValue ) { return mContextData.fFind( enumTypeValue ); }
		const tGrowableArray< tContextData >& fContextData( ) const { return mContextData; }

		void fEvaluate( tEvalutateData& data );

		void fStep( f32 dt );

	private:
		tGrowableArray< tBlendTrack > mBlendTracks;
		tGrowableArray< tBlendData > mBlendDatas;
		tGrowableArray< tAnimData > mAnimDatas;
		tGrowableArray< tContextData > mContextData;

		///////////////////////////////////////////////////////////////////////////////////
		void fInitialize( );

		static void fPushTrack( tAnimatedSkeleton& skeleton, tAnimTrack* track )
		{
			skeleton.fPushTrack( tAnimTrackPtr( track ) );
		}

		static void fPushTrack( tBlendAnimTrack& blendTrack, tAnimTrack* track )
		{
			blendTrack.fSubTracks( ).fPushBack( tAnimTrackPtr( track ) );
		}

		void fPushAnimData( const tAniMapFile::tMapping* mapping, const tMotionMapFile::tAnimTrackData* data, tBlendAnimTrack& blendTrack, tBlendTrack* parentBlendTrack, b32 oneShot )
		{
			mAnimDatas.fPushBack( tAnimData( mapping, data, &blendTrack, blendTrack.fSubTracks( ).fCount( ), parentBlendTrack, oneShot ) );
			blendTrack.fSubTracks( ).fPushBack( tAnimTrackPtr( ) );
		}

		void fPushAnimData( const tAniMapFile::tMapping* mapping, const tMotionMapFile::tAnimTrackData* data, tAnimatedSkeleton& skeleton, tBlendTrack* parentBlendTrack, b32 oneShot )
		{
			mAnimDatas.fPushBack( tAnimData( mapping, data, NULL, skeleton.fTrackCount( ), parentBlendTrack, oneShot ) );
			skeleton.fPushTrack( tAnimTrackPtr( ) );
		}

		template< typename tOutput >
		void fBuildTree( tMotionMapFile::tTrack* def, const tAniMapFile& animap, tOutput& output, tBlendTrack* parentBlendTrack, b32 parentExpectingOneShot )
		{
			if( !def )
			{		
				// this is the current "null terminator", an empty blend track.
				tAnimTrackDesc desc;
				tBlendAnimTrack::tTrackList tracks;
				fPushTrack( output, NEW tBlendAnimTrack( tracks, desc ) );
			}
			else if( def->mAnim )
			{
				const tAniMapFile::tMapping* animMapping = animap.fFind( def->mAnim->mName->fGetStringPtr( ) );
				fPushAnimData( animMapping, def->mAnim, output, parentBlendTrack, parentExpectingOneShot );
				if( animMapping )
				{
					for( u32 i = 0; i < animMapping->mContextsIndexed.fCount( ); ++i )
						mContextData[ animMapping->mContextsIndexed[ i ] ].fAddAnimData( &mAnimDatas.fBack( ) );
				}
			}
			else 
			{
				// if stack, it's simply a blend track whom we'll never set the blend weights for. for now.
				tAnimTrackDesc desc;
				desc.mBlendIn = 0.f;

				tBlendAnimTrack::tTrackList tracks;
				tracks.fSetCapacity( def->mChildren.fCount( ) );

				tBlendAnimTrack* blendTrack = NEW tBlendAnimTrack( tracks, desc );
				fPushTrack( output, blendTrack );

				tBlendData* data = NULL;

				if( def->mBlend )
				{
					data = fFindBlendData( def->mBlend->mName->fGetStringPtr( ) );
					if( !data )
					{
						// none found, make a new one.
						mBlendDatas.fPushBack( tBlendData( def->mBlend->mName->fGetStringPtr( ), def->mBlend->mDigital, def->mBlend->mOneShot ) );
						data = &mBlendDatas.fBack( );
					}

					mBlendTracks.fPushBack( tBlendTrack( blendTrack, def->mBlend, data, parentBlendTrack ) );
					data->fAddTrack( &mBlendTracks.fBack( ) );

					// configure information to propagate
					parentBlendTrack = &mBlendTracks.fBack( );
					if( def->mBlend->mOneShot )
						parentExpectingOneShot = true;

					blendTrack->fSetTimeScale( def->mBlend->mTimeScale );
				}

				for( s32 i = def->mChildren.fCount( ) - 1; i >= 0; --i )
				//for( u32 i = 0; i < def->mChildren.fCount( ); ++i )
				{
					fBuildTree( def->mChildren[ i ], animap, *blendTrack, parentBlendTrack, parentExpectingOneShot );
				}
			}
		}
	};
} }

#endif//__tMotionMap__
