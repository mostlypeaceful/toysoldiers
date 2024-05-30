#include "UnitTestsPch.hpp"

using namespace Sig;

struct tNotBuiltIn {};

define_unittest(TestCoreTraits)
{
	fAssert( fIsBuiltinType<u16>() );
	fAssert( fIsBuiltinType<const u16>() );
	fAssert( fIsBuiltinType<const u16* const>() );
	fAssert( fIsBuiltinType<const u16* const * const * const * const>() );
	fAssert( fIsBuiltinType<volatile u16>() );
	fAssert( fIsBuiltinType<volatile u16* volatile>() );
	fAssert( fIsBuiltinType<volatile u16* volatile * volatile * volatile * volatile>() );
	fAssert( fIsBuiltinType<const volatile u16>() );
	fAssert( fIsBuiltinType<const volatile u16* const volatile>() );
	fAssert( fIsBuiltinType<const volatile u16* const volatile * const volatile * const volatile * const volatile>() );

	fAssert( !fIsBuiltinType<tNotBuiltIn>() );
	fAssert( !fIsBuiltinType<const tNotBuiltIn>() );
	fAssert( !fIsBuiltinType<const tNotBuiltIn* const>() );
	fAssert( !fIsBuiltinType<const tNotBuiltIn* const * const * const * const>() );
	fAssert( !fIsBuiltinType<volatile tNotBuiltIn>() );
	fAssert( !fIsBuiltinType<volatile tNotBuiltIn* volatile>() );
	fAssert( !fIsBuiltinType<volatile tNotBuiltIn* volatile * volatile * volatile * volatile>() );
	fAssert( !fIsBuiltinType<const volatile tNotBuiltIn>() );
	fAssert( !fIsBuiltinType<const volatile tNotBuiltIn* const volatile>() );
	fAssert( !fIsBuiltinType<const volatile tNotBuiltIn* const volatile * const volatile * const volatile * const volatile>() );
}
