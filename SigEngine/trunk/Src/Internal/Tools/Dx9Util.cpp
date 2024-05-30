#include "ToolsPch.hpp"
#include "Dx9Util.hpp"
#include "FileSystem.hpp"
#include "Tatml.hpp"
#include "Gfx/tDynamicTextureVRam.hpp"
#include "tTextureSysRam.hpp"
#include "Gfx/tRenderContext.hpp"

namespace Sig { namespace Dx9Util
{

	b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut )
	{
		b32 justWarnings = true;

		if( shErrors )
		{
			const char* errmsg = ( const char* )shErrors->GetBufferPointer( );
			justWarnings = StringUtil::fStrStrI( errmsg, "error" ) == NULL;

			if( errorsOut )
				*errorsOut = errmsg;
			else
				log_warning( "Shader compilation " << (justWarnings ? "warnings:" : "errors:") << std::endl << errmsg );
			shErrors->Release( );
		}

		if( shBuffer )
		{
			obuffer.fNewArray( shBuffer->GetBufferSize( ) );
			fMemCpy( obuffer.fBegin( ), shBuffer->GetBufferPointer( ), obuffer.fCount( ) );
			shBuffer->Release( );
			return justWarnings;
		}

		return false;
	}

	b32 fCompileShader( const char* profileName, const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		if( !entryPoint )
			entryPoint = "main";

		ID3DXBuffer* buffer = 0;
		ID3DXBuffer* errors = 0;
		D3DXCompileShader( 
			hlsl.c_str( ),
			( UINT )hlsl.length( ),
			0,
			0,
			entryPoint,
			profileName,
			0,
			&buffer,
			&errors,
			0 );

		return fCopyShaderBuffer( obuffer, buffer, errors, errorsOut );
	}

