/////////////////////////////////////////////////////////////////////
// SWRepoCore.cpp - Test stub of SWRepoCore package			       //
// ver 1.0                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include "SWRepoCore.h"
#include "../NoSqlDb/Test/Test.h"
using namespace SWRTB;
#ifdef TEST_SWREPOCORE

bool test1() {
	Utilities::Title("Test1: Basic feature of repo core.");
	Utilities::putline();
	Core core("../coreTest/");
	NoSqlDb::showDb(core.core());
	return true;
}

// ----< test stub >-----
int main() {
	DbTest::test tester;
	tester.registerTest(test1, "Basic feature of repo core.");
	return 0;
}
#endif // TEST_SWREPOCORE

