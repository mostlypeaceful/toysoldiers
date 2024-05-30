#ifndef __tHeightFieldMeshObject__
#define __tHeightFieldMeshObject__
#include "Sigml.hpp"
#include "Editor/tTerrainGeometry.hpp"

namespace Sig { namespace Sigml
{
	class tools_export tHeightFieldMeshObject : public tObject
	{
		define_dynamic_cast( tHeightFieldMeshObject, tObject );
		implement_rtti_serializable_base_class( tHeightFieldMeshObject, 0xE27CF9E2 );
	public:
		static inline const char* fStrTesselation( ) { return "HeightField.Tesselation"; }
		static inline const char* fStrDimensions( ) { return "HeightField.Dimensions"; }
		static inline const char* fStrMaterialRes( ) { return "HeightField.MaterialRes"; }
		static inline const char* fStrOptimizationTarget( ) { return "HeightField.OptimizeTarget"; }

		static inline Math::tVec2f fDefaultTesselation( ) { return Math::tVec2f( 128.f, 128.f ); }
		static inline Math::tVec2f fDefaultDimensions( ) { return Math::tVec2f( 256.f, 256.f ); }
		static inline Math::tVec2i fDefaultMaterialRes( ) { return Math::tVec2i( 256, 256 ); }

	public:

		tTerrainGeometry::tHeightField mHeightField;
		tTextureSysRam::tSurface mMaterialMaskTexture;
		tTextureSysRam::tSurface mMaterialIdsTexture;
		tDynamicArray<tTerrainGeometry::tGroundCover> mGroundCover;
		u32 mHFMatType; //tTerrainGeometry::tMaterialType

	public:
		tHeightFieldMeshObject( );
		void fDumpRawTriangles( tGrowableArray< Math::tVec3f >& rawVerts, tGrowableArray< Math::tVec3u >& rawTris ) const;
		virtual void fSerialize( tXmlSerializer& s );
		virtual void fSerialize( tXmlDeserializer& s );
		virtual tEntityDef* fCreateEntityDef( tSigmlConverter& sigmlConverter );
		virtual tEditableObject* fCreateEditableObject( tEditableObjectContainer& container );
	};

	template<class tSerializer>
	void fSerializeXmlObject( tSerializer& s, tHeightFieldMeshObject& o )
	{
		s( "HeightField", o.mHeightField.mHeights );
		s( "QuadsDisabled", o.mHeightField.mQuadsDisabled );
		s( "MaterialMaskTexture", o.mMaterialMaskTexture );
		s( "MaterialIdsTexture", o.mMaterialIdsTexture );
		s( "MaterialType", o.mHFMatType );
		s( "GroundCover", o.mGroundCover );
	}
}}

#endif//__tHeightFieldMeshObject__
