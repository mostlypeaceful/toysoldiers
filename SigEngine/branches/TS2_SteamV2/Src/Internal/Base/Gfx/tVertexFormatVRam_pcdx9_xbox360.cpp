#include "BasePch.hpp"
#if defined( platform_pcdx9 ) || defined( platform_xbox360 )
#include "tVertexFormatVRam.hpp"
#include "tDevice.hpp"

namespace Sig { namespace Gfx
{
	namespace
	{
		void fConvertVertexElement( D3DVERTEXELEMENT9& dxve, const tVertexElement& ve )
		{
			static const u32 semanticMap[]=
			{
				D3DDECLUSAGE_POSITION,//cSemanticPosition,
				D3DDECLUSAGE_NORMAL,//cSemanticNormal,
				D3DDECLUSAGE_TANGENT,//cSemanticTangent,
				D3DDECLUSAGE_BINORMAL,//cSemanticBinormal,
				D3DDECLUSAGE_TEXCOORD,//cSemanticTexCoord,
				D3DDECLUSAGE_COLOR,//cSemanticColor,
				D3DDECLUSAGE_BLENDWEIGHT,//cSemanticBoneWeights,
				D3DDECLUSAGE_BLENDINDICES,//cSemanticBoneIndices,
			};
			sig_static_assert( array_length( semanticMap ) == tVertexElement::cSemanticCount );

			static const u32 formatMap[]=
			{
				D3DDECLTYPE_FLOAT1,//cFormat_f32_1,
				D3DDECLTYPE_FLOAT2,//cFormat_f32_2,
				D3DDECLTYPE_FLOAT3,//cFormat_f32_3,
				D3DDECLTYPE_FLOAT4,//cFormat_f32_4,
				D3DDECLTYPE_FLOAT16_2,//cFormat_f16_2,
				D3DDECLTYPE_FLOAT16_4,//cFormat_f16_4,
				D3DDECLTYPE_UBYTE4,//cFormat_u8_4,
				D3DDECLTYPE_D3DCOLOR,//cFormat_u8_4_Color,
				D3DDECLTYPE_UBYTE4N,//cFormat_u8_4_Normalized,
			};
			sig_static_assert( array_length( formatMap ) == tVertexElement::cFormatCount );

			dxve.Stream			= ve.mStreamIndex;
			dxve.Method			= D3DDECLMETHOD_DEFAULT;
			dxve.Offset			= ve.mOffsetFromBase;
			dxve.Type			= formatMap[ ve.mFormat ];
			dxve.Usage			= semanticMap[ ve.mSemantic ];
			dxve.UsageIndex		= ve.mSemanticIndex;
		}

		struct tCachedFormat : public tRefCounter
		{
			define_class_pool_new_delete( tCachedFormat, 32 );
			b32 fCompare( const tFixedArray< D3DVERTEXELEMENT9, 16 >& dxVtxElems, u32 numElems ) const
			{
				if( mNumElems != numElems )
					return false;
				if( memcmp( mDxVtxElems.fBegin( ), dxVtxElems.fBegin( ), (numElems-1) * sizeof( D3DVERTEXELEMENT9 ) ) != 0 )
					return false;
				return true;
			}
			IDirect3DVertexDeclaration9* mDxVd;
			u32 mNumElems;
			tFixedArray< D3DVERTEXELEMENT9, 16 > mDxVtxElems;
		};
		static tGrowableArray< tCachedFormat* > gCachedFormats;
		static IDirect3DVertexDeclaration9* fGlobalRegister( tVertexFormatVRam* format, IDirect3DDevice9* d3ddev )
		{
			const u32 numDxElems = format->fElementCount( ) + 1;

			// convert our engine/platform-independent representation to dx-style
			tFixedArray< D3DVERTEXELEMENT9, 16 > dxVtxElems;
			fZeroOut( dxVtxElems );
			sigassert( dxVtxElems.fCount( ) >= numDxElems );
			for( u32 ive = 0; ive < numDxElems-1; ++ive )
				fConvertVertexElement( dxVtxElems[ ive ], ( *format )[ ive ] );
			const D3DVERTEXELEMENT9 nullDxVtxElem = D3DDECL_END( );
			dxVtxElems[ numDxElems - 1 ] = nullDxVtxElem;

			// look for existing, identical format
			for( u32 i = 0; i < gCachedFormats.fCount( ); ++i )
			{
				if( gCachedFormats[ i ]->fCompare( dxVtxElems, numDxElems ) )
				{
					gCachedFormats[ i ]->fAddRef( );
					return gCachedFormats[ i ]->mDxVd;
				}
			}

			// no identical vertex format exists yet, so create and store
			IDirect3DVertexDeclaration9* dxvd = 0;
			d3ddev->CreateVertexDeclaration(
				dxVtxElems.fBegin( ),
				&dxvd );
			sigassert( dxvd );

			gCachedFormats.fPushBack( NEW tCachedFormat( ) );
			gCachedFormats.fBack( )->fAddRef( );
			gCachedFormats.fBack( )->mDxVd = dxvd;
			gCachedFormats.fBack( )->mNumElems = numDxElems;
			gCachedFormats.fBack( )->mDxVtxElems = dxVtxElems;

			return dxvd;
		}
		static void fGlobalUnregister( tVertexFormatVRam* format )
		{
			for( u32 i = 0; i < gCachedFormats.fCount( ); ++i )
			{
				if( format->fPlatformHandle( ) == ( Gfx::tVertexFormatVRam::tPlatformHandle )gCachedFormats[ i ]->mDxVd )
				{
					if( gCachedFormats[ i ]->fDecRef( ) == 0 )
					{
						gCachedFormats[ i ]->mDxVd->Release( );
						delete gCachedFormats[ i ];
						gCachedFormats.fErase( i );
					}
					return;
				}
			}
			sigassert( !"should never call fGlobalUnregister without having called fGlobalRegister" );
		}
	}

	void tVertexFormatVRam::fAllocateInternal( const tDevicePtr& device )
	{
		sigassert( !device.fNull( ) );

		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		IDirect3DVertexDeclaration9* dxvd = fGlobalRegister( this, d3ddev );

		sigassert( dxvd );
		mPlatformHandle = ( tPlatformHandle )dxvd;
	}

	void tVertexFormatVRam::fDeallocateInternal( )
	{
		if( mPlatformHandle )
		{
			fGlobalUnregister( this );
			mPlatformHandle = 0;
		}
	}

	void tVertexFormatVRam::fApply( const tDevicePtr& device ) const
	{
		IDirect3DVertexDeclaration9* dxvd = ( IDirect3DVertexDeclaration9* )mPlatformHandle;
		sigassert( dxvd );
		device->fGetDevice( )->SetVertexDeclaration( dxvd );
	}

}}
#endif//#if defined( platform_pcdx9 ) || defined( platform_xbox360 )

