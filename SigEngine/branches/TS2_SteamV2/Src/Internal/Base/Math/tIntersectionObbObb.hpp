#ifndef __tIntersectionObbObb__
#define __tIntersectionObbObb__

namespace Sig { namespace Math
{

	struct tIObbObbContact
	{
		tVec3f mPoint;
		tVec3f mNormal;
		f32 mDepth;

		tIObbObbContact( ) { }

		tIObbObbContact( const tVec3f& point, const tVec3f& normal, f32 depth )
			: mPoint(point), mNormal(normal), mDepth( depth )
		{ }
	};

	class tIntersectionObbObbWithContact
	{
	protected:

		b32				mIntersects; ///< This will be true as long as they are not totally disjoint

		tGrowableArray<tIObbObbContact> mResults;
		
	public:

		tIntersectionObbObbWithContact( ) 
		{ }

		tIntersectionObbObbWithContact( const tObb<f32>& a, const tObb<f32>& b )
		{
			fIntersect( a, b );
		}

		// Will return normals sticking out of B.
		void fIntersect( const tObb<f32>& a, const tObb<f32>& b );

		b32 fIntersects( ) const	{ return mIntersects; }

		tGrowableArray<tIObbObbContact>& fResults( ) { return mResults; }
	};

}}

#endif//__tIntersectionObbObb__
