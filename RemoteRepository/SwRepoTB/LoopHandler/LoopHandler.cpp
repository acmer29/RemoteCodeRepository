/////////////////////////////////////////////////////////////////////
// LoopHandler.cpp - Test stub of LoopHandler package			   //
// ver 1.0                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include "LoopHandler.h"
#include "../NoSqlDb/Test/Test.h"
using namespace SWRTB;
#ifdef TEST_LOOPHANDLER

bool test1() {
	Utilities::Title("Test1: Basic feature of LoopHandler");
	Utilities::putline();
	std::vector<NoSqlDb::DbElement<std::string>> forTest(2);
	forTest[1].name("test1"); forTest[1].children().push_back("test2");
	forTest[2].name("test2"); forTest[2].children().push_back("test1");
	LoopHandler worker(forTest);
	return (!worker.isLoopExists());
}

// ----< test stub >-----
int main() {
	DbTest::test tester;
	tester.registerTest(test1, "Test1: Basic feature of LoopHandler");
	return 0;
}
#endif