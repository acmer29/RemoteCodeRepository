/////////////////////////////////////////////////////////////////////////
// RemoteRepository.cpp - Console App that processes incoming messages //
// ver 1.0                                                             //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018             //
/////////////////////////////////////////////////////////////////////////

#include "RemoteRepository.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <chrono>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;


// -----< vectorToString: Helper function converting vector to string >-----
std::string vectorToString(const std::vector<std::string>& toConvert) {
	std::string result = "";
	for (auto item : toConvert) {
		result = result + item + "$";
	}
	if (result != "") result = result.substr(0, result.length() - 1);
	return result;
}

// -----< getFiles: get files from path >-----
Files Server::getFiles(const Repository::SearchPath& path)
{
	return Directory::getFiles(path);
}

// -----< getDirs: get directories from path  >-----
Dirs Server::getDirs(const Repository::SearchPath& path)
{
	return Directory::getDirectories(path);
}

// -----< getFilesPlus: Modified version of getFiles  >-----
std::string Server::getFilesPlus(const Repository::SearchPath& searchPath) {
	Files files = getFiles(searchPath);
	std::string fileComplex = "";
	for (auto item : files) {
		fileComplex += (item + "$");
	}
	if (fileComplex != "") fileComplex = fileComplex.substr(0, fileComplex.length() - 1);
	return fileComplex;
}

// -----< getDirsPlus: Modified version of getDirs  >-----
std::string Server::getDirsPlus(const Repository::SearchPath& searchPath) {
	Dirs dirs = getDirs(searchPath);
	std::string dirComplex = "";
	for (auto item : dirs) {
		if (item != "." && item != "..")
			dirComplex += (item + "$");
	}
	if (dirComplex != "") dirComplex = dirComplex.substr(0, dirComplex.length() - 1);
	return dirComplex;
}

// -----< fileInfoAssembler: Assemble the file info  >-----
std::vector<std::string> Server::fileInfoAssembler(const std::string& NSNFileName) {
	SWRTB::Core repo(Repository::repoHeartPath);
	std::vector<std::string> ans;
	NoSqlDb::DbQuery<std::string> querier(repo.core());
	std::vector<NoSqlDb::DbElement<std::string>> result =
		querier.from(repo.core()).find("name", NSNFileName).eval();
	if (result.size() == 0) return ans;
	else {
		ans.push_back(result[0].nameSpace());
		ans.push_back(SWRTB::nameOf(result[0].name(), result[0].nameSpace()));
		ans.push_back(SWRTB::versionOf(result[0].name()));
		ans.push_back(result[0].descrip());
		ans.push_back(std::string(result[0].dateTime()));
		ans.push_back(result[0].status());
		ans.push_back(result[0].owner());
		ans.push_back(vectorToString(result[0].children()));
		ans.push_back(vectorToString(result[0].category()));
		return ans;
	}
}

// -----< showFileMessge: Assemble the showFile message >-----
Msg Server::showFileMessge(const EndPoint& from, const EndPoint& to, const std::string& fileName) {
	Msg reply(to, from);
	reply.command("showFileCallback");
	std::vector<std::string> fileInfo = fileInfoAssembler(fileName);
	if (fileInfo.size() == 0) {
		reply.attribute("error", "The request file does not exist.");
		return reply;
	}
	std::string NSPFileName = SWRTB::NSNFileNameToNSPFileName(fileName);
	reply.attribute("file", NSPFileName);
	try {
		SWRTB::copyFile(repoHeartPath + NSPFileName, sendFilePath + NSPFileName);
	}
	catch (std::exception & ex) { std::cout << ex.what() << std::endl; }
	
	if (fileInfo.size()) {
		reply.attribute("file-Namespace", fileInfo[0]);
		reply.attribute("file-Name", fileInfo[1]);
		reply.attribute("file-Version", fileInfo[2]);
		reply.attribute("file-Description", fileInfo[3]);
		reply.attribute("file-DateTime", fileInfo[4]);
		reply.attribute("file-Status", fileInfo[5]);
		reply.attribute("file-Owner", fileInfo[6]);
		reply.attribute("file-Dependencies", fileInfo[7]);
		reply.attribute("file-Categories", fileInfo[8]);
	}
	return reply;
}

// -----< trackAllRecordsMessage: Assemble trackAllRecords message  >-----
Msg Server::trackAllRecordsMessage(const EndPoint& from, const EndPoint& to) {
	Msg reply(to, from);
	reply.attribute("command", "trackAllRecordsCallback");
	SWRTB::Core repo(Repository::repoHeartPath);
	NoSqlDb::DbQuery<std::string> querier(repo.core());
	std::vector<NoSqlDb::DbElement<std::string>> result =
		querier.from(repo.core()).find().eval();
	size_t count = 0;
	for (auto item : result) {
		std::string recordBrief = item.nameSpace() + "$"
			+ SWRTB::nameOf(item.name(), item.nameSpace()) + "$"
			+ SWRTB::versionOf(item.name()) + "$"
			+ item.status() + "$" 
			+ item.owner();
		reply.attribute("record" + Utilities::Converter<size_t>::toString(count++), recordBrief);
	}
	return reply;
}

