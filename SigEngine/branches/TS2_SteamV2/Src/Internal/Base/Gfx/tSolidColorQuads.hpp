//------------------------------------------------------------------------------
// \file tSolidColorQuads.hpp - 07 Dec 2010
// \author ksorgetoomey
//
// Copyright Signal Studios 2010, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tSolidColorQuads__
#define __tSolidColorQuads__
#include "Gfx/tSolidColorGeometry.hpp"
#include "Gfx/tSolidColorMaterial.hpp"

namespace Sig { namespace Gfx
{
	class base_export tSolidColorQuads : public tSolidColorGeometry
	{
		tGrowableArray< tSolidColorRenderVertex > mVerts;
		tGrowableArray< u16 > mIds;
	public:
		tSolidColorQuads( );
		virtual void fOnDeviceReset( tDevice* device );
		void fGenerate( f32 halfEdgeLen = 1.f );

		Math::tAabbf fGetBounds( ) const;

		void fSetQuadCount( u32 count );
		u32 fQuadCount( );
		tSolidColorRenderVertex* fQuad( u32 index );

		void fAddQuad( const Math::tVec3f& v0, const Math::tVec3f& v1, const Math::tVec3f& v2, const Math::tVec3f& v3 );

		void fSubmitGeometry( );
	};

	typedef tRefCounterPtr<tSolidColorQuads> tSolidColorQuadsPtr;
}}

#endif //__tSolidColorQuads__
