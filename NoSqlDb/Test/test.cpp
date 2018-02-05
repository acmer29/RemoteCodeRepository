
#include "../DateTime/DateTime.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include "Test.h"

using namespace DbTest;

bool testCase::testR1() {
	Utilities::title("#1: Implemented in C++ and using the facilities of the standard C++ Libraries and Visual Studio 2017");
	Utilities::putline();
	std::cout << "Already satisified." << std::endl;
	return true;
}

bool testCase::testR2() {
	Utilities::title("#2: Use the C++ standard library\'s streams for all console I/O and new and delete for all heap-based memory management");
	Utilities::putline();
	std::cout << "Already satisified." << std::endl;
	return true;
}

bool testCase::testR3() {
	Utilities::title("#3: Implement a generic key-value in-memory database where each value consists of metadata and a generic type of payload");
	Utilities::putline();
	std::cout << "Already satisified by prototype provided by professor" << std::endl;
	return true;
}

bool testCase::testR4() {
	Utilities::title("#4: Support addition and deletion of key/value pairs");
	Utilities::putline();
	DbQuery::queryResult<std::string> querier(db);

	std::cout << "\n Before insert operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"");
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE-687\", \"Male, CHINESE, Asian\"");
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking The CSE 687\", \"C++, Python, Haskell\"");

	std::cout << "\n After insert 3 elements into the database" << std::endl;
	Utilities::putline();
	querier.from(db).resultDisplay();
	Utilities::putline();

	if (!db.contains("Tianyu Qi") || !db.contains("Tianyu Qi0") || !db.contains("Tianyu Qi1")) return false;

	std::cout << "\n Before remove operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	querier.from(db).find("name", "Tianyu Qi").remove();

	std::cout << "\n After remove an element name \"Tianyu Qi\" from the database" << std::endl;

	querier.from(db).resultDisplay();
	Utilities::putline();

	if (db.contains("Tianyu Qi") == true) return false;

	return true;
}

bool testCase::testR5() {
	Utilities::title("#5: Support update of database elements, including replace one element with a new instance");
	Utilities::putline();
	DbQuery::queryResult<std::string> querier(db);
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking The CSE 687\", \"C++, Python, Haskell\"");

	std::cout << "\n Before update operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	querier.from(db).find("name: \"Tianyu Qi1\"").update("payLoad: \"Droped CSE 687\", description: \"A student\"");

	std::cout << "\n After finding the element name \"Tianyu Qi1\" updating its payLoad and description";

	querier.from(db).find("name: \"Tianyu Qi1\"").resultDisplay();

	querier.from(db).find("name: \"Tianyu Qi0\"").update("children: \"$(add)(Tianyu Qi1)\"");
	querier.from(db).find("name", "Tianyu Qi").update("children: \"$(remove)(Tianyu Qi, Tianyu Qi0)\"");
	querier.from(db).find("name: \"Tianyu Qi\"").update("dateTime: \"Mon Jan 01 23:59:59 2018\"");
	querier.from(db).find("name: \"Tianyu Qi0\"").update("dateTime: \"Mon Jul 01 23:59:59 2017\"");

	std::cout << "\n After update the element key \"Tianyu Qi0\", add it a child \"Tianyu Qi1\", and remove child \"Tianyu Qi0\" from element key \"Tianyu Qi\"";
	std::cout << "\n And also find element name \"Tianyu Qi\" and \"Tianyu Qi0\", update their datetime column";
	querier.from(db).find("name", "Tianyu Qi0").orFind("name", "Tianyu Qi1").orFind("name", "Tianyu Qi").resultDisplay();

	try {
		querier.from(db).find("name: \"Tianyu Qi\"").update("name", "Tianyu QiX");
		return false;
	}
	catch (std::exception& ex) {
		std::cout << "Catch exception of try editing the name";
		std::cout << ex.what() << std::endl;
	}

	if (querier.from(db).find("name: \"Tianyu Qi1\"").eval()[0].payLoad() != "Droped CSE 687" || querier.eval()[0].descrip() != "A student") return false;

	if (querier.from(db).find("name: \"Tianyu Qi0\"").hasChild("Tianyu Qi1") == false || querier.from(db).find("name: \"Tianyu Qi\"").hasChild("Tianyu Qi0") == true) return false;

	Utilities::putline();
	return true;
}

