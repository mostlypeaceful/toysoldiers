#ifndef __tCamera__
#define __tCamera__

namespace Sig { namespace Gfx
{
	///
	/// \brief Simple class for managing projection-transform.
	class base_export tLens
	{
	public:
		enum tProjectionType
		{
			cProjectionPersp,
			cProjectionOrtho,
			cProjectionScreen,
			cProjectionMax
		};

	public:
		f32 mNearPlane;
		f32 mFarPlane;

		// far plane size
		f32 mLeft, mRight;
		f32 mBottom, mTop;
		f32 mFov; // Not necessary to store, possible to recompute, stored for convenience.

		u32 mProjectionType;

	public:
		struct tIveFixed { };

		tLens( );
		tLens( tNoOpTag );

		// Aspect ratio is expected to be width/height
		// FOV is half horizontal, in radians, typically 45deg (cPiOver4), for a total of 90deg of vision.
		void fSetPerspective( f32 nearPlane, f32 farPlane, f32 aspect, f32 fov );
		void fSetOrtho( f32 nearPlane, f32 farPlane, f32 left, f32 right, f32 bottom, f32 top );
		void fSetOrtho( f32 nearPlane, f32 farPlane, f32 width, f32 height );
		void fSetScreen( f32 nearPlane, f32 farPlane, f32 left, f32 right, f32 bottom, f32 top );
		void fSetScreen( f32 nearPlane, f32 farPlane, f32 width, f32 height );

		// Only for perspective. (asserts)
		void fChangeFOV( f32 newFOV );

		f32 fFOV( ) const { return mFov; }
		f32 fWidth( ) const { return mRight - mLeft; }
		f32 fHeight( ) const { return mTop - mBottom; }
		f32 fAspectRatio( ) const { return fWidth( ) / fHeight( ); }

		void fConstructProjectionMatrix( Math::tMat4f& projOut ) const;

		Math::tAabbf fAAbb( const Math::tMat3f& viewMatrix ) const;

		enum tCorners
		{
			cFarTopLeft,
			cFarTopRight,
			cFarBottomLeft,
			cFarBottomRight,
			cNearTopLeft,
			cNearTopRight,
			cNearBottomLeft,
			cNearBottomRight,
			cCornersCount
		};
		void fGetCorners( tFixedArray< Math::tVec3f, cCornersCount >& cornersOut, const Math::tMat3f& viewMatrix ) const;

		f32 fCalculateNearPlaneRadius( ) const;

	private:
		void fConstructProjectionMatrixPersp( Math::tMat4f& projOut ) const;
		void fConstructProjectionMatrixOrtho( Math::tMat4f& projOut ) const;
		void fConstructProjectionMatrixScreen( Math::tMat4f& projOut ) const;
	};

	///
	/// \brief Simple class for managing view-transform.
	class base_export tTripod
	{
	public:
		Math::tVec3f mEye;
		Math::tVec3f mLookAt;
		Math::tVec3f mUp;
	public:
		tTripod( );
		tTripod( const Math::tVec3f& eye, const Math::tVec3f& lookAt, const Math::tVec3f& up );
		explicit tTripod( const Math::tMat3f& xform );
		void	fConstructCameraMatrix( Math::tMat3f& camOut ) const;
		void	fConstructWorldMatrix( Math::tMat3f& worldOut ) const;
		tTripod fComputeFramedTripod( const Math::tAabbf& aabb ) const;
		void fTranslate( const Math::tVec3f& delta ) { mEye += delta; mLookAt += delta; }
		Math::tVec3f fLookDelta( ) const { return mLookAt - mEye; }

		template<class tArchive>
		void fSaveLoad( tArchive& archive )
		{
			archive.fSaveLoad( mEye );
			archive.fSaveLoad( mLookAt );
			archive.fSaveLoad( mUp );
		}
	};

	///
	/// \brief Manages camera-related transformation matrices and frusta. Combines
	/// tTripod and tLens.
	class base_export tCamera
	{
	protected:

		tLens				mLens;
		tTripod				mTripod;

		Math::tMat3f		mLocalToWorld;
		Math::tMat3f		mWorldToCamera;
		Math::tMat4f		mCameraToProjection;
		Math::tMat4f		mWorldToProjection; ///< combines mWorldToCamera and mCameraToProjection
		Math::tFrustumf		mWorldSpaceFrustum;

	public:

