/////////////////////////////////////////////////////////////////////
// Checkout.cpp - Implementation and test stub of Checkout package //
// ver 1.1                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include "Checkout.h"
#include "../NoSqlDb/Test/Test.h"
using namespace SWRTB;

// -----< Constructor: Initialize the working directory of the checkout >-----
Checkout::Checkout(Core& target, const std::string& targetDirectory) : 
	repo(target), 
	querier(target.core()), sourceDirectory(target.root()), 
	targetDirectory(targetDirectory) {
	if (dirHelper.exists(sourceDirectory) == false) throw std::exception("Check-out: Cannot find the given source directory.\n");
	if (dirHelper.exists(targetDirectory) == false) dirHelper.create(targetDirectory);
}

// -----< relocateDirectory: Reset the checkout working directory >-----
Checkout& Checkout::relocateDirectory(const std::string& newDirectory) {
	if (isDirectory(newDirectory) == false) throw std::exception("Check-out: New directory is not valid.\n");
	targetDirectory = newDirectory;
	return *this;
}

Checkout& Checkout::setRequestor(const std::string& requestor) {
	requestor_ = requestor;
	return *this;
}

void Checkout::checkout(const std::string& NSNFileNameVersion, const std::string& requestor, bool recursive) {
	setRequestor(requestor).checkout(NSNFileNameVersion, recursive);
	return;
}

// -----< checkout: Provide means to checkout files >---------------------------
// -----< By default checkout all files as well as dependencies >----------------
void Checkout::checkout(const std::string& NSNFileNameVersion, bool recursive) {
	std::vector<NoSqlDb::DbElement<std::string>> result = 
		querier.from(repo.core()).find("name", NSNFileNameVersion).eval();
	if (canTouch(result[0].owner(), requestor_) == false) throw std::exception("Checkout: This file is not owned by you!\n");
	if (recursive == true) {
		std::vector<NoSqlDb::DbElement<std::string>> tmp = 
			querier.from(repo.core()).find("name", NSNFileNameVersion).childOf().eval();
		for (auto iter = tmp.begin(); iter != tmp.end(); iter++) {
			result.push_back(*iter);
		}
	}
	for (auto item : result) { 
		if (isFile(item.payLoad()) == false)
			throw std::exception("Check-out: The file cannot be checken out.\n");
		if (canTouch(item.owner(), requestor_) == false) {
			std::cout << "Check out " << item.name() << " failed because this file is not owned by you.\n";
			failCheckoutFiles.push_back(removeVersion(NSNFileNameToNSPFileName(item.name())));
			continue;
		}
		std::string target = NSNFileNameToNSPFileName(item.name());
		if (item.status() == "open")
			std::cout << "  Check-out: The file \"" + item.name() + "\" is currently in \"open\" status.\n";
		std::string NSPfileName = removeVersion(target);
		successCheckoutFiles.push_back(target);
		copyFile(sourceDirectory + target, targetDirectory + NSPfileName);
	}
	return;
}

std::vector<std::string> Checkout::successCheckouts() {
	return successCheckoutFiles;
}

std::vector<std::string> Checkout::failCheckouts() {
	return failCheckoutFiles;
}

// -----< removeVersion: Remove the version number in the fileName >-----
std::string Checkout::removeVersion(const std::string& fileNameVersion) {
	std::string fileName = fileNameVersion;
	size_t last = fileName.length() - 1;
	while (last != 0) {
		if ('0' <= fileName[last] && fileName[last] <= '9') last -= 1;
		else if (fileName[last] == '.') break;
		else throw std::exception("Check-out: Either given file name or repo stored file name violates the naming rule.\n");
	}
	fileName = fileName.substr(0, last);
	return fileName;
}

#ifdef TEST_CHECKOUT

bool test1() {
	Utilities::title("Test1: Check-out single file.");
	Utilities::putline();
	Core repoCore("D:/test/");
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();
	Checkout worker(repoCore);
	worker.checkout("_test.txt.1");
	worker.checkout("_test.cpp.2");
	return false;
}

// -----< test stub >-----
int main() {
	DbTest::test tester;
	tester.registerTest(test1, "Test1: Check-out single file.");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif
