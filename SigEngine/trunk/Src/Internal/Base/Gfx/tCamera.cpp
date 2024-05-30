#include "BasePch.hpp"
#include "tCamera.hpp"

namespace Sig { namespace Gfx
{

	tLens::tLens( )
	{
		fSetPerspective( 0.1f, 1.f, 1.f, Math::cPiOver4 );
	}

	tLens::tLens( tNoOpTag )
	{
	}

	void tLens::fSetPerspective( f32 nearPlane, f32 farPlane, f32 aspect, f32 fov )
	{
		mNearPlane = nearPlane;
		mFarPlane = farPlane;

		f32 fovScale = atan( fov );
		f32 halfVpWidth = mFarPlane * fovScale;
		f32 halfVpHeight = mFarPlane * fovScale / aspect;

		mLeft = -halfVpWidth;
		mRight = halfVpWidth;
		mBottom = -halfVpHeight;
		mTop = halfVpHeight;

		mFov = fov;
		mProjectionType = cProjectionPersp;
	}

	void tLens::fSetOrtho( f32 nearPlane, f32 farPlane, f32 left, f32 right, f32 bottom, f32 top )
	{
		mNearPlane = nearPlane;
		mFarPlane = farPlane;
		mLeft = left;
		mRight = right;
		mBottom = bottom;
		mTop = top;
		mFov = 0;
		mProjectionType = cProjectionOrtho;
	}

	void tLens::fSetOrtho( f32 nearPlane, f32 farPlane, f32 width, f32 height )
	{
		fSetOrtho( nearPlane, farPlane, -0.5f * width, 0.5f * width, -0.5f * height, 0.5f * height );
	}

	void tLens::fSetScreen( f32 nearPlane, f32 farPlane, f32 left, f32 right, f32 bottom, f32 top )
	{
		mNearPlane = nearPlane;
		mFarPlane = farPlane;
		mLeft = left;
		mRight = right;
		mBottom = bottom;
		mTop = top;
		mFov = 0;
		mProjectionType = cProjectionScreen;
	}

	void tLens::fSetScreen( f32 nearPlane, f32 farPlane, f32 width, f32 height )
	{
		fSetScreen( nearPlane, farPlane, -0.5f * width, 0.5f * width, -0.5f * height, 0.5f * height );
	}

	void tLens::fChangeFOV( f32 newFOV )
	{
		sigassert( mProjectionType == cProjectionPersp );
		fSetPerspective( mNearPlane, mFarPlane, fAspectRatio( ), newFOV );
	}

	void tLens::fConstructProjectionMatrix( Math::tMat4f& projOut ) const
	{
		if( mProjectionType == cProjectionPersp )
			fConstructProjectionMatrixPersp( projOut );
		else if( mProjectionType == cProjectionOrtho )
			fConstructProjectionMatrixOrtho( projOut );
		else if( mProjectionType == cProjectionScreen )
			fConstructProjectionMatrixScreen( projOut );
		else
		{
			sigassert( !"invalid projection mode" );
		}
	}

	void tLens::fConstructProjectionMatrixPersp( Math::tMat4f& projOut ) const
	{
		const f32 wFar  = (mRight - mLeft);
		const f32 hFar  = (mTop - mBottom);
		const f32 nf = mNearPlane - mFarPlane;

		sigassert( wFar != 0.f );
		sigassert( hFar != 0.f );
		sigassert( nf != 0.f );

		const f32 q = mFarPlane / nf;
		projOut = Math::tMat4f::cIdentity;
		projOut(0,0) = 2.f * mFarPlane / wFar;
		projOut(1,1) = 2.f * mFarPlane / hFar;
		projOut(2,2) = q;
		projOut(2,3) = mNearPlane * q;

		projOut(0,3) = ( mLeft + mRight ) / wFar;
		projOut(1,3) = ( mBottom + mTop ) / hFar;

		projOut(3,3) = 0.f;
		projOut(3,2) = -1.f;

		Math::tMat4f sigToDx = Math::tMat4f::cIdentity;
		sigToDx.fXAxis( -Math::tVec3f::cXAxis );
		sigToDx.fZAxis( -Math::tVec3f::cZAxis );
		projOut = projOut * sigToDx;
	}

