#include "Test.h"
#include "../DateTime/DateTime.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include "../Query/Query.h"

using namespace DbTest;

#ifndef TEST_TEST
bool test_always_passes() { return true; }
bool test_always_fails() { return false; }
bool test_always_throws() {
	std::exception ex("exception\n         -- msg: this test always throws -- ");
	throw(ex);
}

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