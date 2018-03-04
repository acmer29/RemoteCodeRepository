#pragma once
////////////////////////////////////////////////////////////////////////////
// Checkin.h - Provide means for checkin files							  //
// ver 1.1																  //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018                //
////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - Checkin
* This class provides means to checkin files.
* This class provides one public function, and a set of functions for cascade calling.
*  - checkin : One for cascade calling, and one requires all parameters, provide for 
*			   frontend using.
*  - selectFile : Provide for cascade calling, select a file or files in a folder.
*  - setNameSpace : Provide for cascade calling, set the namespace for checkin files.
*  - setDependence : Provide for cascade calling, set the dependencies for checkin files.
*  - setDescription : Provide for cascade calling, set the description for checkin files.
*  - setCategory : Provide for cascade calling, set the category for checkin files.

* Required Files:
* ---------------
* SWRepoCore.h
* SWRTBUtilities.h
* FileSystem.h, FileSystem.cpp
* DbCore.h, DbCore.cpp
*
* Build Process:
* --------------
* devenv NoSqlDb.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 28 Feb 2018 - First release
* ver 1.1 : 03 Mar 2018 - Add useful functions, remove useless functions
*/
#ifndef CHECKIN_H
#define CHECKIN_H
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../SWRTBUtilities/SWRTBUtilities.h"
#include "../LoopHandler/LoopHandler.h"
namespace SWRTB {
	class Checkin {
	public:
		// FileInfo: first member represents the NSNFileNameVersion
		//			 second member represents the pathFileName
		using FileInfo = std::pair<std::string, std::string>;

		explicit Checkin(Core& target);
		
		// The function mainly provided for frontend using.
		void checkin(const std::string& path, \
					 const std::string& dependency, \
					 const std::string& description, \
					 const std::string& category, \
					 const std::string& nameSpace, \
					 bool close = false);
		// The function provided for cascade calling.
		void checkin(bool close = false);

		Checkin& selectFile(const std::string& pathFileName);

		Checkin& setNameSpace(const std::string& nameSpace = "");

		Checkin& setDependence(const std::string& dependencies = "");

		Checkin& setDescription(const std::string& description = "");

		Checkin& setCategory(const std::string& categories = "");

	private:
		// Repo Core
		Core& repo;

		// Information data
		std::string nameSpace_;
		std::string dependencies_;
		std::string description_;
		std::string categories_;
		std::vector<std::string> filesForCheckin;

		// Query Helper
		DbQuery::queryResult<std::string> querier;

		// Persistance Helper
		DbPersistence::persistence<std::string> persistor;

		// Directories and files
		std::string workDirectory;

		// FileSystem helpers
		FileSystem::Path pathHelper;
		FileSystem::Directory dirHelper;

		//Helper functions
		void pathSolver(const std::string& path);
		void localPathSolver(const std::string& fileName);
		int versionSetter(const std::string& fileName);
		bool canClose(const std::string& key);
		bool isNew(const std::string& fileName);
		
		// Checkin worker functions
		void newCheckin(const std::string& pathFileName);
		void resumeCheckin(const std::string& pathFileName);
		void closeCheckin(const std::string& fileName);
		
		// Cleanup stage functions
		void saveRepo();
		void cleanUp();

	};
}
#endif // !CHECKIN_H
