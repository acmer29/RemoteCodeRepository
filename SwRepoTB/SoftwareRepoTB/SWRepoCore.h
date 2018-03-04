#pragma once
////////////////////////////////////////////////////////////////////////////
// SWRepoCore.h - Provide the core of software repo						  //
// ver 1.0																  //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018                //
////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - SWRepoCore.
* This class provide the core of software repository, and can restore exist
* records by default.
*
* The public interface function operations are as below:
*  - core : Provides reference as well as value access to the core - a NoSqlDb, 
*			also provide means to change the core.
*  - root : Provides reference as well as value access to the working directory
*			of the core.

* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* Persistence.h, Persistence.cpp
* Query.h, Query.cpp
* StringUtilites.h
* FileSystem.h, FileSystem.cpp
*
* Build Process:
* --------------
* devenv NoSqlDb.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 28 Feb 2018 - First release
*/
#ifndef SWREPOCORE_H
#define SWREPOCORE_H
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include "../NoSqlDb/DbCore/DbCore.h"
#include "../NoSqlDb/Persistence/Persisence.h"
#include "../NoSqlDb/Query/Query.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
namespace SWRTB{
	class Core {
	public:
		using Repo = NoSqlDb::DbCore<std::string>;

		explicit Core(const std::string& targetDirectory = FileSystem::Directory().getCurrentDirectory());

		Repo& core() { return repo_; }
		Repo core() const { return repo_;  }
		void core(const Repo& repo) { repo_ = repo; }

		std::string& root() { return root_; }
		std::string root() const { return root_; }

	private:
		NoSqlDb::DbCore<std::string> repo_;
		std::string root_;
	};

	// -----< Constructor: The targetDirectory is required to indicated working directory of the repo core >-----
	// -----< Once the working directory is confirmed, it will scan whether a repo is already exists, then >-----
	// -----< recover from it by default >-----------------------------------------------------------------------
	inline Core::Core(const std::string& targetDirectory) : root_(targetDirectory) {
		if (FileSystem::Directory().exists(targetDirectory) == false)
			FileSystem::Directory().create(targetDirectory);
#ifndef DEBUG_PROJ2
		else {
			std::string heart = targetDirectory + "HeartOfRepo";
			if (FileSystem::File(heart).exists(heart + ".xml")) {
				std::cout << "RepoCore initialization: Detected existing repo record file, restore from " << heart << ".xml" << std::endl;
				std::vector<NoSqlDb::DbElement<std::string>> result = DbPersistence::persistence<std::string>().restore(heart + ".xml");
				DbQuery::queryResult<std::string> querier(repo_);
				for (auto item : result) querier.from(repo_).insert(item);
			}
		}
#endif // !DEBUG_PROJ2
	}
}
#endif // SWREPOCORE_H

