#include "ToolsPch.hpp"
#include "Animl.hpp"
#include "tXmlSerializeMath.hpp"
#include "tXmlDeserializeMath.hpp"

namespace Sig { namespace Animl
{
	const char* fGetFileExtension( )
	{
		return ".animl";
	}

	b32 fIsAnimlFile( const tFilePathPtr& path )
	{
		return StringUtil::fCheckExtension( path.fCStr( ), fGetFileExtension( ) );
	}


	///
	/// \section tBone
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tBone& o )
	{
		s( "Name", o.mName );
		s( "Exclude", o.mExclude );
		s( "Additive", o.mAdditive );
		s( "IKPriority", o.mIKPriority );
		s( "IKRotLimits", o.mIKRotLimits );
		s( "IKRotLimitOrder", o.mIKRotLimitOrder );
		s( "KeyFrames", o.mKeyFrames );
	}

	tBone::tBone( )
		: mExclude( false )
		, mAdditive( false )
		, mIKPriority( 0 )
		, mIKRotLimits( -Math::cInfinity, +Math::cInfinity )
		, mIKRotLimitOrder( 0 )
	{
	}

	void tBone::fDecomposeKeys( tPRSKeyFrameList& decomposed, const tKeyFrameList& keys )
	{
		decomposed.fNewArray( keys.fCount( ) );

		// convert matrix keys to isolated values of Position, Rotation, and Scale
		for( u32 i = 0; i < decomposed.fCount( ); ++i )
			decomposed[ i ] = Math::tPRSXformf( keys[ i ] );

		// fix anti-podal quaternions (prevents rotations that look 
		// like they're going the "long" way around a circle)
		for( u32 i = 1; i < decomposed.fCount( ); ++i )
		{
			if( decomposed[ i - 1 ].mRotation.fAntiPodal( decomposed[ i ].mRotation ) )
				decomposed[ i ].mRotation = -decomposed[ i ].mRotation;
		}
	}
	
	void tBone::fDeltaCompress( tDeltaCompressionResult& result, const tKeyFrameList& keys, f32 pMaxErrorInMeters, f32 rMaxErrorInPctg, f32 sMaxErrorInPctg )
	{
		// first we decompose the keys so we can analyze position, rotation, and scale independently
		fDecomposeKeys( result.mPRSKeys, keys );

		// make sure we have at least one key
		if( result.mPRSKeys.fCount( ) == 0 )
		{
			result.mPRSKeys.fNewArray( 1 );
			result.mPRSKeys.fFront( ) = Math::tPRSXformf::cIdentity;
		}

		// set compression key array to same size
		result.mCompressionKeys.fNewArray( result.mPRSKeys.fCount( ) );

		// compress the keys, as long as there's more than 2
		if( result.mPRSKeys.fCount( ) > 2 )
		{
			fDeltaCompressPosition( result, pMaxErrorInMeters ); // max error in meters (so 1.f/100.f or 0.01f would be 1cm)
			fDeltaCompressRotation( result, rMaxErrorInPctg ); // max error as a percentage... i.e., all quats are assumed unit length, so 1.f/100.f or 0.01f is 1% error
			fDeltaCompressScale(	result, sMaxErrorInPctg ); // max error = ?
		}

		// compute combined compression keys
		for( u32 i = 0; i < result.mCompressionKeys.fCount( ); ++i )
		{
			tDeltaCompressionKey& dck = result.mCompressionKeys[ i ];
			if( dck.mP )
			{
				++result.mPKeyCount;
				const f32 pMin = result.mPRSKeys[ i ].mPosition.fMin( );
				const f32 pMax = result.mPRSKeys[ i ].mPosition.fMax( );
				if( pMin < result.mPMin ) result.mPMin = pMin;
				if( pMax > result.mPMax ) result.mPMax = pMax;
			}
			if( dck.mR )
			{
				++result.mRKeyCount;
			}
			if( dck.mS )
			{
				++result.mSKeyCount;
				const f32 sMin = result.mPRSKeys[ i ].mScale.fMin( );
				const f32 sMax = result.mPRSKeys[ i ].mScale.fMax( );
				if( sMin < result.mSMin ) result.mSMin = sMin;
				if( sMax > result.mSMax ) result.mSMax = sMax;
			}
		}

		// check for equal end points on key frame array that have only two significant keys
		if( result.mPKeyCount == 2 && result.mPRSKeys.fFront( ).mPosition == result.mPRSKeys.fBack( ).mPosition )
		{
			result.mPKeyCount -= 1;
			result.mCompressionKeys.fBack( ).mP = false;
		}
		if( result.mRKeyCount == 2 && result.mPRSKeys.fFront( ).mRotation == result.mPRSKeys.fBack( ).mRotation )
		{
			result.mRKeyCount -= 1;
			result.mCompressionKeys.fBack( ).mR = false;
		}
		if( result.mSKeyCount == 2 && result.mPRSKeys.fFront( ).mScale == result.mPRSKeys.fBack( ).mScale )
		{
			result.mSKeyCount -= 1;
			result.mCompressionKeys.fBack( ).mS = false;
		}
	}

