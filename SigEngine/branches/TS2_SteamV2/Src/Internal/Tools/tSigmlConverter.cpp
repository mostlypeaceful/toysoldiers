#include "ToolsPch.hpp"
#include "tSigmlConverter.hpp"
#include "tLoadInPlaceSerializer.hpp"
#include "tExporterToolbox.hpp"
#include "iAssetGenPlugin.hpp"
#include "tMesh.hpp"
#include <limits>

// from bullet
#include "btBulletCollisionCommon.h"

// from graphics
#include "Gfx/tDevice.hpp"
#include "Gfx/tGeometryFile.hpp"
#include "Gfx/tMaterialFile.hpp"
#include "Gfx/tDefaultMaterial.hpp"

// for conversion from float->half for vertex attribute compression
#include "Dx360Util.hpp"

namespace Sig
{

	class tSigmlGeometryConverter : public Gfx::tGeometryBufferSysRam::tForEachVertexElement
	{
	public:

		tSigmlConverter::tMeshData&		mMeshData;
		const Sigml::tMeshPtr&			mSigmlMesh;
		u32								mIndexBuffersStart;
		u32								mDefaultColorIndex;

		tSigmlGeometryConverter( tSigmlConverter::tMeshData& meshData, const Sigml::tMeshPtr& sigmlMesh, u32 ibsStart )
			: mMeshData( meshData )
			, mSigmlMesh( sigmlMesh )
			, mIndexBuffersStart( ibsStart )
			, mDefaultColorIndex( ~0 )
		{
			sigmlMesh->fFindDefaultRgbaSet( mDefaultColorIndex );

			if( mSigmlMesh->mTangents.fCount( ) == 0 ||
				mSigmlMesh->mNormals.fCount( ) == 0 ||
				mSigmlMesh->mBinormals.fCount( ) == 0 )
			{
				log_warning( 0, "No tangents present! Default UV set is empty." );
			}
		}

		inline u32 fTriToIndexBufferIdx( u32 itri ) const
		{
			for( u32 i = mIndexBuffersStart + 1; i < mMeshData.mIndexData.fCount( ); ++i )
			{
				if( itri < mMeshData.mIndexData[ i ].mTriOffset )
					return i - 1;
			}

			return mMeshData.mIndexData.fCount( ) - 1;
		}

