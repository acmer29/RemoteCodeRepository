#pragma once
#ifndef CHECKOUT_H
#define CHECKOUT_H
#include <regex>
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../SWRTBUtilities/SWRTBUtilities.h"
namespace SWRTB {
	class Checkout {
	public:
		explicit Checkout(Core& target, const std::string& targetDirectory = "../repoCheckout/");

		void checkout(const std::string& NSPfileNameVersion);

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
		bool canCheckout(const std::string& fileNameVersion);
		std::string removeVersion(const std::string& fileNameVersion);

	};
}
#endif // CHECKOUT_H
