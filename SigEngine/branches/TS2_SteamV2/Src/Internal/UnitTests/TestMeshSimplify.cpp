#include "UnitTestsPch.hpp"
#include "MeshSimplify.hpp"

using namespace Sig;
using namespace Sig::MeshSimplify;


define_unittest( TestMeshSimplify )
{
	tGrowableArray< Math::tVec3f > verts;
	tGrowableArray< Math::tVec3u > triangleIndices;

	verts.fPushBack( Math::tVec3f( -1.f, 0.f, 1.f ) );	//0
	verts.fPushBack( Math::tVec3f( 0.f, 0.f, 1.f ) );	//1
	verts.fPushBack( Math::tVec3f( 1.f, 0.f, 1.f ) );	//2

	verts.fPushBack( Math::tVec3f( -1.f, 0.f, 0.f ) );	//3
	verts.fPushBack( Math::tVec3f( 0.f, 0.f, 0.f ) );	//4
	verts.fPushBack( Math::tVec3f( 1.f, 0.f, 0.f ) );	//5

	verts.fPushBack( Math::tVec3f( -1.f, 0.f, -1.f ) );	//6
	verts.fPushBack( Math::tVec3f( 0.f, 0.f, -1.f ) );	//7
	verts.fPushBack( Math::tVec3f( 1.f, 0.f, -1.f ) );	//8

	triangleIndices.fPushBack( Math::tVec3u( 0, 1, 3 ) );
	triangleIndices.fPushBack( Math::tVec3u( 1, 4, 3 ) );

	triangleIndices.fPushBack( Math::tVec3u( 1, 2, 4 ) );
	triangleIndices.fPushBack( Math::tVec3u( 2, 5, 4 ) );

	triangleIndices.fPushBack( Math::tVec3u( 3, 4, 6 ) );
	triangleIndices.fPushBack( Math::tVec3u( 4, 7, 6 ) );

	triangleIndices.fPushBack( Math::tVec3u( 4, 5, 7 ) );
	triangleIndices.fPushBack( Math::tVec3u( 5, 8, 7 ) );

	MeshSimplify::fSimplify( verts, triangleIndices, 0.01f );

	fAssert( verts.fCount( ) == 4 && triangleIndices.fCount( ) == 2 );
}