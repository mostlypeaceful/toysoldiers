#include "BasePch.hpp"
#include "tLight.hpp"
#include "tSceneGraphFile.hpp"

namespace Sig { namespace Gfx
{
	tLight::tLight( )
		: mRadii( 0.f, 0.f )
		, mLightType( cLightTypeDirection )
	{
		mColors[ cColorTypeFront ]	= Math::tVec4f::cOnesVector;
		mColors[ cColorTypeRim ]	= Math::tVec4f::cZeroVector;
		mColors[ cColorTypeBack ]	= Math::tVec4f::cZeroVector;
		mColors[ cColorTypeAmbient ]= Math::tVec4f::cZeroVector;
	}

	tLight::tLight( tNoOpTag )
		: mColors( cNoOpTag )
		, mRadii( cNoOpTag )
	{
	}

	void tLight::fSetTypeDirection( )
	{
		mLightType = cLightTypeDirection;
	}

	void tLight::fSetTypePoint( const Math::tVec2f& radii )
	{
		mLightType = cLightTypePoint;
		mRadii = radii;
	}

	void tLight::fSetColors( const Math::tVec3f& front, const Math::tVec3f& rim, const Math::tVec3f& back, const Math::tVec3f& ambient )
	{
		mColors[ cColorTypeFront ] = Math::tVec4f( front, 1.f );
		mColors[ cColorTypeRim ] = Math::tVec4f( rim, 1.f );
		mColors[ cColorTypeBack ] = Math::tVec4f( back, 1.f );
		mColors[ cColorTypeAmbient ] = Math::tVec4f( ambient, 1.f );
	}

	void tLight::fConvertFrom( tSceneGraphDefaultLight* const light )
	{
		sigassert( light );
		fSetTypeDirection( );
		fSetColors( light->mFrontColor, light->mRimColor, light->mBackColor, light->mAmbientColor );
	}

	void tLight::fSetColorForScript( u32 type, const Math::tVec4f& val )
	{
		sigassert( type < cColorTypeCount );
		mColors[ type ] = val;
	}

	Math::tVec4f tLight::fGetColorForScript( u32 type )
	{
		sigassert( type < cColorTypeCount );
		return mColors[ type ];
	}

	Math::tVec2f tLight::fGetRadiiForScript( )
	{
		return mRadii;
	}
}}

namespace Sig { namespace Gfx
{
	void tLight::fExportScriptInterface( tScriptVm& vm )
	{
		Sqrat::Class< tLight > classDesc( vm.fSq( ) );
		classDesc
			.Func(_SC("SetColor"), &tLight::fSetColorForScript)
			.Func(_SC("GetColor"), &tLight::fGetColorForScript)
			.Func(_SC("SetTypePoint"), &tLight::fSetTypePoint)
			.Func(_SC("GetRadii"), &tLight::fGetRadiiForScript)
			;

		vm.fRootTable( ).Bind(_SC("LightDesc"),classDesc);

		vm.fConstTable( ).Const(_SC("COLOR_TYPE_FRONT"),tLight::cColorTypeFront);
		vm.fConstTable( ).Const(_SC("COLOR_TYPE_RIM"),tLight::cColorTypeRim);
		vm.fConstTable( ).Const(_SC("COLOR_TYPE_BACK"),tLight::cColorTypeBack);
		vm.fConstTable( ).Const(_SC("COLOR_TYPE_AMBIENT"),tLight::cColorTypeAmbient);
	}

}}//Sig::Gfx
