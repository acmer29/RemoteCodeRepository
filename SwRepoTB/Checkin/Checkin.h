#pragma once
#ifndef CHECKIN_H
#define CHECKIN_H
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../SWRTBUtilities/SWRTBUtilities.h"
#include "../NoSqlDb/Query/Query.h"
#include "../NoSqlDb/Persistence/Persisence.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
namespace SWRTB {
	class Checkin {
	public:
		// FileInfo: first member represents the NSNFileNameVersion
		//			 second member represents the pathFileName
		using FileInfo = std::pair<std::string, std::string>;

		explicit Checkin(Core& target);
		
		void checkin(const std::string& path, \
					 const std::string& dependency, \
					 const std::string& description, \
					 const std::string& category, \
					 const std::string& nameSpace, \
					 bool close = false);
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
		
		void newCheckin(const std::string& pathFileName);
		void resumeCheckin(const std::string& pathFileName);
		void closeCheckin(const std::string& fileName);
		
		void saveRepo();

		void cleanUp();

		bool isFile(const std::string& path);
		bool isDirectory(const std::string& path);

	};
}
#endif // !CHECKIN_H
