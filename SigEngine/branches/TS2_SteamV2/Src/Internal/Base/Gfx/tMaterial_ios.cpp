#include "BasePch.hpp"
#if defined( platform_ios )
#include "tMaterial.hpp"
#include "tDevice.hpp"
#include "tLight.hpp"

namespace Sig { namespace Gfx
{
	void tMaterial::fApplyVector3VS( const tDevicePtr& device, u32 id, const Math::tVec3f& vector ) const
	{
	}

	void tMaterial::fApplyVector4VS( const tDevicePtr& device, u32 id, const Math::tVec4f& vector, u32 count ) const
	{
	}

	void tMaterial::fApplyMatrix3VS( const tDevicePtr& device, u32 id, const Math::tMat3f& matrix, u32 count ) const
	{
	}

	void tMaterial::fApplyMatrix4VS( const tDevicePtr& device, u32 id, const Math::tMat4f& matrix, u32 count ) const
	{
	}

	void tMaterial::fApplyMatrixPaletteVS( const tDevicePtr& device, u32 id, const Math::tMat3f* matrix, u32 numEntries ) const
	{
	}

	void tMaterial::fApplyVector3PS( const tDevicePtr& device, u32 id, const Math::tVec3f& vector ) const
	{
	}

	void tMaterial::fApplyVector4PS( const tDevicePtr& device, u32 id, const Math::tVec4f& vector, u32 count ) const
	{
	}

	void tMaterial::fApplyMatrix3PS( const tDevicePtr& device, u32 id, const Math::tMat3f& matrix, u32 count ) const
	{
	}

	void tMaterial::fApplyMatrix4PS( const tDevicePtr& device, u32 id, const Math::tMat4f& matrix, u32 count ) const
	{
	}

	void tMaterial::fApplyRimLight( const tDevicePtr& device, u32 id, const tRimLightShaderConstants& lightConstants ) const
	{
	}

	void tMaterial::fApplyLights( const tDevicePtr& device, u32 id, const tLightShaderConstantsArray& lightConstants ) const
	{
	}


}}
#endif//#if defined( platform_ios )

