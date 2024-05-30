#include "UnitTestsPch.hpp"


using namespace Sig;

define_unittest(TestStringPtr)
{

	// TODO need actual tests, was just using this to step through in debugger... looks right! :)

	{
		tStringPtr a("lol"), b("holy a BOMB"), c("cracker");
		tStringPtr d;

		b = tStringPtr("LOL");
		c = b;
	}

	{
		tFilePathPtr a("\\lol///\\\\whatcHAMACallit.jpg"), b("holyaBOMB\\\\\\"), c("////Cracker\\JacK.xml");
		tFilePathPtr d;

		b = tFilePathPtr("LOL");
		c = b;
	}

	// TODO add some tests to validate the global string table ordering, duplicates testing, etc.
	// TODO also verify size of global string table, it should have incremented by 6, then gone back to what it was before this function
}
