#include "TestProj2.h"
using namespace SWRTB;

// ----< demo requirement #1 >-----
bool testProj2::testR1() {
	Utilities::Title("#1: Use VS2017 and standard C++ libraries.");
	Utilities::putline();
	std::cout << "  " << typeid(std::function<bool()>).name()
		<< ", declared in this function, "
		<< "\n  is only valid for C++11 and later versions.";
	return true;
}

// ----< demo requirement #2 >-----
bool testProj2::testR2() {
	Utilities::Title("#2: Use standard library streams and heap management.");
	Utilities::putline();
	std::cout << " A visual examination of all the submitted code "
		<< "will show only\n  use of streams and operators new and delete.";
	return true;
}

// ----< demo requirement #3 >-----
bool testProj2::testR3() {
	Utilities::Title("#3: Provide packages including TestExecutive, RepositoryCore,	Check-in, Check-out, Version and Browse");
	Utilities::putline();
	if (isDirectory("../Checkin")) std::cout << "  Checkin package contained.\n";
	else return false;
	if (isDirectory("../Checkout")) std::cout << "  Checkout package contained.\n";
	else return false;
	if (isDirectory("../SoftwareRepoTB")) std::cout << "  RepositoryCore package contained.\n";
	else return false;
	if (isDirectory("../TestProj2")) std::cout << "  TestExecutive package contained.\n";
	else return false;
	if (isDirectory("../Browse")) std::cout << "  Browse package contained.\n";
	else return false;

	std::cout << "  versionSetter() in Checkin.cpp line 86-90 and removeVersion() in checkout.cpp line 30-40 \n" \
		<< "  handle all version capabilities, there is no need for giving version a seperate package.\n";
	return true;
}

// ----< demo requirement #4 >-----
bool testProj2::testR4() {
	Utilities::Title("#4: Demostrate check-in function - Checkin files, update version, close the checkin.");
	Utilities::putline();
	Core repoCore("../DemoCheckin/");
	Checkin CIWorker(repoCore);
	CIWorker.checkin("../NoSqlDb/DbCore/DbCore.h", "", "Header file of DbCore", "Project 1, DbCore", "DbCore");
	CIWorker.checkin("../NoSqlDb/DbCore/DbCore.cpp", "DbCore::DbCore.h.1", "Source file of DbCore", "Project 1, DbCore", "DbCore");
	
	std::cout << "\n  The records in NoSqlDb are as below.\n";
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();
	if (FileSystem::File("").exists("../DemoCheckin/DbCore_DbCore.h.1") == false || FileSystem::File("").exists("../DemoCheckin/DbCore_DbCore.cpp.1") == false)
		return false;

	CIWorker.checkin("../PseudoPackages/DbCore.cpp", "DbCore::DbCore.h.1", "Modified source file of DbCore", "Project 1, DbCore", "DbCore");

	std::cout << "\n  After update the DbCore.cpp, and change its description.\n";
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	CIWorker.checkin("$DbCore_DbCore.h", "$", "$", "$", "$", true);
	CIWorker.checkin("$DbCore_DbCore.cpp", "$", "$", "$", "$", true);
	std::cout << "\n  After close DbCore.h.1 and DbCore.cpp.1.\n";
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();
	if (FileSystem::File("").exists("../DemoCheckin/DbCore_DbCore.h.1") == false || FileSystem::File("").exists("../DemoCheckin/DbCore_DbCore.cpp.1") == false)
		return false;
	std::cout << "I alive!" << std::endl;
	CIWorker.checkin("../PseudoPackages/DbCore.h", "", "Version 2 DbCore header", "Project 1, DbCore", "DbCore");
	std::cout << "\n  After checkin a new version of DbCore.h.\n";
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();
	if (FileSystem::File("").exists("../DemoCheckin/DbCore_DbCore.h.2") == false)
		return false;

	return true;
}

// ----< demo requirement #5 >-----
bool testProj2::testR5() {
	Utilities::Title("#5: Demostrate check-in function - Checkin files, update version, close the checkin.");
	Utilities::putline();

	Core repoCore("../DemoCheckin/");

	std::cout << "Display content in DbCore.cpp.1" << std::endl;
	std::vector<NoSqlDb::DbElement<std::string>> result = \
		DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find("name", "DbCore::DbCore.cpp.1").eval();
	std::string demoFile = pathNSPFileNameVersionOf(result[0].payLoad());
	Browse().browse(demoFile);

	return true;
}

// ----< demo requirement #6 >-----
bool testProj2::testR6() {
	Utilities::Title("#6: Submit with several demo packages.");
	Utilities::putline();
	Core repoCore("../DemoCheckin");
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();
	std::cout << "All demo packages are saved in ../DemoCheckin/.\n";

	return true;
}

// ----< demo requirement #7 >-----
bool testProj2::testR7() {
	Utilities::Title("#7: TextExectuive clearly demonstrate project satisfied all requirements.");
	Utilities::putline();

	std::cout << "All above as well as this test function can be executed currently" << std::endl;
	return true;
}

// ----< casesRun: Load all requirement demos and provide a try-catch block to execute them >-----
void testProj2::casesRun(std::ostream& out) {
	cases.push_back(std::mem_fn(&testProj2::testR1));
	cases.push_back(std::mem_fn(&testProj2::testR2));
	cases.push_back(std::mem_fn(&testProj2::testR3));
	cases.push_back(std::mem_fn(&testProj2::testR4));
	cases.push_back(std::mem_fn(&testProj2::testR5));
	cases.push_back(std::mem_fn(&testProj2::testR6));
	cases.push_back(std::mem_fn(&testProj2::testR7));
	for (size_t i = 0; i < cases.size(); ++i) {
		auto now = cases[i];
		try {
			bool result = now(*this);
			check(result, out);
			out << " -- " << "Requirement " << (i + 1) << "\n";
		}
		catch (std::exception &ex) {
			check(false, out);
			out << " -- " << "Requirement " << (i + 1) << "\n";
			out << ex.what() << "\n";
		}
	}
}

// -----< check: Helper functions for casesRun, stole from TestExecutor >-----
void testProj2::check(bool result, std::ostream& out) {
	if (result)
		out << "\n  passed";
	else
		out << "\n  failed";
}

#ifdef TEST_TESTPROJ2
int main() {
	testProj2 tester;
	tester.casesRun();
	return 0;
}
#endif