	void tLens::fConstructProjectionMatrixOrtho( Math::tMat4f& projOut ) const
	{
		const f32 wFar  = mRight - mLeft;
		const f32 hFar  = mTop - mBottom;
		const f32 nf = mNearPlane - mFarPlane;

		sigassert( wFar  != 0.f );
		sigassert( hFar  != 0.f );
		sigassert( nf != 0.f );

		const f32 q = 1.f / nf;
		projOut = Math::tMat4f::cIdentity;
		projOut(0,0) = 2.f / wFar;
		projOut(1,1) = 2.f / hFar;
		projOut(2,2) = q;
		projOut(2,3) = mNearPlane * q;

		projOut(0,3) = ( mLeft + mRight ) / wFar;
		projOut(1,3) = ( mBottom + mTop ) / hFar;

		Math::tMat4f sigToDx = Math::tMat4f::cIdentity;
		sigToDx.fXAxis( -Math::tVec3f::cXAxis );
		sigToDx.fZAxis( -Math::tVec3f::cZAxis );
		projOut = projOut * sigToDx;
	}

	void tLens::fConstructProjectionMatrixScreen( Math::tMat4f& projOut ) const
	{
		const f32 wFar  = mRight - mLeft;
		const f32 hFar  = mTop - mBottom;
		const f32 fn = mFarPlane - mNearPlane;

		sigassert( hFar != 0.f );
		sigassert( wFar != 0.f );
		sigassert( fn != 0.f );

		const f32 q = 1.f / fn;
		projOut = Math::tMat4f::cIdentity;
		projOut(0,0) = 2.f / wFar;
		projOut(1,1) = 2.f / hFar;
		projOut(2,2) = q;
		projOut(2,3) = mNearPlane * q;

		projOut(0,3) = -( mLeft + mRight ) / wFar;
		projOut(1,3) = -( mBottom + mTop ) / hFar;
	}

	Math::tAabbf tLens::fAAbb( const Math::tMat3f& viewMatrix ) const
	{
		tFixedArray< Math::tVec3f, cCornersCount > points;
		fGetCorners( points, viewMatrix );

		// Initialize bounds to one of the corners
		Math::tAabbf bounds( points[ 0 ], points[ 0 ] );

		// Union in other corners.
		for( u32 i = 1; i < cCornersCount; ++i )
			bounds |= points[ i ];

		return bounds;
	}

	void tLens::fGetCorners( tFixedArray< Math::tVec3f, cCornersCount >& cornersOut, const Math::tMat3f& viewMatrix ) const
	{
		Math::tVec3f p = viewMatrix.fGetTranslation( );
		Math::tVec3f d = viewMatrix.fZAxis( );
		Math::tVec3f up = viewMatrix.fYAxis( );
		Math::tVec3f right = viewMatrix.fXAxis( );
		const f32 w  = mRight - mLeft;
		const f32 h  = mTop - mBottom;
		const f32 toNear = mNearPlane / mFarPlane;

		f32 Hfar = h;
		f32 Wfar = w;
		f32 Hnear = h * toNear;
		f32 Wnear = w * toNear;

		Math::tVec3f fc = p + d * mFarPlane;

		cornersOut[ cFarTopLeft ] = fc + (up * Hfar/2) - (right * Wfar/2);
		cornersOut[ cFarTopRight ] = fc + (up * Hfar/2) + (right * Wfar/2);
		cornersOut[ cFarBottomLeft ] = fc - (up * Hfar/2) - (right * Wfar/2);
		cornersOut[ cFarBottomRight ] = fc - (up * Hfar/2) + (right * Wfar/2);

		Math::tVec3f nc = p + d * mNearPlane;

		cornersOut[ cNearTopLeft ] = nc + (up * Hnear/2) - (right * Wnear/2);
		cornersOut[ cNearTopRight ] = nc + (up * Hnear/2) + (right * Wnear/2);
		cornersOut[ cNearBottomLeft ] = nc - (up * Hnear/2) - (right * Wnear/2);
		cornersOut[ cNearBottomRight ] = nc - (up * Hnear/2) + (right * Wnear/2);
	}

