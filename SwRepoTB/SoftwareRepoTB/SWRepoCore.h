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
		void init();

	private:
		NoSqlDb::DbCore<std::string> repo_;
		std::string root_;
	};

	Core::Core(const std::string& targetDirectory) : root_(targetDirectory) {
		if (FileSystem::Directory().exists(targetDirectory) == false)
			FileSystem::Directory().create(targetDirectory);
		else init();
	}

	bool Core::isClosed(const std::string& fileNameVersion) {
		std::regex expression(root_ + "open/" + fileNameVersion + "\\.[0-9]*");
		if (repo_.contains(fileNameVersion) == false) return false;
		std::string location = repo_[fileNameVersion].payLoad();
		return !(std::regex_match(location, expression));
	}

	// NOT COMPLETED
	void Core::init() {
		std::cout << "init invoked" << std::endl;
		if (FileSystem::Directory().remove(root_) == false) {
			
			std::cout << "What the fuck" << std::endl;
		}
		if(FileSystem::Directory().exists(root_) == false) std::cout << root_ << " is not existed" << std::endl;
		else std::cout << root_ << " is exist" << std::endl;
		FileSystem::Directory().create(root_);
	}
}
#endif // SWREPOCORE_H

