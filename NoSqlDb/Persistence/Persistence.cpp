/////////////////////////////////////////////////////////////////////
// Persistence.cpp - Test persistence function of persistence.h	   //
// ver 1.1                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////

#include "Persisence.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <functional>
#include "../XmlDocument/XmlDocument/XmlDocument.h"
#include "../XmlDocument/XmlParser/XmlParser.h"
#include "../DbCore/DbCore.h"
#include "../Query/Query.h"
#include "../Test/Test.h"
using namespace NoSqlDb;
#ifdef TEST_PERSISTENCE

bool test1() {
	Utilities::title("#1: Testing persist operation:");
	std::cout << "Persist the query results.";
	NoSqlDb::DbCore<std::string> db;
	NoSqlDb::DbQuery<std::string> querier(db);
	DbPersist<std::string> persistor;

	querier.from(db).insert("\"齐天宇\", \"Loser\", \"\", \"砍口垒,哈吉马路呦\", \"没实习, 没钱, 没女朋友\", \"\", \"\", \"\"", false);
	querier.from(db).insert("\"Tianyu Qi2\", \"Loser2\", \"\", \"Really a loser\", \"\", \"\", \"\", \"\"");
	querier.from(db).insert("\"Tianyu Qi3\", \"Loser3\", \"\", \"He has no intenship\", \"\", \"\", \"\", \"\"");
	querier.from(db).insert("\"Tianyu Qi4\", \"Loser4\", \"Tianyu Qi1, Tianyu Qi2\", \"And he has no child\", \"\", \"\", \"\", \"\"", false);
	querier.from(db).insert("\"Tianyu Qi5\", \"Loser5\", \"Tianyu Qi3, Tianyu Qi4\", \"Really cannot do anything\", \"\", \"\", \"\", \"\"", false);
	querier.from(db).insert("\"Tianyu Qi6\", \"Loser6\", \"\", \"Really cannot do anything\", \"some category1, same category2\", \"\", \"\", \"\"");

	std::cout << "\n Records in datebase" << std::endl;
	querier.from(db).find().resultDisplay();
	Utilities::putline();

	querier.from(db).find();
	persistor.persist(querier.eval(), "sample");

	return true;
}

bool test2() {
	Utilities::title("#2: Testing restore operation:");
	std::cout << "Parse the XML document persist by test1";
	NoSqlDb::DbCore<std::string> db;
	NoSqlDb::DbQuery<std::string> querier(db);

	DbPersist<std::string> persistor;
	
	DbPersist<std::string>::Content result = persistor.restore("sample.xml");
	for (DbPersist<std::string>::Content::iterator iter = result.begin(); iter != result.end(); iter++) {
		querier.from(db).insert(*iter, false);
	}

	std::cout << "\n Restore the records from sample.xml" << std::endl;

	querier.from(db).resultDisplay();

	if (querier.from(db).eval().size() != 6) return false;

	return true;
}

bool test3() {
	Utilities::title("#3: Testing restore broken XML file:");
	std::cout << "Parse the modified XML document persist by test1, which is broken in format";
	NoSqlDb::DbCore<std::string> db;
	NoSqlDb::DbQuery<std::string> querier(db);

	DbPersist<std::string> persistor;

	try {
		persistor.restore("sample - broken.xml");
		return false;
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
	}

	return true;
}

//----< test stub >----------------------------------------------------

int main() {
	NoSqlDb::test tester;
	tester.registerTest(test1, "Test1: persist operation");
	tester.registerTest(test2, "Test2: restore operation");
	tester.registerTest(test3, "Test3: restore operation with broken XML file");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif // TEST_PERSISTEENCE
