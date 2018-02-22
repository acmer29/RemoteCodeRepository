#pragma once
#ifndef CHECKIN_H
#define CHECKIN_H
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../NoSqlDb/Query/Query.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
namespace SWRTB {
	class Checkin {
	public:
		explicit Checkin(Core& target);
		
		void checkin(const std::string& path, const std::string& dependency, const std::string& description, const std::string& category, bool close = false);
		void checkin(bool close = false);

		Checkin& selectFile(const std::string& path);

		Checkin& setDependence(const std::string& dependencies = "");

		Checkin& setDescription(const std::string& description = "");

		Checkin& setCategory(const std::string& categories = "");

	private:

		// Repo Core
		Core& repo;

		// Information data
		std::string dependencies_;
		std::string description_;
		std::string categories_;
		std::vector<std::string> filesForCheckin;

		// Query Helper
		DbQuery::queryResult<std::string> querier;

		// Directories
		std::string workDirectory;
		std::string openDirectory;
		std::string closedDirectory;

		// FileSystem helpers
		FileSystem::Path pathHelper;
		FileSystem::Directory dirHelper;

		//Helper functions
		void pathSolver(const std::string& path);
		int versionSetter(const std::string& fileName);
		bool canClose(const std::string& key);
		bool isNew(const std::string& fileName);

		void copyFile(const std::string& fromPath, const std::string& toPath);
		
		void newCheckin(const std::string& pathFileName);
		void resumeCheckin(const std::string& pathFileName);
		void closeCheckin(const std::string& fileName);
		
		bool isFile(const std::string& path);
		bool isDirectory(const std::string& path);

		
	};
}
#endif // !CHECKIN_H
