#pragma once
///////////////////////////////////////////////////////////////////////
// ServerPrototype.h - Console App that processes incoming messages  //
// ver 1.0                                                           //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2018         //
///////////////////////////////////////////////////////////////////////
/*
*  Package Operations:
* ---------------------
*  Package contains one class, Server, that contains a Message-Passing Communication
*  facility. It processes each message by invoking an installed callable object
*  defined by the message's command key.
*
*  Message handling runs on a child thread, so the Server main thread is free to do
*  any necessary background processing (none, so far).
*
*  Required Files:
* -----------------
*  ServerPrototype.h, ServerPrototype.cpp
*  Comm.h, Comm.cpp, IComm.h
*  Message.h, Message.cpp
*  FileSystem.h, FileSystem.cpp
*  Utilities.h
*
*  Maintenance History:
* ----------------------
*  ver 1.0 : 3/27/2018
*  - first release
*/
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <thread>
#include "../CppCommWithFileXfer/Message/Message.h"
#include "../CppCommWithFileXfer/MsgPassingComm/Comm.h"
#include <windows.h>
#include <tchar.h>

// Headers from project2 use for repository function
#include "../SwRepoTB/SoftwareRepoTB/SWRepoCore.h"
#include "../SwRepoTB/Checkin/Checkin.h"
#include "../SwRepoTB/Checkout/Checkout.h"
#include "../SwRepoTB/Browse/Browse.h"

// Headers from project1 use for db function
#include "../SwRepoTB/NoSqlDb/DbCore/DbCore.h"
#include "../SwRepoTB/NoSqlDb/Query/Query.h"
#include "../SwRepoTB/NoSqlDb/Persistence/Persisence.h"
#include "../SwRepoTB/NoSqlDb/DateTime/DateTime.h"

// Headers from Utilities use for helper function
#include "../SwRepoTB/SWRTBUtilities/SWRTBUtilities.h"
#include "../SwRepoTB/NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"

namespace Repository
{
	using File = std::string;
	using Files = std::vector<File>;
	using Dir = std::string;
	using Dirs = std::vector<Dir>;
	using SearchPath = std::string;
	using Key = std::string;
	using Msg = MsgPassingCommunication::Message;
	using ServerProc = std::function<Msg(Msg)>;
	using MsgDispatcher = std::unordered_map<Key, ServerProc>;

	const SearchPath storageRoot = "../Storage";  // root for all server file storage
	const std::string repoHeartPath = "../Storage/";
	const MsgPassingCommunication::EndPoint serverEndPoint("localhost", 8080);  // listening endpoint

	class Server
	{
	public:
		Server(MsgPassingCommunication::EndPoint ep, const std::string& name);
		void start();
		void stop();
		void addMsgProc(Key key, ServerProc proc);
		void processMessages();
		void postMessage(MsgPassingCommunication::Message msg);
		MsgPassingCommunication::Message getMessage();
		static Dirs getDirs(const SearchPath& path = storageRoot);
		static Files getFiles(const SearchPath& path = storageRoot);

		// helper function from project2
		static std::string fileInfoAssembler(const std::string& NSPFileName);

	private:
		MsgPassingCommunication::Comm comm_;
		MsgDispatcher dispatcher_;
		std::thread msgProcThrd_;

		// data member from project2
		SWRTB::Core repoCore;
		SWRTB::Checkin checkinWorker;
		SWRTB::Checkout checkoutWorker;

		// data member from project1
		DbQuery::queryResult<std::string> querier;
		DbPersistence::persistence<std::string> persistor;
	};
	//----< initialize server endpoint and give server a name >----------

	inline Server::Server(MsgPassingCommunication::EndPoint ep, const std::string& name)
		: comm_(ep, name), repoCore("../Storage/"), checkinWorker(repoCore), checkoutWorker(repoCore), querier(repoCore.core()){}

	//----< start server's instance of Comm >----------------------------

	inline void Server::start()
	{
		comm_.start();
	}
	//----< stop Comm instance >-----------------------------------------

	inline void Server::stop()
	{
		if (msgProcThrd_.joinable())
			msgProcThrd_.join();
		comm_.stop();
	}
	//----< pass message to Comm for sending >---------------------------

	inline void Server::postMessage(MsgPassingCommunication::Message msg)
	{
		comm_.postMessage(msg);
	}
	//----< get message from Comm >--------------------------------------

	inline MsgPassingCommunication::Message Server::getMessage()
	{
		Msg msg = comm_.getMessage();
		return msg;
	}
	//----< add ServerProc callable object to server's dispatcher >------

	inline void Server::addMsgProc(Key key, ServerProc proc)
	{
		dispatcher_[key] = proc;
	}
	//----< start processing messages on child thread >------------------

	inline void Server::processMessages()
	{
		auto proc = [&]()
		{
			if (dispatcher_.size() == 0)
			{
				std::cout << "\n  no server procs to call";
				return;
			}
			while (true)
			{
				Msg msg = getMessage();
				std::cout << "\n receive message name: " << msg.name() << " from " << msg.from().toString();
				std::cout << "\n  received message command: " << msg.command() << " from " << msg.from().toString();
				if (msg.command() == "") {
					continue;
				}
				if (msg.containsKey("verbose"))
				{
					std::cout << "\n";
					msg.show();
				}
				if (msg.command() == "serverQuit")
					break;
				std::cout << "Response to " << msg.command() << std::endl;
				Msg reply = dispatcher_[msg.command()](msg);
				if (msg.to().port != msg.from().port)  // avoid infinite message loop
				{
					postMessage(reply);
					msg.show();
					reply.show();
				}
				else
					std::cout << "\n  server attempting to post to self";
			}
			std::cout << "\n  server message processing thread is shutting down";
		};
		std::thread t(proc);
		//SetThreadPriority(t.native_handle(), THREAD_PRIORITY_HIGHEST);
		std::cout << "\n  starting server thread to process messages";
		msgProcThrd_ = std::move(t);
	}
}