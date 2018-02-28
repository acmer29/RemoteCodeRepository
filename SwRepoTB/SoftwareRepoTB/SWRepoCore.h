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

		bool isClosed(const std::string& NSNFileNameVersion);

	private:
		NoSqlDb::DbCore<std::string> repo_;
		std::string root_;
	};

	Core::Core(const std::string& targetDirectory) : root_(targetDirectory) {
		if (FileSystem::Directory().exists(targetDirectory) == false)
			FileSystem::Directory().create(targetDirectory);
		else {
			std::string heart = targetDirectory + "HeartOfRepo";
			if (FileSystem::File(heart).exists(heart + ".xml")) {
				std::cout << "Detected existing repo record file, restore from " << heart << ".xml" << std::endl;
				std::vector<NoSqlDb::DbElement<std::string>> result = DbPersistence::persistence<std::string>().restore(heart + ".xml");
				DbQuery::queryResult<std::string> querier(repo_);
				for (auto item : result) querier.from(repo_).insert(item);
			}
		}
	}

	bool Core::isClosed(const std::string& NSNFileNameVersion) {
		if (repo_.contains(NSNFileNameVersion) == false) return false;
		std::string location = repo_[NSNFileNameVersion].payLoad();
		std::regex expression(root_ + "open/" + Utilities::regexSafeFilter(NSNFileNameVersion) + "\\.[0-9]*");
		return !(std::regex_match(location, expression));
	}
}
#endif // SWREPOCORE_H

