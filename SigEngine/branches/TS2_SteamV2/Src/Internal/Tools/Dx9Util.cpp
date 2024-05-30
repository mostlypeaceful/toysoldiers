#include "ToolsPch.hpp"
#include "Dx9Util.hpp"
#include "FileSystem.hpp"

namespace Sig { namespace Dx9Util
{

	b32 fCopyShaderBuffer( tDynamicBuffer& obuffer, ID3DXBuffer* shBuffer, ID3DXBuffer* shErrors, std::string* errorsOut )
	{
		if( shErrors )
		{
			const char* errmsg = ( const char* )shErrors->GetBufferPointer( );
			if( errorsOut )
				*errorsOut = errmsg;
			else
				log_warning( 0, "Shader compilation errors:" << std::endl << errmsg );
			shErrors->Release( );
		}

		if( shBuffer )
		{
			obuffer.fNewArray( shBuffer->GetBufferSize( ) );
			fMemCpy( obuffer.fBegin( ), shBuffer->GetBufferPointer( ), obuffer.fCount( ) );
			shBuffer->Release( );
			return true;
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
				log_warning( 0, "Error trying to load texture [" << path << "]" );
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
				log_warning( 0, "Error trying to load texture [" << path << "]" );
			o.fReset( loadedTex );
			Gfx::fReleaseComPtr( loadedTex );
		}
		return o;
	}

}}
