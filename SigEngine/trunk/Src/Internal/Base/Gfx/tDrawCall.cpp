#include "BasePch.hpp"
#include "tDrawCall.hpp"
#include "tLightCombo.hpp"

namespace Sig { namespace Gfx
{
	namespace { static const tLightCombo cNullLightComboInstance; }
	const tLightCombo* tDrawCall::cNullLightCombo = &cNullLightComboInstance;
}}
