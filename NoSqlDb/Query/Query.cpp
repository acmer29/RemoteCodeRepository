//
#include "Query.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include "../Test/Test.h"
#include <string>
#include <iomanip>
#include <iostream>
#include <unordered_map>
using namespace DbQuery;
#ifdef TEST_QUERY

bool test1() {
	Utilities::title("#1: Testing insert operation:");
	std::cout << "\n  Creating a db element with key \"Tianyu Qi\" and insert it:";
	NoSqlDb::DbCore<std::string> db;
	queryResult<std::string> querier(db);
	
	std::cout << "\n Before operation" << std::endl;
	NoSqlDb::showDb(db);
	Utilities::putline();

	NoSqlDb::DbElement<std::string> element;
	element.name("Tianyu Qi");
	element.descrip("Student");
	element.dateTime(DateTime().now());
	element.payLoad("Taking CSE687");
	element.category().push_back("Male"), element.category().push_back("Chinese"), element.category().push_back("Asian");
	element.children().push_back("C++"), element.children().push_back("Python"), element.children().push_back("Haskell");
	querier.from(db).insert(element, true);

	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE-687\", \"Male, CHINESE, Asian\"", true);

	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking The CSE 687\", \"C++, Python, Haskell\"", true);
	
	std::cout << "\n After operation" << std::endl;
	Utilities::putline();
	querier.from(db).resultDisplay();
	Utilities::putline();

	if (!db.contains("Tianyu Qi") || !db.contains("Tianyu Qi0") || !db.contains("Tianyu Qi1")) return false;
	else return true;
}

bool test2() {
	Utilities::title("#2: Testing remove operation");
	std::cout << "\n  remove element(s) returned by find results:";
	NoSqlDb::DbCore<std::string> db;
	queryResult<std::string> querier(db);
	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE-687\", \"Male, CHINESE, Asian\"", true);

	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking The CSE 687\", \"C++, Python, Haskell\"", true);

	std::cout << "\n Before operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	// remove without find (use query string to find the element to remove)
	querier.from(db).remove("name: \"Tianyu Qi\"");

	// remove with find (use the find result as the source of remove)
	querier.from(db).find("name", "Tianyu Qi").remove();

	std::cout << "\n After remove the element key equals to \"Tianyu Qi\"";
	Utilities::putline();
	querier.from(db).resultDisplay();
	Utilities::putline();

	if (!db.contains("Tianyu Qi")) return true;
	else return false;
}

bool test3() {
	Utilities::title("#3: Testing update operation");
	std::cout << "\n update elements by replacing with new element \n update element using new value string \n add / delete child / category";
	
	NoSqlDb::DbCore<std::string> db;
	queryResult<std::string> querier(db);
	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE-687\", \"Male, CHINESE, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking The CSE 687\", \"C++, Python, Haskell\"", true);

	std::cout << "\n Before operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	querier.from(db).find("name: \"Tianyu Qi1\"").update("payLoad: \"Droped CSE 687\", description: \"A student\"");

	std::cout << "\n After update the element key \"Tianyu Qi1\", update payLoad = \"Droped CSE 687\" and description = \"A student\"";
	Utilities::putline();
	querier.from(db).resultDisplay();
	Utilities::putline();

	if (querier.from(db).find("name: \"Tianyu Qi1\"").eval()[0].payLoad() != "Droped CSE 687" || querier.eval()[0].descrip() != "A student") return false;
	
	querier.from(db).find("name: \"Tianyu Qi0\"").update("children: \"$(add)(Tianyu Qi1)\"");
	querier.from(db).find("name", "Tianyu Qi").update("children: \"$(remove)(Tianyu Qi, Tianyu Qi0)\"");

	std::cout << "\n After update the element key \"Tianyu Qi0\", add it a child \"Tianyu Qi1\", and remove child \"Tianyu Qi0\" from element key \"Tianyu Qi\"";
	Utilities::putline();
	querier.from(db).resultDisplay();
	Utilities::putline();

	if (querier.from(db).find("name: \"Tianyu Qi0\"").hasChild("Tianyu Qi1") == false || querier.from(db).find("name: \"Tianyu Qi\"").hasChild("Tianyu Qi0") == true) return false;

	return true;
}

bool test4a() {
	Utilities::title("#4a: Testing basic find operation");
	std::cout << "\n  find elements by cascade find calls (and relationship):";

	NoSqlDb::DbCore<std::string> db;
	queryResult<std::string> querier(db);
	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE 687\", \"Male, CHINESE, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking CSE 687\", \"C++, Python, Haskell\"", true);

	std::cout << "\n Before operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	querier.from(db).find("children", "Tianyu Qi0");
	
	std::cout << "\n After find children name \"Tianyu Qi0\"";
	Utilities::putline();
	querier.resultDisplay();

	if (querier.hasChild("Tianyu Qi0") == false) return false;

	querier.from(db).find("payLoad", "Taking CSE 687").find("description", "student0");

	std::cout << "\n After find payLoad value \"Taking CSE 687\" and descripion euqals to \"student0\"";
	Utilities::putline();
	querier.resultDisplay();

	if (querier.eval()[0].descrip() != "student0") return false;
	
	return true;
}

