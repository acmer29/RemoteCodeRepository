#pragma once
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

	inline Core::Core(const std::string& targetDirectory) : root_(targetDirectory) {
		if (FileSystem::Directory().exists(targetDirectory) == false)
			FileSystem::Directory().create(targetDirectory);
#ifndef DEBUG_PROJ2
		else {
			std::string heart = targetDirectory + "HeartOfRepo";
			if (FileSystem::File(heart).exists(heart + ".xml")) {
				std::cout << "Detected existing repo record file, restore from " << heart << ".xml" << std::endl;
				std::vector<NoSqlDb::DbElement<std::string>> result = DbPersistence::persistence<std::string>().restore(heart + ".xml");
				DbQuery::queryResult<std::string> querier(repo_);
				for (auto item : result) querier.from(repo_).insert(item);
			}
		}
#endif // !DEBUG_PROJ2
	}
}
#endif // SWREPOCORE_H

