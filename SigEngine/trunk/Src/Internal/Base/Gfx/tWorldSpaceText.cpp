//------------------------------------------------------------------------------
// \file tWorldSpaceText.cpp - 03 Dec 2011
// \author jwittner
//
// Copyright Signal Studios 2011, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tWorldSpaceText.hpp"

namespace Sig { namespace Gfx
{

	//------------------------------------------------------------------------------
	// tWorldSpaceText
	//------------------------------------------------------------------------------
	void tWorldSpaceText::fOnGeometryChanged( ) 
	{ 
		const f32 width = fMax( mGeometry.fWidth( ), 0.1f );
		const f32 height = fMax( mGeometry.fHeight( ), 0.1f );
		const f32 depth = 0.1f;
		
		Math::tAabbf aabb;
		aabb.mMin.x = -0.5f * width;
		aabb.mMax.x =  0.5f * width;
		aabb.mMin.y = -0.5f * height;
		aabb.mMax.y =  0.5f * height;
		aabb.mMin.z = -0.5f * depth;
		aabb.mMax.z =  0.5f * depth;
		
		fSetObjectSpaceBox( aabb );

		fSetRenderBatch( mGeometry.fRenderBatch( ) ); 
	}

}} // ::Sig::Gfx