// -----< trackAllCategoriesMessage: Assemble trackAllCategories message  >-----
Msg Server::trackAllCategoriesMessage(const EndPoint& from, const EndPoint& to) {
	Msg reply(to, from);
	reply.attribute("command", "trackAllCategoriesCallback");
	SWRTB::Core repo(Repository::repoHeartPath);
	NoSqlDb::DbQuery<std::string> querier(repo.core());
	std::vector<NoSqlDb::DbElement<std::string>> result =
		querier.from(repo.core()).find().eval();
	std::unordered_set<std::string> hash;
	for (auto item : result) {
		for (auto category : item.category()) {
			if (hash.find(category) == hash.end()) hash.insert(category);
		}
	}
	size_t count = 0;
	for (auto item : hash) {
		reply.attribute("category" + Utilities::Converter<size_t>::toString(count++), item);
	}
	return reply;
}

// -----< checkInCallbackMessage: Assemble checkInCallback message  >-----
Msg Server::checkInCallbackMessage(const EndPoint& from, const EndPoint& to, Msg receiveMessage) {
	Msg reply(to, from);
	reply.attribute("command", "checkinCallback");
	SWRTB::Core repo(Repository::repoHeartPath);
	SWRTB::Checkin worker(repo);
	std::string pathFileName = "../SaveFiles/" + receiveMessage.value("file");
	std::string description = receiveMessage.value("description");
	std::string dependenies = receiveMessage.value("dependencies");
	std::string categories = receiveMessage.value("categories");
	std::string nameSpace = receiveMessage.value("nameSpace");
	std::string owner = receiveMessage.value("owner");
	bool close = (receiveMessage.value("close") == "true") ? true : false;
	std::string errorInfo = "";
	try {
		worker.checkin(pathFileName, dependenies, description, categories, nameSpace, owner, close);
	}
	catch (const std::exception& ex) {
		errorInfo = ex.what();
		for (size_t i = 0; i < errorInfo.length(); ++i) {
			if (errorInfo[i] == '\n') errorInfo[i] = ' ';
		}
	}
	reply.attribute("errorInfo", errorInfo);
	if (receiveMessage.containsKey("name")) reply.attribute("name", "Server replies " + receiveMessage.value("name"));
	return reply;
}

// -----< checkOutCallbackMessage: Assemble checkOutCallback message  >-----
Msg Server::checkOutCallbackMessage(const EndPoint& from, const EndPoint& to, Msg receiveMessage) {
	Msg reply(to, from);
	reply.attribute("command", "checkoutCallback");
	SWRTB::Core repo(Repository::repoHeartPath);
	SWRTB::Checkout worker(repo, "../SendFiles/");
	std::string NSNFileNameVersion = receiveMessage.value("fileName");
	std::string requestor = receiveMessage.value("requestor");
	bool recursive = (receiveMessage.value("recursive") == "true") ? true : false;
	std::string errorInfo = "";
	try {
		worker.checkout(NSNFileNameVersion, requestor, recursive);
	}
	catch (std::exception& ex) {
		errorInfo = ex.what();
	}
	reply.attribute("errorInfo", errorInfo);
	std::vector<std::string> theFiles = worker.successCheckouts();
	for (size_t i = 0; i < theFiles.size(); ++i) reply.attribute("successFile" + std::to_string(int(i)), theFiles[i]);
	theFiles = worker.failCheckouts();
	for (size_t i = 0; i < theFiles.size(); ++i) reply.attribute("failFile" + std::to_string(int(i)), theFiles[i]);
	if (receiveMessage.containsKey("name")) reply.attribute("name", "Server replies " + receiveMessage.value("name"));
	return reply;
}

// -----< sendMultipleFiles: Send multiple files  >-----
void Server::sendMultipleFiles(Msg message) {
	Msg reply(message.from(), message.to());
	if (message.value("for") == "checkoutReceiveFiles")
		reply.attribute("command", "checkoutReceiveFilesCallback");
	for (auto item : message.attributes()) {
		if (item.first.find("fileName") != std::string::npos) {
			reply.attribute("file", SWRTB::nameOf(item.second));
			reply.attribute("fileName", item.second);
			postMessage(reply);
		}
	}
	return;
}

