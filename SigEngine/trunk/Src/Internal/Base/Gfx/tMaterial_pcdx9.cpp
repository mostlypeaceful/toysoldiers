#include "BasePch.hpp"
#if defined( platform_pcdx9 )
#include "tMaterial.hpp"
#include "tDevice.hpp"
#include "tLight.hpp"

namespace Sig { namespace Gfx
{
	void tMaterial::fApplyVector3VS( const tDevicePtr& device, u32 id, const Math::tVec3f& vector ) const
	{
		device->fGetDevice( )->SetVertexShaderConstantF( id, ( const float* )&vector, 1 );
	}

	void tMaterial::fApplyVector4VS( const tDevicePtr& device, u32 id, const Math::tVec4f& vector, u32 count ) const
	{
		device->fGetDevice( )->SetVertexShaderConstantF( id, ( const float* )&vector, count );
	}

	void tMaterial::fApplyMatrix3VS( const tDevicePtr& device, u32 id, const Math::tMat3f& matrix, u32 count ) const
	{
		device->fGetDevice( )->SetVertexShaderConstantF( id, ( const float* )&matrix, 3 * count );
	}

	void tMaterial::fApplyMatrix4VS( const tDevicePtr& device, u32 id, const Math::tMat4f& matrix, u32 count ) const
	{
		device->fGetDevice( )->SetVertexShaderConstantF( id, ( const float* )&matrix, 4 * count );
	}

	void tMaterial::fApplyMatrixPaletteVS( const tDevicePtr& device, u32 id, const Math::tMat3f* matrix, u32 numEntries ) const
	{
		device->fGetDevice( )->SetVertexShaderConstantF( id, ( const float* )matrix, 3 * numEntries);
	}

	void tMaterial::fApplyVector3PS( const tDevicePtr& device, u32 id, const Math::tVec3f& vector ) const
	{
		device->fGetDevice( )->SetPixelShaderConstantF( id, ( const float* )&vector, 1 );
	}

	void tMaterial::fApplyVector4PS( const tDevicePtr& device, u32 id, const Math::tVec4f& vector, u32 count ) const
	{
		device->fGetDevice( )->SetPixelShaderConstantF( id, ( const float* )&vector, count );
	}

	void tMaterial::fApplyMatrix3PS( const tDevicePtr& device, u32 id, const Math::tMat3f& matrix, u32 count ) const
	{
		device->fGetDevice( )->SetPixelShaderConstantF( id, ( const float* )&matrix, 3 * count );
	}

	void tMaterial::fApplyMatrix4PS( const tDevicePtr& device, u32 id, const Math::tMat4f& matrix, u32 count ) const
	{
		device->fGetDevice( )->SetPixelShaderConstantF( id, ( const float* )&matrix, 4 * count );
	}

	void tMaterial::fApplyRimLight( const tDevicePtr& device, u32 id, const tRimLightShaderConstants& lightConstants ) const
	{
		const u32 numSlots = sizeof( lightConstants ) / sizeof( Math::tVec4f );
		device->fGetDevice( )->SetPixelShaderConstantF( id, ( const float* )&lightConstants, numSlots );
	}

	void tMaterial::fApplyLights( const tDevicePtr& device, u32 id, const tLightShaderConstantsArray& lightConstants ) const
	{
		if( lightConstants.mLightCount > 0 )
		{
			const u32 numVec4sPerLight = tLightShaderConstantsArray::cShaderSlotsPerLight;
			device->fGetDevice( )->SetPixelShaderConstantF( id, ( const float* )lightConstants.mLightArray.fBegin( ), numVec4sPerLight * lightConstants.mLightCount );
		}
	}

	void tMaterial::fApplyLight( const tDevicePtr& device, u32 id, const tLightShaderConstants& lightConstants ) const
	{
		const u32 numVec4sPerLight = tLightShaderConstantsArray::cShaderSlotsPerLight;
		device->fGetDevice( )->SetPixelShaderConstantF( id, ( const float* )&lightConstants, numVec4sPerLight );
	}


}}
#endif//#if defined( platform_pcdx9 )