	b32 fCompileVertexShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		const char* profile = "vs_3_0";
		return fCompileShader( profile, hlsl, obuffer, entryPoint, errorsOut );
	}

	b32 fCompilePixelShader( const std::string& hlsl, tDynamicBuffer& obuffer, const char* entryPoint, std::string* errorsOut )
	{
		const char* profile = "ps_3_0";
		return fCompileShader( profile, hlsl, obuffer, entryPoint, errorsOut );
	}

	tVertexShaderPtr fCreateVertexShader( const Gfx::tDevicePtr& device, const std::string& hlsl, const char* entryPoint, std::string* errorsOut )
	{
		if( hlsl.length( ) == 0 )
			return tVertexShaderPtr( );

		tDynamicBuffer obuffer;
		if( !fCompileVertexShader( hlsl, obuffer, entryPoint, errorsOut ) )
			return tVertexShaderPtr( );

		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		sigassert( d3ddev );

		tVertexShader* pShader = 0;
		d3ddev->CreateVertexShader( ( const DWORD* )obuffer.fBegin( ), &pShader );

		tVertexShaderPtr o( pShader );
		Gfx::fReleaseComPtr( pShader );
		return o;
	}

	tPixelShaderPtr fCreatePixelShader( const Gfx::tDevicePtr& device, const std::string& hlsl, const char* entryPoint, std::string* errorsOut )
	{
		if( hlsl.length( ) == 0 )
			return tPixelShaderPtr( );

		tDynamicBuffer obuffer;
		if( !fCompilePixelShader( hlsl, obuffer, entryPoint, errorsOut ) )
			return tPixelShaderPtr( );

		IDirect3DDevice9* d3ddev = device->fGetDevice( );
		sigassert( d3ddev );

		tPixelShader* pShader = 0;
		d3ddev->CreatePixelShader( ( const DWORD* )obuffer.fBegin( ), &pShader );

		tPixelShaderPtr o( pShader );
		Gfx::fReleaseComPtr( pShader );
		return o;
	}


	tTextureCache::tTextureCache( const Gfx::tDevicePtr& device ) 
		: mDevice( device )
		, mEntries( 32 )
		, mDirtyStamp( 0 )
	{
		IDirect3DTexture9* t = 0;

		t = device->fCreateSolidColorTexture( 1, 1, Math::tVec4f( 1.f, 1.f, 1.f, 1.f ) );
		mWhiteTexture.fReset( t );
		Gfx::fReleaseComPtr( t );

		t = device->fCreateSolidColorTexture( 1, 1, Math::tVec4f( 0.f, 0.f, 0.f, 1.f ) );
		mBlackTexture.fReset( t );
		Gfx::fReleaseComPtr( t );

		t = device->fCreateSolidColorTexture( 1, 1, Math::tVec4f( 0.5f, 0.5f, 1.0f, 1.f ) );
		mBlankNormals.fReset( t );
		Gfx::fReleaseComPtr( t );
	}

	tTextureCache::~tTextureCache( )
	{
	}

	tBaseTexturePtr tTextureCache::fFindLoad2D( const tFilePathPtr& simplePath )
	{
		if( simplePath.fLength( ) == 0 )
			return tBaseTexturePtr( );
		const tFilePathPtr path = ToolsPaths::fMakeResAbsolute( simplePath );

		tEntry* find = mEntries.fFind( path );
		if( !find )
		{
			if( !FileSystem::fFileExists( path ) )
				return tBaseTexturePtr( );

			find = mEntries.fInsert( path, tEntry( fLoad( path, false ), FileSystem::fGetLastModifiedTimeStamp( path ) ) );
		}

		sigassert( find );
		return find->mTexPtr;
	}

	tBaseTexturePtr tTextureCache::fFindLoadCube( const tFilePathPtr& simplePath )
	{
		if( simplePath.fLength( ) == 0 )
			return tBaseTexturePtr( );
		const tFilePathPtr path = ToolsPaths::fMakeResAbsolute( simplePath );

		tEntry* find = mEntries.fFind( path );
		if( !find )
		{
			if( !FileSystem::fFileExists( path ) )
				return tBaseTexturePtr( );

			find = mEntries.fInsert( path, tEntry( fLoad( path, true ), FileSystem::fGetLastModifiedTimeStamp( path ), true ) );
		}

		sigassert( find );
		return find->mTexPtr;
	}

	tBaseTexturePtr tTextureCache::fFindLoadAtlas( const tFilePathPtr& simplePath, Math::tVec4f & info )
	{
		if( !simplePath.fLength( ) )
			return tBaseTexturePtr( );

		const tFilePathPtr path = ToolsPaths::fMakeResAbsolute( simplePath );
		tEntry * find = mEntries.fFind( path );
		if( !find )
		{
			if( !FileSystem::fFileExists( path ) )
				return tBaseTexturePtr( );

			tEntry newEntry;
			newEntry.mTexPtr = fLoadAtlas( path, newEntry.mAtlasInfo );
			newEntry.mTimeStamp = FileSystem::fGetLastModifiedTimeStamp( path );

			find = mEntries.fInsert( path, newEntry );
		}

		info = find->mAtlasInfo;
		return find->mTexPtr;
	}
		
	void tTextureCache::fRefresh( )
	{
		b32 dirty = false;
		for( tEntryTable::tIterator i = mEntries.fBegin( ), iend = mEntries.fEnd( ); i != iend; ++i )
		{
			if( i->fNullOrRemoved( ) )
				continue;
			const u64 newTimeStamp = FileSystem::fGetLastModifiedTimeStamp( i->mKey );
			if( !i->mValue.mTexPtr || newTimeStamp != i->mValue.mTimeStamp )
			{
				dirty = true;
				i->mValue.mTexPtr = fLoad( i->mKey, i->mValue.mCubeMap );
				i->mValue.mTimeStamp = newTimeStamp;
			}
		}
		if( dirty )
			++mDirtyStamp;
	}

	namespace
	{
		struct tCubeFace
		{
			enum tFaces
			{
				cXPlus,
				cXMinus,
				cYPlus,
				cYMinus,
				cZPlus,
				cZMinus,
				cFaceCount
			};
				
			Math::tVec3f mZeroZeroDir; //ray shooting from center of cube to texel(0,0)'s world direction
			Math::tVec3f mUDir; //Ray shooting along edge of u direction
			Math::tVec3f mVDir;

			D3DCUBEMAP_FACES mFace;

			u32 mLocalStartX;
			u32 mLocalStartY;
			u32 mLocalWidth;
			u32 mLocalHeight;

			tCubeFace( u32 face, u32 width, u32 height )
			{
				mLocalHeight = height / 3.f;
				mLocalWidth = width / 4.f;

				const f32 edge = 2.f;

				//http://msdn.microsoft.com/en-us/library/windows/desktop/bb204881(v=vs.85).aspx
				switch( face )
				{
				case cXPlus:
					{
						mZeroZeroDir = Math::tVec3f( -1,1,1 );
						mUDir = Math::tVec3f( 0,0,-edge );
						mVDir = Math::tVec3f( 0,-edge,0 );
						mLocalStartX = 2 * mLocalWidth;
						mLocalStartY = 1 * mLocalHeight;
						mFace = D3DCUBEMAP_FACE_POSITIVE_X;
					}
					break;
				case cXMinus:
					{
						mZeroZeroDir = Math::tVec3f( 1,1,-1 );
						mUDir = Math::tVec3f( 0,0,edge );
						mVDir = Math::tVec3f( 0,-edge,0 );
						mLocalStartX = 0 * mLocalWidth;
						mLocalStartY = 1 * mLocalHeight;
						mFace = D3DCUBEMAP_FACE_NEGATIVE_X;
					}
					break;
				case cYPlus:
					{
						mZeroZeroDir = Math::tVec3f( 1,1,-1 );
						mUDir = Math::tVec3f( -edge,0,0 );
						mVDir = Math::tVec3f( 0,0,edge );
						mLocalStartX = 1 * mLocalWidth;
						mLocalStartY = 0 * mLocalHeight;
						mFace = D3DCUBEMAP_FACE_POSITIVE_Y;
					}
					break;
				case cYMinus:
					{
						mZeroZeroDir = Math::tVec3f( 1,-1,1 );
						mUDir = Math::tVec3f( -edge,0,0 );
						mVDir = Math::tVec3f( 0,0,-edge );
						mLocalStartX = 1 * mLocalWidth;
						mLocalStartY = 2 * mLocalHeight;
						mFace = D3DCUBEMAP_FACE_NEGATIVE_Y;
					}
					break;
				case cZPlus:
					{
						mZeroZeroDir = Math::tVec3f( 1,1,1 );
						mUDir = Math::tVec3f( -edge,0,0 );
						mVDir = Math::tVec3f( 0,-edge,0 );
						mLocalStartX = 1 * mLocalWidth;
						mLocalStartY = 1 * mLocalHeight;
						mFace = D3DCUBEMAP_FACE_POSITIVE_Z;
					}
					break;
				case cZMinus:
					{
						mZeroZeroDir = Math::tVec3f( -1,1,-1 );
						mUDir = Math::tVec3f( edge,0,0 );
						mVDir = Math::tVec3f( 0,-edge,0 );
						mLocalStartX = 3 * mLocalWidth;
						mLocalStartY = 1 * mLocalHeight;
						mFace = D3DCUBEMAP_FACE_NEGATIVE_Z;
					}
					break;
				}

				mZeroZeroDir.fNormalize( );
			}

			Math::tVec3f fTexelDir( f32 localU, f32 localV ) const
			{
				return (mZeroZeroDir + mUDir * localU + mVDir * localV).fNormalize( );
			}

			void fAddHarmonics( IDirect3DTexture9* tex, Gfx::tSphericalHarmonics& harmonicsOut, const Gfx::tShBasisWeights& weights ) const
			{
				const f32 cNormalizingFactor = 3.f / (6 * mLocalHeight * mLocalWidth);
				
				D3DLOCKED_RECT rect;

				HRESULT result = tex->LockRect( 0, &rect, NULL, D3DLOCK_READONLY );
				sigassert( result == D3D_OK );

				DWORD *imagedata = (DWORD*)rect.pBits; 
				sigassert( imagedata );

				for( u32 x = 0; x < mLocalWidth; ++x )
				{
					f32 xPercent = (f32)x/(mLocalWidth-1);
					for( u32 y = 0; y < mLocalHeight; ++y )
					{
						// Sample world direction
						f32 yPercent = (f32)y/(mLocalHeight-1);
						Math::tVec3f dir = fTexelDir( xPercent, yPercent );

						// Sample world texel
						u32 worldX = x + mLocalStartX;
						u32 worldY = y + mLocalStartY;
						u32 index = (worldY * rect.Pitch/4) + worldX;
						D3DXCOLOR color( imagedata[ index ] );
						
						// Add to harmonics
						harmonicsOut.fAddSphericalHarmonic( Math::tVec4f( color.r, color.g, color.b, 0.f ) * cNormalizingFactor, dir, weights );
					}
				}

				result = tex->UnlockRect( 0 );
				sigassert( result == D3D_OK );
			}

			// copy a single face into the correct place on the full cube map.
			void fCopyTextureToMaster( IDirect3DTexture9* face, IDirect3DTexture9* master )
			{
				D3DLOCKED_RECT faceRect;
				HRESULT result = face->LockRect( 0, &faceRect, NULL, D3DLOCK_READONLY );
				sigassert( result == D3D_OK );
				
				D3DLOCKED_RECT masterRect;
				result = master->LockRect( 0, &masterRect, NULL, 0 );
				sigassert( result == D3D_OK );

				DWORD *masterImagedata = (DWORD*)masterRect.pBits; 
				sigassert( masterImagedata );

				DWORD *faceImagedata = (DWORD*)faceRect.pBits; 
				sigassert( faceImagedata );

				for( u32 x = 0; x < mLocalWidth; ++x )
				{
					for( u32 y = 0; y < mLocalHeight; ++y )
					{
						// Sample world texel
						u32 worldX = x + mLocalStartX;
						u32 worldY = y + mLocalStartY;
						u32 worldindeX = (worldY * masterRect.Pitch/4) + worldX;

						u32 faceIndex = (y* faceRect.Pitch / 4) + x;

						masterImagedata[ worldindeX ] = faceImagedata[ faceIndex ];
					}
				}

				result = face->UnlockRect( 0 );
				sigassert( result == D3D_OK );

				result = master->UnlockRect( 0 );
				sigassert( result == D3D_OK );
			}
		};

		void fAddHarmonics( IDirect3DTexture9* tex, Gfx::tSphericalHarmonics& harmonicsOut, const Gfx::tShBasisWeights& weights )
		{
			D3DSURFACE_DESC desc;
			tex->GetLevelDesc( 0, &desc );

			for( u32 i = 0; i < tCubeFace::cFaceCount; ++i )
			{
				tCubeFace face( (tCubeFace::tFaces)i, desc.Width, desc.Height );
				face.fAddHarmonics( tex, harmonicsOut, weights );
			}
		}
	}

	void fAssembleCubeMapFromTextures( const tFixedArray<IDirect3DTexture9*,6>& textures, IDirect3DTexture9*& output )
	{
		D3DSURFACE_DESC masterDesc;
		output->GetLevelDesc( 0, &masterDesc );

		for( u32 i = 0; i < textures.fCount( ); ++i )
		{
			tCubeFace face( i, masterDesc.Width, masterDesc.Height );
			face.fCopyTextureToMaster( textures[ i ], output );
		}

	}



	Math::tVec2f tTextureCache::fTextureDims2D( tBaseTexture& tex )
	{
		D3DSURFACE_DESC desc;	
		IDirect3DTexture9* tex2D = static_cast<IDirect3DTexture9*>( &tex );	
		tex2D->GetLevelDesc( 0, &desc );
		return Math::tVec2f( desc.Width, desc.Height );
	}

	void tTextureCache::fComputeSphericalHarmonics( tBaseTexturePtr& input, Gfx::tSphericalHarmonics& output, const Gfx::tShBasisWeights& weights )
	{
		IDirect3DTexture9* tex = (IDirect3DTexture9*)input.fGetRawPtr( );
		fAddHarmonics( tex, output, weights );
	}

	tBaseTexturePtr tTextureCache::fLoad( const tFilePathPtr& path, b32 cubeMap ) const
	{
		tBaseTexturePtr o;
		if( !cubeMap )
		{
			IDirect3DTexture9* loadedTex = 0;
			const HRESULT hresult = D3DXCreateTextureFromFileEx(
				mDevice->fGetDevice( ),
				path.fCStr( ),
				D3DX_DEFAULT_NONPOW2,
				D3DX_DEFAULT_NONPOW2,
				D3DX_DEFAULT,
				0,
				D3DFMT_A8R8G8B8,
				D3DPOOL_MANAGED,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,
				0,
				0,
				&loadedTex );
			if( !SUCCEEDED( hresult ) || !loadedTex )
				log_warning( "Error trying to load texture [" << path << "]" );
			o.fReset( loadedTex );

			Gfx::fReleaseComPtr( loadedTex );
		}
		else
		{
			IDirect3DCubeTexture9* loadedTex = 0;
			const HRESULT hresult = D3DXCreateCubeTextureFromFileEx(
				mDevice->fGetDevice( ),
				path.fCStr( ),
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,
				D3DFMT_A8R8G8B8,
				D3DPOOL_MANAGED,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,
				0,
				0,
				&loadedTex );
			if( !SUCCEEDED( hresult ) || !loadedTex )
				log_warning( "Error trying to load texture [" << path << "]" );
			o.fReset( loadedTex );
			Gfx::fReleaseComPtr( loadedTex );
		}
		return o;
	}

	tBaseTexturePtr tTextureCache::fLoadAtlas( const tFilePathPtr& path, Math::tVec4f & info ) const
	{
		Tatml::tFile file;
		if( !file.fLoadXml( path ) )
			return tBaseTexturePtr( );

		Gfx::tTextureVRam tex2D;
		tTextureAtlasSysRam atlas;
	
		file.fToTextureAtlas( atlas );
		atlas.fConvertToVRamTexture( mDevice, tex2D );

		info.x = atlas.fNumTexturesX( );
		info.y = atlas.fNumTexturesY( );
		info.z = atlas.fSubTexWidth( );
		info.w = atlas.fSubTexHeight( );

		return tBaseTexturePtr( (IDirect3DTexture9*) tex2D.fGetPlatformHandle( ) );
	}

}}
