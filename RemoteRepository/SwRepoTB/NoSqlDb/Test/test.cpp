/////////////////////////////////////////////////////////////////////
// Test.cpp - Implements requirements demo function				   //
//			  And implements the tests of test class		       //
// ver 1.2                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////

#include "../DateTime/DateTime.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include "Test.h"

using namespace NoSqlDb;
#ifdef TEST_TEST
// ----< test function #1 >-----
bool test_always_passes() { return true; }
// ----< test function #2 >-----
bool test_always_fails() { return false; }
// ----< test function #3 >-----
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

