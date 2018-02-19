#pragma once
/////////////////////////////////////////////////////////////////////
// Executive.h - Implements the entry of the database			   // 
//				 as well as demos                                  //
// ver 1.0                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - dbExecutor
* dbExecutor now provides demo function, for display the project has met all requirements
* It will finally serve as the only entry of the project

* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* StringUtilities.h, StringUtilities.cpp
* Query.h, Query.cpp
* Persistence.h, Persistence.cpp
* Executive.h, Executive.cpp
*
* Build Process:
* --------------
* devenv NoSqlDb.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 02 Feb 2018 - Implement dbExecutive class and test demos
*/
#ifndef EXECUTIVE_H
#define EXECUTIVE_H
#include <iostream>
#include "../DbCore/DbCore.h"
#include "../Persistence/Persisence.h"
#include "../Query/Query.h"
#include "../Test/Test.h"
namespace DbExecutive {
	class dbExecutor {
	public:
		void demo();
	private:
		NoSqlDb::DbCore<std::string> db;
		DbTest::test tester;
		DbTest::testCase presenter;
	};

	void dbExecutor::demo() {
		presenter.casesRun();
	}
}
#endif