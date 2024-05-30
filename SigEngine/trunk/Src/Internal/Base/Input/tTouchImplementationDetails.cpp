#include "BasePch.hpp"
#include "tTouchImplementationDetails.hpp"

namespace Sig { namespace Input { namespace ImplDetails {
	b32 fCheckFlag( const char*& s, const char* flagName, b32& flag )
	{
		using namespace StringUtil;

		const u32 flagNameLength = strlen(flagName);

		if ( fStrnicmp( s, flagName, flagNameLength ) == 0 && (s[flagNameLength]=='\0' || fIsAnyOf( cValidAfterFlag, s[flagNameLength] )) )
		{
			flag = true;
			s += flagNameLength;
			return true;
		}
		return false;
	}
}}} // Sig::Input::ImplDetails
