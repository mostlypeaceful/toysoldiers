#include "ToolsPch.hpp"
#include "tSigmlConverter.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "tExporterToolbox.hpp"
#include "iAssetGenPlugin.hpp"
#include "tMesh.hpp"
#include "tProgressiveMeshTool.hpp"
#include "tMeshCacheOptimize.hpp"
#include <limits>
#include "FileSystem.hpp"
#include "tAssetPluginDll.hpp"

// from bullet
#include "btBulletCollisionCommon.h"

// from graphics
#include "Gfx/tDevice.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tDefaultMaterial.hpp"

// for conversion from float->half for vertex attribute compression
#include "Dx360Util.hpp"

// for mesh lod target bias/min
#include "tProjectFile.hpp"

namespace Sig
{

	///
	/// \class tSigmlGeometryConverter
	/// \brief Helper to convert geometry from mshml format to binary format
	class tSigmlGeometryConverter : public Gfx::tGeometryBufferSysRam::tForEachVertexElement
	{
	public:

		tSigmlConverter::tMeshData&		mMeshData;
		const Sigml::tMeshPtr&			mSigmlMesh;
		u32								mSubMeshIndex;
		u32								mIndexBuffersStart;
		u32								mDefaultColorIndex;

		tSigmlGeometryConverter( tSigmlConverter::tMeshData& meshData, const Sigml::tMeshPtr& sigmlMesh, u32 ibsStart, u32 subMeshIdx )
			: mMeshData( meshData )
			, mSigmlMesh( sigmlMesh )
			, mIndexBuffersStart( ibsStart + subMeshIdx )
			, mSubMeshIndex( subMeshIdx )
			, mDefaultColorIndex( ~0 )
		{
			sigmlMesh->fFindDefaultRgbaSet( mDefaultColorIndex );

			if( mSigmlMesh->mTangents.fCount( ) == 0 ||
				mSigmlMesh->mNormals.fCount( ) == 0 ||
				mSigmlMesh->mBinormals.fCount( ) == 0 )
			{
				log_warning( "No tangents present! Default UV set is empty." );
			}
		}

