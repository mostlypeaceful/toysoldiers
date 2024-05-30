#ifndef __tPathDecalEntity__
#define __tPathDecalEntity__
#include "tEntityDef.hpp"
#include "Gfx/tDecalMaterial.hpp"

namespace Sig
{
	class base_export tPathDecalEntityDef : public tEntityDef
	{
		declare_reflector( );
		implement_rtti_serializable_base_class( tPathDecalEntityDef, 0xDFDBEE33 );
	public:
		f32 mCameraDepthOffset;
		b8 mAcceptsLights;
		s8 mDepthBias;
		s8 mSlopeScaleDepthBias;
		b8 pad0;
		tDynamicArray< Gfx::tDecalRenderVertex > mVerts;
		tDynamicArray< u16 > mTriIndices;
		tLoadInPlaceResourcePtr* mDiffuseTexture;
		tLoadInPlaceResourcePtr* mNormalMap;
	public:
		tPathDecalEntityDef( );
		tPathDecalEntityDef( tNoOpTag );
		~tPathDecalEntityDef( );
		virtual b32 fHasRenderableBounds( ) const { return true; }
		virtual void fCollectEntities( const tCollectEntitiesParams& params ) const;
	};
}

#endif//__tPathDecalEntity__