	f32 tLens::fCalculateNearPlaneRadius( ) const
	{
		const f32 toNear = mNearPlane / mFarPlane;
		const f32 heightSq = Math::fSquare( (mTop - mBottom) * toNear );
		const f32 widthSq = Math::fSquare( (mRight - mLeft) * toNear );

		return Math::fSqrt( 0.25f * ( heightSq + widthSq ) );
	}

	tTripod::tTripod( )
		: mEye( Math::tVec3f::cZeroVector )
		, mLookAt( 0.0f, 0.0f, 1.0f )
		, mUp( 0.0f, 1.0f, 0.0f )
	{
	}

	tTripod::tTripod( const Math::tVec3f& eye, const Math::tVec3f& lookAt, const Math::tVec3f& up )
		: mEye( eye )
		, mLookAt( lookAt )
		, mUp( up )
	{
	}

	tTripod::tTripod( const Math::tMat3f& xform )
		: mEye( xform.fGetTranslation( ) )
		, mLookAt( mEye + xform.fZAxis( ) )
		, mUp( xform.fYAxis( ) )
	{
	}

	void tTripod::fConstructCameraMatrix( Math::tMat3f& camOut ) const
	{
		const Math::tVec3f z = ( mLookAt - mEye ).fNormalizeSafe( Math::tVec3f::cZAxis );
		const Math::tVec3f x = mUp.fCross( z ).fNormalizeSafe( Math::tVec3f::cXAxis );
		const Math::tVec3f y = z.fCross( x ).fNormalize( );

		camOut.fSetRow( 0, x );
		camOut.fSetRow( 1, y );
		camOut.fSetRow( 2, z );

		const Math::tVec3f t = -camOut.fXformVector( mEye );
		camOut.fSetTranslation( t );
	}

	void tTripod::fConstructWorldMatrix( Math::tMat3f& worldOut ) const
	{
		const Math::tVec3f z = ( mLookAt - mEye ).fNormalizeSafe( Math::tVec3f::cZAxis );
		Math::tVec3f up = mUp;
		up.fNormalizeSafe( Math::tVec3f::cXAxis );

		worldOut.fOrientZAxis( z, up );
		worldOut.fSetTranslation( mEye );
	}

	tTripod tTripod::fComputeFramedTripod( const Math::tAabbf& aabb ) const
	{
		tTripod target;

		const Math::tVec3f lookAtToEyeDir = ( mEye - mLookAt ).fNormalizeSafe( Math::tVec3f::cZAxis );
		const Math::tVec3f closestPoint = aabb.fMaxVertex( lookAtToEyeDir );
		const Math::tPlanef closestPointPlane = Math::tPlanef( lookAtToEyeDir, closestPoint );
		target.mLookAt = aabb.fComputeCenter( );
		const f32 distancePointToCenter = -closestPointPlane.fSignedDistance( target.mLookAt );
		const f32 distance = distancePointToCenter + aabb.fComputeDiagonal( ).fMaxMagnitude( );
		target.mEye = target.mLookAt + distance * lookAtToEyeDir;
		target.mUp = mUp;

		return target;
	}

	tCamera::tCamera( )
	{
		fUpdateWorldToCamera( );
		fUpdateCameraToProjection( );
		fUpdateFrustum( );
	}

	tCamera::tCamera( const tLens& lens, const tTripod& tripod )
		: mLens( lens )
		, mTripod( tripod )
	{
		fUpdateWorldToCamera( );
		fUpdateCameraToProjection( );
		fUpdateFrustum( );
	}

	void tCamera::fSetup( const tLens& lens, const tTripod& tripod )
	{
		mLens = lens;
		mTripod = tripod;

		fUpdateWorldToCamera( );
		fUpdateCameraToProjection( );
		fUpdateFrustum( );
	}

	void tCamera::fSetFromMatrices( const Math::tMat3f& viewMatrix, const Math::tMat4f& projMatrix )
	{
		mWorldToCamera = viewMatrix;
		mLocalToWorld = mWorldToCamera.fInverseNoScale( );
		mTripod.mEye = mLocalToWorld.fGetTranslation( );
		mCameraToProjection = projMatrix;
		fUpdateFrustum( );
	}

