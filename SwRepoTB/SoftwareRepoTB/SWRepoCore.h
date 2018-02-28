#pragma once
#ifndef SWREPOCORE_H
#define SWREPOCORE_H
#include <iostream>
#include <vector>
#include <string>
#include <regex>
#include "../NoSqlDb/DbCore/DbCore.h"
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

		bool isClosed(const std::string& fileNameVersion);

	private:
		NoSqlDb::DbCore<std::string> repo_;
		std::string root_;
	};

	Core::Core(const std::string& targetDirectory) : root_(targetDirectory) {
		if (FileSystem::Directory().exists(targetDirectory) == false)
			FileSystem::Directory().create(targetDirectory);
	}

	bool Core::isClosed(const std::string& fileNameVersion) {
		std::regex expression(root_ + "open/" + Utilities::regexSafeFilter(fileNameVersion) + "\\.[0-9]*");
		if (repo_.contains(fileNameVersion) == false) return false;
		std::string location = repo_[fileNameVersion].payLoad();
		return !(std::regex_match(location, expression));
	}
}
#endif // SWREPOCORE_H

