/////////////////////////////////////////////////////////////////////
// Browse.cpp - Test stub of Browse package						   //
// ver 1.0                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include "Browse.h"
#include "../NoSqlDb/Test/Test.h"
using namespace SWRTB;
#ifdef TEST_BROWSE

bool test1() {
	Utilities::Title("Test1: Basic feature of browsing.");
	Utilities::putline();
	Browse worker;
	NoSqlDb::DbElement<std::string> forBrowse;
	worker.browse(forBrowse);
	return true;
}

// -----< test stub >-----
int main() {
	DbTest::test tester;
	tester.registerTest(test1, "Test1: Basic feature of browsing.");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif // TEST_BROWSE
