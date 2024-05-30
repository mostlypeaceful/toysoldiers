#ifndef __Animl__
#define __Animl__
#include "Sklml.hpp"

namespace Sig { namespace Animl
{
	tools_export const char* fGetFileExtension( );
	tools_export b32 fIsAnimlFile( const tFilePathPtr& path );

	typedef tDynamicArray< Math::tPRSXformf >	tPRSKeyFrameList;
	typedef tDynamicArray< Math::tMat3f >		tKeyFrameList;

	struct tools_export tDeltaCompressionKey
	{
		b8 mP;
		b8 mR;
		b8 mS;
		b8 pad;

		// compression keys default to on
		tDeltaCompressionKey( ) : mP( true ), mR( true ), mS( true ), pad( 0 ) { }
	};

	typedef tDynamicArray< tDeltaCompressionKey > tDeltaCompressionKeyList;

	struct tools_export tDeltaCompressionResult
	{
		u32 mPKeyCount, mRKeyCount, mSKeyCount;
		f32 mPMin, mPMax;
		f32 mSMin, mSMax;
		tDeltaCompressionKeyList	mCompressionKeys;
		tPRSKeyFrameList			mPRSKeys;

		tDeltaCompressionResult( ) 
			: mPKeyCount( 0 ), mRKeyCount( 0 ), mSKeyCount( 0 )
			, mPMin( +Math::cInfinity ), mPMax( -Math::cInfinity )
			, mSMin( +Math::cInfinity ), mSMax( -Math::cInfinity )
		{ }
	};

	class tools_export tBone : public tRefCounter
	{
	public:
		std::string		mName;
		b32				mExclude;
		b32				mAdditive;
		u32				mIKPriority;
		Math::tAabbf	mIKRotLimits;
		u32				mIKRotLimitOrder;
		tKeyFrameList	mKeyFrames;

	public:
		static void fDecomposeKeys( tPRSKeyFrameList& decomposed, const tKeyFrameList& keys );
		static void fDeltaCompress( 
			tDeltaCompressionResult& result, 
			const tKeyFrameList& keys, 
			f32 pMaxErrorInMeters = 1.f/100.f,
			f32 rMaxErrorInPctg = 1.f/100.f,
			f32 sMaxErrorInPctg = 1.f/100.f );
	public:
		tBone( );
	private:
		static void fDeltaCompressPosition( tDeltaCompressionResult& result, f32 maxError );
		static void fDeltaCompressRotation( tDeltaCompressionResult& result, f32 maxError );
		static void fDeltaCompressScale( tDeltaCompressionResult& result, f32 maxError );
	};

	typedef tRefCounterPtr< tBone >			tBonePtr;
	typedef tGrowableArray< tBonePtr >		tBoneList;

	class tools_export tSkeleton : public tRefCounter
	{
	public:
		tBone		mRefFrame;
		tBoneList	mBones;
	};

	class tools_export tFile
	{
	public:
		u32					mVersion;
		tStringPtr			mModellingPackage;
		tFilePathPtr		mSrcFile;
		u32					mFramesPerSecond;
		u32					mTotalFrames;
		tSkeleton			mSkeleton;

	public:
		tFile( );
		b32 fSaveXml( const tFilePathPtr& path, b32 promptToCheckout );
		b32 fLoadXml( const tFilePathPtr& path );
		const tBone* fFindBone( const char* name ) const;

	};

}}

#endif//__Animl__
