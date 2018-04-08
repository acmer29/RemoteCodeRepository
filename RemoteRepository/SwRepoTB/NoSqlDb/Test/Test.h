#pragma once
/////////////////////////////////////////////////////////////////////
// Test.h - Implements database test operations, and test cases    //
// ver 1.2                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides two class test and testCase
* test class inherited from testExecutive, 
* provides methods for unit test, include registerTest, testsRun and testSummary
* testCase class has all test functions of the project, 
* mainly used as demo all the requirement
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* StringUtilities.h, StringUtilities.cpp
* Query.h Query.cpp
* Persistence.h, Persistence.cpp
* Test.h, Test.cpp
*
* Build Process:
* --------------
* devenv NoSqlDb.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 26 Jan 2018 - Test inherited from testExecutive
* ver 1.1 : 01 Feb 2018 - Test class finished, all unit tests finished
* ver 1.2 : 04 Feb 2018 - Refactored testCase, add requirement demo functions
*/

#ifndef TEST_H
#define TEST_H
#include <iostream>
#include <iomanip>
#include <functional>
#include "../Query/Query.h"
#include "../DbCore/DbCore.h"
#include "../Persistence/Persisence.h"
#include "../Utilities/TestUtilities/TestUtilities.h"
#include "../Utilities/StringUtilities/StringUtilities.h"

namespace NoSqlDb{

	class test : public TestExecutive {
	public:
		bool testsRun();
		void testsSummary();
	private:
		std::vector<std::string> passedTests;
		std::vector<std::string> failedTests;
		Tests tests_;
	};

	// -----< testsRun: Provide a simple form for test functions to run >-----
	inline bool test::testsRun() {
		tests_ = getTest();
		TestExecutor<Test> executor;
		bool final = true;
		for (auto item : tests_) {
			bool result = executor.execute(item.test, item.testName);
			if (result == true) passedTests.push_back(item.testName);
			else {
				failedTests.push_back(item.testName);
				final = false;
			}
		}
		return final;
	}

	// -----< testSummary: Shows which test passed and which test failed >------
	inline void test::testsSummary() {
		Utilities::putline();
		std::ostream& out = std::cout;
		out << "Test summary: " << std::endl;
		std::vector<std::string>::iterator iter;
		for (iter = passedTests.begin(); iter != passedTests.end(); iter++) {
			out << std::left << "  " << *iter << std::right << std::setw(10) << "  --PASSED" << std::endl;
		}
		for (iter = failedTests.begin(); iter != failedTests.end(); iter++) {
			out << std::left << "  " << *iter << std::right << std::setw(10) << "  --FAILED" << std::endl;
		}
		out << "------" << std::endl;
		out << "  " << "PASSED: " << std::setw(4) << passedTests.size() << std::endl;
		out << "  " << "FAILED: " << std::setw(4) << failedTests.size() << std::endl;
	}
}
#endif // !TEST_H