// -----< buildSource: Helper function of setFilterMessage >----------------------------
std::vector<NoSqlDb::DbElement<std::string>> buildSource(const std::string& rawSource) {
	SWRTB::Core repo(Repository::repoHeartPath);
	NoSqlDb::DbQuery<std::string> querier(repo.core());
	std::vector<std::string> names = Utilities::splitPlus(rawSource, '$');
	std::vector<NoSqlDb::DbElement<std::string>> result;
	for (auto item : names) {
		std::vector<NoSqlDb::DbElement<std::string>> tmp = querier.from(repo.core()).find("name", item).eval();
		if (tmp.size()) result.push_back(tmp[0]);
	}
	return result;
}

// -----< setFilterMessage: Assemble setFilter message >------------------------------------
Msg Server::setFilterMessage(const EndPoint& from, const EndPoint& to, Msg receiveMessage) {
	Msg reply(to, from);
	reply.attribute("command", "setFilterCallback");
	SWRTB::Core repo(Repository::repoHeartPath);
	NoSqlDb::DbQuery<std::string> querier(repo.core());
	std::vector<NoSqlDb::DbElement<std::string>> result;
	std::string queryString = "", nameSpace = ".*", fileName = ".*", version = "[0-9]*", fileNameRegex = "";
	if (receiveMessage.containsKey("nameSpace")) nameSpace = Utilities::regexSafeFilter(receiveMessage.value("nameSpace"));
	if (receiveMessage.containsKey("fileName")) fileName = Utilities::regexSafeFilter(receiveMessage.value("fileName"));
	if (receiveMessage.containsKey("version")) version = Utilities::regexSafeFilter(receiveMessage.value("version"));
	fileNameRegex = "/" + Utilities::regexSafeFilter(repo.root()) + nameSpace + "_" + fileName + "\\." + version +"/";
	queryString += "payLoad: \"" + fileNameRegex + "\",";
	if (receiveMessage.containsKey("dependencies")) queryString += "children: \"" + receiveMessage.value("dependencies") + "\",";
	if (receiveMessage.containsKey("categories")) queryString += "category: \"" + receiveMessage.value("category") + "\",";
	reply.attribute("query", queryString.substr(0, queryString.length() - 1));
	result = querier.from(buildSource(receiveMessage.value("source"))).find(queryString.substr(0, queryString.length() - 1)).eval();
	size_t count = 0;
	for (auto item : result) {
		std::string recordBrief = item.nameSpace() + "$"
			+ SWRTB::nameOf(item.name(), item.nameSpace()) + "$"
			+ SWRTB::versionOf(item.name()) + "$"
			+ item.status() + "$"
			+ item.owner();
		reply.attribute("record" + Utilities::Converter<size_t>::toString(count++), recordBrief);
	}
	return reply;
}

// -----< shouldClose: Helper function of resumeCheckinMessage >-----
bool shouldClose(const std::string& name) {
	SWRTB::Core repo(Repository::repoHeartPath);
	NoSqlDb::DbQuery<std::string> querier(repo.core());
	std::vector<NoSqlDb::DbElement<std::string>> tmp = querier.from(repo.core()).find("name", name).eval();
	if (tmp.size() == 0) return false;
	else if (tmp[0].status() != "open") return false;
	else return true;
}

// -----< speratorConverter: Convert seperator >-----
std::string seperatorConverter(std::string toConvert, char oldOne, char newOne) {
	size_t i = 0, len = toConvert.length();
	while (i < len) {
		if (toConvert[i] == oldOne) toConvert[i] = newOne;
		i++;
	}
	return toConvert;
}

// -----< resumeCheckinMessage: Assamble resumeCheckinMessage >---------------------------------
Msg Server::resumeCheckinMessage(const EndPoint& from, const EndPoint& to, Msg receiveMessage) {
	Msg reply(to, from);
	reply.attribute("command", "resumeCheckinCallback");
	SWRTB::Core repo(Repository::repoHeartPath);
	SWRTB::Checkin worker(repo);
	std::string name = receiveMessage.value("fileKey");
	std::string errorInfo = "";
	try {
		if (shouldClose(name) == true && receiveMessage.value("status") != "open") {
			worker.checkin(receiveMessage.value("fileName"), seperatorConverter(receiveMessage.value("dependencies"), '$', ','), receiveMessage.value("description"),
				seperatorConverter(receiveMessage.value("categories"), '$', ','), receiveMessage.value("nameSpace"), receiveMessage.value("owner"), true);
		}
		else {
			worker.checkin(receiveMessage.value("fileName"), seperatorConverter(receiveMessage.value("dependencies"), '$', ','), receiveMessage.value("description"),
				seperatorConverter(receiveMessage.value("categories"), '$', ','), receiveMessage.value("nameSpace"), receiveMessage.value("owner"), false);
		}
	}
	catch (std::exception& ex) {
		errorInfo = ex.what();
	}
	reply.attribute("errorInfo", errorInfo);
	return reply;
}

