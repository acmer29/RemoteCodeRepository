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

Files Server::getFiles(const Repository::SearchPath& path)
{
	return Directory::getFiles(path);
}

Dirs Server::getDirs(const Repository::SearchPath& path)
{
	return Directory::getDirectories(path);
}

std::string Server::fileInfoAssembler(const std::string& NSPFileName) {
	SWRTB::Core repo(Repository::repoHeartPath);
	std::cout << "I need to find " << NSPFileName << std::endl;
	DbQuery::queryResult<std::string> querier(repo.core());
	try{
		SWRTB::NSPFileNameToNSNFileName(NSPFileName);
	}
	catch (std::exception &ex) {
		return "";
	}
	//std::cout << "I need to find " << SWRTB::NSPFileNameToNSNFileName(NSPFileName) << std::endl;
	std::vector<NoSqlDb::DbElement<std::string>> result = querier.from(repo.core()).find("name", SWRTB::NSPFileNameToNSNFileName(NSPFileName)).eval();
	std::cout << "I find " << result.size() << "!" << std::endl;
	if (result.size() == 0) return "";
	else {
		std::string res = "";
		res += SWRTB::nameCleaner(result[0].name()) + "$";
		res += result[0].name() + "$" + result[0].descrip() + "$" + std::string(result[0].dateTime()) + "$";
		res += SWRTB::modeOf(result[0].payLoad()) + "$";
		return res;
	}
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

std::function<Msg(Msg)> listContent = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	std::string path = msg.value("path");
	if(path == "") std::cout << "listContent: message did not define a path attribute.\n";
	else if (SWRTB::isDirectory(msg.value("path")) == true) {
		reply.command("listContent");
		std::string searchPath = storageRoot;
		if (path != ".") searchPath = searchPath + "\\" + path;
		Files files = Server::getFiles(searchPath);
		Files dirs = Server::getDirs(searchPath);
		std::string dirComplex = "", fileComplex = "";
		for (auto item : dirs) {
			if(item != "." && item != "..")
				dirComplex += (item + "$");
		}
		if(dirComplex != "") reply.attribute("dirs", dirComplex.substr(0, dirComplex.length() - 1));
		for (auto item : files) {
			fileComplex += (item + "$");
		}
		if(fileComplex != "") reply.attribute("files", fileComplex.substr(0, fileComplex.length() - 1));
		reply.attribute("path", path);
	}
	else if (SWRTB::isFile(msg.value("path")) == true) {
		reply.command("showFile");
		std::string searchPath = storageRoot + "\\" + path;
		reply.attribute("file", searchPath);
		reply.contentLength(FileSystem::FileInfo(searchPath).size());
	}
	else {
		std::cout << "listContent: \"" << path << "\" did not represent a file or a directory.\n";
	}
	return reply;
};

std::function<Msg(Msg)> getFiles = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getFiles");
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		if (path != ".")
			searchPath = searchPath + "\\" + path;
		Files files = Server::getFiles(searchPath);
		size_t count = 0;
		for (auto item : files)
		{
			std::string countStr = Utilities::Converter<size_t>::toString(++count);
			reply.attribute("file" + countStr, item);
		}
	}
	else
	{
		std::cout << "\n  getFiles message did not define a path attribute";
	}
	return reply;
};

std::function<Msg(Msg)> getDirs = [](Msg msg) {
	Msg reply;
	reply.to(msg.from());
	reply.from(msg.to());
	reply.command("getDirs");
	std::string path = msg.value("path");
	if (path != "")
	{
		std::string searchPath = storageRoot;
		if (path != ".")
			searchPath = searchPath + "\\" + path;
		Files dirs = Server::getDirs(searchPath);
		size_t count = 0;
		for (auto item : dirs)
		{
			if (item != ".." && item != ".")
			{
				std::string countStr = Utilities::Converter<size_t>::toString(++count);
				reply.attribute("dir" + countStr, item);
			}
		}
	}
	else
	{
		std::cout << "\n  getDirs message did not define a path attribute";
	}
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
	reply.file("package.json");
	reply.contentLength(FileSystem::FileInfo("../package.json").size());
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
	server.addMsgProc("getFiles", getFiles);
	server.addMsgProc("getDirs", getDirs);
	server.addMsgProc("listContent", listContent);
	server.addMsgProc("fileCheckin", fileCheckin);
	server.addMsgProc("fileCheckout", fileCheckout);
	server.addMsgProc("serverQuit", echo);
	server.processMessages();

	Msg msg(serverEndPoint, serverEndPoint);  // send to self
	msg.name("msgToSelf");
	msg.command("echo");
	msg.attribute("verbose", "show me");
	server.postMessage(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	msg.command("getFiles");
	msg.remove("verbose");
	msg.attributes()["path"] = storageRoot;
	server.postMessage(msg);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	msg.command("getDirs");
	msg.attributes()["path"] = storageRoot;
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

