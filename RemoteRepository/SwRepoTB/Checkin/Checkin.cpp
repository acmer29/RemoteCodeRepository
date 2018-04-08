/////////////////////////////////////////////////////////////////////
// Chekcin.cpp - Implementation and test stub of checkin package   //
// ver 1.1                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
#include "Checkin.h"
#include "../NoSqlDb/Test/Test.h"

using namespace SWRTB;

// -----< Constructor: Initialize the working directory of checkin >-----
Checkin::Checkin(Core& target) :
	repo(target), querier(target.core()), workDirectory(target.root()) {}

void Checkin::checkin(const std::string& path, const std::string& dependency, \
					  const std::string& description, const std::string& category, \
					  const std::string& nameSpace, const std::string& owner, bool close) {
	selectFile(path).setNameSpace(nameSpace).setCategory(category).setDependence(dependency).setDescription(description).setOwner(owner).checkin(close);
	return;
}

// -----< checkin: Provide for cascade call, only use at the end of a cascade calling >-----
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

// -----< selectFile: Provide for cascade calling, select one file or files in a folder >-----
Checkin& Checkin::selectFile(const std::string& path) {
	if (filesForCheckin.size() != 0) filesForCheckin.clear();
	if (path == "") throw std::exception("Check-in: Please enter file spec.\n");
	else if (path[0] == '$') localPathSolver(path.substr(1, path.length()));
	else pathSolver(path);
	return *this;
}

// -----< setNameSpace: Provide for cascade calling, set the namespace for checkin files. >-----
Checkin& Checkin::setNameSpace(const std::string& nameSpace) {
	nameSpace_ = nameSpace;
	return *this;
}

// -----< setDescription: Provide for cascade calling, set the description for checkin files. >-----
Checkin& Checkin::setDescription(const std::string& description) {
	description_ = description;
	return *this;
}

// -----< setDependence: Provide for cascade calling, set the dependencies for checkin files. >-----
Checkin& Checkin::setDependence(const std::string& dependencies) {
	dependencies_ = dependencies;
	return *this;
}

// -----< setCategory: Provide for cascade calling, set the category for checkin files. >-----
Checkin& Checkin::setCategory(const std::string& categories) {
	categories_ = categories;
	return *this;
}

Checkin& Checkin::setOwner(const std::string& owner) {
	if (owner == "") owner_ = "Anonymous";
	else owner_ = owner;
	return *this;
}

// -----< pathSolver: Solve the path indicated, fills the filesForCheckin vector >-----
void Checkin::pathSolver(const std::string& pathFileName) {
	if (isFile(pathFileName)) {
		filesForCheckin.push_back(pathFileName);
	}
	else if (isDirectory(pathFileName)) {
		filesForCheckin = dirHelper.getFiles(pathFileName);
		for (size_t i = 0; i < filesForCheckin.size(); ++i) {
			filesForCheckin[i] = pathFileName + filesForCheckin[i];
		}
	}
	else {
		throw std::exception("Check-in: Invalid path given.\n");
	}
	if (filesForCheckin.size() == 0) throw std::exception("Check-in: Cannot checkin no file.\n");
	return;
}