	void tCamera::fCorrectForShadowMap( u32 shadowMapRes )
	{
		sigassert( mLens.mProjectionType == tLens::cProjectionOrtho );

		const u32 halfShadowMapRes = shadowMapRes / 2;

		mWorldToProjection(0,3) *= halfShadowMapRes;
		mWorldToProjection(0,3) = fRoundDown<f32>( mWorldToProjection(0,3) );
		mWorldToProjection(0,3) /= halfShadowMapRes;

		mWorldToProjection(1,3) *= halfShadowMapRes;
		mWorldToProjection(1,3) = fRoundDown<f32>( mWorldToProjection(1,3) );
		mWorldToProjection(1,3) /= halfShadowMapRes;
	}

	void tCamera::fSetLens( const tLens& lens )
	{
		mLens = lens;

		fUpdateCameraToProjection( );
		fUpdateFrustum( );
	}

	void tCamera::fSetTripod( const tTripod& tripod )
	{
		mTripod = tripod;

		fUpdateWorldToCamera( );
		fUpdateFrustum( );
	}

	void tCamera::fSetTripodAndLens( const tTripod& tripod, const tLens& lens )
	{
		mTripod = tripod;
		mLens = lens;

		fUpdateCameraToProjection( );
		fUpdateWorldToCamera( );
		fUpdateFrustum( );
	}

	void tCamera::fMoveGlobal( const Math::tVec3f& deltaInWorldSpace )
	{
		mTripod.mEye += deltaInWorldSpace;
		mTripod.mLookAt += deltaInWorldSpace;
		fUpdateWorldToCamera( );
		fUpdateFrustum( );
	}

	void tCamera::fMoveLocal( const Math::tVec3f& deltaInLocalSpace )
	{
		fMoveGlobal( deltaInLocalSpace.x * fXAxis( ) +
					 deltaInLocalSpace.y * fYAxis( ) +
					 deltaInLocalSpace.z * fZAxis( ) );
	}

	void tCamera::fSetEye( const Math::tVec3f& newEye )
	{
		mTripod.mEye = newEye;
		fUpdateWorldToCamera( );
		fUpdateFrustum( );
	}

	void tCamera::fSetLookAt( const Math::tVec3f& newLookAt )
	{
		mTripod.mLookAt = newLookAt;
		fUpdateWorldToCamera( );
		fUpdateFrustum( );
	}
	
	void tCamera::fSetUp( const Math::tVec3f& newUp )
	{
		mTripod.mUp = Math::tVec3f( newUp ).fNormalizeSafe( Math::tVec3f::cYAxis );
		fUpdateWorldToCamera( );
		fUpdateFrustum( );
	}


	Math::tRayf tCamera::fComputePickRay( 
		const Math::tVec2u& windowPos, 
		const Math::tVec2u& windowRes ) const
	{
		// invert the current Proj * View transformation
		sync_event_v_c( mWorldToProjection, tSync::cSCCamera | tSync::cSCRaycast );
		const Math::tMat4f projToWorld = mWorldToProjection.fInverse( );

		return fComputePickRay( windowPos, windowRes, projToWorld );
	}

	Math::tRayf tCamera::fComputePickRay( 
		const Math::tVec2u& windowPos, 
		const Math::tVec2u& windowRes,
		const Math::tMat4f& projToWorld ) const
	{
		const Math::tVec3f world = fUnproject( windowPos, windowRes, projToWorld );

		// return ray originating from eye, directed toward point
		Math::tRayf ray;
		
		if( mLens.mProjectionType == tLens::cProjectionPersp )
			ray = Math::tRayf( mTripod.mEye, world - mTripod.mEye );
		else
			ray = Math::tRayf( world, fZAxis( ) );

		// extend the ray way out
		ray.mExtent = Math::cInfinity * Math::tVec3f( ray.mExtent ).fNormalizeSafe( Math::tVec3f::cZAxis );

		// return the extended ray
		return ray;
	}