// -----< show: show different type of message >-----
template<typename T>
void show(const T& t, const std::string& msg)
{
	std::cout << "\n  " << msg.c_str();
	for (auto item : t)
	{
		std::cout << "\n    " << item.c_str();
	}
}

// -----< echo: reply echo message >--------
std::function<Msg(Msg)> echo = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

// -----< trackAllCategories: reply trackAllCategories message >-----
std::function<Msg(Msg)> trackAllCategories = [](Msg msg) {
	Msg reply = Server::trackAllCategoriesMessage(msg.to(), msg.from());
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

// -----< trackAllRecords: reply trackAllCategories message >-----
std::function<Msg(Msg)> trackAllRecords = [](Msg msg) {
	Msg reply = Server::trackAllRecordsMessage(msg.to(), msg.from());
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

// -----< showFileCleanUp: reply showFileCleanUp message >-----
std::function<Msg(Msg)> showFileCleanUp = [](Msg msg) {
	Msg reply(serverEndPoint, serverEndPoint);
	reply.command("echo");
	std::string file = msg.value("fileName");
	FileSystem::File("").remove(Repository::sendFilePath + file);
	//std::cout << "showFileCleanUp: " << Repository::sendFilePath + file << " has been cleaned up.\n";
	return reply;
};

// -----< showFile: reply showFile message >-----
std::function<Msg(Msg)> showFile = [](Msg msg) {
	Msg reply = Server::showFileMessge(msg.to(), msg.from(), msg.value("fileName"));
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

// -----< fileCheckin: reply fileCheckin message >-----
std::function<Msg(Msg)> fileCheckin = [](Msg msg) {
	Msg reply = Server::checkInCallbackMessage(msg.to(), msg.from(), msg);
	return reply;
};

// -----< fileCheckout: reply fileCheckout message >-----
std::function<Msg(Msg)> fileCheckout = [](Msg msg) {
	Msg reply = Server::checkOutCallbackMessage(msg.to(), msg.from(), msg);
	return reply;
};

// -----< ping: reply ping message >-----
std::function<Msg(Msg)> ping = [](Msg msg) {
	Msg reply(msg.from(), msg.to());
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	reply.attribute("command", "ping");
	return reply;
};

// -----< uploadFiles: reply uploadFiles message >-----
std::function<Msg(Msg)> uploadFiles = [](Msg msg) {
	Msg reply(msg.from(), msg.to());
	reply.attribute("command", "uploadFileCallback");
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

// -----< downloadFiles: reply downloadFiles message >-----
std::function<Msg(Msg)> downloadFiles = [](Msg msg) {
	Msg reply(msg.from(), msg.to());
	reply.attribute("command", "downloadFileCallback");
	reply.attribute("file", msg.value("requestFile"));
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

// -----< setFilter: reply setFilter message >-----
std::function<Msg(Msg)> setFilter = [](Msg msg) {
	Msg reply = Server::setFilterMessage(msg.to(), msg.from(), msg);
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

// -----< resumeCheckin: reply resumeCheckin message >-----
std::function<Msg(Msg)> resumeCheckin = [](Msg msg) {
	Msg reply = Server::resumeCheckinMessage(msg.to(), msg.from(), msg);
	if (msg.containsKey("name")) reply.attribute("name", "Server replies " + msg.value("name"));
	return reply;
};

int main()
{
	SetConsoleTitleA("Server Console");
	//StaticLogger<1>::attach(&std::cout);
	//StaticLogger<1>::start();
	Server server(serverEndPoint, "ServerPrototype");
	server.start();

	std::cout << "\n  Demostrate message processing as requirement 2 - 6 of Project4";
	std::cout << "\n ----------------------------";
	server.addMsgProc("echo", echo);
	server.addMsgProc("showFile", showFile);
	server.addMsgProc("fileCheckin", fileCheckin);
	server.addMsgProc("fileCheckout", fileCheckout);
	server.addMsgProc("showFileCleanUp", showFileCleanUp);
	server.addMsgProc("trackAllRecords", trackAllRecords);
	server.addMsgProc("trackAllCategories", trackAllCategories);
	server.addMsgProc("serverQuit", echo);
	server.addMsgProc("ping", ping);
	server.addMsgProc("downloadFile", downloadFiles);
	server.addMsgProc("uploadFile", uploadFiles);
	server.addMsgProc("setFilter", setFilter);
	server.addMsgProc("resumeCheckin", resumeCheckin);
	server.processMessages();

	Msg msg(serverEndPoint, serverEndPoint);  // send to self

	std::cout << "\n  press enter to exit";
	std::cin.get();
	std::cout << "\n";

	msg.command("serverQuit");
	server.postMessage(msg);
	server.stop();
	return 0;
}