bool testCase::testR6() {
	Utilities::title("#6: Support queries in different forms");
	Utilities::putline();
	DbQuery::queryResult<std::string> querier(db);

	std::cout << "\n Before query operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	std::cout << "\n Query specified value - After find an element with name \"Tianyu Qi\"" << std::endl;
	querier.find("name", "Tianyu Qi");
	querier.resultDisplay();
	if (querier.eval().size() != 1 && querier.eval()[0].name() != "Tianyu Qi") return false;

	std::cout << "\n Query the child of a spcified key - After finding the children of the element with name \"Tianyu Qi\"" << std::endl;
	querier.find("name", "Tianyu Qi").childOf();
	querier.resultDisplay();
	if (querier.eval().size() != 1 && querier.eval()[0].name() != "Tianyu Qi1") return false;

	std::cout << "\n Query set of keys matching regular expression pattern - After finding elements whose description contains \"student\"" << std::endl;
	querier.from(db).find("description", "/.*student.*/");
	querier.resultDisplay();
	if (querier.eval().size() != 3) return false;

	std::cout << "\n Query the key with a spcecified written date time value - After finding element whose dateTime is 1 week before now" << std::endl;
	querier.from(db).find("dateTime", "$(before)(1 week)");
	querier.resultDisplay();
	if (querier.eval().size() != 2) return false;

	return true;
}

bool testCase::testR7() {
	Utilities::title("#7: Support queries on the set of keys returned by a previous query & queries on the union of results of one or more previous queries");
	Utilities::putline();
	DbQuery::queryResult<std::string> querier(db);

	std::cout << "\n Before query operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	std::cout << "\n \"And\" relationship - After finding the dateTime between 1 year and 1 week as well as name equals to \"Tianyu Qi\"" << std::endl;
	querier.from(db).find("dateTime", "$(between)(1 year, 1week)").find("name", "Tianyu Qi");
	querier.resultDisplay();
	if (querier.eval().size() != 1 && querier.eval()[0].name() != "Tianyu Qi") return false;

	std::cout << "\n \"Or\" relationship - After finding the element with payload \"Taking CSE 687\" as well as \"Droped CSE 687\"" << std::endl;
	querier.from(db).find("payLoad", "Taking CSE 687").orFind("payLoad", "Droped CSE 687");
	querier.resultDisplay();
	if (querier.eval().size() != 2) return false;

	return true;
}

bool testCase::testR8() {
	Utilities::title("#8: Persist a collection of database contents, defined by a collection of keys, to an XML file, on command");
	Utilities::putline();
	DbQuery::queryResult<std::string> querier(db);
	querier.from(db).drop();
	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE 687\", \"Male, CHINESE, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking CSE 687\", \"C++, Python, Haskell\"", true);
	querier.from(db).insert("\"Tianyu QiX\", \"Another\", \"Tianyu Qi, Tianyu Qi1\", \"I don\'t know\", \"balabala, always wrong\"", true);
	querier.from(db).find("name: \"Tianyu Qi\"").update("dateTime: \"Mon Jan 01 23:59:59 2018\"");
	querier.from(db).find("name: \"Tianyu Qi0\"").update("dateTime: \"Mon Jul 01 23:59:59 2017\"");

	std::cout << "\n Before persist operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	std::cout << "Persist all the elements with payLoad equals to \"Taking CSE 687\" to \"Persistence\\test.xml\"" << std::endl;
	DbPersistence::persistence<std::string> persistor;
	persistor.persist(querier.from(db).find("payLoad", "Taking CSE 687").eval(), "test");
	std::cout << XmlProcessing::XmlDocument("test.xml", XmlProcessing::XmlDocument::file).toString() << std::endl;

	std::cout << "\n Restored and augmented from \"test.xml\"" << std::endl;
	querier.from(db).remove("name: \"/Tianyu Qi[0-9]/\"");
	std::cout << "Before restore" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();
	std::vector<NoSqlDb::DbElement<std::string>> result = persistor.restore("test.xml");
	for (auto item : result) {
		querier.from(db).insert(item, false);
	}
	std::cout << "\n After restore operation" << std::endl;
	querier.from(db).resultDisplay();

	if (querier.eval().size() != 4) return false;

	return true;
}

bool testCase::testR9() {
	Utilities::title("#9: Implement a Payload type that contains a string and a vector of std::string");
	Utilities::putline();
	std::cout << "\n Implemented in DbCore.h line 52-54, 83-88, 95" << std::endl;
	return true;
}

bool testCase::testR10() {
	Utilities::title("#10: Provide, in your implementation, at least the following packages: Executive, DBCore, Query, Test");
	Utilities::putline();
	std::cout << "Already satisified" << std::endl;
	return true;
}

