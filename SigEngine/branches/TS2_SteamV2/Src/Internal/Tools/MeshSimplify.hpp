#ifndef __tMeshSimplifier__
#define __tMeshSimplifier__

namespace Sig { namespace MeshSimplify
{
	tools_export void fSimplify( tGrowableArray< Math::tVec3f >& verts, tGrowableArray< Math::tVec3u >& triangleIndices, const f32 optimizeTarget, const b32 disregardEdges = false, const b32 reIndex = true );
} }

#endif //__tMeshSimplifier__
