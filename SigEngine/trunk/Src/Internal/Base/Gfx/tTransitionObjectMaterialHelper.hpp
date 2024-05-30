//------------------------------------------------------------------------------
// \file tTransitionObjectMaterialHelper.hpp - 28 Mar 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#ifndef __tTransitionObjectMaterialHelper__
#define __tTransitionObjectMaterialHelper__

namespace Sig { namespace Gfx
{
	class tDevicePtr;
	class tRenderInstance;
	class tMaterial;
	
	///
	/// \class tTransitionObjectMaterialHelper
	/// \brief Helper for materials that rely on transition objects
	class base_export tTransitionObjectMaterialHelper
	{
	public:

		static const u32 cMaxTransitionObjects = 4;
		static const u32 cMaxAlignments = 4;
		
		static const char cDefaultIdName[];
		static const char cEdgeColorName[];
		static const char cObjectsName[];

	public:

		static u32 fGetDefaultAlignment( );
		static void fSetDefaultAlignment( u32 alignment );

		// There are cMaxAlignments edge colors
		static const Math::tVec4f * fGetEdgeColors( );
		static void fSetEdgeColors( const Math::tVec4f colors[], u32 count );

		static void fApplyDefaultId( u32 regIndex, const tDevicePtr& device, const tMaterial & mtl );
		static void fApplyEdgeColors( u32 regIndex, const tDevicePtr& device, const tMaterial & mtl );
		static void fApplyTransitionObjects( 
			u32 regIndex, const tDevicePtr& device, const tMaterial & mtl, const tRenderInstance& instance );

		static void fAddDefaultIdDeclaration( 
			std::stringstream & ss, u32 regIndex, const char defaultIdName[] = cDefaultIdName );
		static void fAddEdgeColorDeclaration( 
			std::stringstream & ss, u32 regIndex, const char edgeColorName[] = cEdgeColorName );
		static void fAddObjectsDeclaration( 
			std::stringstream & ss, u32 regIndex, const char objectsName[] = cObjectsName );

		static void fAddFunctionDefinition( 
			std::stringstream & ss, 
			const char globalTimeName[],
			const char defaultIdName[] = cDefaultIdName,
			const char edgeColorsName[] = cEdgeColorName,
			const char objectsName[] = cObjectsName );
		static void fAddFunctionCall( std::stringstream & ss, const char worldPosXYZText[] );
	};

}} // ::Sig::Gfx

#endif//__tTransitionObjectMaterialHelper__