	Math::tVec3f tCamera::fUnproject( 
		const Math::tVec2u& windowPos, 
		const Math::tVec2u& windowRes,
		const Math::tMat4f& projToWorld ) const
	{
		if( mLens.mProjectionType == tLens::cProjectionPersp )
		{
			// convert screen pos to normalized device coordinates
			const Math::tVec4f ndc( 
				2.f * ( f32 )windowPos.x / windowRes.x - 1.f, 
				2.f * ( f32 )( windowRes.y - windowPos.y ) / windowRes.y - 1.f,
				0.01f,
				1.f );

			// unproject point into world space
			const Math::tVec4f unprojected = projToWorld.fXform( ndc );
			return Math::tVec3f( unprojected.x, unprojected.y, unprojected.z ) / unprojected.w;
		}
		else//if( mLens.mProjectionType == tLens::cProjectionOrtho || mLens.mProjectionType == tLens::cProjectionScreen )
		{
			// convert screen pos to normalized device coordinates
			Math::tVec4f ndc( 
				( f32 )windowPos.x / windowRes.x, 
				( f32 )windowPos.y / windowRes.y,
				0.01f,
				1.f );

			const f32 l = mLens.mLeft;
			const f32 r = mLens.mRight;
			const f32 t = mLens.mTop;
			const f32 b = mLens.mBottom;

			ndc.x = l + ndc.x * ( r - l );
			ndc.y = b + (1.f-ndc.y) * ( t - b );

			return
				mTripod.mEye
				- ndc.x * fXAxis( )
				+ ndc.y * fYAxis( )
				+ ndc.z * fZAxis( );
		}
	}

	Math::tVec3f tCamera::fProject( const Math::tVec3f& worldPos ) const
	{
		const Math::tVec4f ndc = mWorldToProjection.fXform( Math::tVec4f( worldPos, 1.f ) );
		if( fAbs( ndc.w ) < 0.001f )
			return Math::tVec3f::cZeroVector;
		const Math::tVec3f o = ndc.fXYZ( ) / ndc.w;
		return Math::tVec3f( 0.5f *( o.x + 1.f ), 1.f - 0.5f * ( o.y + 1.f ), o.z );
	}