// -----< localPathSolver: Solve the path appears to be the working directory >-----
void Checkin::localPathSolver(const std::string& fileName) {
	querier.from(repo.core()).find("payLoad", 
		"/" + Utilities::regexSafeFilter(workDirectory) + Utilities::regexSafeFilter(fileName) + "\\.[0-9]*/").find("status", "open");
	if (querier.eval().size() != 1) throw std::exception("Check-in: Cannot locate local file by given fileName.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	filesForCheckin.push_back(fileCplx.payLoad());
	return;
}

// -----< versionSetter: Determine the version number of a file >-----
int Checkin::versionSetter(const std::string& fileName) {
	std::string value = "/" + Utilities::regexSafeFilter(workDirectory) + Utilities::regexSafeFilter(nameConcater(fileName, nameSpace_, "_")) + "\\.[0-9]*/";
	querier.from(repo.core()).find("payLoad", value).find("status", "/clos(ed|ing)/");
	return querier.eval().size() + 1;
}

// -----< canClose: Check if a file checkin can be closed > --------
// -----< Assume the file record has already exists in the db >-----
bool Checkin::canClose(const std::string& key) {
	if (filesForCheckin.size() == 0) throw std::exception("Check-in: No file for closing.\n");
	std::vector<NoSqlDb::DbElement<std::string>> dependencies = querier.from(repo.core()).find("name", key).childOf(true).eval();
	for (auto item : dependencies) {
		if (item.name() == key) {
			querier.from(repo.core()).find("name", item.name()).update("status", "closed");
			continue;
		}
		if (item.status() == "closed") continue;
		else if (item.status() == "closing") {
			querier.from(repo.core()).find("name", item.name()).update("status", "closed");
		}
		else if (item.status() == "open") return false;
		else {
			// std::cout << item.payLoad() << std::endl;
			throw std::exception("Check-in: Detected bad mode in stored file.\n");
		}
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
	std::string recordPayLoadValue = workDirectory + nameConcater(fileNameAct, nameSpace_, "_");
	std::string status = "open";
	std::string record = \
		"\"" + recordName + "\"" + ", " + \
		"\"" + description_ + "\"" + ", " + \
		"\"" + dependencies_ + "\"" + ", " + \
		"\"" + recordPayLoadValue + "\"" + ", " + \
		"\"" + categories_ + "\"" + ", " + \
		"\"" + status + "\"" + ", " + \
		"\"" + owner_ + "\"" + ", " + \
		"\"" + nameSpace_ + "\"";
	std::cout << "The record is: " << record << std::endl;
	querier.from(repo.core()).insert(record);
	if (querier.from(repo.core()).find("name", recordName).eval().size() != 1)
		throw std::exception("Check-in: Check-in fails because of invalid parameter.\n");
	copyFile(pathFileName, recordPayLoadValue);
	// std::cout << "File: \"" << fileNameAct << "\" inserted into the database. Checkin type: New.\n";
	return;
}

// -----< resumeCheckin: Checkin a package which has an open version. >-----
// -----< This is an OPEN check-in >----------------------------------------
void Checkin::resumeCheckin(const std::string& pathFileName) {
	std::string fileName = pathHelper.getName(pathFileName);
	if (pathFileName.substr(0, workDirectory.length()) != workDirectory) fileName = nameConcater(fileName, nameSpace_, "_");
	else fileName = fileName.substr(0, fileName.find_last_of('.'));
	if (querier.from(repo.core()).find("payLoad", "/" + Utilities::regexSafeFilter(workDirectory) + Utilities::regexSafeFilter(fileName) + "\\.[0-9]*/").find("status", "open").eval().size() != 1 && \
		querier.from(repo.core()).find("payLoad", pathFileName).find("status", "open").eval().size() != 1)
		throw std::exception("This file has no open version.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	if (canTouch(fileCplx.owner(), owner_) == false) 
		throw std::exception("Checkin: This file is not owned by you!\n");
	if (dependencies_ != "$") querier.update("children", dependencies_);
	if (description_ != "$") querier.update("description", description_);
	if (categories_ != "$") querier.update("category", categories_);
	return;
}

// -----< closeCheckin: Close a checkin file, change its mode to either closed or closing >-----
void Checkin::closeCheckin(const std::string& pathFileName) {
	std::string fileName = pathHelper.getName(pathFileName);
	if (pathFileName.substr(0, workDirectory.length()) != workDirectory) fileName = nameConcater(fileName, nameSpace_, "_");
	else fileName = fileName.substr(0, fileName.find_last_of('.'));
	if (querier.from(repo.core()).find("payLoad", "/" + Utilities::regexSafeFilter(workDirectory) + Utilities::regexSafeFilter(fileName) + "\\.[0-9]*/").find("status", "open").eval().size() != 1)
		throw std::exception("Check-in: No correct file for close checkin.\n");
	NoSqlDb::DbElement<std::string> fileCplx = querier.eval()[0];
	if (canTouch(fileCplx.owner(), owner_) == false) 
		throw std::exception("Checkin: This file is not owned by you!\n");
	std::string key = fileCplx.name();
	if (canClose(key) == false) {
		std::vector<NoSqlDb::DbElement<std::string>> suspects = 
			querier.from(repo.core()).find("payLoad", "/" + Utilities::regexSafeFilter(workDirectory) + ".*\\.[0-9]*/").find("status", "open").eval();
		if (LoopHandler(suspects).isInLoop(fileCplx.name())) {
			fileCplx.status("closing");
			querier.from(repo.core()).update(fileCplx);
			return;
		}
		else {
			std::cout << "File: \"" << fileName << "\" does not meet the requirement of close check-in, operation skipped.\n";
			return;
		}
	}
	fileCplx.status("closed");
	querier.from(repo.core()).update(fileCplx);
	return;
}

// -----< isNew: Check if a file for checkin should be a new checkin or not >-----
bool Checkin::isNew(const std::string& pathFileName) {
	std::string fileName = pathHelper.getName(pathFileName);
	if (pathFileName.substr(0, workDirectory.length()) != workDirectory) fileName = nameConcater(fileName, nameSpace_, "_");
	else fileName = fileName.substr(0, fileName.find_last_of('.'));
	return !(querier.from(repo.core()).find("payLoad", "/" + Utilities::regexSafeFilter(workDirectory) + Utilities::regexSafeFilter(fileName) + "\\.[0-9]*/").find("status", "open").eval().size());
}

// -----< saveRepo: Persist the current DB into XML file >-----
void Checkin::saveRepo() {
	persistor.persist(querier.from(repo.core()).eval(), workDirectory + "HeartOfRepo");
}

// -----< cleanUp: Reset all private data members related to current checkin operation >-----
void Checkin::cleanUp() {
	nameSpace_ = "";
	dependencies_ = "";
	description_ = "";
	categories_ = "";
	owner_ = "";
	filesForCheckin.clear();
}

#ifdef TEST_CHECKIN

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
	worker.selectFile("D:/Spring2018/cse687/SwRepoTB/PseudoPackages/").setDependence("::test.txt.1").setDescription("some packages").setCategory("has header, has source").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 4" << std::endl;
	worker.selectFile("$_test.txt").setDescription("$").setCategory("$").setDependence("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 5" << std::endl;
	worker.selectFile("$_DbCore.cpp").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 6" << std::endl;
	worker.selectFile("$_DbCore.h").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	std::cout << "Experiment 7" << std::endl;
	worker.selectFile("D:/Spring2018/cse687/SwRepoTB/PseudoPackages/DbCore.cpp").setDependence("::test.txt.1").setDescription("some packages").setCategory("has source").checkin(false);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();

	/*worker.selectFile("$SurfacePro_BMR_15_2.177.0.zip").setDependence("$").setDescription("$").setCategory("$").checkin(true);
	DbQuery::queryResult<std::string>(repoCore.core()).from(repoCore.core()).find().resultDisplay();*/

	return false;
}

// -----< test stub >-----
int main() {
	DbTest::test tester;
	tester.registerTest(test1, "Test1: Checkin single file");
	tester.testsRun();
	tester.testsSummary();
	return 0;
}
#endif // TEST_CHECKIN
