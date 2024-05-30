//------------------------------------------------------------------------------
// \file tTransitionObjectMaterialHelper.cpp - 28 Mar 2012
// \author jwittner
//
// Copyright Signal Studios 2012, All Rights Reserved
//------------------------------------------------------------------------------
#include "BasePch.hpp"
#include "tTransitionObjectMaterialHelper.hpp"
#include "tMaterial.hpp"
#include "tRenderInstance.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		static u32 gDefaultAlignment = 0;
		static Math::tVec4f gDefaultAlignmentVec( 0 );
		static tFixedArray<Math::tVec4f, tTransitionObjectMaterialHelper::cMaxAlignments> gAlignmentEdgeColors;
	}

	//------------------------------------------------------------------------------
	// tTransitionObjectMaterialHelper
	//------------------------------------------------------------------------------
	const char tTransitionObjectMaterialHelper::cDefaultIdName[] = "gDefaultId";
	const char tTransitionObjectMaterialHelper::cEdgeColorName[] = "gEdgeColors";
	const char tTransitionObjectMaterialHelper::cObjectsName[] = "gTransitionObjects";

	//------------------------------------------------------------------------------
	u32 tTransitionObjectMaterialHelper::fGetDefaultAlignment( )
	{
		return gDefaultAlignment;
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fSetDefaultAlignment( u32 alignment )
	{
		gDefaultAlignment = alignment;
		gDefaultAlignmentVec.x = (f32)alignment;
	}

	//------------------------------------------------------------------------------
	const Math::tVec4f * tTransitionObjectMaterialHelper::fGetEdgeColors( )
	{
		return gAlignmentEdgeColors.fBegin( );
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fSetEdgeColors( const Math::tVec4f colors[], u32 count )
	{
		count = fMin( count, gAlignmentEdgeColors.fCount( ) );
		for( u32 c = 0; c < count; ++c )
			gAlignmentEdgeColors[ c ] = colors[ c ];
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fApplyDefaultId( 
		u32 regIndex, const tDevicePtr& device, const tMaterial & mtl )
	{
		mtl.fApplyVector4PS( device, regIndex, gDefaultAlignmentVec );
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fApplyEdgeColors( 
		u32 regIndex, const tDevicePtr& device, const tMaterial & mtl )
	{
		mtl.fApplyVector4PS( 
			device, regIndex, 
			gAlignmentEdgeColors[ 0 ], 
			gAlignmentEdgeColors.fCount( ) );
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fApplyTransitionObjects( 
		u32 regIndex, const tDevicePtr& device, const tMaterial & mtl, const tRenderInstance& instance )
	{
		// set pixel shader constant for transition objects
		tFixedArray<Math::tVec4f,cMaxTransitionObjects> transObjs;
		const u32 numSet = instance.fRI_TransitionObjects( transObjs.fBegin( ), transObjs.fCount( ) );

		// If max transition objects is 4 then the shader uses an optimized vector math pathway
		if( cMaxTransitionObjects == 4 )
		{
			// Shader expects data in this order, so swizzle our objects
			//const float4 radius = gTransitionObjects[0];
			//const float4 alignment = gTransitionObjects[1];
			//const float4 sX = gTransitionObjects[2];
			//const float4 sZ = gTransitionObjects[3];

			Math::tVec4f radii, alignment, sX, sZ;
			for( u32 i = 0; i < numSet; ++i )
			{
				const Math::tVec4f & obj = transObjs[ i ];

				sX[ i ] = obj.x;
				sZ[ i ] = obj.y;
				radii[ i ] = obj.z;
				alignment[ i ] = obj.w;
			}

			for(u32 i = numSet; i < transObjs.fCount( ); ++i)
				radii[ i ] = -1.0f; //invalidate sphere radius

			transObjs[ 0 ] = radii;
			transObjs[ 1 ] = alignment;
			transObjs[ 2 ] = sX;
			transObjs[ 3 ] = sZ;
		}
		else
		{
			for(u32 i = numSet; i < transObjs.fCount( ); ++i)
				transObjs[ i ].z = -1.0f; //invalidate sphere radius
		}

		mtl.fApplyVector4PS( device, regIndex, transObjs[0], transObjs.fCount( ) );
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fAddDefaultIdDeclaration( 
		std::stringstream & ss, u32 regIndex, const char defaultIdName[] )
	{
		ss << "float4 " << defaultIdName << " : register( c" << regIndex << " );" << std::endl;
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fAddEdgeColorDeclaration( 
		std::stringstream & ss, u32 regIndex, const char edgeColorName[] )
	{
		ss << "float4 " << edgeColorName << "[" << cMaxAlignments << "] : register( c" << regIndex << " );" << std::endl;
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fAddObjectsDeclaration( 
		std::stringstream & ss, u32 regIndex, const char objectsName[] )
	{
		ss << "float4 " << objectsName << "[" << cMaxTransitionObjects << "] : register( c" << regIndex << " );" << std::endl;
	}

	//------------------------------------------------------------------------------
	void tTransitionObjectMaterialHelper::fAddFunctionDefinition( 
		std::stringstream & ss,
		const char globalTimeName[],
		const char defaultIdName[],
		const char edgeColorsName[],
		const char objectsName[] )
	{
		ss << std::endl;
		ss << "// Returns information about the world pos relative to the transition objects" << std::endl;
		ss << "float4 fGetTransitionInfo( float3 worldPos )" << std::endl;
		ss << "{" << std::endl;
		ss << "    float bestAlignment, bestEdgeAlignment, bestNearEdgeness;" << std::endl;
		if( cMaxTransitionObjects == 4 )
		{
			ss << "    // Pull out information" << std::endl;
			ss << "    float4 radius = " << objectsName << "[0];" << std::endl;
			ss << "    const float4 alignment = " << objectsName << "[1];" << std::endl;
			ss << "    const float4 sX = " << objectsName << "[2];" << std::endl;
			ss << "    const float4 sZ = " << objectsName << "[3];" << std::endl << std::endl;

			ss << "    // Compute validity and whether we're in the circles" << std::endl;
			ss << "    float4 isBetter = step( 0, radius );" << std::endl;
			ss << "    const float4 toPixelX = worldPos.xxxx - sX;" << std::endl;
			ss << "    const float4 toPixelZ = worldPos.zzzz - sZ;" << std::endl;
			ss << "    const float4 angles = atan2( toPixelZ, toPixelX );" << std::endl;
			ss << "    radius -= (1.0f - cos( angles + " << globalTimeName << ".xxxx ) * sin( round( 0.05f * 2 * 3.141592f * radius ) * angles + " << globalTimeName << ".xxxx ) );" << std::endl;
			ss << "    const float4 radiusSq = radius * radius;" << std::endl;
			ss << "    const float4 distSq = ( toPixelX * toPixelX + toPixelZ * toPixelZ );" << std::endl;
			ss << "    isBetter *= step( distSq, radiusSq );" << std::endl << std::endl;

			// Collapse to results
			ss << "    // Collapse the results" << std::endl;
			ss << "    float bestDistSq = 100000000;" << std::endl;
			ss << "    isBetter.x *= step( distSq.x, bestDistSq );" << std::endl;
			ss << "    bestDistSq += ( distSq.x - bestDistSq ) * isBetter.x;" << std::endl;
			ss << "    isBetter.y *= step( distSq.y, bestDistSq );" << std::endl;
			ss << "    bestDistSq += ( distSq.y - bestDistSq ) * isBetter.y;" << std::endl;
			ss << "    isBetter.z *= step( distSq.z, bestDistSq );" << std::endl;
			ss << "    bestDistSq += ( distSq.z - bestDistSq ) * isBetter.z;" << std::endl;
			ss << "    isBetter.w *= step( distSq.w, bestDistSq );" << std::endl;
			ss << "    bestDistSq += ( distSq.w - bestDistSq ) * isBetter.w;" << std::endl<< std::endl;

			ss << "    // Compute the best radius and alignment" << std::endl;
			ss << "    float2 bestRadAlign = float2( -1, " << defaultIdName << ".x );" << std::endl;
			ss << "    bestRadAlign += ( float2( radius.x, alignment.x ) - bestRadAlign ) * isBetter.x;" << std::endl;
			ss << "    bestRadAlign += ( float2( radius.y, alignment.y ) - bestRadAlign ) * isBetter.y;" << std::endl;
			ss << "    bestRadAlign += ( float2( radius.z, alignment.z ) - bestRadAlign ) * isBetter.z;" << std::endl;
			ss << "    bestRadAlign += ( float2( radius.w, alignment.w ) - bestRadAlign ) * isBetter.w;" << std::endl << std::endl;

			ss << "    // Are we in a sphere, and if so how on the edge are we?" << std::endl;
			ss << "    const float bestDist = sqrt( bestDistSq );" << std::endl;
			ss << "    const float goodRadius = step( 0, bestRadAlign.x );" << std::endl;
			ss << "    const float2 minEdgeRadii = float2( bestRadAlign.x - 1.0f, bestRadAlign.x - 2.0f );" << std::endl;
			ss << "    const float2 maxEdgeRadii = float2( bestRadAlign.x, minEdgeRadii.x );" << std::endl;
			ss << "    const float inAlignSphere = goodRadius * step( bestDist, minEdgeRadii.x );" << std::endl;
			ss << "    float2 edgeNess = smoothstep( minEdgeRadii, maxEdgeRadii, bestDist.xx );" << std::endl << std::endl;
			
			ss << "    bestNearEdgeness = goodRadius * ( 1.0f - edgeNess.x ) * edgeNess.y;" << std::endl;
			ss << "    bestEdgeAlignment = bestRadAlign.y;" << std::endl;
			ss << "    bestAlignment = bestRadAlign.y + ( " << defaultIdName << ".x - bestRadAlign.y ) * ( 1.0f - inAlignSphere );" << std::endl << std::endl;
		}
		else
		{
			ss << "    float bestDistSq = 100000000;" << std::endl;
			ss << "    float bestRadius = -1;" << std::endl;
			ss << "    bestAlignment = " << defaultIdName << ".x;" << std::endl;
			ss << "    for(int spherei = 0; spherei < " << cMaxTransitionObjects << "; ++spherei)" << std::endl;
			ss << "    {" << std::endl;
			ss << "        const float4 sphereData = " << objectsName << "[ spherei ];" << std::endl;
			ss << "        const float2 toPixel = worldPos.xz - sphereData.xy;" << std::endl;
			ss << "        const float angle = atan2( toPixel.y, toPixel.x );" << std::endl << std::endl;

			//Slow, undulating wave
			ss << "		   const float radiusTweak = (1.0f - cos( angle + " << globalTimeName << ".x ) * sin( round( 0.05f * 2 * 3.141592f * sphereData.z ) * angle + " << globalTimeName << ".x ) );" << std::endl;
			ss << "        const float radius = sphereData.z + radiusTweak;" << std::endl;
			ss << "        const float radiusSq = radius * radius;" << std::endl;
			ss << "        const float alignment = sphereData.w;" << std::endl;
			ss << "        const float distSq = dot( toPixel, toPixel );" << std::endl << std::endl;

			ss << "        const float goodRadius = step( 0, sphereData.z );" << std::endl; //radius >= 0.f. code should ensure that radius is never == 0, an artifact may appear if we check a pixel at the same position as this 0-radius sphere
			ss << "        const float inSphere = step( 0, radiusSq - distSq );" << std::endl; // (radius - dist) >= 0.f?
			ss << "        const float isCloser = step( distSq, bestDistSq );" << std::endl; //dist <= bestDist?
			ss << "        const float isBetter = goodRadius * inSphere * isCloser;" << std::endl;
			ss << "        bestAlignment += ( alignment - bestAlignment ) * isBetter;" << std::endl;
			ss << "        bestDistSq += ( distSq - bestDistSq ) * isBetter;" << std::endl;
			ss << "        bestRadius += ( radius - bestRadius ) * isBetter;" << std::endl;
			ss << "    }" << std::endl << std::endl;

			ss << "    const float goodRadius = step( 0, bestRadius );" << std::endl;
			ss << "    const float bestDist = sqrt( bestDistSq );" << std::endl;
			ss << "    const float alignRadius = bestRadius - 1.0f;" << std::endl; // If the band half width changes from 1.0f we'll have to change the bestNearEdgeness computation.
			ss << "    const float inAlignSphere = goodRadius * step( 0, alignRadius - bestDist );" << std::endl << std::endl;
			
			ss << "    bestNearEdgeness = goodRadius * (1.0f - smoothstep( alignRadius, bestRadius, bestDist ) ) * smoothstep( alignRadius - 1.0f, alignRadius, bestDist );" << std::endl;
			ss << "    bestEdgeAlignment = bestAlignment;" << std::endl;
			ss << "    bestAlignment += ( " << defaultIdName << ".x - bestAlignment ) * ( 1.0f - inAlignSphere );" << std::endl << std::endl;
		}

		ss << "    float notDefaultAlignment = 1.0f - ( step( bestEdgeAlignment, " << defaultIdName << ".x ) * step( " << defaultIdName << ".x, bestEdgeAlignment ) );" << std::endl;
		ss << "    bestNearEdgeness *= notDefaultAlignment;" << std::endl;
		ss << "    return float4( " << edgeColorsName << "[bestEdgeAlignment].rgb, bestAlignment + clamp( bestNearEdgeness, 0, 0.9999f ) );" << std::endl;
		ss << "}" << std::endl;
	}

	void tTransitionObjectMaterialHelper::fAddFunctionCall( std::stringstream & ss, const char worldPosXYZText[] )
	{
		ss << "fGetTransitionInfo( " << worldPosXYZText << " );" << std::endl;
	}
	

}} // ::Sig::Gfx
