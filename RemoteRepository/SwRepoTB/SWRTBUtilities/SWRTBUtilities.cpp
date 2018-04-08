/////////////////////////////////////////////////////////////////////
// SWRTBUtilities.cpp - Test stub of SWRTBUtilies package		   //
// ver 1.0                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include "SWRTBUtilities.h"
#include "../NoSqlDb/Test/Test.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
using namespace SWRTB;
#ifdef TEST_SWRTBUTILITIES

bool test1() {
	Utilities::Title("Basic feature of SWRTBUtilites.");
	Utilities::putline();
	std::string nameDemo1 = "nameSpace_filename.cpp.1";
	std::string nameDemo2 = "nameSpace::filename.cpp.2";
	std::cout << NSPFileNameToNSNFileName(nameDemo1) << std::endl;
	std::cout << NSNFileNameToNSPFileName(nameDemo2) << std::endl;
	return true;
}

int main() {
	NoSqlDb::test tester;
	tester.registerTest(test1, "Test1: Basic feature of SWRTBUtilities.");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif