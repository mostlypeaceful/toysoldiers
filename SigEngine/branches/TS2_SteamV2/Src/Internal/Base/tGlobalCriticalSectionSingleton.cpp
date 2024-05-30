#include "BasePch.hpp"
#include "tSharedStringTable.hpp"

// meant to be included within anonymous namespace in some translation units
namespace Sig
{
	namespace
	{

		class tGlobalCriticalSectionSingleton
		{
		public:
			static tSharedStringCriticalSection& fInstance( )
			{
				static tSharedStringCriticalSection sCritSec;
				return sCritSec;
			}
		private:
			tGlobalCriticalSectionSingleton();
		};
	}
}