#pragma once
#ifndef TESTPROJ1_H
#define TESTPROJ1_H
#include <iostream>
#include <iomanip>
#include <functional>
#include "../Query/Query.h"
#include "../DbCore/DbCore.h"
#include "../Persistence/Persisence.h"
#include "../Utilities/TestUtilities/TestUtilities.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include "../Test/Test.h"

namespace NoSqlDb {

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
}
#endif