	void tCamera::fComputePickFrustum( 
		Math::tFrustumf& frustum,
		const Math::tVec2u& tl, 
		const Math::tVec2u& tr, 
		const Math::tVec2u& bl, 
		const Math::tVec2u& br, 
		const Math::tVec2u& windowRes ) const
	{
		const b32 ortho = ( mLens.mProjectionType != tLens::cProjectionPersp );

		// invert the current Proj * View transformation
		const Math::tMat4f projToWorld = mWorldToProjection.fInverse( );

		// unproject the four corners of the screen-space rectangle
		const Math::tVec3f v0 = fUnproject( tl,	windowRes, projToWorld );
		const Math::tVec3f v1 = fUnproject( br,	windowRes, projToWorld );
		const Math::tVec3f v2 = fUnproject( bl,	windowRes, projToWorld );
		const Math::tVec3f v3 = fUnproject( tr,	windowRes, projToWorld );

		const f32 veryLargeNumber = 9999999.f;

		if( ortho )
		{
			// create the frustum planes
			frustum[ Math::tFrustumf::cPlaneBack  ] = Math::tPlanef(  fZAxis( ), v0 + Math::tVec3f( fZAxis( ) ).fSetLength( veryLargeNumber ) );
			frustum[ Math::tFrustumf::cPlaneFront ] = Math::tPlanef( -fZAxis( ), v0 );
			frustum[ Math::tFrustumf::cPlaneLeft  ] = Math::tPlanef(  fXAxis( ), v0 );
			frustum[ Math::tFrustumf::cPlaneRight ] = Math::tPlanef( -fXAxis( ), v1 );
			frustum[ Math::tFrustumf::cPlaneTop   ] = Math::tPlanef(  fYAxis( ), v0 );
			frustum[ Math::tFrustumf::cPlaneBottom] = Math::tPlanef( -fYAxis( ), v1 );
		}
		else
		{
			const Math::tVec3f lookDir = ( mTripod.mLookAt - mTripod.mEye ).fNormalizeSafe( Math::tVec3f::cZAxis );
			const Math::tVec3f origin = mTripod.mEye;

			// compute the four rays starting from the eye, extending through the unprojected positions
			Math::tRayf ray0( origin, v0 - origin );
			Math::tRayf ray1( origin, v1 - origin );
			Math::tRayf ray2( origin, v2 - origin );
			Math::tRayf ray3( origin, v3 - origin );

			// compute the 6 plane normals for the frustum
			const Math::tVec3f nBack = lookDir;
			const Math::tVec3f nFront = -lookDir;
			const Math::tVec3f nLeft = ray0.mExtent.fCross( ray2.mExtent ).fNormalizeSafe( Math::tVec3f::cZAxis ); //Just making random assumptions here, bad if these aren't > 0
			const Math::tVec3f nRight = ray1.mExtent.fCross( ray3.mExtent ).fNormalizeSafe( Math::tVec3f::cZAxis );
			const Math::tVec3f nTop = ray3.mExtent.fCross( ray0.mExtent ).fNormalizeSafe( Math::tVec3f::cZAxis );
			const Math::tVec3f nBot = ray2.mExtent.fCross( ray1.mExtent ).fNormalizeSafe( Math::tVec3f::cZAxis );

			// create the frustum planes
			frustum[ Math::tFrustumf::cPlaneBack  ] = Math::tPlanef( nBack, origin + Math::tVec3f( lookDir ).fSetLength( veryLargeNumber ) );
			frustum[ Math::tFrustumf::cPlaneFront ] = Math::tPlanef( nFront, origin );
			frustum[ Math::tFrustumf::cPlaneLeft  ] = Math::tPlanef( nLeft, origin );
			frustum[ Math::tFrustumf::cPlaneRight ] = Math::tPlanef( nRight, origin );
			frustum[ Math::tFrustumf::cPlaneTop   ] = Math::tPlanef( nTop, origin );
			frustum[ Math::tFrustumf::cPlaneBottom] = Math::tPlanef( nBot, origin );
		}
	}

	void tCamera::fUpdateWorldToCamera( )
	{
		mTripod.fConstructCameraMatrix( mWorldToCamera );
		sigassert( !mWorldToCamera.fIsNan( ) );
		mLocalToWorld = mWorldToCamera.fInverseNoScale( );
	}

	void tCamera::fUpdateCameraToProjection( )
	{
		mLens.fConstructProjectionMatrix( mCameraToProjection );
		sigassert( !mCameraToProjection.fIsNan( ) );
	}

	void tCamera::fUpdateFrustum( )
	{
		mWorldToProjection = mCameraToProjection * Math::tMat4f( mWorldToCamera );
		sigassert( !mWorldToProjection.fIsNan( ) );

		// calculate planes from combined mWorldToProjection matrix
		// this calculation creates a frustum with outwardly facing
		// normals, and assumes normalized device coordinates of
		// -1 <= x < 1, -1 <= y < 1, 0 <= z < 1
		mWorldSpaceFrustum[Math::tFrustumf::cPlaneLeft]		= -mWorldToProjection[0] - mWorldToProjection[3];
		mWorldSpaceFrustum[Math::tFrustumf::cPlaneRight]	=  mWorldToProjection[0] - mWorldToProjection[3];
		mWorldSpaceFrustum[Math::tFrustumf::cPlaneBottom]	= -mWorldToProjection[1] - mWorldToProjection[3];
		mWorldSpaceFrustum[Math::tFrustumf::cPlaneTop]		=  mWorldToProjection[1] - mWorldToProjection[3];
		mWorldSpaceFrustum[Math::tFrustumf::cPlaneFront]	= -mWorldToProjection[2];
		mWorldSpaceFrustum[Math::tFrustumf::cPlaneBack]		=  mWorldToProjection[2] - mWorldToProjection[3];

		// normalize the planes so distances computed will be "true"
		mWorldSpaceFrustum.fNormalize( );
	}

	Math::tAabbf tCamera::fWorldSpaceFrustumBounds( ) const
	{
		return mLens.fAAbb( mLocalToWorld );
	}

}}

