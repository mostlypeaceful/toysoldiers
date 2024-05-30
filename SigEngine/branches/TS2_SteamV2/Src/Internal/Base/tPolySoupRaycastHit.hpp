#ifndef __tPolySoupRaycastHit__
#define __tPolySoupRaycastHit__

namespace Sig
{
	typedef tDynamicArray< Math::tVec3f > tPolySoupVertexList;
	typedef tDynamicArray< Math::tVec3u > tPolySoupTriangleList;


	///
	/// \brief Output/result structure for a raycast query
	class base_export tPolySoupRayCastHit : public Math::tRayCastHit
	{
	public:
		s32	mTriIndex;

		inline tPolySoupRayCastHit( ) : mTriIndex( -1 ) { }
		inline tPolySoupRayCastHit( f32 t, const Math::tVec3f& n, s32 i ) : tRayCastHit( t, n ), mTriIndex( i ) { }
		inline b32 fHit( ) const { return fInBounds( mT, 0.f, 1.f ) && mTriIndex >= 0; }
	};

}

#endif//__tPolySoupRaycastHit__

