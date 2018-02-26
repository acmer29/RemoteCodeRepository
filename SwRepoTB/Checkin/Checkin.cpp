#include "Checkin.h"
#include "../NoSqlDb/Test/Test.h"

using namespace SWRTB;

Checkin::Checkin(Core& target) :
	repo(target), querier(target.core()), workDirectory(target.root()) {
	openDirectory = workDirectory + "open/";
	if (dirHelper.exists(openDirectory) == false) dirHelper.create(openDirectory);
	closedDirectory = workDirectory + "closed/";
	if (dirHelper.exists(closedDirectory) == false) dirHelper.create(closedDirectory);
	structurePathFileName = workDirectory + "HeartOfRepo.xml";
}

void Checkin::checkin(bool close) {
	if (filesForCheckin.size() == 0) throw std::exception("Check-in: No files for checkin.\n");
	for (auto item : filesForCheckin) {
		if (isNew(pathHelper.getName(item)) == true) {
			newCheckin(item);
		}
		else {
			resumeCheckin(item);
		}
		if (close == true) {
			closeCheckin(pathHelper.getName(item));
		}
	}
	saveRepo();
	cleanUp();
	return;
}

Checkin& Checkin::selectFile(const std::string& path) {
	if (filesForCheckin.size() != 0) filesForCheckin.clear();
	if (path == "") throw std::exception("Check-in: Please enter file spec.\n");
	else if (path[0] == '$') localPathSolver(path.substr(1, path.length()));
	else pathSolver(path);
	return *this;
}

Checkin& Checkin::setNameSpace(const std::string& nameSpace) {
	nameSpace_ = nameSpace;
	return *this;
}

Checkin& Checkin::setDescription(const std::string& description) {
	description_ = description;
	return *this;
}

Checkin& Checkin::setDependence(const std::string& dependencies) {
	dependencies_ = dependencies;
	return *this;
}

Checkin& Checkin::setCategory(const std::string& categories) {
	categories_ = categories;
	return *this;
}

void Checkin::pathSolver(const std::string& path) {
	std::vector<std::string> fileNames;
	std::string pathAct = path;
	if (isFile(path)) {
		fileNames.push_back(pathHelper.getName(path));
		pathAct = pathHelper.getPath(path);
	}
	else if (isDirectory(path)) {
		fileNames = dirHelper.getFiles(path);
	}
	else throw std::exception("Check-in: Invalid path given.\n");

	if (fileNames.size() == 0) throw std::exception("Check-in: Cannot checkin no file.\n");
	for (auto item : fileNames) {
		int currentVersion = versionSetter(item);
		filesForCheckin.push_back(pathHelper.toLower(pathAct + item));
	}
	return;
}

void Checkin::localPathSolver(const std::string& fileName) {
	querier.from(repo.core()).find("payLoad", "/" + openDirectory + fileName + "\\.[0-9]*/");
	if (querier.eval().size() != 1) throw std::exception("Check-in: Cannot locate local file by given fileName.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	filesForCheckin.push_back(fileCplx.payLoad());
}

int Checkin::versionSetter(const std::string& fileName) {
	std::string value = "/" + closedDirectory + fileName + "\\.[0-9]*/";
	querier.from(repo.core()).find("payLoad", value);
	std::cout << "Has version " << querier.eval().size() << std::endl;
	return querier.eval().size() + 1;
}

// -----< canClose: Check if a file checkin can be closed > --------
// -----< Assume the file record has already exists in the db >-----
bool Checkin::canClose(const std::string& key) {
	if (filesForCheckin.size() == 0) throw std::exception("Check-in: No file for closing.\n");
	std::vector<NoSqlDb::DbElement<std::string>> dependencies = querier.from(repo.core()).find("name", key).childOf(true).eval();
	for (auto item : dependencies) {
		if (repo.isClosed(item.name()) == false) return false;
	}
	return true;
}

// -----< newCheckin: Check-in a whole new package or a new version of package of an existing one >-----
// -----< This is an OPEN check-in >--------------------------------------------------------------------
void Checkin::newCheckin(const std::string& pathFileName) {
	if (pathFileName == "$" || description_ == "$" || dependencies_ == "$" || categories_ == "$")
		throw std::exception("Check-in: New Checkin parameter cannot be $.\n");
	std::string fileNameAct = pathHelper.getName(pathFileName) + "." + std::to_string(versionSetter(pathHelper.getName(pathFileName)));
	std::string record = \
		"\"" + nameSpace_ + "::" + fileNameAct + "\"" + ", " + \
		"\"" + description_ + "\"" + ", " + \
		"\"" + dependencies_ + "\"" + ", " + \
		"\"" + openDirectory + fileNameAct + "\"" + ", " + \
		"\"" + categories_ + "\"";
	std::cout << "The record is: " << record << std::endl;
	querier.from(repo.core()).insert(record);
	if (querier.from(repo.core()).find("name", nameSpace_ + "::" + fileNameAct).eval().size() != 1)
		throw std::exception("Check-in: Check-in fails because of invalid parameter.\n");
	copyFile(pathFileName, openDirectory + fileNameAct);
	std::cout << "File: \"" << fileNameAct << "\" inserted into the database. Checkin type: New.\n";
	return;
}

// -----< resumeCheckin: Checkin a package which has an open version. >-----
// -----< This is an OPEN check-in >----------------------------------------
void Checkin::resumeCheckin(const std::string& pathFileName) {
	std::string fileName = pathHelper.getName(pathFileName);
	if (querier.from(repo.core()).find("payLoad", "/" + openDirectory + fileName + "\\.[0-9]*/").eval().size() != 1 && \
		querier.from(repo.core()).find("payLoad", pathFileName).eval().size() != 1)
		throw std::exception("This file has no open version.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	std::string key = fileCplx.name();
	if (dependencies_ != "$") querier.update("children", dependencies_);
	if (description_ != "$") querier.update("description", description_);
	if (categories_ != "$") querier.update("category", categories_);
	copyFile(pathFileName, fileCplx.payLoad());
	return;
}

void Checkin::closeCheckin(const std::string& fileName) {
	std::cout << "/" + openDirectory + fileName + "\\.*/" << std::endl;
	if (querier.from(repo.core()).find("payLoad", "/" + openDirectory + fileName + ".*/").eval().size() != 1)
		throw std::exception("Check-in: No correct file for close checkin.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	std::string key = fileCplx.name();
	if (canClose(key) == false) {
		std::cout << "File: \"" << fileName << "\" does not meet the requirement of close check-in, operation skipped.\n";
		return;
	}
	copyFile(fileCplx.payLoad(), closedDirectory + nameCleaner(key));
	FileSystem::File(fileCplx.payLoad()).remove(fileCplx.payLoad());
	fileCplx.payLoad(closedDirectory + nameCleaner(key));
	querier.from(repo.core()).update(fileCplx);
	return;
}

bool Checkin::isNew(const std::string& fileName) {
	return !(querier.from(repo.core()).find("payLoad", "/" + openDirectory + fileName + "\\.*/").eval().size());
}

std::string Checkin::nameCleaner(const std::string& NSFileName) {
	size_t start = 0, end = NSFileName.length() - 1;
	while (start != end) {
		if (NSFileName[start] == ':' && NSFileName[start + 1] != ':') {
			start += 1;
			break;
		}
		else start += 1;
	}
	return NSFileName.substr(start, NSFileName.length());
}

// -----< copyFile: copy "path/to/source/file.ext" to "path/to/target/file.ext" >-----
void Checkin::copyFile(const std::string& fromPath, const std::string& toPath) {
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
		}
		std::cout << "File \"" << fromPath << "\" has been copied as \"" << toPath << "\" \n";*/
	}
	else throw std::exception("Check-in: Bad state of target file.\n");
}

void Checkin::saveRepo() {
	persistor.persist(querier.from(repo.core()).eval(), workDirectory + "HeartOfRepo");
}

void Checkin::cleanUp() {
	nameSpace_ = "";
	dependencies_ = "";
	description_ = "";
	categories_ = "";
	filesForCheckin.clear();
}

bool Checkin::isFile(const std::string& path) {
	DWORD dwAttrib = GetFileAttributesA(path.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0);
}

bool Checkin::isDirectory(const std::string& path) {
	DWORD dwAttrib = GetFileAttributesA(path.c_str());
	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

bool test1() {
	Utilities::title("Test1: Checkin single file");
	Utilities::putline();
	Core repoCore("D:/test/");
	Checkin worker(repoCore);

	worker.selectFile("../test.txt").setDependence("").setCategory("").setDescription("this is something").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();
	
	worker.selectFile("$test.txt").setDependence("").setCategory("test, optional").setDescription("change it").checkin();
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	worker.selectFile("D:/Spring2018/cse687/SwRepoTB/somepackage/").setDependence("test.txt.1").setDescription("some packages").setCategory("has header, has source").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	worker.selectFile("$test.txt").setDescription("$").setCategory("$").setDependence("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	worker.selectFile("$test.cpp").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	worker.selectFile("$test.h").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	worker.selectFile("D:/Spring2018/cse687/SwRepoTB/somepackage/test.cpp").setDependence("test.txt.1").setDescription("some packages").setCategory("has source").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	/*worker.selectFile("D:/tools/SurfacePro_BMR_15_2.177.0.zip").setDependence("test.txt.1").setDescription("some packages").setCategory("has source").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	worker.selectFile("$SurfacePro_BMR_15_2.177.0.zip").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();*/

	return false;
}

bool test2() {
	Utilities::title("Test2: Checkin multiple files with dependencies");
	Utilities::putline();
	Core repoCore("D:/test/");
	return false;
}

#ifdef TEST_CHECKIN
int main() {
	DbTest::test tester;
	tester.registerTest(test1, "Test1: Checkin single file");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif // TEST_CHECKIN