	template<class tKeyCompare>
	void fDeltaCompress( tDeltaCompressionResult& result, f32 maxError )
	{
		tKeyCompare comparator;
		const f32 maxSqError = maxError * maxError;
		const u32 numKeys = result.mCompressionKeys.fCount( );

		u32 ileft = 0, iright = 2;
		while( iright < numKeys )
		{
			const Math::tPRSXformf& l = result.mPRSKeys[ ileft ];

			for( ; iright < numKeys; ++iright )
			{
				const u32 i			= iright - 1;
				const u32 span		= iright - ileft;
				const f32 t			= f32( i - ileft ) / span;

				const f32 sqError = comparator.fSqError( l, result.mPRSKeys[ iright ], result.mPRSKeys[ i ], t );
				if( sqError > maxSqError )
					break;
				comparator.fClearCompressionKey( result.mCompressionKeys[ i ] );
			}

			ileft	= iright - 1;
			iright	= iright + 1;
		}
	}

	struct tDeltaCompressPosition
	{
		f32 fSqError( const Math::tPRSXformf& l, const Math::tPRSXformf& r, const Math::tPRSXformf& actual, f32 t )
		{
			return ( actual.mPosition - Math::fLerp( l.mPosition, r.mPosition, t ) ).fLengthSquared( );
		}
		void fClearCompressionKey( tDeltaCompressionKey& dck ) { dck.mP = false; }
	};
	struct tDeltaCompressRotation
	{
		f32 fSqError( const Math::tPRSXformf& l, const Math::tPRSXformf& r, const Math::tPRSXformf& actual, f32 t )
		{
			return ( actual.mRotation - Math::fNLerp( l.mRotation, r.mRotation, t ) ).fLengthSquared( );
		}
		void fClearCompressionKey( tDeltaCompressionKey& dck ) { dck.mR = false; }
	};
	struct tDeltaCompressScale
	{
		f32 fSqError( const Math::tPRSXformf& l, const Math::tPRSXformf& r, const Math::tPRSXformf& actual, f32 t )
		{
			return ( actual.mScale - Math::fLerp( l.mScale, r.mScale, t ) ).fLengthSquared( );
		}
		void fClearCompressionKey( tDeltaCompressionKey& dck ) { dck.mS = false; }
	};

	void tBone::fDeltaCompressPosition( tDeltaCompressionResult& result, f32 maxError )
	{
		Sig::Animl::fDeltaCompress<tDeltaCompressPosition>( result, maxError );
	}

	void tBone::fDeltaCompressRotation( tDeltaCompressionResult& result, f32 maxError )
	{
		Sig::Animl::fDeltaCompress<tDeltaCompressRotation>( result, maxError );
	}

	void tBone::fDeltaCompressScale( tDeltaCompressionResult& result, f32 maxError )
	{
		Sig::Animl::fDeltaCompress<tDeltaCompressScale>( result, maxError );
	}

	///
	/// \section tSkeleton
	///

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tSkeleton& o )
	{
		s( "RefFrame", o.mRefFrame );
		s( "Bones", o.mBones );
	}

	///
	/// \section tFile
	///

	namespace
	{
		static u32 gAnimlVersion = 1;
	}

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tFile& o )
	{
		s.fAsAttribute( "Version", o.mVersion );

		s( "ModellingPackage", o.mModellingPackage );
		s( "SrcFile", o.mSrcFile );

		if( o.mVersion != gAnimlVersion )
		{
			log_warning( "Animl file format is out of date (exported from " << o.mSrcFile << ") -> Please re-export." );
			return;
		}

		s( "FramesPerSecond", o.mFramesPerSecond );
		s( "TotalFrames", o.mTotalFrames );
		s( "Skeleton", o.mSkeleton );
	}

	tFile::tFile( )
		: mVersion( gAnimlVersion )
		, mFramesPerSecond( 0 )
		, mTotalFrames( 0 )
	{
	}

	b32 tFile::fSaveXml( const tFilePathPtr& path, b32 promptToCheckout )
	{
		tXmlSerializer ser;
		if( !ser.fSave( path, "Animl", *this, promptToCheckout ) )
		{
			log_warning( "Couldn't save Animl file [" << path << "]" );
			return false;
		}

		return true;
	}

	b32 tFile::fLoadXml( const tFilePathPtr& path )
	{
		tXmlDeserializer des;
		if( !des.fLoad( path, "Animl", *this ) )
		{
			log_warning( "Couldn't load Animl file [" << path << "]" );
			return false;
		}

		return true;
	}

	const tBone* tFile::fFindBone( const char* name ) const
	{
		for( u32 i = 0; i < mSkeleton.mBones.fCount( ); ++i )
		{
			if( _stricmp( name, mSkeleton.mBones[ i ]->mName.c_str( ) ) == 0 )
				return mSkeleton.mBones[ i ].fGetRawPtr( );
		}

		return 0;
	}

}}

