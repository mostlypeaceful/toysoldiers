#ifndef __tKeyFrameAnimation__
#define __tKeyFrameAnimation__

namespace Sig
{

	class tAnimPackFile;

	///
	/// \brief Contains a list of bones and the bones' position, rotation, and scale key frames.
	/// The key frames are stored in each bone's parent-relative space, so the final set of
	/// bone transforms must be concatenated (B0->B1->B2 etc) to get to object or world space.
	class base_export tKeyFrameAnimation
	{
		declare_reflector( );
	public:

		static const u16 cMaxKeyFrameNum = 65530;

		typedef Math::tVec3f tPositionKey;
		typedef Math::tQuatf tRotationKey;
		typedef Math::tVec3f tScaleKey;

		static inline tPositionKey fToPositionKey( const Math::tVec3f& p, f32 min, f32 max )
		{
			// TODO compression
			return p;
		}
		static inline Math::tVec3f fFromPositionKey( const tPositionKey& p, f32 min, f32 max )
		{
			// TODO de-compression
			return p;
		}

		static inline tRotationKey fToRotationKey( const Math::tQuatf& r )
		{
			// TODO compression
			return r;
		}
		static inline Math::tQuatf fFromRotationKey( const tRotationKey& r )
		{
			// TODO de-compression
			return r;
		}

		static inline tScaleKey fToScaleKey( const Math::tVec3f& s, f32 min, f32 max )
		{
			// TODO compression
			return s;
		}
		static inline Math::tVec3f fFromScaleKey( const tScaleKey& s, f32 min, f32 max )
		{
			// TODO de-compression
			return s;
		}

		typedef tDynamicArray<u16>			tKeyFrameNumberArray;
		typedef tDynamicArray<tPositionKey> tPositionKeyArray;
		typedef tDynamicArray<tRotationKey> tRotationKeyArray;
		typedef tDynamicArray<tScaleKey>	tScaleKeyArray;

		class base_export tBone
		{
			declare_reflector( );
		public:
			enum tFlags
			{
				cFlagAdditive = ( 1 << 0 ),
			};
		public:
			u16						mMasterBoneIndex;
			u16						mFlags;
			tLoadInPlaceStringPtr*	mName;
			f32						mPMin, mPMax;
			f32						mSMin, mSMax;
			tKeyFrameNumberArray	mPositionFrameNums;
			tPositionKeyArray		mPositionKeys;
			tKeyFrameNumberArray	mRotationFrameNums;
			tRotationKeyArray		mRotationKeys;
			tKeyFrameNumberArray	mScaleFrameNums;
			tScaleKeyArray			mScaleKeys;
			u16						mParentMasterBoneIndex;
			u16						mIKPriority;
			Math::tAabbf			mIKAxisLimits;
			u32						mIKAxisLimitsOrder;

		public:
			tBone( );
			tBone( tNoOpTag );
			~tBone( );

			inline b32 fIsAdditive( ) const { return mFlags & cFlagAdditive; }

			void fFirstFrameXform( Math::tPRSXformf& o ) const
			{
				o.mP = fFromPositionKey(	mPositionKeys.fFront( ), mPMin, mPMax );
				o.mR = fFromRotationKey(	mRotationKeys.fFront( ) );
				o.mS = fFromScaleKey(		mScaleKeys.fFront( ), mSMin, mSMax );
			}

			void fLastFrameXform(  Math::tPRSXformf& o ) const
			{
				o.mP = fFromPositionKey(	mPositionKeys.fBack( ), mPMin, mPMax );
				o.mR = fFromRotationKey(	mRotationKeys.fBack( ) );
				o.mS = fFromScaleKey(		mScaleKeys.fBack( ), mSMin, mSMax );
			}

			b32 operator < ( const tBone &right ) const 
			{
				if( mParentMasterBoneIndex == right.mParentMasterBoneIndex ) return this < &right;
				else return (s16)mParentMasterBoneIndex < (s16)right.mParentMasterBoneIndex;
			}
		};

		typedef tDynamicArray<tBone> tBoneArray;

		struct base_export tKeyFrameEvent
		{
			declare_reflector( );
		public:
			f32 mTime;
			u32 mEventTypeCppValue;
			tLoadInPlaceStringPtr* mTag;
			tKeyFrameEvent( ) : mTime( 0.f ), mEventTypeCppValue( ~0 ), mTag( 0 ) { }
			tKeyFrameEvent( tNoOpTag ) { }
		};

		typedef tDynamicArray<tKeyFrameEvent> tKeyFrameEventArray;

		struct base_export tBracket
		{
			declare_reflector( );
		public:
			u32 mLeft, mRight;
			f32 mZeroToOne;
			tBracket( ) : mLeft( 0 ), mRight( 0 ), mZeroToOne( 0.f ) { }
		};

		enum tFlags
		{
			cFlagPartial			= ( 1 << 0 ),
			cFlagContainsRefFrame	= ( 1 << 1 ),
		};

	public:

		tLoadInPlaceStringPtr*	mName;
		u32						mFlags;
		u32						mFramesPerSecond;
		f32						mLengthOneShot;
		f32						mLengthLooping;
		tBone					mReferenceFrame;
		tBoneArray				mBones;
		tKeyFrameEventArray		mEvents;
		tAnimPackFile*			mPackFile;

	public:
		tKeyFrameAnimation( );
		tKeyFrameAnimation( tNoOpTag );
		inline b32 fPartial( ) const { return mFlags & cFlagPartial; }
		inline b32 fContainsRefFrame( ) const { return mFlags & cFlagContainsRefFrame; }
		inline f32 fLength( b32 looping ) const { return looping ? mLengthLooping : mLengthOneShot; }
		void fSampleBone( Math::tPRSXformf& xformOut, f32 timeInZeroToOne, const tBone& bone ) const;
		void fSampleBone( Math::tPRSXformf& xformOut, f32 timeInZeroToOne, const tBone& bone, const Math::tPRSXformf * remap ) const;
		void fSampleBoneAdditive( Math::tPRSXformf& xformOut, f32 timeInZeroToOne, const tBone& bone ) const;
		void fBracket( tBracket& bracketOut, const tKeyFrameNumberArray& keyFrameNums, f32 timeInZeroToOne ) const;

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
		f32 fOneShotLength( ) const { return fLength( false ); }
		f32 fLoopingLength( ) const { return fLength( true ); }
	};

}

#endif//__tKeyFrameAnimation__
