#include "BasePch.hpp"
#include "tLight.hpp"

namespace Sig { namespace Gfx
{
	tLight::tLight( )
		: mRadii( 0.f, 0.f )
		, mAngles( 0.f, 0.f )
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
		, mAngles( cNoOpTag )
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

	void tLight::fSetTypeSpot( const Math::tVec2f& radii, const Math::tVec2f& angles )
	{
		mLightType = cLightTypeSpot;
		mRadii = radii;
		mAngles = angles;
	}

}}
