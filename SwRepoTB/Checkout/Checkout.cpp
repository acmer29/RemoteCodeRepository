#include "Checkout.h"
#include "../NoSqlDb/Test/Test.h"
using namespace SWRTB;

Checkout::Checkout(Core& target, const std::string& targetDirectory) : 
	repo(target), 
	querier(target.core()), sourceDirectory(target.root()), 
	targetDirectory(targetDirectory) {
	if (dirHelper.exists(sourceDirectory) == false) throw std::exception("Check-out: Cannot find the given source directory.\n");
	if (dirHelper.exists(targetDirectory) == false) dirHelper.create(targetDirectory);
}

void Checkout::checkout(const std::string& NSPfileNameVersion) {
	if (canCheckout(NSPfileNameVersion) == false)
		throw std::exception("Check-out: The file cannot be checken out.\n");
	std::string NSPfileName = removeVersion(NSPfileNameVersion);
	std::cout << NSPfileName << std::endl;
	copyFile(sourceDirectory + NSPfileNameVersion, targetDirectory + NSPfileName);
	return;
}

bool Checkout::canCheckout(const std::string& NSPfileNameVersion) {
	if (NSPfileNameVersion.length() == 0) throw std::exception("Check-out: No file name given.\n");
	std::cout << sourceDirectory + NSPfileNameVersion << std::endl;
	if (querier.from(repo.core()).find("payLoad", sourceDirectory + NSPfileNameVersion + "$closed").eval().size() != 1)
		return false;
	return true;
}

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

int main() {
	DbTest::test tester;
	tester.registerTest(test1, "Test1: Check-out single file.");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif
