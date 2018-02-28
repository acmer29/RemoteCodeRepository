#pragma once
#ifndef CHECKIN_H
#define CHECKIN_H
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../SWRTBUtilities/SWRTBUtilities.h"
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

		Checkin& selectFile(const std::string& path);

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
		std::string openDirectory;
		std::string closedDirectory;
		std::string structurePathFileName;

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
