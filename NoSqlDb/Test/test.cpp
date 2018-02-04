
#include "../DateTime/DateTime.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include "Test.h"
using namespace DbTest;

#ifdef TEST_TEST
bool test_always_passes() { return true; }
bool test_always_fails() { return false; }
bool test_always_throws() {
	throw std::exception("exception\n         -- msg: this test always throws -- ");
}

//----< test stub >----------------------------------------------------

int main() {
	test tester;
	tester.registerTest(test_always_passes, "A should-pass test");
	tester.registerTest(test_always_fails, "A should-fail test");
	tester.registerTest(test_always_throws, "A should-throw test");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif

