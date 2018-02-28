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

void Checkin::checkin(const std::string& path, const std::string& dependency, \
					  const std::string& description, const std::string& category, \
					  const std::string& nameSpace, bool close) {
	selectFile(path).setNameSpace(nameSpace).setCategory(category).setDependence(dependency).setDescription(description).checkin(close);
}

void Checkin::checkin(bool close) {
	if (filesForCheckin.size() == 0) throw std::exception("Check-in: No files for checkin.\n");
	for (auto item : filesForCheckin) {
		if (isNew(item) == true) 
			newCheckin(item);
		else 
			resumeCheckin(item);
		if (close == true) {
			closeCheckin(item);
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
	std::cout << "local file: " << fileName << std::endl;
	querier.from(repo.core()).find("payLoad", 
		"/" + Utilities::regexSafeFilter(openDirectory) + Utilities::regexSafeFilter(fileName) + "\\.[0-9]*/");
	if (querier.eval().size() != 1) throw std::exception("Check-in: Cannot locate local file by given fileName.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	filesForCheckin.push_back(fileCplx.payLoad());
}

int Checkin::versionSetter(const std::string& fileName) {
	std::string value = "/" + Utilities::regexSafeFilter(closedDirectory) + Utilities::regexSafeFilter(fileName) + "\\.[0-9]*/";
	querier.from(repo.core()).find("payLoad", value);
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

	// Get the file name from pathFileName and add version number on it the create fileNameAct
	std::string fileNameAct = pathHelper.getName(pathFileName) + "." + std::to_string(versionSetter(pathHelper.getName(pathFileName)));
	
	std::string recordName = nameConcater(fileNameAct, nameSpace_, "::");
	std::string recordPayLoadValue = openDirectory + nameConcater(fileNameAct, nameSpace_, "_");
	std::string record = \
		"\"" + recordName + "\"" + ", " + \
		"\"" + description_ + "\"" + ", " + \
		"\"" + dependencies_ + "\"" + ", " + \
		"\"" + recordPayLoadValue + "\"" + ", " + \
		"\"" + categories_ + "\"";
	std::cout << "The record is: " << record << std::endl;
	querier.from(repo.core()).insert(record);
	if (querier.from(repo.core()).find("name", recordName).eval().size() != 1)
		throw std::exception("Check-in: Check-in fails because of invalid parameter.\n");
	copyFile(pathFileName, recordPayLoadValue);
	std::cout << "File: \"" << fileNameAct << "\" inserted into the database. Checkin type: New.\n";
	return;
}

// -----< resumeCheckin: Checkin a package which has an open version. >-----
// -----< This is an OPEN check-in >----------------------------------------
void Checkin::resumeCheckin(const std::string& pathFileName) {
	std::string fileNameAct = nameConcater(pathHelper.getName(pathFileName), nameSpace_, "_");
	if (querier.from(repo.core()).find("payLoad", "/" + Utilities::regexSafeFilter(openDirectory) + Utilities::regexSafeFilter(fileNameAct) + "\\.[0-9]*/").eval().size() != 1 && \
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

void Checkin::closeCheckin(const std::string& pathFileName) {
	std::string fileName = pathHelper.getName(pathFileName);
	if (pathFileName.substr(0, openDirectory.length()) != openDirectory) fileName = nameConcater(fileName, nameSpace_, "_");
	std::cout << "/" + Utilities::regexSafeFilter(openDirectory) + Utilities::regexSafeFilter(fileName) + "\\.*/" << std::endl;
	if (querier.from(repo.core()).find("payLoad", "/" + Utilities::regexSafeFilter(openDirectory) + Utilities::regexSafeFilter(fileName) + "\\.*/").eval().size() != 1)
		throw std::exception("Check-in: No correct file for close checkin.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	std::string key = fileCplx.name();
	if (canClose(key) == false) {
		std::cout << "File: \"" << fileName << "\" does not meet the requirement of close check-in, operation skipped.\n";
		return;
	}
	std::string closedPathNSPFileNameVersion = closedDirectory + pathHelper.getName(fileCplx.payLoad());
	copyFile(fileCplx.payLoad(), closedPathNSPFileNameVersion);
	FileSystem::File(fileCplx.payLoad()).remove(fileCplx.payLoad());
	fileCplx.payLoad(closedPathNSPFileNameVersion);
	querier.from(repo.core()).update(fileCplx);
	return;
}

bool Checkin::isNew(const std::string& pathFileName) {
	std::string fileName = pathHelper.getName(pathFileName);
	if (pathFileName.substr(0, openDirectory.length()) != openDirectory) fileName = nameConcater(fileName, nameSpace_, "_");
	return !(querier.from(repo.core()).find("payLoad", "/" + Utilities::regexSafeFilter(openDirectory) + Utilities::regexSafeFilter(fileName) + "\\.*/").eval().size());
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

	std::cout << "Experiment 1" << std::endl;
	worker.selectFile("../test.txt").setDependence("").setCategory("").setDescription("this is something").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();
	
	std::cout << "Experiment 2" << std::endl;
	worker.selectFile("$_test.txt").setDependence("").setCategory("test, optional").setDescription("change it").checkin();
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 3" << std::endl;
	worker.selectFile("D:/Spring2018/cse687/SwRepoTB/somepackage/").setDependence("test.txt.1").setDescription("some packages").setCategory("has header, has source").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 4" << std::endl;
	worker.selectFile("$_test.txt").setDescription("$").setCategory("$").setDependence("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 5" << std::endl;
	worker.selectFile("$_test.cpp").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 6" << std::endl;
	worker.selectFile("$_test.h").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 7" << std::endl;
	worker.selectFile("D:/Spring2018/cse687/SwRepoTB/somepackage/test.cpp").setDependence("test.txt.1").setDescription("some packages").setCategory("has source").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	/*worker.selectFile("$SurfacePro_BMR_15_2.177.0.zip").setDependence("$").setDescription("$").setCategory("$").checkin(true);
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
