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
*
* The public interface function operations are as below:
*  - relocateDirectory : Change working directory of checkout
*  - checkout : Copy files to the checkout working directory

* Required Files:
* ---------------
* SWRepoCore.h
* SWRTBUtilities.h
* FileSystem.h, FileSystem.cpp
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
*
* Build Process:
* --------------
* devenv SoftwareRepoTB.sln /rebuild debug
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

		Checkout& setRequestor(const std::string& requestor);

		void checkout(const std::string& NSNFileNameVersion, const std::string& requestor, bool recursive = true);
		void checkout(const std::string& NSNfileNameVersion, bool recursive = true);

		std::vector<std::string> successCheckouts();
		std::vector<std::string> failCheckouts();

	private:
		Core& repo;

		// Checkout requestor
		std::string requestor_;

		// Query Helper
		NoSqlDb::DbQuery<std::string> querier;

		// FileSystem helpers
		FileSystem::Path pathHelper;
		FileSystem::Directory dirHelper;

		// Directories
		std::string sourceDirectory;
		std::string targetDirectory;

		// Helper functions
		std::string removeVersion(const std::string& fileNameVersion);

		// fileName Helpers
		// NSPFileName manner
		std::vector<std::string> successCheckoutFiles;
		std::vector<std::string> failCheckoutFiles;

	};
}
#endif // CHECKOUT_H