		virtual void operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc );
	};
	void tSigmlGeometryConverter::operator( )( u32 ivtx, Sig::byte* vertexElem, const Gfx::tVertexElement& vertexElemDesc )
	{
		const u32 itri = ivtx / 3;

		const u32 idxBufferIdx = fTriToIndexBufferIdx( itri );
		const u32 subMeshIdx = idxBufferIdx - mIndexBuffersStart;
		const Sigml::tSubMeshPtr sigmlSubMesh = mSigmlMesh->mSubMeshes[ subMeshIdx ];

		const u32 ivtxRelative = ivtx % 3;
		const u32 itriRelative = itri - mMeshData.mIndexData[ idxBufferIdx ].mTriOffset;

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
			log_warning( 0, "Invalid mesh: no submeshes (i.e., no geometry)" );
			return false;
		}

		const b32 skinned = sigmlMesh->mSkin ? true : false;

		// convert skin
		if( skinned )
		{
			gfxMesh.mSkin = new tSkin( );

			const u32 userBoneCount = sigmlMesh->mSkin->mBoneNames.fCount( );
			const u32 boneCount = fMin( userBoneCount, Gfx::tMaterial::cMaxBoneCount );
			if( userBoneCount > Gfx::tMaterial::cMaxBoneCount )
				log_warning( 0, "Skin contains more bones than is allowed: your bone count (" << userBoneCount << "), max bone count (" << Gfx::tMaterial::cMaxBoneCount << ")" );

			gfxMesh.mSkin->mInfluences.fNewArray( boneCount );
			for( u32 i = 0; i < gfxMesh.mSkin->mInfluences.fCount( ); ++i )
				gfxMesh.mSkin->mInfluences[ i ].mName = fAddLoadInPlaceStringPtr( sigmlMesh->mSkin->mBoneNames[ i ].c_str( ) );
		}

		gfxMesh.mGeometryFile = fAddLoadInPlaceResourcePtr( tResourceId::fMake<Gfx::tGeometryFile>( meshData.mResourcePath ) );
		gfxMesh.mSubMeshes.fNewArray( numSubMeshes );

		const u32 ibsStart = meshData.mIndexData.fCount( );
		meshData.mIndexData.fGrowCount( numSubMeshes );

		u32 totalTris = 0;
		Gfx::tVertexFormat combinedVertexFormat;

		// first convert sub-meshes
		for( u32 i = 0; i < numSubMeshes; ++i )
		{
			tSubMesh& sb = gfxMesh.mSubMeshes[ i ];

			sb.mGeometryBufferIndex = ithVb;
			sb.mIndexBufferIndex	= ithIb + i;

			// copy material pointer
			tMaterialData& mtlData = mMaterials[ sigmlMesh->mSubMeshes[ i ]->mMtlIndex ];
			sb.mMaterial = skinned ? mtlData.mSkinned.fGetRawPtr( ) : mtlData.mNonSkinned.fGetRawPtr( );

			// combine vertex formats
			combinedVertexFormat.fCombine( sb.mMaterial->fVertexFormat( ) );

			// count total tris
			tIndexBufferData& indexData = meshData.mIndexData[ ibsStart + i ];
			indexData.mNumTris = sigmlMesh->mSubMeshes[ i ]->mVertexTris.fCount( );
			indexData.mTriOffset = totalTris;
			totalTris += indexData.mNumTris;
		}

		// now that we have the combined vertex format, go back over sub-meshes
		// and compute the "intersection" of the material's format and the mesh's format
		for( u32 i = 0; i < numSubMeshes; ++i )
		{
			tSubMesh& sb = gfxMesh.mSubMeshes[i];

			// make a copy of the combined mesh format so as not to modify it
			Gfx::tVertexFormat subMeshFormat = combinedVertexFormat;

			// now intersect with material's format; N.B. this is not commutative
			subMeshFormat.fIntersect( sb.mMaterial->fVertexFormat( ) );

			// now look for this format in our master list
			u32 ifmt = 0;
			for( ; ifmt < mSubMeshVertexFormats.fCount( ); ++ifmt )
				if( mSubMeshVertexFormats[ ifmt ]->fFullyEqual( subMeshFormat ) )
					break;

			// check if format didn't exist, add if not add it
			if( ifmt == mSubMeshVertexFormats.fCount( ) )
				mSubMeshVertexFormats.fPushBack( tStrongPtr<Gfx::tVertexFormatVRam>( new Gfx::tVertexFormatVRam( subMeshFormat ) ) );
		
			sb.mVertexFormat = mSubMeshVertexFormats[ ifmt ].fGetRawPtr( );
		}

		if( totalTris == 0 )
		{
			log_warning( 0, "Invalid mesh: zero triangle count" );
			return false;
		}

		meshData.mGeometryData.fGrowCount( 1 );
		Gfx::tGeometryBufferSysRam& geometryBuffer = meshData.mGeometryData.fBack( );

		// allocate big geometry buffer
		geometryBuffer.fAllocate( combinedVertexFormat, totalTris * 3 );

		// convert all geometry
		tSigmlGeometryConverter converter( meshData, sigmlMesh, ibsStart );
		geometryBuffer.fForEachVertexElement( converter );

		// increment global vb/ib counters
		ithVb += 1;
		ithIb += numSubMeshes;

		// now we have a fully expanded (or un-indexed) geometry buffer; first step
		// is to index it (i.e., remove duplicates)
		tDynamicArray< u32 > reorderedVertexIndices;
		geometryBuffer.fRemoveDuplicates( reorderedVertexIndices );

		// next step is to create the index buffers for each sub mesh
		for( u32 i = 0; i < numSubMeshes; ++i )
		{
			tIndexBufferData& indexData = meshData.mIndexData[ ibsStart + i ];

			const u32 startIdx = indexData.mTriOffset * 3;
			const u32 numIds = indexData.mNumTris * 3;

			// determine appropriate index format
			const Gfx::tIndexFormat idxFormat = Gfx::tIndexFormat::fCreateAppropriateFormat( 
				Gfx::tIndexFormat::cPrimitiveTriangleList, 
				geometryBuffer.fVertexCount( ) );

			indexData.mIndexBuffer.fAllocate( idxFormat, numIds );
			indexData.mIndexBuffer.fSetIndices( 0, &reorderedVertexIndices[ startIdx ], numIds );
		}

		// assign ray cast data
		fConvertRayCastMeshData( gfxMesh, sigmlMesh );
		return true;
	}

	void tSigmlConverter::fOutputGeometryFile( const tMeshData& meshData, tPlatformId pid )
	{
		tFilePathPtr outputPath = iAssetGenPlugin::fCreateAbsoluteOutputPath( pid, meshData.mResourcePath );
		tFileWriter ofile( outputPath );
		sigassert( ofile.fIsOpen( ) );

		Gfx::tGeometryFile geoFile;

		// make sure we set the signature prior to serializing
		geoFile.fSetSignature( pid, Rtti::fGetClassId<Gfx::tGeometryFile>( ), Gfx::tGeometryFile::cVersion );

		// TODO determine for this platform whether we need to keep the buffers around;
		// for now we assume that we can discard these buffers safely
		geoFile.mDiscardSysRamBuffers = true;

		tDynamicArray< tDynamicBuffer > geometryBuffers( meshData.mGeometryData.fCount( ) );
		tDynamicArray< tDynamicBuffer > indexBuffers( meshData.mIndexData.fCount( ) );
		geoFile.mGeometryPointers.fNewArray( geometryBuffers.fCount( ) );
		geoFile.mIndexListPointers.fNewArray( indexBuffers.fCount( ) );

		for( u32 i = 0; i < geometryBuffers.fCount( ); ++i )
		{
			// copy geometry buffer
			geometryBuffers[ i ] = meshData.mGeometryData[ i ].fGetBuffer( );
			// endian swap the index buffer (the serializer won't know how to, as it's just a raw stream of bytes)
			meshData.mGeometryData[ i ].fEndianSwapBuffer( geometryBuffers[ i ], pid );
			// perform a "fake" allocation of the vram buffer so it gets correct parameters
			geoFile.mGeometryPointers[ i ].mVRamBuffer.fPseudoAllocate( 
				meshData.mGeometryData[ i ].fVertexFormat( ), 
				meshData.mGeometryData[ i ].fVertexCount( ), 
				0 );
		}

		for( u32 i = 0; i < indexBuffers.fCount( ); ++i )
		{
			// copy index buffer
			indexBuffers[ i ] = meshData.mIndexData[ i ].mIndexBuffer.fGetBuffer( );
			// endian swap the index buffer (the serializer won't know how to, as it's just a raw stream of bytes)
			meshData.mIndexData[ i ].mIndexBuffer.fEndianSwapBuffer( indexBuffers[ i ], pid  );
			// perform a "fake" allocation of the vram buffer so it gets correct parameters
			geoFile.mIndexListPointers[ i ].mVRamBuffer.fPseudoAllocate( 
				meshData.mIndexData[ i ].mIndexBuffer.fIndexFormat( ), 
				meshData.mIndexData[ i ].mIndexBuffer.fIndexCount( ), 
				meshData.mIndexData[ i ].mNumTris,
				0 );
		}


		// create load in place serializer, and serialize the file header (persistent portion of the file)
		tLoadInPlaceSerializer ser;
		geoFile.mHeaderSize = ser.fSave( geoFile, ofile, pid );

		// now continue saving buffers from the current location of the output file;
		// we'll store the offset of each buffer in the header
		u32 offsetFromStartOfFile = geoFile.mHeaderSize;
		for( u32 i = 0; i < geometryBuffers.fCount( ); ++i )
		{
			// store current file offset
			geoFile.mGeometryPointers[ i ].mBufferOffset = offsetFromStartOfFile;

			// store buffer size
			geoFile.mGeometryPointers[ i ].mBufferSize = geometryBuffers[ i ].fCount( );

			// write shader buffer
			ofile( geometryBuffers[ i ].fBegin( ), geometryBuffers[ i ].fCount( ) );

			// increment current file offset
			offsetFromStartOfFile += geometryBuffers[ i ].fCount( );
		}
		for( u32 i = 0; i < indexBuffers.fCount( ); ++i )
		{
			// store current file offset
			geoFile.mIndexListPointers[ i ].mBufferOffset = offsetFromStartOfFile;

			// store buffer size
			geoFile.mIndexListPointers[ i ].mBufferSize = indexBuffers[ i ].fCount( );

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
				log_warning( 0, "Materials could not be converted, aborting." );
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
				mMeshData[ i ]->mGeometryData.fSetCapacity( numMeshes );
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
			log_warning( 0, "There are zero objects in the current sigml file - aborting conversion." );
			return false;
		}

		// store objects
		mObjects.fNewArray( convertedObjects.fCount( ) );
		for( u32 i = 0; i < mObjects.fCount( ); ++i )
			mObjects[ i ] = convertedObjects[ i ];

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
		tBinaryFileBase::fSetSignature( pid, Rtti::fGetClassId<tSceneGraphFile>( ), tSceneGraphFile::cVersion );

		// output
		tLoadInPlaceSerializer ser;
		ser.fSave( static_cast<tSceneGraphFile&>( *this ), ofile, pid );

		return true;
	}
}
