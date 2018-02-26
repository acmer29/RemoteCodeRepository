#include "Checkout.h"
#include "../NoSqlDb/Test/Test.h"
using namespace SWRTB;

Checkout::Checkout(Core& target, const std::string& targetDirectory) : 
	repo(target), 
	querier(target.core()), sourceDirectory(target.root() + "closed/"), 
	targetDirectory(targetDirectory) {
	if (dirHelper.exists(sourceDirectory) == false) throw std::exception("Check-out: Cannot find the given source directory.\n");
	if (dirHelper.exists(targetDirectory) == false) dirHelper.create(targetDirectory);
}

Checkout& Checkout::restore(const std::string& pathFileName) {
	std::string heart;
	if (pathFileName == "") heart = repo.root() + "HeartOfRepo.xml";
	else heart = pathFileName;
	FileSystem::File heartOfRepo(heart);
	if (heartOfRepo.exists(heart) == false) throw std::exception("I have lost my heart.\n");
	std::vector<NoSqlDb::DbElement<std::string>> result = persistor.restore(heart);
	for (auto item : result) querier.from(repo.core()).insert(item);
	querier.from(repo.core()).resultDisplay();
	return *this;
}

void Checkout::checkout(const std::string& fileNameVersion) {
	
	if (canCheckout(fileNameVersion) == false)
		throw std::exception("Check-out: The file cannot be checken out.\n");
	std::string fileName = removeVersion(fileNameVersion);
	std::cout << fileName << std::endl;
	copyFile(sourceDirectory + fileNameVersion, targetDirectory + fileName);
	return;
}

bool Checkout::canCheckout(const std::string& fileNameVersion) {
	if (fileNameVersion.length() == 0) throw std::exception("Check-out: No file name given.\n");
	if (querier.from(repo.core()).find("payLoad", sourceDirectory + fileNameVersion).eval().size() != 1)
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

// -----< copyFile: copy "path/to/source/file.ext" to "path/to/target/file.ext" >-----
void Checkout::copyFile(const std::string& fromPath, const std::string& toPath) {
	if (fromPath == toPath) return;
	FileSystem::File me(fromPath);
	me.open(FileSystem::File::in, FileSystem::File::binary);
	if (!me.isGood())
		throw std::exception("Check-in: Bad state of accepted file.\n");
	FileSystem::File you(toPath);
	you.open(FileSystem::File::out, FileSystem::File::binary);
	if (you.isGood()) {
		while (me.isGood()) {
			FileSystem::Block filePiece = me.getBlock(1024);
			you.putBlock(filePiece);
		}
		/*if (FileSystem::FileInfo(fromPath).size() != FileSystem::FileInfo(toPath).size()) {
			std::cout << FileSystem::FileInfo(fromPath).size() << " " << FileSystem::FileInfo(toPath).size() << std::endl;
			std::cout << toPath << std::endl;
			you.remove(toPath);
			throw std::exception("Check-in: Copy error.\n");
		}*/
		std::cout << "File \"" << fromPath << "\" has been copied as \"" << toPath << "\" \n";
	}
	else throw std::exception("Check-in: Bad state of target file.\n");
}

#ifdef TEST_CHECKOUT

bool test1() {
	Utilities::title("Test1: Check-out single file.");
	Utilities::putline();
	Core repoCore("D:/test/");
	Checkout worker(repoCore);
	worker.restore().checkout("test.txt.1");
	worker.checkout("test.cpp.2");
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
