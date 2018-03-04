#pragma once
////////////////////////////////////////////////////////////////////////////
// Checkout.h - Provide means for checkout files						  //
// ver 1.1																  //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018                //
////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - Checkout
* This class provides means to checkout files from checkin working directory.
* This class provides two pulice functions: 
*  - relocateDirectory() : Change working directory of checkout
*  - checkout() : Copy files to the checkout working directory

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
* ver 1.1 : 03 Mar 2018 - Remove useless functions
*/
#ifndef CHECKOUT_H
#define CHECKOUT_H
#include <regex>
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../SWRTBUtilities/SWRTBUtilities.h"
namespace SWRTB {
	class Checkout {
	public:
		explicit Checkout(Core& target, const std::string& targetDirectory = "../repoCheckout/");

		Checkout& relocateDirectory(const std::string& newDirectory);

		void checkout(const std::string& NSPfileNameVersion, bool recursive = true);

	private:
		Core& repo;

		// Query Helper
		DbQuery::queryResult<std::string> querier;

		// FileSystem helpers
		FileSystem::Path pathHelper;
		FileSystem::Directory dirHelper;

		// Directories
		std::string sourceDirectory;
		std::string targetDirectory;

		// Helper functions
		std::string removeVersion(const std::string& fileNameVersion);

	};
}
#endif // CHECKOUT_H