bool testCase::testR11() {
	Utilities::title("#11: Submitted with contents, in the form of an XML file, that describe your project's package structure and dependency relationships");
	DbQuery::queryResult<std::string> querier(db);
	querier.from(db).drop();
	querier.from(db).insert("\"DbCore\", \"Provide the database and data element\", \"DbCore.h, DbCore.cpp, DateTime.h, DateTime.cpp, Utilities.h, Utilities.cpp\", \"DbCore.h, DbCore.cpp\", \"NoSqlDb::\"", false);
	querier.from(db).insert("\"Query\", \"Provide query methods of the database\", \"DbCore.h, DbCore.cpp, DateTime.h, DateTime.cpp, Utilities.h, Utilities.cpp, Query.h, Query.cpp\", \"Query.h, Query.cpp\", \"DbQuery::\"", false);
	querier.from(db).insert("\"Persistence\", \"Provide persist methods of database \", \"DbCore.h, DbCore.cpp, DateTime.h, DateTime.cpp, Utilities.h, Utilities.cpp, Query.h, Query.cpp, Persistence.h, Persistence.cpp, XmlDocument.h, XmlDocument.cpp\", \"Persistence.h, Persisence.cpp\", \"DbPersistence\"", false);
	querier.from(db).insert("\"Test\", \"Provide methods of unit test and requirements demostration\", \"DbCore.h, DbCore.cpp, DateTime.h, DateTime.cpp, Utilities.h, Utilities.cpp, Query.h, Query.cpp, Persistence.h, Persistence.cpp, Test.h, Test.cpp, XmlDocument.h, XmlDocument.cpp\", \"Test.h, Test.cpp\", \"DbTest\"", false);
	querier.from(db).insert("\"Executive\", \"Provide entry of database and invoke requirement demostration\", \"DbCore.h, DbCore.cpp, DateTime.h, DateTime.cpp, Utilities.h, Utilities.cpp, Query.h, Query.cpp, Persistence.h, Persistence.cpp, Test.h, Test.cpp, XmlDocument.h, XmlDocument.cpp, Executive.h, Executive.cpp\", \"Executive.h, Executive.cpp\", \"DbExecutive\"", false);

	querier.from(db).resultDisplay();
	DbPersistence::persistence<std::string> persistor;
	persistor.persist(querier.from(db).eval(), "Project packege structure");
	return true;
}

bool testCase::testR12() {
	Utilities::title("#12: Accompanied by a test executive that clearly demonstrates you've met all the functional requirements");
	Utilities::putline();
	std::cout << "Implemented by Test.h and Test.cpp" << std::endl;
	return true;
}

bool testCase::testR13() {
	Utilities::title("provide a pdf file containing a package diagram");
	Utilities::putline();
	return true;
}

void testCase::casesRun(std::ostream& out) {
	cases.push_back(std::mem_fn(&testCase::testR1));
	cases.push_back(std::mem_fn(&testCase::testR2));
	cases.push_back(std::mem_fn(&testCase::testR3));
	cases.push_back(std::mem_fn(&testCase::testR4));
	cases.push_back(std::mem_fn(&testCase::testR5));
	cases.push_back(std::mem_fn(&testCase::testR6));
	cases.push_back(std::mem_fn(&testCase::testR7));
	cases.push_back(std::mem_fn(&testCase::testR8));
	cases.push_back(std::mem_fn(&testCase::testR9));
	cases.push_back(std::mem_fn(&testCase::testR10));
	cases.push_back(std::mem_fn(&testCase::testR11));
	cases.push_back(std::mem_fn(&testCase::testR12));
	cases.push_back(std::mem_fn(&testCase::testR13));
	for (size_t i = 0; i < cases.size(); ++i) {
		auto now = cases[i];
		try {
			bool result = now(*this);
			check(result, out);
			out << " -- \"" << "Requirement " << (i + 1) << "\"\n";
		}
		catch (std::exception &ex) {
			check(false, out);
			out << " -- \"" << "Requirement " << (i + 1) << "\"\n";
			out << ex.what() << "\n";
		}
	}
}

void testCase::check(bool result, std::ostream& out) {
	if (result)
		out << "  passed";
	else
		out << "  failed";
}

#ifdef TEST_TEST
bool test_always_passes() { return true; }
bool test_always_fails() { return false; }
bool test_always_throws() {
	throw std::exception("exception\n         -- msg: this test always throws -- ");
}

//----< test stub >----------------------------------------------------

int main() {
	test tester;
	tester.registerTest(test_always_passes, "A should-pass test");
	tester.registerTest(test_always_fails, "A should-fail test");
	tester.registerTest(test_always_throws, "A should-throw test");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif

