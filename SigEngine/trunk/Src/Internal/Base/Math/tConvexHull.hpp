#ifndef __tConvexHull__
#define __tConvexHull__

namespace Sig { 

	class tMeshEntity;

namespace Math
{

	class base_export tConvexHull
	{
		declare_reflector( );
	public:
		struct tFace
		{
			declare_reflector( );

			u16 mA;
			u16 mB;
			u16 mC;

			tFace( ) { }
			tFace( u16 a, u16 b, u16 c );

			void fFlip( );
			tVec3f fNormal( const tDynamicArray<Math::tVec3f>& verts ) const; //NOT normalized

			template< typename tSerializer >
			void fSerializeXml( tSerializer& s )
			{
				s( "A", mA );
				s( "B", mB );
				s( "C", mC );
			}
		};

		const tDynamicArray< tFace >&	fFaces( ) const { return mFaces; }
		const tDynamicArray< tVec2u >&	fEdges( ) const { return mEdges; }
		const tDynamicArray< tVec3f >&	fVerts( ) const { return mVerts; }

		tConvexHull fTransform( const tMat3f& xform ) const;
		tAabbf fToAABB( ) const;
		tVec3f fComputeCenter( ) const;
		tTrianglef fMakeTriangle( u32 face ) const;
		u32 fSupportingVertexIndex( const Math::tVec3f& dir ) const;
		tVec3f fSupportingVertex( const Math::tVec3f& dir ) const;
		b32 fContainsPtGJK( const Math::tMat3f& hullXform, const Math::tVec3f& pt ) const;

	public: //Builder interface
		void fClear( )
		{
			mFaces.fResize( 0 );
			mEdges.fResize( 0 );
			mVerts.fResize( 0 );
		}

		void fConstruct( tMeshEntity* mesh, const tMat3f& invTransform );
		void fConstruct( const tGrowableArray<tVec3f>& points, const Math::tMat3f& invTransform );
		void fConstruct( const tDynamicArray<tVec3f>& verts, const tDynamicArray<u32>& indices );
		void fOptimize( );

		// ensure the faces are all actually convex
		b32 fValid( ) const;

	private:
		tDynamicArray< tVec3f > mVerts;
		tDynamicArray< tFace > mFaces;
		tDynamicArray< tVec2u > mEdges;

		void fComputeNormal( u32 face );
		void fRemoveVertex( u32 vert );
		b32 fTestRemoveEdge( u32 face, u32 a, u32 b );
		u32 fTestAddEdge( u32 a, u32 b );

	public: //serialization
		template< typename tSerializer >
		void fSerializeXml( tSerializer& s )
		{
			s( "V", mVerts );
			s( "F", mFaces );
			s( "E", mEdges );
		}
	};


	class base_export tCapsule
	{
	public:
		// any random capsule shape that is non zero sized.
		static const tCapsule cNonZeroSized;

	public:
		tVec3f mCenter;
		tVec3f mPrimaryAxis; //along the main axis (+cYAxis if "not rotated")
		f32 mHalfHeight; //of the flat part
		f32 mRadius;

		tCapsule( const Math::tVec3f& center, const Math::tVec3f& primaryAxis, f32 halfHeight, f32 radius )
			: mCenter( center )
			, mPrimaryAxis( primaryAxis )
			, mHalfHeight( halfHeight )
			, mRadius( radius )
		{ }

		// Will use the translation, y axis, and the fDimsFromScale for the height and radius magnitudes
		tCapsule( const Math::tMat3f& shapeEntityXform )
		{
			mCenter = shapeEntityXform.fGetTranslation( );
			mPrimaryAxis = shapeEntityXform.fYAxis( ).fNormalizeSafe( tVec3f::cYAxis );
			
			tVec2f dims = fDimsFromScale( shapeEntityXform.fGetScale( ) );
			mRadius = dims.x;
			mHalfHeight = dims.y;
		}

		// returns an unscaled xform aligned to the capsule, not the same as the shapeEntityXform above
		tMat3f fGetTransform( ) const
		{
			return tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, mPrimaryAxis ) ), mCenter );
		}

		tCapsule fTransform( const Math::tMat3f& xform ) const
		{
			sigassert( xform.fGetScale( ).fEqual( tVec3f::cOnesVector ) && "Capsule does not support scaling!" );
			return tCapsule( xform.fXformPoint( mCenter ), xform.fXformVector( mPrimaryAxis ), mHalfHeight, mRadius );
		}

		tAabbf fToAABB( ) const
		{
			const tVec3f radius( mRadius );
			const Math::tVec3f halfAxis = mPrimaryAxis * mHalfHeight;
			const Math::tVec3f center1 = mCenter - halfAxis;
			const Math::tVec3f center2 = mCenter + halfAxis;

			Math::tAabbf shape( center1 - radius, center1 + radius );
			shape |= center2 + radius;
			shape |= center2 - radius;

			return shape;
		}

		// returns the radius and half capsule height (flat part)
		static tVec2f fDimsFromScale( const tVec3f& scale )
		{
			f32 height = scale.y;
			f32 radius = fMin( scale.x, scale.z );

			radius = fMin( height, radius );
			height = height - radius;

			if( height < 0 )
			{
				height = 0;
				radius = scale.y;
			}

			return tVec2f( radius, height );
		}
	};


	class base_export tCylinder
	{
	public:
		Math::tVec3f mCenter;
		Math::tVec3f mPrimaryAxis;	///< Lengthwise along the cylinder (+cYAxis if "not rotated")
		f32 mHalfHeight;
		f32 mRadius;

		tCylinder( const Math::tVec3f& center, const Math::tVec3f& primaryAxis, f32 halfHeight, f32 radius )
			: mCenter( center )
			, mPrimaryAxis( primaryAxis )
			, mHalfHeight( halfHeight )
			, mRadius( radius )
		{ }

		// Will use the translation, y axis, and the fDimsFromScale for the height and radius magnitudes
		tCylinder( const Math::tMat3f& shapeEntityXform )
		{
			mCenter = shapeEntityXform.fGetTranslation( );
			mPrimaryAxis = shapeEntityXform.fYAxis( ).fNormalizeSafe( tVec3f::cYAxis );

			tVec2f dims = fDimsFromScale( shapeEntityXform.fGetScale( ) );
			mRadius = dims.x;
			mHalfHeight = dims.y;
		}

		// returns an unscaled xform aligned to the cylinder, not the same as the shapeEntityXform above
		tMat3f fGetTransform( ) const
		{
			return tMat3f( tQuatf( tAxisAnglef( tVec3f::cYAxis, mPrimaryAxis ) ), mCenter );
		}

		tCylinder fTransform( const Math::tMat3f& xform ) const
		{
			sigassert( xform.fGetScale( ).fEqual( tVec3f::cOnesVector ) && "Cylinder does not support scaling!" );
			return tCylinder( xform.fXformPoint( mCenter ), xform.fXformVector( mPrimaryAxis ), mHalfHeight, mRadius );
		}

		tAabbf fToAABB( ) const
		{
			tVec3f arm( mRadius, mHalfHeight, mRadius );
			return Math::tAabbf( -arm, arm ).fTransform( fGetTransform( ) );
		}

		// returns the radius and half height
		static tVec2f fDimsFromScale( const tVec3f& scale )
		{
			f32 height = scale.y;
			f32 radius = fMin( scale.x, scale.z );
			return tVec2f( radius, height );
		}

	};

}}


#endif//__tConvexHull__
