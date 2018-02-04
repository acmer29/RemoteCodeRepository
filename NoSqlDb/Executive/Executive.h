#pragma once
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