		virtual void operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc );
	};

	//------------------------------------------------------------------------------
	void tSigmlGeometryConverter::operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc )
	{
		const u32 itri = ivtx / 3;

		const u32 idxBufferIdx = mIndexBuffersStart;
		const u32 subMeshIdx = mSubMeshIndex;
		const Sigml::tSubMeshPtr sigmlSubMesh = mSigmlMesh->mSubMeshes[ subMeshIdx ];

		const u32 ivtxRelative = ivtx % 3;
		const u32 itriRelative = itri;

		// FIXME handle all semantics and all formats

		switch( vertexElemDesc.mSemantic )
		{
		case Gfx::tVertexElement::cSemanticPosition:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default

				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mSigmlMesh->mVertices[ sigmlSubMesh->mVertexTris[ itriRelative ][ ivtxRelative ] ];
			}
			break;
		case Gfx::tVertexElement::cSemanticNormal:
			{
				const Math::tVec3f n = mSigmlMesh->mNormals[ sigmlSubMesh->mNormalTris[ itriRelative ][ ivtxRelative ] ];
				if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 )
				{
					Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
					*o = n;
				}
				else if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					u16* o = reinterpret_cast<u16*>( vertexElem );
					o[0] = Dx360Util::fConvertFloatToHalf( n.x );
					o[1] = Dx360Util::fConvertFloatToHalf( n.y );
					o[2] = Dx360Util::fConvertFloatToHalf( n.z );
					o[3] = Dx360Util::fConvertFloatToHalf( 0.f );
				}
				else
				{
					sigassert( !"invalid vertex format for cSemanticNormal" );
				}
			}
			break;
		case Gfx::tVertexElement::cSemanticTangent:
			{
				Math::tVec3f t,b,n;

				if( mSigmlMesh->mTangents.fCount( ) == 0 ||
					mSigmlMesh->mNormals.fCount( ) == 0 ||
					mSigmlMesh->mBinormals.fCount( ) == 0 )
				{
					t = Math::tVec3f::cXAxis;
					b = Math::tVec3f::cYAxis;
					n = Math::tVec3f::cZAxis;
				}
				else
				{
					t = mSigmlMesh->mTangents[ sigmlSubMesh->mTangentBinormalTris[ itriRelative ][ ivtxRelative ] ];
					n = mSigmlMesh->mNormals[ sigmlSubMesh->mNormalTris[ itriRelative ][ ivtxRelative ] ];
					b = mSigmlMesh->mBinormals[ sigmlSubMesh->mTangentBinormalTris[ itriRelative ][ ivtxRelative ] ];
				}

				const Math::tVec3f cross = t.fCross( n );
				const f32 sign = cross.fDot( b ) < 0.f ? -1.f : +1.f;

				if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_4 )
				{
					Math::tVec4f* o = reinterpret_cast<Math::tVec4f*>( vertexElem );
					o->x = t.x;
					o->y = t.y;
					o->z = t.z;
					o->w = sign;
				}
				else if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					u16* o = reinterpret_cast<u16*>( vertexElem );
					o[0] = Dx360Util::fConvertFloatToHalf( t.x );
					o[1] = Dx360Util::fConvertFloatToHalf( t.y );
					o[2] = Dx360Util::fConvertFloatToHalf( t.z );
					o[3] = Dx360Util::fConvertFloatToHalf( sign );
				}
				else
				{
					sigassert( !"invalid vertex format for cSemanticTangent" );
				}
			}
			break;
		case Gfx::tVertexElement::cSemanticBinormal:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_3 ); // until we support all formats, ensure it's the default

				Math::tVec3f* o = reinterpret_cast<Math::tVec3f*>( vertexElem );
				*o = mSigmlMesh->mBinormals[ sigmlSubMesh->mTangentBinormalTris[ itriRelative ][ ivtxRelative ] ];
			}
			break;
		case Gfx::tVertexElement::cSemanticTexCoord:
			{
				Math::tVec3f uvw( 0.f );
				if( vertexElemDesc.mSemanticIndex < mSigmlMesh->mUvwSetVerts.fCount( ) )
				{
					const u32 uvwTri = sigmlSubMesh->mUvwSetTris[ vertexElemDesc.mSemanticIndex ].mUvwTris[ itriRelative ][ ivtxRelative ];
					if( uvwTri < mSigmlMesh->mUvwSetVerts[ vertexElemDesc.mSemanticIndex ].mUvws.fCount( ) )
					{
						uvw = mSigmlMesh->mUvwSetVerts[ vertexElemDesc.mSemanticIndex ].mUvws[ uvwTri ];
						uvw.y = 1.f - uvw.y; // modelling packages seem to like to make us do this; or else you could say DX likes to make us do this
					}
				}

				if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_2 )
				{
					Math::tVec2f* o = reinterpret_cast<Math::tVec2f*>( vertexElem );
					*o = Math::tVec2f( uvw.x, uvw.y );
				}
				else if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f32_4 )
				{
					Math::tVec4f* o = reinterpret_cast<Math::tVec4f*>( vertexElem );
					if( vertexElemDesc.mSemanticIndex & 1 )
					{
						o->z = uvw.x;
						o->w = uvw.y;
					}
					else
					{
						o->x = uvw.x;
						o->y = uvw.y;
					}
				}
				else if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f16_2 )
				{
					u16* o = reinterpret_cast<u16*>( vertexElem );
					o[0] = Dx360Util::fConvertFloatToHalf( uvw.x );
					o[1] = Dx360Util::fConvertFloatToHalf( uvw.y );
				}
				else if( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_f16_4 )
				{
					u16* o = reinterpret_cast<u16*>( vertexElem );
					if( vertexElemDesc.mSemanticIndex & 1 )
					{
						o[2] = Dx360Util::fConvertFloatToHalf( uvw.x );
						o[3] = Dx360Util::fConvertFloatToHalf( uvw.y );
					}
					else
					{
						o[0] = Dx360Util::fConvertFloatToHalf( uvw.x );
						o[1] = Dx360Util::fConvertFloatToHalf( uvw.y );
					}
				}
				else
				{
					sigassert( !"invalid UV type" );
				}
			}
			break;
		case Gfx::tVertexElement::cSemanticColor:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_u8_4_Color ); // until we support all formats, ensure it's the default

				u32* o = reinterpret_cast<u32*>( vertexElem );

				Math::tVec4f rgba( 1.f ); // default to white
				if( mDefaultColorIndex < mSigmlMesh->mRgbaSetVerts.fCount( ) )
				{
					rgba = mSigmlMesh->mRgbaSetVerts[ mDefaultColorIndex ].mRgbas[ 
						sigmlSubMesh->mRgbaSetTris[ mDefaultColorIndex ].mRgbaTris[ itriRelative ][ ivtxRelative ] ];
				}

				*o = Gfx::tVertexColor( ( u8 )( rgba.x * 255.f ), ( u8 )( rgba.y * 255.f ), ( u8 )( rgba.z * 255.f ), ( u8 )( rgba.w * 255.f ) ).fForGpu( );
			}
			break;
		case Gfx::tVertexElement::cSemanticBoneWeights:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_u8_4_Normalized ); // until we support all formats, ensure it's the default

				u8* o = reinterpret_cast<u8*>( vertexElem );

				const Math::tVec4f defWgts( 1, 0, 0, 0 ); // default to first bone at full, rest at zero

				Math::tVec4f wgts = defWgts;

				const Sigml::tSkin::tVertex& skinVtx = mSigmlMesh->mSkin->mVertices[ sigmlSubMesh->mVertexTris[ itriRelative ][ ivtxRelative ] ];

				const u32 numWgts = fMin( 4u, skinVtx.fCount( ) );

				f32 sum = 0.f;
				for( u32 i = 0; i < numWgts; ++i )
				{
					wgts.fAxis( i ) = skinVtx[ i ].mBoneWeight;
					sum += skinVtx[ i ].mBoneWeight;
				}

				if( sum > 0.f ) wgts /= sum;
				else			wgts = defWgts;

				o[0] = fRound<u8>( wgts.x * 255.f );
				o[1] = fRound<u8>( wgts.y * 255.f );
				o[2] = fRound<u8>( wgts.z * 255.f );
				o[3] = fRound<u8>( wgts.w * 255.f );

				u32 isum = 0;
				for( u32 i = 0; i < numWgts; ++i )
					isum += o[i];
				const s32 extra = 255 - isum;
				if( extra != 0 )
				{
					for( u32 i = 0; i < numWgts; ++i )
					{
						const s32 afterFixup = o[i] + extra;
						if( fInBounds( afterFixup, 0, 255 ) )
						{
							o[i] += extra;
							break;
						}
					}
				}
			}
			break;
		case Gfx::tVertexElement::cSemanticBoneIndices:
			{
				sigassert( vertexElemDesc.mFormat == Gfx::tVertexElement::cFormat_u8_4 ); // until we support all formats, ensure it's the default

				u8* o = reinterpret_cast<u8*>( vertexElem );

				Math::tVec4i ids( 0, 0, 0, 0 ); // default to zero'th bone

				const Sigml::tSkin::tVertex& skinVtx = mSigmlMesh->mSkin->mVertices[ sigmlSubMesh->mVertexTris[ itriRelative ][ ivtxRelative ] ];

				const u32 numWgts = fMin( 4u, skinVtx.fCount( ) );

				for( u32 i = 0; i < numWgts; ++i )
					ids.fAxis( i ) = skinVtx[ i ].mBoneIndex;

				o[0] = ( u8 )( ids.x );
				o[1] = ( u8 )( ids.y );
				o[2] = ( u8 )( ids.z );
				o[3] = ( u8 )( ids.w );
			}
			break;
		default:
			sigassert( !"invalid vertex semantic" );
			break;
		}
	}

	tSigmlConverter::tSigmlConverter( )
	{
	}

	tSigmlConverter::~tSigmlConverter( )
	{
	}

	namespace
	{
		const Gfx::tVertexElement gVertexFormatElements[]=
		{
			Gfx::tVertexElement( Gfx::tVertexElement::cSemanticPosition, Gfx::tVertexElement::cFormat_f32_3 ),
		};

		const Gfx::tVertexFormat cRayCastVertexFormat( gVertexFormatElements, array_length( gVertexFormatElements ) );
	}
	

	void tSigmlConverter::fConvertRayCastSubMeshData( u32 ithSubMesh, tMesh& gfxMesh, const Sigml::tMeshPtr& sigmlMesh )
	{
		tSubMesh& sb = gfxMesh.mSubMeshes[ ithSubMesh ];
		const Sigml::tSubMeshPtr& sigmlSb = sigmlMesh->mSubMeshes[ ithSubMesh ];

		sb.mBounds.fInvalidate( );
		sb.mTriangles.fNewArray( sigmlSb->mVertexTris.fCount( ) );

		tGrowableArray< Math::tVec3f > verts;
		verts.fSetCapacity( 3 * sigmlSb->mVertexTris.fCount( ) );

		for( u32 itri = 0; itri < sigmlSb->mVertexTris.fCount( ); ++itri )
		{
			const Math::tVec3f& v0 = verts.fFindOrAdd( sigmlMesh->mVertices[ sigmlSb->mVertexTris[ itri ].x ] );
			const Math::tVec3f& v1 = verts.fFindOrAdd( sigmlMesh->mVertices[ sigmlSb->mVertexTris[ itri ].y ] );
			const Math::tVec3f& v2 = verts.fFindOrAdd( sigmlMesh->mVertices[ sigmlSb->mVertexTris[ itri ].z ] );

			sb.mBounds |= v0;
			sb.mBounds |= v1;
			sb.mBounds |= v2;

			sb.mTriangles[ itri ] = Math::tVec3u( 
				fPtrDiff( &v0, verts.fBegin( ) ), fPtrDiff( &v1, verts.fBegin( ) ), fPtrDiff( &v2, verts.fBegin( ) ) );
		}

		if( verts.fCount( ) > 0 )
		{
			const f32 cFirstPassReduction = tProjectFile::fInstance( ).mEngineConfig.mAssetGenConfig.mLodFirstPassTargetCost;
			const f32 cRayCastReduction = tProjectFile::fInstance( ).mEngineConfig.mAssetGenConfig.mRayCastMeshReduce;
			LOD::tDecimationControls controls;
			controls.mTargetPolicy = LOD::tDecimationControls::cTargetPolicyOptimal;
			controls.mCascadeTolerance = -1; // Cascades allowed

			// Scale to normalized cube
			//const f32 xfactor = (sb.mBounds.fWidth() > 0.f) ? 10.f / sb.mBounds.fWidth() : 1.f;
			//const f32 yfactor = (sb.mBounds.fHeight() > 0.f) ? 10.f / sb.mBounds.fHeight() : 1.f;
			//const f32 zfactor = (sb.mBounds.fDepth() > 0.f) ? 10.f / sb.mBounds.fDepth() : 1.f;
			//const Math::tVec3f scaleFactor( xfactor, yfactor, zfactor );

			// Scale to normalized volume
			Math::tAabbf fudgedAabb = sb.mBounds;
			// Prevent an axis from being zero.
			if( fudgedAabb.fWidth() == 0.f )
			{
				fudgedAabb |= Math::tVec3f( -0.1f, 0.f, 0.f ); fudgedAabb |= Math::tVec3f( 0.1f, 0.f, 0.f ); 
			}
			if( fudgedAabb.fHeight() == 0.f )
			{
				fudgedAabb |= Math::tVec3f( 0.f, -0.1f, 0.f ); fudgedAabb |= Math::tVec3f( 0.f, 0.1f, 0.f ); 
			}
			if( fudgedAabb.fDepth() == 0.f )
			{
				fudgedAabb |= Math::tVec3f( 0.f, 0.f, -0.1f ); fudgedAabb |= Math::tVec3f( 0.f, 0.f, 0.1f ); 
			}

			const f32 realVolume = fudgedAabb.fWidth() * fudgedAabb.fHeight() * fudgedAabb.fDepth();
			const f32 volumeFactor = Math::fPow( (10.f * 10.f * 10.f) / realVolume, 1.f/3.f );
			const Math::tVec3f scaleFactor( volumeFactor );

			// Normalize the vertices so that decimations are using the same cost scale.
			for( u32 i = 0; i < verts.fCount(); ++i )
				verts[i] *= scaleFactor;

			// Reduce the geometry
			LOD::tProgressiveMeshTool pmTool(
				cRayCastVertexFormat, 
				verts.fBegin( ),
				verts.fCount( ),
				( const u32* )sb.mTriangles.fBegin( ),
				sb.mTriangles.fCount( ) * 3,
				controls );

			b32 dirty = pmTool.fReduceToCost( cRayCastReduction );

			// No need to do a much of memory work if no decimations were performed
			if( dirty )
			{
				pmTool.fCaptureM0( ); // We don't care about progressive construction

				verts.fSetCount( pmTool.fM0Geometry( ).fVertexCount( ) );
				sigassert( pmTool.fM0Geometry( ).fSizeInBytes( ) == verts.fTotalSizeOf( ) );
				fMemCpy( verts.fBegin( ), pmTool.fM0Geometry( ).fBegin( ), verts.fTotalSizeOf( ) );

				sb.mTriangles.fNewArray( pmTool.fM0Indices( ).fIndexCount( ) / 3 );
				sigassert( sb.mTriangles.fCount( ) * 3 == pmTool.fM0Indices( ).fIndexCount( ) );
				pmTool.fM0Indices( ).fGetIndices( 0, (u32*)sb.mTriangles.fBegin( ), pmTool.fM0Indices( ).fIndexCount( ) );
			}

			// Return the verts to their normal space.
			for( u32 i = 0; i < verts.fCount(); ++i )
				verts[i] /= scaleFactor;

			sb.mVertices = verts;
			sb.mPolySoupKDTree.fConstruct( sb.mTriangles, sb.mVertices );
		}
	}

	void tSigmlConverter::fConvertRayCastMeshData( tMesh& gfxMesh, const Sigml::tMeshPtr& sigmlMesh )
	{
		gfxMesh.mBounds = sigmlMesh->mAabb;
		for( u32 i = 0; i < gfxMesh.mSubMeshes.fCount( ); ++i )
			fConvertRayCastSubMeshData( i, gfxMesh, sigmlMesh );
	}

	b32 tSigmlConverter::fConvertMesh( tMesh& gfxMesh, tMeshData& meshData, const Sigml::tMeshPtr& sigmlMesh, u32& ithVb, u32& ithIb )
	{
		const u32 numSubMeshes = sigmlMesh->mSubMeshes.fCount( );

		if( numSubMeshes == 0 )
		{
			log_warning( "Invalid mesh: no submeshes (i.e., no geometry)" );
			return false;
		}

		const tProjectFile & projectFile = tProjectFile::fInstance( );
		const b32 skinned = sigmlMesh->mSkin ? true : false;

		// convert skin
		if( skinned )
		{
			gfxMesh.mSkin = new tSkin( );

			const u32 userBoneCount = sigmlMesh->mSkin->mBoneNames.fCount( );
			const u32 boneCount = fMin( userBoneCount, Gfx::tMaterial::cMaxBoneCount );
			if( userBoneCount > Gfx::tMaterial::cMaxBoneCount )
				log_warning( "Skin contains more bones than is allowed: your bone count (" << userBoneCount << "), max bone count (" << Gfx::tMaterial::cMaxBoneCount << ")" );

			gfxMesh.mSkin->mInfluences.fNewArray( boneCount );
			for( u32 i = 0; i < gfxMesh.mSkin->mInfluences.fCount( ); ++i )
				gfxMesh.mSkin->mInfluences[ i ].mName = fAddLoadInPlaceStringPtr( sigmlMesh->mSkin->mBoneNames[ i ].c_str( ) );
		}

		gfxMesh.mGeometryFile = fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tGeometryFile>( meshData.mResourcePath ) );
		gfxMesh.mSubMeshes.fNewArray( numSubMeshes );

		const u32 ibsStart = meshData.mIndexData.fCount( );
		meshData.mIndexData.fGrowCount( numSubMeshes );

		// First convert the submeshes
		u32 totalTris = 0;
		for( u32 i = 0; i < numSubMeshes; ++i )
		{
			tSubMesh& sb = gfxMesh.mSubMeshes[ i ];

			sb.mGeometryBufferIndex = ithVb + i;
			sb.mIndexBufferIndex	= ithIb + i;

			// copy material pointer
			tMaterialData& mtlData = mMaterials[ sigmlMesh->mSubMeshes[ i ]->mMtlIndex ];
			sb.mMaterial = skinned ? mtlData.mSkinned.fGetRawPtr( ) : mtlData.mNonSkinned.fGetRawPtr( );

			// Count tris
			tIndexBufferData& indexData = meshData.mIndexData[ ibsStart + i ];
			indexData.mNumTris = sigmlMesh->mSubMeshes[ i ]->mVertexTris.fCount( );
			totalTris += indexData.mNumTris;

			// Now look for this vertex format in our master list
			u32 ifmt = 0;
			for( ; ifmt < mSubMeshVertexFormats.fCount( ); ++ifmt )
				if( mSubMeshVertexFormats[ ifmt ]->fFullyEqual( sb.mMaterial->fVertexFormat( ) ) )
					break;

			// Check if format didn't exist, add if not add it
			if( ifmt == mSubMeshVertexFormats.fCount( ) )
			{
				mSubMeshVertexFormats.fPushBack( tStrongPtr<Gfx::tVertexFormatVRam>( 
					new Gfx::tVertexFormatVRam( sb.mMaterial->fVertexFormat( ) ) ) );
			}
		
			// Set the format index
			sb.mVertexFormat = mSubMeshVertexFormats[ ifmt ].fGetRawPtr( );
		}

		if( totalTris == 0 )
		{
			log_warning( "Invalid mesh: zero triangle count" );
			return false;
		}

		const u32 vbsStart = meshData.mGeometryData.fCount( );
		meshData.mGeometryData.fGrowCount( numSubMeshes );

		//log_line( 0, "LODing: " << gfxMesh.mGeometryFile->mRawPath.fBegin( ) << " (" << totalTris << " tris)" );

		// Build the geometry for each sub mesh
		for( u32 i = 0; i < numSubMeshes; ++i )
		{
			const tSubMesh& sb = gfxMesh.mSubMeshes[ i ];
			tIndexBufferData& indexData = meshData.mIndexData[ ibsStart + i ];
			Gfx::tGeometryBufferSysRam& geometryData = meshData.mGeometryData[ vbsStart + i ];

			if( indexData.mNumTris > 10000 )
			{
				log_line( 0, "    submesh" << i << " has " << indexData.mNumTris << " triangles. This may take some time..." );
			}

			// allocate big geometry buffer
			geometryData.fAllocate( *sb.mVertexFormat, indexData.mNumTris * 3 );

			// convert all geometry
			tSigmlGeometryConverter converter( meshData, sigmlMesh, ibsStart, i );
			geometryData.fForEachVertexElement( converter );

			// Now we have a fully expanded (or un-indexed) geometry buffer; first step
			// is to index it (i.e., remove duplicates)
			tDynamicArray< u32 > reorderedVertexIndices;
			geometryData.fRemoveDuplicates( reorderedVertexIndices );

			// LOD ratios
			const f32 highLodRatio = fClamp( sigmlMesh->mHighLodRatio + projectFile.mEngineConfig.mAssetGenConfig.mMeshHighLodRatioBias, 0.f, 1.f );
			const f32 mediumLodRatio = fClamp( sigmlMesh->mMediumLodRatio + projectFile.mEngineConfig.mAssetGenConfig.mMeshMediumLodRatioBias, 0.f, 1.f );
			const f32 lowLodRatio = fClamp( sigmlMesh->mLowLodRatio + projectFile.mEngineConfig.mAssetGenConfig.mMeshLowLodRatioBias, 0.f, 1.f );

			fProcessLods(
				projectFile.mEngineConfig.mAssetGenConfig.mLodFirstPassTargetCost,
				highLodRatio, mediumLodRatio, lowLodRatio,
				geometryData, indexData, reorderedVertexIndices );
		}

		// increment global vb/ib counters
		ithVb += numSubMeshes;
		ithIb += numSubMeshes;

		// assign ray cast data
		fConvertRayCastMeshData( gfxMesh, sigmlMesh );
		return true;
	}

	//------------------------------------------------------------------------------
	static void fFindClosestWindow( 
		const tGrowableArray<tProgressiveMesh::tProgressiveIndexBuffer> & pibs,
		const f32 lodRatio, 
		u32 & outPibIndex, u32 & outWinIndex )
	{
		sigassert( pibs.fCount( ) > 0 && pibs[ 0 ].mWindows.fCount( ) > 0 );

		const u32 bestPib = pibs.fCount( ) - 1;
		const u32 bestWin = pibs[ bestPib ].mWindows.fCount( ) - 1;
		const u32 goalFaceCount = lodRatio * pibs[ bestPib ].mWindows[ bestWin ].mNumFaces; //implicit round down

		//pibs + wins should already be sorted from least num faces to greatest num faces
		for(u32 pi = 0; pi < pibs.fCount( ); ++pi)
		{
			const tProgressiveMesh::tProgressiveIndexBuffer & pib = pibs[ pi ];
			for(u32 wi = 0; wi < pib.mWindows.fCount( ); ++wi)
			{
				const tProgressiveMesh::tProgressiveIndexBuffer::tWindow & win = pib.mWindows[ wi ];
				const u32 faceCount = win.mNumFaces;
				if( faceCount > goalFaceCount )
				{
					//close enough to our goal. lets use it
					outPibIndex = pi;
					outWinIndex = wi;
					return;
				}
			}
		}
	}

	//------------------------------------------------------------------------------
	static void fFillOutIndexBuffer( 
		tSigmlConverter::tIndexBuffer & outBuff,
		const tProgressiveMesh::tProgressiveIndexBuffer & pib,
		const u32 winIndex )
	{
		sigassert( pib.mWindows.fCount( ) > winIndex );
		const tProgressiveMesh::tProgressiveIndexBuffer::tWindow & win = pib.mWindows[ winIndex ];
		outBuff.mWindows.fSetCount( 1 );
		outBuff.mWindows.fBack( ).mFirstIndex = 0;
		outBuff.mWindows.fBack( ).mNumVerts = win.mNumVerts;
		outBuff.mWindows.fBack( ).mNumFaces = win.mNumFaces;

		const u32 numIndices = win.mNumFaces * 3;
		outBuff.mIndexBuffer.fAllocate( pib.mIndexBuffer.fIndexFormat( ), numIndices );
		outBuff.mIndexBuffer.fSetIndices( 0, win.mFirstIndex, pib.mIndexBuffer, numIndices );
	}

	namespace
	{
		void fFillIndexArray( Gfx::tIndexBufferSysRam& ib, tDynamicArray<u32>& output )
		{
			output.fResize( ib.fIndexCount( ) );
			ib.fGetIndices( 0, output.fBegin( ), ib.fIndexCount( ) );
		}

		void fFillIndexBuffer( Gfx::tIndexBufferSysRam& ib, tDynamicArray<u32>& output )
		{
			ib.fSetIndices( 0, output.fBegin( ), ib.fIndexCount( ) );
		}
	}

	//------------------------------------------------------------------------------
	void tSigmlConverter::fProcessLods(
		f32 firstPassCost,
		f32 highLodRatio, f32 mediumLodRatio, f32 lowLodRatio,
		Gfx::tGeometryBufferSysRam & geoData,
		tIndexBufferData & ibData,
		tDynamicArray< u32 > & indices )
	{
		sigassert( fInBounds( highLodRatio, 0.f, 1.f ) );
		sigassert( fInBounds( mediumLodRatio, 0.f, 1.f ) );
		sigassert( fInBounds( lowLodRatio, 0.f, 1.f ) );

		// LOD - Initial size reduction to reduce "excessive" geometry
		if( firstPassCost > 0 )
		{
			LOD::tDecimationControls controls;
			controls.mTargetPolicy = LOD::tDecimationControls::cTargetPolicyOptimal;
			controls.mCascadeTolerance = -1; // Cascades allowed

			// Reduce the geometry
			LOD::tProgressiveMeshTool pmTool(
				geoData.fVertexFormat( ), 
				geoData.fBegin( ), 
				geoData.fVertexCount( ),
				indices.fBegin( ),
				indices.fCount( ),
				controls );

			// No need to do a much of memory work if no decimations were performed
			if( pmTool.fReduceToCost( firstPassCost ) )
			{
				pmTool.fCaptureM0( ); // We don't care about progressive construction

				geoData = pmTool.fM0Geometry( );

				//log_line( 0, "    Initial LOD Reduction: " << indices.fCount( ) / 3 << 
				//			 " --> " << pmTool.fM0Indices( ).fIndexCount( ) / 3 << 
				//			 ", -" << ( indices.fCount( ) / 3 ) - ( pmTool.fM0Indices( ).fIndexCount( ) / 3 ) <<
				//			 " tris." );

				indices.fNewArray( pmTool.fM0Indices( ).fIndexCount( ) );
				pmTool.fM0Indices( ).fGetIndices( 0, indices.fBegin( ), indices.fCount( ) );
			}
			//else
			//{
			//	log_line( 0, "    No initial LOD Reduction - target cost is too low - " << firstPassCost << " < " << pmTool.fMeshConnectivity( ).fNextDecimationCost( ) << "!" );
			//}
		}

		// User wants more than just the high-lod
		const b32 buildLOD = tProjectFile::fInstance( ).mEngineConfig.mRendererSettings.mBuildLOD;
		if( buildLOD && lowLodRatio < 1.f )
		{
			LOD::tDecimationControls controls;
			controls.mTargetPolicy = LOD::tDecimationControls::cTargetPolicySubset;

			// % of the average cost variance from the best move
			controls.mCascadeTolerance = 0.25f;

			// Reduce the geometry
			LOD::tProgressiveMeshTool pmTool(
				geoData.fVertexFormat( ), 
				geoData.fBegin( ), 
				geoData.fVertexCount( ),
				indices.fBegin( ),
				indices.fCount( ),
				controls );

			pmTool.fReduceToRatio( lowLodRatio );
			pmTool.fCapture( );

			// Build the progressive windows
			tGrowableArray<tProgressiveMesh::tProgressiveIndexBuffer> progIbs;
			pmTool.fBuildProgressiveWindows( geoData, progIbs );

			sigassert( progIbs.fCount( ) > 0 );

			// Figure out which Progressive Index Buffer to use and which Window within that PIB to use
			const u32 lowPib = 0;
			const u32 lowWin = 0;

			u32 mediumPib = ~0, mediumWin = ~0;
			fFindClosestWindow( progIbs, mediumLodRatio, mediumPib, mediumWin );
			sigassert( mediumPib != ~0 && mediumWin != ~0 );

			const u32 highPib = progIbs.fCount( ) - 1;
			const u32 highWin = progIbs[ highPib ].mWindows.fCount( ) - 1;

			// Setup Lods
			//log_output( 0, "    LOD ( " );
			if( lowPib != highPib || lowWin != highWin )
			{
				// low LOD
				ibData.mBuffers.fGrowCount( 1 );
				fFillOutIndexBuffer( ibData.mBuffers.fBack( ), progIbs[ lowPib ], lowWin );
				//log_output( 0, ibData.mBuffers.fBack( ).mIndexBuffer.fIndexCount( ) / 3 << ", " );
			}
			//else
			//	log_output( 0, " - , " );

			if( ( mediumPib != lowPib || mediumWin != lowWin ) && ( mediumPib != highPib || mediumWin != highWin ) )
			{
				// medium LOD
				ibData.mBuffers.fGrowCount( 1 );
				fFillOutIndexBuffer( ibData.mBuffers.fBack( ), progIbs[ mediumPib ], mediumWin );
				//log_output( 0, ibData.mBuffers.fBack( ).mIndexBuffer.fIndexCount( ) / 3 << ", " );
			}
			//else
			//	log_output( 0, " - , " );

			//always push back the high LOD
			ibData.mBuffers.fGrowCount( 1 );
			fFillOutIndexBuffer( ibData.mBuffers.fBack( ), progIbs[ highPib ], highWin );
			//log_line( 0, ibData.mBuffers.fBack( ).mIndexBuffer.fIndexCount( ) / 3 << " ) triangles." );
		}
		// No progressive windows
		else
		{
			Gfx::tIndexFormat indexFormat = Gfx::tIndexFormat::fCreateAppropriateFormat( 
				Gfx::tIndexFormat::cPrimitiveTriangleList, geoData.fVertexCount( ) );

			ibData.mBuffers.fSetCount( 1 );
			ibData.mBuffers.fBack( ).mIndexBuffer.fAllocate(
				indexFormat, indices.fCount( ) );
			ibData.mBuffers.fBack( ).mIndexBuffer.fSetIndices( 
				0, indices.fBegin( ), indices.fCount( ) );

			ibData.mBuffers.fBack( ).mWindows.fSetCount( 1 );
			ibData.mBuffers.fBack( ).mWindows.fBack( ).mFirstIndex = 0;
			ibData.mBuffers.fBack( ).mWindows.fBack( ).mNumFaces = ( indices.fCount( ) / 3 );
			ibData.mBuffers.fBack( ).mWindows.fBack( ).mNumVerts = geoData.fVertexCount( );
		}

		// cache organize the data.

		// we'll base the optimization off of the high LOD.
		tDynamicArray<u32> highLODBuffer;
		fFillIndexArray( ibData.mBuffers.fBack( ).mIndexBuffer, highLODBuffer );

		// the other buffers need to be reordered when the vertex buffer changes.
		tGrowableArray< tDynamicArray<u32> > otherBuffers;
		otherBuffers.fSetCount( fMax( (s32)ibData.mBuffers.fCount( ) - 1, 0 ) );
		for( u32 i = 0; i < otherBuffers.fCount( ); ++i )
			fFillIndexArray( ibData.mBuffers[ i ].mIndexBuffer, otherBuffers[ i ] );

		// do the work
		// reorder high lod and vertex buffer to be most optimal
		tMeshCacheOptimize( geoData, highLODBuffer, true, otherBuffers );

		// reorder lower lod's index buffer, leaving vertex buffer alone.
		for( u32 i = 0; i < otherBuffers.fCount( ); ++i )
			tMeshCacheOptimize( geoData, otherBuffers[ i ], false, otherBuffers );

		// put the data back
		fFillIndexBuffer( ibData.mBuffers.fBack( ).mIndexBuffer, highLODBuffer );
		for( u32 i = 0; i < otherBuffers.fCount( ); ++i )
			fFillIndexBuffer( ibData.mBuffers[ i ].mIndexBuffer, otherBuffers[ i ] );
	}

	void tSigmlConverter::fOutputGeometryFile( const tMeshData& meshData, tPlatformId pid )
	{
		tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, meshData.mResourcePath );
		tFileWriter ofile( outputPath );
		sigassert( ofile.fIsOpen( ) );

		Gfx::tGeometryFile geoFile;

		// make sure we set the signature prior to serializing
		geoFile.fSetSignature<Gfx::tGeometryFile>( pid );

		// Construct geometry buffers
		tDynamicArray< tDynamicBuffer > geometryBuffers( meshData.mGeometryData.fCount( ) );
		geoFile.mGeometryPointers.fNewArray( geometryBuffers.fCount( ) );
		
		// Build geometry pointers
		for( u32 i = 0; i < geometryBuffers.fCount( ); ++i )
		{
			// copy geometry buffer
			geometryBuffers[ i ] = meshData.mGeometryData[ i ].fGetBuffer( );
			// endian swap the geom buffer (the serializer won't know how to, as it's just a raw stream of bytes)
			meshData.mGeometryData[ i ].fEndianSwapBuffer( geometryBuffers[ i ], pid );
			// perform a "fake" allocation of the vram buffer so it gets correct parameters
			geoFile.mGeometryPointers[ i ].mVRamBuffer.fPseudoAllocate( 
				meshData.mGeometryData[ i ].fVertexFormat( ), 
				meshData.mGeometryData[ i ].fVertexCount( ), 
				0 );
		}

		// First pass to construct the groups, lods, copy the windows,
		// and accumulate the total count of index lists
		u32 indexListCount = 0;
		geoFile.mIndexGroups.fNewArray( meshData.mIndexData.fCount( ) );
		for( u32 i = 0; i < meshData.mIndexData.fCount( ); ++i )
		{
			const tIndexBufferData & src = meshData.mIndexData[ i ];
			Gfx::tGeometryFile::tIndexLodGroup & group = geoFile.mIndexGroups[ i ];
			
			group.mTotalWindowCount = 0;
			group.mPointerStart = indexListCount; // Cache the starting index
			indexListCount += src.mBuffers.fCount( );
			
			group.mLods.fNewArray( src.mBuffers.fCount( ) );
			for( u32 j = 0; j < src.mBuffers.fCount( ); ++j )
			{
				const tIndexBuffer & srcBuffer = src.mBuffers[ j ];
				Gfx::tGeometryFile::tIndexLod & lod = group.mLods[ j ];

				// Accumulate total window count for the group
				group.mTotalWindowCount += srcBuffer.mWindows.fCount( );

				// Copy the windows
				lod.mWindows.fNewArray( srcBuffer.mWindows.fCount( ) );
				for( u32 w = 0; w < srcBuffer.mWindows.fCount( ); ++w )
				{
					const tIndexWindow & srcWindow = srcBuffer.mWindows[ w ];
					Gfx::tGeometryFile::tIndexWindow & destWindow = lod.mWindows[ w ];

					if( ( srcWindow.mNumFaces & 0xffff0000 ) != 0 ) log_warning( "Too many faces in mesh (u16 storage capacity). count: " << srcWindow.mNumFaces );
					if( ( srcWindow.mNumVerts & 0xffff0000 ) != 0 ) log_warning( "Too many verts in mesh (u16 storage capacity). count: " << srcWindow.mNumVerts );

					destWindow.mFirstIndex = srcWindow.mFirstIndex;
					destWindow.mNumFaces = (u16)srcWindow.mNumFaces;
					destWindow.mNumVerts = (u16)srcWindow.mNumVerts;
				}
			}
		}

		// Second pass to build the index list pointers
		tDynamicArray< tDynamicBuffer > indexBuffers( indexListCount );
		geoFile.mIndexListPointers.fNewArray( indexListCount );
		for( u32 i = 0, ibIdx = 0; i < meshData.mIndexData.fCount( ); ++i )
		{
			const tIndexBufferData & src = meshData.mIndexData[ i ];
			for( u32 j = 0; j < src.mBuffers.fCount( ); ++j, ++ibIdx )
			{
				const tIndexBuffer & srcBuffer = src.mBuffers[ j ];

				// copy index buffer
				indexBuffers[ ibIdx ] = srcBuffer.mIndexBuffer.fGetBuffer( );

				// endian swap the index buffer (the serializer won't know how to, as it's just a raw stream of bytes)
				srcBuffer.mIndexBuffer.fEndianSwapBuffer( indexBuffers[ ibIdx ], pid  );

				// perform a "fake" allocation of the vram buffer so it gets correct parameters
				geoFile.mIndexListPointers[ ibIdx ].mVRamBuffer.fPseudoAllocate( 
					srcBuffer.mIndexBuffer.fIndexFormat( ), 
					srcBuffer.mIndexBuffer.fIndexCount( ), 
					srcBuffer.mIndexBuffer.fIndexCount( ) / 3,
					0 );
			}
		}


		// create load in place serializer, and serialize the file header (persistent portion of the file)
		tLoadInPlaceSerializer ser;
		geoFile.mHeaderSize = ser.fSave( geoFile, ofile, pid );

		// now continue saving buffers from the current location of the output file;
		// we'll store the offset of each buffer in the header
		u32 offsetFromStartOfFile = geoFile.mHeaderSize;
		for( u32 i = 0; i < geometryBuffers.fCount( ); ++i )
		{
			Gfx::tGeometryFile::tGeometryPointer & pointer = geoFile.mGeometryPointers[ i ];

			// clear the stream info
			pointer.mRefCount = 0;
			pointer.mState = Gfx::tGeometryFile::cBufferStateNull;

			// store current file offset
			pointer.mBufferOffset = offsetFromStartOfFile;

			// store buffer size
			pointer.mBufferSize = geometryBuffers[ i ].fCount( );

			// write shader buffer
			ofile( geometryBuffers[ i ].fBegin( ), geometryBuffers[ i ].fCount( ) );

			// increment current file offset
			offsetFromStartOfFile += geometryBuffers[ i ].fCount( );
		}
		for( u32 i = 0; i < indexBuffers.fCount( ); ++i )
		{
			Gfx::tGeometryFile::tIndexListPointer & pointer = geoFile.mIndexListPointers[ i ];

			// clear the stream info
			pointer.mRefCount = 0;
			pointer.mState = Gfx::tGeometryFile::cBufferStateNull;

			// store current file offset
			pointer.mBufferOffset = offsetFromStartOfFile;

			// store buffer size
			pointer.mBufferSize = indexBuffers[ i ].fCount( );

			// write shader buffer
			ofile( indexBuffers[ i ].fBegin( ), indexBuffers[ i ].fCount( ) );

			// increment current file offset
			offsetFromStartOfFile += indexBuffers[ i ].fCount( );
		}

		// seek back to the start of the file
		ofile.fSeek( 0 );

		// re-write the table of header information now that it has proper offsets
		const u32 headerSizeVerify = ser.fSave( geoFile, ofile, pid );

		// sanity check
		sigassert( geoFile.mHeaderSize == headerSizeVerify );
	}

	void tSigmlConverter::fOutputTextureFile( const tTextureData& texData, tPlatformId pid )
	{
		tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, texData.mResourcePath );
		texData.mTextureObject.fSaveGameBinary( outputPath, pid );
	}

	b32 tSigmlConverter::fLoadSigmlFile( const tFilePathPtr& sigmlFilePath, const tFilePathPtr& skinFilePath, const tFilePathPtr& outputResourcePath )
	{
		const u32 fileSizeInBytes = FileSystem::fGetFileSize( sigmlFilePath );
		if( fileSizeInBytes > (5 * 1024 * 1024) )//5MB
			log_line( 0, sigmlFilePath << " is a big file! (" << Memory::fToMB<f32>(fileSizeInBytes) << "MB) This may take some time..." );

		if( mSigmlFile.fLoadXml( sigmlFilePath ) )
		{
			if(		skinFilePath.fLength( ) > 0
				&& !mSigmlFile.fApplySkinFile( skinFilePath ) )
				return false;

			mResourcePath = outputResourcePath;
			return true;
		}

		return false;
	}


	namespace
	{
		struct tPluginConverter
		{
			tLoadInPlaceFileBase* mFile;
			tEditorPluginData* mInputData;
			tEntityData*& mOutput;

			tPluginConverter( tLoadInPlaceFileBase* file, tEditorPluginData* data, tEntityData*& output )
				: mFile( file )
				, mInputData( data )
				, mOutput( output )
			{
				sigassert( mInputData );
				mOutput = NULL;
			}

			b32 operator()( const tAssetPluginDllPtr& dllPtr, iAssetPlugin& assetPlugin ) const
			{
				iSigEdPlugin* plugin = assetPlugin.fGetSigEdPluginInterface( );

				if( plugin && plugin->fUniqueID( ) == mInputData->mPluginID )
				{
					plugin->fConstruct( NULL );
					mOutput = plugin->fSerializeData( mFile, mInputData );
				}

				return true;
			}
		};
	}

	b32 tSigmlConverter::fConvertPlatformCommon( )
	{
		// add explicit dependencies
		for( u32 i = 0; i < fSigmlFile( ).mExplicitDependencies.fCount( ); ++i )
			iAssetGenPlugin::fAddDependency( *this, fSigmlFile( ).mExplicitDependencies[ i ] );

		// convert all the materials up front; we do this before
		// converting meshes, as the meshes are dependent on the materials
		const u32 numMaterials = fSigmlFile( ).mMaterials.fCount( );
		mMaterials.fSetCount( numMaterials );
		for( u32 i = 0 ; i < numMaterials; ++i )
		{
			Gfx::tMaterial* mtlA = fSigmlFile( ).mMaterials[ i ]->fCreateGfxMaterial( *this, false );
			Gfx::tMaterial* mtlB = fSigmlFile( ).mMaterials[ i ]->fCreateGfxMaterial( *this, true );

			if( !mtlA || !mtlB )
			{
				log_warning( "Materials could not be converted, aborting." );
				return false;
			}

			mMaterials[ i ].mNonSkinned.fReset( mtlA );
			mMaterials[ i ].mSkinned.fReset( mtlB );
		}

		// convert all the meshes
		u32 ithVb = 0, ithIb = 0;
		const u32 numMeshes = fSigmlFile( ).mMeshes.fCount( );
		mMeshes.fSetCount( numMeshes );
		mMeshData.fSetCount( fMin( numMeshes, 1u ) ); // i.e., combine all mesh data into a single geometry file
		for( u32 i = 0; i < numMeshes; ++i )
		{
			mMeshes[ i ].fReset( new tMesh( ) );

			if( i == 0 ) // i.e., combine all mesh data into a single geometry file
			{
				mMeshData[ i ].fReset( new tMeshData( ) );

				// pre-allocate arrays for estimated upper bounds based on the number of meshes
				mMeshData[ i ]->mGeometryData.fSetCapacity( 3*numMeshes );
				mMeshData[ i ]->mIndexData.fSetCapacity( 3*numMeshes );

				std::stringstream ss;
				ss << "." << i << Gfx::tGeometryFile::fGetFileExtension( );
				mMeshData[ i ]->mResourcePath = tFilePathPtr::fSwapExtension( fGetResourcePath( ), ss.str( ).c_str( ) );
			}

			if( !fConvertMesh( *mMeshes[ i ], *mMeshData[ 0 ], fSigmlFile( ).mMeshes[ i ], ithVb, ithIb ) )
				return false;
		}

		// convert all objects
		tGrowableArray<tEntityDef*> convertedObjects;
		for( u32 i = 0; i < fSigmlFile( ).mObjects.fCount( ); ++i )
		{
			// convert object (virtually)
			if( fSigmlFile( ).mObjects[ i ] )
			{
				tEntityDef* object = fSigmlFile( ).mObjects[ i ]->fCreateEntityDef( *this );
				if( object )
					convertedObjects.fPushBack( object );
			}
		}

		if( convertedObjects.fCount( ) == 0 )
		{
			log_warning( "There are zero objects in the current sigml file - aborting conversion." );
			return false;
		}

		// store objects
		mObjects.fNewArray( convertedObjects.fCount( ) );
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[ i ] = convertedObjects[ i ];

		// convert plugin objects
		tGrowableArray<tEntityData*> convertedPluginData;
		for( u32 i = 0; i < fSigmlFile( ).mPluginData.fCount( ); ++i )
		{
			tEditorPluginData* dataIn = fSigmlFile( ).mPluginData[ i ].fGetRawPtr( );

			if( dataIn )
			{
				tEntityData* dataOut = NULL;
				tAssetPluginDllDepot::fInstance( ).fForEachPlugin( tPluginConverter( this, dataIn, dataOut) );
				if( dataOut )
					convertedPluginData.fPushBack( dataOut );
				else
					log_warning( "Could not convert tEditorPluginData with plugin ID: " << std::hex << dataIn->mPluginID );
			} 
		}

		// store objects
		if( convertedPluginData.fCount( ) )
		{
			mEntityData = new tEntityDataArray( );
			mEntityData->mData.fNewArray( convertedPluginData.fCount( ) );
			for( u32 i = 0; i < mEntityData->mData.fCount( ); ++i )
				mEntityData->mData[ i ] = convertedPluginData[ i ];
		}

		u32 numRenderableObjs = 0;

		// compute ray-casting total bounding volume
		mBounds.fInvalidate( );
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
		{
			const b32 hasRenderableBounds = mObjects[ i ]->fHasRenderableBounds( );
			numRenderableObjs += hasRenderableBounds ? 1 : 0;
			if( hasRenderableBounds && mObjects[ i ]->mBounds.fIsValid( ) )
				mBounds |= mObjects[ i ]->mBounds.fTransform( mObjects[ i ]->mObjectToLocal );
		}

		// add global properties
		fSigmlFile( ).fAddEntityDefProperties( this, *this );

		return true;
	}

	b32 tSigmlConverter::fConvertPlatformSpecific( tPlatformId pid )
	{
		return true;
	}

	b32 tSigmlConverter::fOutput( tFileWriter& ofile, tPlatformId pid )
	{
		// output secondary output files (geometry files)
		for( u32 i = 0; i < mMeshData.fCount( ); ++i )
			fOutputGeometryFile( *mMeshData[ i ], pid );
		// (texture files)
		for( u32 i = 0; i < mTextures.fCount( ); ++i )
			fOutputTextureFile( *mTextures[ i ], pid );


		sigassert( ofile.fIsOpen( ) );

		// set the binary file signature
		this->fSetSignature<tSceneGraphFile>( pid );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tSceneGraphFile&>( *this ), ofile, pid );

		return true;
	}

	//------------------------------------------------------------------------------
	tLoadInPlaceResourcePtr* tSigmlConverter::fAddMeshData( 
		u32 meshCount,
		const tFilePathPtr & resourcePath, 
		const Gfx::tGeometryBufferSysRam geo[], 
		const Gfx::tIndexBufferSysRam idx[] )
	{
		tStrongPtr< tSigmlConverter::tMeshData > meshData( new tSigmlConverter::tMeshData );

		meshData->mResourcePath = resourcePath;

		meshData->mGeometryData.fSetCount( meshCount );
		meshData->mIndexData.fSetCount( meshCount );

		for( u32 m = 0; m < meshCount; ++m )
		{
			const u32 numVerts = geo[ m ].fVertexCount( );
			const u32 numFaces = idx[ m ].fIndexCount( ) / 3;

			meshData->mGeometryData[ m ] = geo[ m ];
			meshData->mIndexData[ m ].mNumTris = numFaces;
			meshData->mIndexData[ m ].mBuffers.fSetCount( 1 );
			meshData->mIndexData[ m ].mBuffers[ 0 ].mIndexBuffer = idx[ m ];
			meshData->mIndexData[ m ].mBuffers[ 0 ].mWindows.fSetCount( 1 ); 
			meshData->mIndexData[ m ].mBuffers[ 0 ].mWindows[ 0 ].mFirstIndex = 0;
			meshData->mIndexData[ m ].mBuffers[ 0 ].mWindows[ 0 ].mNumFaces = numFaces;
			meshData->mIndexData[ m ].mBuffers[ 0 ].mWindows[ 0 ].mNumVerts = numVerts;
		}

		fAddMeshData( meshData );

		return fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tGeometryFile>( resourcePath ) );
	}

	//------------------------------------------------------------------------------
	tLoadInPlaceResourcePtr* tSigmlConverter::fAddProgressiveMeshData( 
		f32 highLodRatioIn, f32 mediumLodRatioIn, f32 lowLodRatioIn,
		u32 meshCount,
		const tFilePathPtr & resourcePath,
		const Gfx::tGeometryBufferSysRam geo[], 
		const Gfx::tIndexBufferSysRam idx[] )
	{
		const tProjectFile & projectFile = tProjectFile::fInstance( );

		// LOD ratios
		const f32 highLodRatio = fClamp( highLodRatioIn + projectFile.mEngineConfig.mAssetGenConfig.mMeshHighLodRatioBias, 0.f, 1.f );
		const f32 mediumLodRatio = fClamp( mediumLodRatioIn + projectFile.mEngineConfig.mAssetGenConfig.mMeshMediumLodRatioBias, 0.f, 1.f );
		const f32 lowLodRatio = fClamp( lowLodRatioIn + projectFile.mEngineConfig.mAssetGenConfig.mMeshLowLodRatioBias, 0.f, 1.f );

		tStrongPtr< tSigmlConverter::tMeshData > meshData( new tSigmlConverter::tMeshData );

		meshData->mResourcePath = resourcePath;
		
		meshData->mGeometryData.fSetCount( meshCount );
		meshData->mIndexData.fSetCount( meshCount );

		//log_line( 0, "LODing TERRAIN: " << resourcePath );
	
		for( u32 m = 0; m < meshCount; ++m )
		{
			meshData->mGeometryData[ m ] = geo[ m ];

			tDynamicArray<u32> indices( idx[ m ].fIndexCount( ) );
			idx[ m ].fGetIndices( 0, indices.fBegin( ), indices.fCount( ) );

			fProcessLods( 
				projectFile.mEngineConfig.mAssetGenConfig.mLodFirstPassTargetCost,
				highLodRatio, mediumLodRatio, lowLodRatio, 
				meshData->mGeometryData[ m ], 
				meshData->mIndexData[ m ], indices );
		}

		fAddMeshData( meshData );

		return fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tGeometryFile>( resourcePath ) );
	}
}
