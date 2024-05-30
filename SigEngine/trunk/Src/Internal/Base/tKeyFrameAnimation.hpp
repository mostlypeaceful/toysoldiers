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

		typedef Math::tVector3<u16> tPositionKey;
		typedef Math::tVector4<u16> tRotationKey;
		typedef Math::tVector3<u8> tScaleKey;

		static inline tPositionKey fToPositionKey( const Math::tVec3f& p, f32 min, f32 max )
		{
			return tPositionKey( 
				fRound<u16>( Math::fRemapZeroToOneSafe( min, max, p.x ) * 65535.f ),
				fRound<u16>( Math::fRemapZeroToOneSafe( min, max, p.y ) * 65535.f ),
				fRound<u16>( Math::fRemapZeroToOneSafe( min, max, p.z ) * 65535.f ) );
		}
		static inline Math::tVec3f fFromPositionKey( const tPositionKey& p, f32 min, f32 max )
		{
			const f32 delta = max - min;
			return Math::tVec3f( 
				min + ( p.x / 65535.f ) * delta,
				min + ( p.y / 65535.f ) * delta,
				min + ( p.z / 65535.f ) * delta );
		}

		static inline tRotationKey fToRotationKey( const Math::tQuatf& r )
		{
			return tRotationKey(
				fRound<u16>( Math::fRemapZeroToOneSafe( -1.f, 1.f, r.x ) * 65535.f ),
				fRound<u16>( Math::fRemapZeroToOneSafe( -1.f, 1.f, r.y ) * 65535.f ),
				fRound<u16>( Math::fRemapZeroToOneSafe( -1.f, 1.f, r.z ) * 65535.f ),
				fRound<u16>( Math::fRemapZeroToOneSafe( -1.f, 1.f, r.w ) * 65535.f ) );
		}
		static inline Math::tQuatf fFromRotationKey( const tRotationKey& r )
		{
			const f32 delta = 2.f;
			return Math::tQuatf( 
				-1.f + ( r.x / 65535.f ) * delta,
				-1.f + ( r.y / 65535.f ) * delta,
				-1.f + ( r.z / 65535.f ) * delta,
				-1.f + ( r.w / 65535.f ) * delta );
		}

		static inline tScaleKey fToScaleKey( const Math::tVec3f& s, f32 min, f32 max )
		{
			return tScaleKey( 
				fRound<u8>( Math::fRemapZeroToOneSafe( min, max, s.x ) * 255.f ),
				fRound<u8>( Math::fRemapZeroToOneSafe( min, max, s.y ) * 255.f ),
				fRound<u8>( Math::fRemapZeroToOneSafe( min, max, s.z ) * 255.f ) );
		}
		static inline Math::tVec3f fFromScaleKey( const tScaleKey& s, f32 min, f32 max )
		{
			const f32 delta = max - min;
			return Math::tVec3f( 
				min + ( s.x / 255.f ) * delta,
				min + ( s.y / 255.f ) * delta,
				min + ( s.z / 255.f ) * delta );
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
				o.mPosition = fFromPositionKey(	mPositionKeys.fFront( ), mPMin, mPMax );
				o.mRotation = fFromRotationKey(	mRotationKeys.fFront( ) );
				o.mScale = fFromScaleKey(		mScaleKeys.fFront( ), mSMin, mSMax );
			}

			void fLastFrameXform(  Math::tPRSXformf& o ) const
			{
				o.mPosition = fFromPositionKey(	mPositionKeys.fBack( ), mPMin, mPMax );
				o.mRotation = fFromRotationKey(	mRotationKeys.fBack( ) );
				o.mScale = fFromScaleKey(		mScaleKeys.fBack( ), mSMin, mSMax );
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

		u32 fComputeStorage( ) const;

	public: // script interface
		static void fExportScriptInterface( tScriptVm& vm );
		f32 fOneShotLength( ) const { return fLength( false ); }
		f32 fLoopingLength( ) const { return fLength( true ); }
	};

}

#endif//__tKeyFrameAnimation__