		tCamera( );
		tCamera( const tLens& lens, const tTripod& tripod );
		void fSetup( const tLens& lens, const tTripod& tripod );
		void fSetFromMatrices( const Math::tMat3f& viewMatrix, const Math::tMat4f& projMatrix );
		void fCorrectForShadowMap( u32 shadowMapRes );
		void fSetLens( const tLens& lens );
		void fSetTripod( const tTripod& tripod );
		void fSetTripodAndLens( const tTripod& tripod, const tLens& lens );
		void fMoveGlobal( const Math::tVec3f& deltaInWorldSpace );
		void fMoveLocal( const Math::tVec3f& deltaInLocalSpace );
		void fSetEye( const Math::tVec3f& newEye );
		void fSetLookAt( const Math::tVec3f& newLookAt );
		void fSetUp( const Math::tVec3f& newUp );

		///
		/// \brief Compute a picking ray from the camera's eye to
		/// the supplied windowPos; windowPos is "un-projected"
		/// using the inverse of the camera world-to-projection matrix;
		/// note that this inverse is computed in the method; if you
		/// already have this, call the other variant.
		/// \return A ray from the camera's eye to the unprojected windowPos in world space.
		Math::tRayf fComputePickRay( 
			const Math::tVec2u& windowPos, 
			const Math::tVec2u& windowRes ) const;
		
		///
		/// \brief This version accepts the inverted projection-to-world matrix,
		/// and hence doesn't have to compute the inverse (faster if you already
		/// have this data lying around).
		Math::tRayf fComputePickRay( 
			const Math::tVec2u& windowPos, 
			const Math::tVec2u& windowRes,
			const Math::tMat4f& projToWorld ) const;

		///
		/// \brief Unproject the screen space 'windowPos' to 3d world space.
		Math::tVec3f fUnproject( 
			const Math::tVec2u& windowPos, 
			const Math::tVec2u& windowRes,
			const Math::tMat4f& projToWorld ) const;

		///
		/// \brief Project the world-space 'worldPos' to 2d screen space (0-1,0-1).
		Math::tVec3f fProject( 
			const Math::tVec3f& worldPos ) const;

		///
		/// \brief Compute a frustum from the eye through the 
		/// unprojection of the screen-space rectangle specified
		/// by tl, tr, bl, br (top-left, top-right, bottom-left, bottom-right).
		/// I.e., this is used for drag-rectangle selection.
		void fComputePickFrustum( 
			Math::tFrustumf& frustum,
			const Math::tVec2u& tl, 
			const Math::tVec2u& tr, 
			const Math::tVec2u& bl, 
			const Math::tVec2u& br, 
			const Math::tVec2u& windowRes ) const;

		inline const tLens&			fGetLens( ) const { return mLens; }
		inline const tTripod&		fGetTripod( ) const { return mTripod; }

		inline const Math::tMat3f&	fGetWorldToCamera( ) const		{ return mWorldToCamera; }
		inline const Math::tMat4f&	fGetCameraToProjection( ) const	{ return mCameraToProjection; }
		inline const Math::tMat4f&	fGetWorldToProjection( ) const	{ return mWorldToProjection; }
		inline const Math::tFrustumf& fGetWorldSpaceFrustum( ) const { return mWorldSpaceFrustum; }

		Math::tAabbf fWorldSpaceFrustumBounds( ) const;

		inline Math::tVec3f			fXAxis( ) const { return mWorldToCamera.fGetRow( 0 ); }
		inline Math::tVec3f			fYAxis( ) const { return mWorldToCamera.fGetRow( 1 ); }
		inline Math::tVec3f			fZAxis( ) const { return mWorldToCamera.fGetRow( 2 ); }
		inline const Math::tMat3f&	fLocalToWorld( ) const { return mLocalToWorld; }

		inline Math::tVec3f fXformCameraRelativeVector( const Math::tVec3f& v ) const
		{
			return Math::tVec3f(
				v.fDot( mWorldToCamera.fXAxis( ) ),
				v.fDot( mWorldToCamera.fYAxis( ) ),
				v.fDot( mWorldToCamera.fZAxis( ) ) );
		}

		inline f32 fCameraDepth( const Math::tVec3f& pointInWorld ) const
		{
			return -mWorldSpaceFrustum[ Math::tFrustumf::cPlaneFront ].fSignedDistance( pointInWorld );
		}

		inline u16 fCameraDepthU16( const Math::tVec3f& pointInWorld ) const
		{
			const f32 d = fClamp( fCameraDepth( pointInWorld ), mLens.mNearPlane, mLens.mFarPlane );
			return fRound<u16>( ( d - mLens.mNearPlane ) / ( mLens.mFarPlane - mLens.mNearPlane ) );
		}

	private:

		void fUpdateWorldToCamera( );
		void fUpdateCameraToProjection( );
		void fUpdateFrustum( );
	};

}}


#endif//__tCamera__