bool test4b() {
	Utilities::title("#4b: Testing advanced find operation");
	std::cout << "\n  find elements by regular expression, other form of dateTime, and \"or\" relation of two find results (orFind): ";

	NoSqlDb::DbCore<std::string> db;
	queryResult<std::string> querier(db);
	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE 687\", \"Male, CHINESE, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking CSE 687\", \"C++, Python, Haskell\"", true);
	querier.from(db).find("name: \"Tianyu Qi\"").update("dateTime: \"Mon Jan 01 23:59:59 2018\"");
	querier.from(db).find("name: \"Tianyu Qi0\"").update("dateTime: \"Mon Jul 01 23:59:59 2017\"");

	std::cout << "\n Before operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	querier.from(db).find("description", "/.*student.*/");

	std::cout << "\n After find the element whose description contains \"student\"";
	querier.resultDisplay();
	Utilities::putline();

	if (querier.eval().size() != 3) return false;
	
	querier.from(db).find("dateTime", "$(before)(2 weeks)");

	std::cout << "\n After find the element whose dateTime is \"2 weeks\" from now";
	querier.resultDisplay();
	Utilities::putline();

	if (querier.eval().size() != 2) return false;

	querier.from(db).find("dateTime", "$(between)(1 weeks, 1 year)");

	std::cout << "\n After find the element whose dateTime is between \"1 year\" and \"1 week\"";
	querier.resultDisplay();
	Utilities::putline();

	if (querier.eval().size() != 2) return false;

	return true;
}

bool test5() {
	Utilities::title("#5: Testing result filter / decorator");
	std::cout << "\n Filter / Decorate the find result using childOf, ascendBy and descendBy: ";

	NoSqlDb::DbCore<std::string> db;
	queryResult<std::string> querier(db);
	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE 687\", \"Male, CHINESE, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking CSE 687\", \"C++, Python, Haskell\"", true);
	querier.from(db).insert("\"Tianyu QiX\", \"Another\", \"Tianyu Qi, Tianyu Qi1\", \"I don\'t know\", \"balabala, always wrong\"", true);

	std::cout << "\n Before operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	querier.from(db).find("name", "Tianyu QiX").childOf();

	std::cout << "\n After find children of the element name \"Tianyu QiX\" in descending order of \"name\" column" << std::endl;
	querier.descendBy("name");
	querier.resultDisplay();
	Utilities::putline();

	if (querier.eval().size() != 2) return false;

	querier.from(db).find("name", "Tianyu QiX").childOf(true);

	std::cout << "\n After find all descendent of the element name \"Tianyu QiX\"in ascending order of \"name\" column" << std::endl;
	querier.ascendBy("name");
	querier.resultDisplay();
	Utilities::putline();

	if (querier.eval().size() != 3) return false;

	querier.from(db).find("category: \"/.*Asian.*/\"").orFind("payLoad", "I don\'t know");

	std::cout << "\n After find all descendent of the element name \"Tianyu QiX\"in ascending order of \"name\" column" << std::endl;
	querier.ascendBy("name");
	querier.resultDisplay();
	Utilities::putline();

	if (querier.eval().size() != 3) return false;

	return true;
}

bool test6() {
	Utilities::title("#6: Testing all functions integrated, with tricky cases");
	std::cout << "\n No string in this test is well performed, no case in this test makes actual sense, embrace yourself for impact!";

	NoSqlDb::DbCore<std::string> db;
	queryResult<std::string> querier(db);
	querier.from(db).insert("\"Tianyu Qi0\", \"student0\", \"\", \"Taking CSE 687\", \"Male, Chinese, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi1\", \"student1\", \"\", \"Taking CSE 687\", \"Male, CHINESE, Asian\"", true);
	querier.from(db).insert("\"Tianyu Qi\", \"True student\", \"Tianyu Qi0, Tianyu Qi1\", \"Taking CSE 687\", \"C++, Python, Haskell\"", true);
	querier.from(db).insert("\"Tianyu QiX\", \"Another\", \"Tianyu Qi, Tianyu Qi1\", \"I don\'t know\", \"balabala, always wrong\"", true);
	std::cout << "\n Before operation" << std::endl;
	querier.from(db).resultDisplay();
	Utilities::putline();

	try {
		querier.from(db).insert("sdafasdfasdfafssadfdsasdfdssdaf");
		return false;
	}
	catch (std::exception& ex) {
		std::cout << "Exception catched - " << ex.what() << std::endl;
	}



	return true;
}

int main()
{
	DbTest::test tester;
	tester.registerTest(test1, "Test 1: Insert operation");
	tester.registerTest(test2, "Test 2: Remove operation");
	tester.registerTest(test3, "Test 3: Update operation");
	tester.registerTest(test4a, "Test 4a: Basic find operation");
	tester.registerTest(test4b, "Test 4b: Advanced find operation");
	tester.registerTest(test5, "Test 5: Result filter / decorator");
	tester.registerTest(test6, "Test 6: FINAL");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif // TEST_QUERY