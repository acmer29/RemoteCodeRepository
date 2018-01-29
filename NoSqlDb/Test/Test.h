#pragma once
#ifndef TEST_H
#define TEST_H
#include <iostream>
#include "../Utilities/TestUtilities/TestUtilities.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include <functional>
#include <iomanip>
namespace DbTest{
	class test : public TestExecutive {
	public:
		bool testsRun();
		void testsSummary();
	private:
		std::vector<std::string> passedTests;
		std::vector<std::string> failedTests;
		Tests tests_;
	};

	bool test::testsRun() {
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

	void test::testsSummary() {
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
		out << "  " << "FAILED: " << std::setw(4) << passedTests.size() << std::endl;
	}
}
#endif // !TEST_H
