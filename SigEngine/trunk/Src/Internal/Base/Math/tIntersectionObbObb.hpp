#ifndef __tIntersectionObbObb__
#define __tIntersectionObbObb__

#include "Physics/tContactPoint.hpp"

namespace Sig { namespace Math
{

	class base_export tIntersectionObbObbWithContact
	{
	protected:

		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

	public:

		// The points are on B, and the normal points to A
		Physics::tContactPoint mResult;

		// RND 
		// which ever pointer in the this result is non zero is the one with the vertex.
		// so if mB is non zero then obb b is the one with the vertex.
		Physics::tFullContactWitness mFullResult;

		tIntersectionObbObbWithContact( ) 
		{ }

		tIntersectionObbObbWithContact( const tObb<f32>& a, const tObb<f32>& b, b32 keepNonPenetrating = false, b32 sphereCheckFirst = true )
		{
			fIntersect( a, b, keepNonPenetrating, sphereCheckFirst );
		}

		// Will return normals sticking out of B.
		//  pass true to keepNonPenetrating if you want a closest contact even if they are disjoint (slower)
		void fIntersect( const tObb<f32>& a, const tObb<f32>& b, b32 keepNonPenetrating = false, b32 sphereCheckFirst = false );

		b32 fIntersects( ) const	{ return mIntersects; }
	};

}}

#endif//__tIntersectionObbObb__
