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
* Maintenance History:
* --------------------
* ver 1.0 : 26 Jan 2018 - Test inherited from testExecutive
* ver 1.1 : 01 Feb 2018 - Test class finished, all unit tests finished
* ver 1.1 : 04 Feb 2018 - Refactored testCase, add requirement demo functions
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

namespace DbTest{

	class testCase {
	public:
		bool testR1();
		bool testR2();
		bool testR3();
		bool testR4();
		bool testR5();
		bool testR6();
		bool testR7();
		bool testR8();
		bool testR9();
		bool testR10();
		bool testR11();
		bool testR12();
		bool testR13();
		void casesRun(std::ostream& out = std::cout);
	private:
		std::vector<std::_Mem_fn<bool(testCase::*) ()>> cases;
		void check(bool result, std::ostream& out);
		NoSqlDb::DbCore<std::string> db;
	};

	class test : public TestExecutive {
	public:
		bool testsRun();
		void testsSummary();
	private:
		std::vector<std::string> passedTests;
		std::vector<std::string> failedTests;
		Tests tests_;
	};

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
