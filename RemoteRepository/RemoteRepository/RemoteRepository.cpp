/////////////////////////////////////////////////////////////////////////
// ServerPrototype.cpp - Console App that processes incoming messages  //
// ver 1.0                                                             //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018           //
/////////////////////////////////////////////////////////////////////////

#include "RemoteRepository.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include <chrono>

namespace MsgPassComm = MsgPassingCommunication;

using namespace Repository;
using namespace FileSystem;
using Msg = MsgPassingCommunication::Message;

std::string vectorToString(const std::vector<std::string>& toConvert) {
	std::string result = "";
	for (auto item : toConvert) {
		result = item + "$";
	}
	if (result != "") result.substr(0, result.length() - 1);
	return result;
}

Files Server::getFiles(const Repository::SearchPath& path)
{
	return Directory::getFiles(path);
}

Dirs Server::getDirs(const Repository::SearchPath& path)
{
	return Directory::getDirectories(path);
}

std::string Server::getFilesPlus(const Repository::SearchPath& searchPath) {
	Files files = getFiles(searchPath);
	std::string fileComplex = "";
	for (auto item : files) {
		fileComplex += (item + "$");
	}
	if (fileComplex != "") fileComplex = fileComplex.substr(0, fileComplex.length() - 1);
	return fileComplex;
}

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

std::vector<std::string> Server::fileInfoAssembler(const std::string& NSPFileName) {
	SWRTB::Core repo(Repository::repoHeartPath);
	std::vector<std::string> ans;
	DbQuery::queryResult<std::string> querier(repo.core());
	try{
		SWRTB::NSPFileNameToNSNFileName(NSPFileName);
	}
	catch (std::exception &ex) {
		return ans;
	}
	//std::cout << "I need to find " << SWRTB::NSPFileNameToNSNFileName(NSPFileName) << std::endl;
	std::vector<NoSqlDb::DbElement<std::string>> result = 
		querier.from(repo.core()).find("name", SWRTB::NSPFileNameToNSNFileName(NSPFileName)).eval();
	std::cout << "I find " << result.size() << "!" << std::endl;
	if (result.size() == 0) return ans;
	else {
		ans.push_back(SWRTB::namespaceOf(result[0].name()));
		ans.push_back(SWRTB::nameCleanerPlus(result[0].name()));
		ans.push_back(SWRTB::versionOf(result[0].name()));
		ans.push_back(result[0].descrip());
		ans.push_back(std::string(result[0].dateTime()));
		ans.push_back(SWRTB::modeOf(result[0].payLoad()));
		ans.push_back(vectorToString(result[0].children()));
		ans.push_back(vectorToString(result[0].category()));
		return ans;
	}
}

Msg Server::listContentMessage(const std::string& path) {
	Msg reply;
	reply.command("listContent");
	std::string searchPath = storageRoot;
	if (path != ".") searchPath = searchPath + "\\" + path;
	std::string dirComplex = Server::getDirsPlus(searchPath), fileComplex = Server::getFilesPlus(searchPath);
	if (dirComplex != "") reply.attribute("dirs", dirComplex);
	if (fileComplex != "") reply.attribute("files", fileComplex);
	reply.attribute("path", path);
	return reply;
}

Msg Server::showFileMessge(const std::string& path) {
	Msg reply;
	reply.command("showFile");
	std::string searchPath = storageRoot + "\\" + path;
	reply.attribute("file", FileSystem::Path::getName(searchPath));
	reply.contentLength(FileSystem::FileInfo(searchPath).size());
	try {
		SWRTB::copyFile(searchPath, sendFilePath + FileSystem::Path::getName(searchPath));
	}
	catch (std::exception & ex) { std::cout << ex.what() << std::endl; }
	std::vector<std::string> fileInfo = fileInfoAssembler(FileSystem::Path::getName(searchPath));
	if (fileInfo.size()) {
		reply.attribute("file-Namespace", fileInfo[0]);
		reply.attribute("file-Name", fileInfo[1]);
		reply.attribute("file-Version", fileInfo[2]);
		reply.attribute("file-Description", fileInfo[3]);
		reply.attribute("file-DateTime", fileInfo[4]);
		reply.attribute("file-Status", fileInfo[5]);
		reply.attribute("file-Dependencies", fileInfo[6]);
		reply.attribute("file-Categories", fileInfo[7]);
	}
	else {
		reply.attribute("error", "The file has no database record");
	}
	return reply;
}

