#ifndef __tIntersectionAabbSphere__
#define __tIntersectionAabbSphere__

namespace Sig { namespace Math
{

	template<class t>
	class tIntersectionAabbSphere
	{
	protected:

		b32				mContained; ///< This will be true if the aabb is fully contained in the sphere
		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:

		inline tIntersectionAabbSphere( ) { }

		inline tIntersectionAabbSphere( const tAabb<t>& a, const tSphere<t>& b )
		{
			fIntersect( a, b );
		}
		inline tIntersectionAabbSphere( const tSphere<t>& a, const tAabb<t>& b )
		{
			fIntersect( b, a );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fContained( ) const { return mContained; }

		inline void fIntersect( const tAabb<t>& a, const tSphere<t>& b )
		{
			const f32 sqRadius = fSquare( b.mRadius );
			if( ( a.mMax - b.mCenter ).fLengthSquared( ) <= sqRadius &&
				( a.mMin - b.mCenter ).fLengthSquared( ) <= sqRadius )
			{
				mContained = true;
				mIntersects = true;
			}
			else
			{
				mContained = false;

				t d = t( 0.f ); 

				// find the square of the distance from the sphere to the box
				for( u32 i = 0; i < 3; ++i )
				{
					if( b.mCenter[ i ] < a.mMin.fAxis( i ) )
					{
						const t s = b.mCenter[ i ] - a.mMin.fAxis( i );
						d += fSquare( s );
					}
					else if( b.mCenter[ i ] > a.mMax.fAxis( i ) )
					{
						const t s = b.mCenter[ i ] - a.mMax.fAxis( i );
						d += fSquare( s ); 
					}
				}

				mIntersects = ( d <= sqRadius ) ? true : false;
				if( !mIntersects )
					mIntersects = a.fContains( b.mCenter );
			}
		}

	};


	template<class t>
	class tIntersectionAabbSphereWithContact
	{
	protected:

		b32				mContained; ///< This will be true if the aabb is fully contained in the sphere
		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

		tVec3f			mContactPt;
		tVec3f			mNormal; /// From sphere to box.
		f32				mPenetration;

	public:

		inline tIntersectionAabbSphereWithContact( ) { }

		inline tIntersectionAabbSphereWithContact( const tAabb<t>& a, const tSphere<t>& b )
		{
			fIntersect( a, b );
		}
		inline tIntersectionAabbSphereWithContact( const tSphere<t>& a, const tAabb<t>& b )
		{
			fIntersect( b, a );
		}

		inline b32 fIntersects( ) const { return mIntersects; }
		inline b32 fContained( ) const { return mContained; }
		inline tVec3f fContactPt( ) const { return mContactPt; }
		inline tVec3f fSphereNormal( ) const { return mNormal; }
		inline f32 fPenetration( ) const { return mPenetration; }

		inline void fIntersect( const tAabb<t>& a, tSphere<t> b, b32 keepNonPenetrating = false )
		{
			mIntersects = true;

			const f32 sqRadius = fSquare( b.mRadius );
			if( ( a.mMax - b.mCenter ).fLengthSquared( ) <= sqRadius &&
				( a.mMin - b.mCenter ).fLengthSquared( ) <= sqRadius )
			{
				mContained = true;

				// make up some stuff, todo find closest way out
				mContactPt = b.mCenter;
				mNormal = tVec3f::cYAxis;
				mPenetration = 0.f;
			}
			else
			{
				mContained = false;

				tVec3f closestPt = fClamp( b.mCenter, a.mMin, a.mMax );
				tVec3f offset = closestPt - b.mCenter;
				f32 sqDist = offset.fLengthSquared( );

				if( sqDist > sqRadius )
				{
					mIntersects = false;
					if( !keepNonPenetrating )
					{
						// make up some results
						mContactPt = b.mCenter;
						mNormal = tVec3f::cYAxis;
						mPenetration = 0.f;
						return;
					}
				}

				f32 separation;
				mNormal = offset.fNormalizeSafe( tVec3f::cYAxis, separation );
				mPenetration = b.mRadius - separation;
				mContactPt = closestPt;
			}
		}

	};

}}

#endif//__tIntersectionAabbSphere__
