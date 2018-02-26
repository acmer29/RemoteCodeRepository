#pragma once
#ifndef CHECKOUT_H
#define CHECKOUT_H
#include <regex>
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../NoSqlDb/Persistence/Persisence.h"
#include "../NoSqlDb/Query/Query.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
namespace SWRTB {
	class Checkout {
	public:
		explicit Checkout(Core& target, const std::string& targetDirectory = "../repoCheckout/");

		Checkout& restore(const std::string& pathFileName = "");

		void checkout(const std::string& fileNameVersion);

	private:
		Core& repo;

		// Query Helper
		DbQuery::queryResult<std::string> querier;

		// Restore Helper
		DbPersistence::persistence<std::string> persistor;

		// FileSystem helpers
		FileSystem::Path pathHelper;
		FileSystem::Directory dirHelper;

		// Directories
		std::string sourceDirectory;
		std::string targetDirectory;

		bool canCheckout(const std::string& fileNameVersion);
		void copyFile(const std::string& fromPath, const std::string& toPath);
		std::string removeVersion(const std::string& fileNameVersion);

	};
}
#endif // !CHECKOUT_H