Msg Server::trackAllRecordsMessage() {
	Msg reply;
	reply.attribute("command", "trackAllRecordsCallback");
	SWRTB::Core repo(Repository::repoHeartPath);
	DbQuery::queryResult<std::string> querier(repo.core());
	std::vector<NoSqlDb::DbElement<std::string>> result = 
		querier.from(repo.core()).find().eval();
	size_t count = 0;
	for (auto item : result) {
		std::string recordBrief = SWRTB::namespaceOf(item.name()) + "$" 
			+ SWRTB::nameCleanerPlus(item.name()) + "$" 
			+ SWRTB::versionOf(item.name()) + "$" 
			+ SWRTB::modeOf(item.payLoad());
		reply.attribute("record" + Utilities::Converter<size_t>::toString(count++), recordBrief);
	}
	return reply;
}

template<typename T>
void show(const T& t, const std::string& msg)
{
	std::cout << "\n  " << msg.c_str();
	for (auto item : t)
	{
		std::cout << "\n    " << item.c_str();
	}
}

std::function<Msg(Msg)> echo = [](Msg msg) {
	Msg reply = msg;
	reply.to(msg.from());
	reply.from(msg.to());
	return reply;
};

std::function<Msg(Msg)> trackAllRecords = [](Msg msg) {
	Msg reply = Server::trackAllRecordsMessage();
	reply.to(msg.from());
	reply.from(msg.to());
	std::cout << "Ready to reply--------------------------------------------------------" << std::endl;
	return reply;
};

std::function<Msg(Msg)> showFileCleanUp = [](Msg msg) {
	Msg reply(serverEndPoint, serverEndPoint);
	reply.command("echo");
	std::string file = msg.value("fileName");
	FileSystem::File("").remove(Repository::sendFilePath + file);
	std::cout << "showFileCleanUp: " << Repository::sendFilePath + file << " has been cleaned up.\n";
	return reply;
};

std::function<Msg(Msg)> listContent = [](Msg msg) {
	Msg reply;
	std::string path = msg.value("path");
	if(path == "") std::cout << "listContent: message did not define a path attribute.\n";
	else if (SWRTB::isDirectory(msg.value("path")) == true) {
		reply = Server::listContentMessage(path);
	}
	else if (SWRTB::isFile(msg.value("path")) == true) {
		reply = Server::showFileMessge(path);
	}
	else {
		std::cout << "listContent: \"" << path << "\" did not represent a file or a directory.\n";
	}
	reply.to(msg.from());
	reply.from(msg.to());
	return reply;
};

std::function<Msg(Msg)> fileCheckin = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.name("File Received");
	reply.command("checkinCallback");
	return reply;
};

std::function<Msg(Msg)> fileCheckout = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.name("File sended");
	reply.command("checkoutCallback");
	reply.file("server.js");
	reply.contentLength(FileSystem::FileInfo("../server.js").size());
	return reply;
};

int main()
{
	SetConsoleTitleA("Project3 Help Server");
	std::cout << "\n  Testing Server Prototype";
	std::cout << "\n ==========================";
	std::cout << "\n";

	//StaticLogger<1>::attach(&std::cout);
	//StaticLogger<1>::start();

	Server server(serverEndPoint, "ServerPrototype");
	server.start();

	std::cout << "\n  testing getFiles and getDirs methods";
	std::cout << "\n --------------------------------------";
	Files files = server.getFiles();
	show(files, "Files:");
	Dirs dirs = server.getDirs();
	show(dirs, "Dirs:");
	std::cout << "\n";

	std::cout << "\n  testing message processing";
	std::cout << "\n ----------------------------";
	server.addMsgProc("echo", echo);
	server.addMsgProc("listContent", listContent);
	server.addMsgProc("fileCheckin", fileCheckin);
	server.addMsgProc("fileCheckout", fileCheckout);
	server.addMsgProc("showFileCleanUp", showFileCleanUp);
	server.addMsgProc("trackAllRecords", trackAllRecords);
	server.addMsgProc("serverQuit", echo);
	server.processMessages();

	Msg msg(serverEndPoint, serverEndPoint);  // send to self
	msg.name("msgToSelf");
	msg.command("echo");
	msg.attribute("verbose", "show me");
	server.postMessage(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	std::cout << "\n  press enter to exit";
	std::cin.get();
	std::cout << "\n";

	msg.command("serverQuit");
	server.postMessage(msg);
	server.stop();
	return 0;
}

