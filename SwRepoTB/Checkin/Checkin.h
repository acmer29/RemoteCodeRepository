#pragma once
#ifndef CHECKIN_H
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../NoSqlDb/Query/Query.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
namespace SWRTBCheckin {
	class Checkin {
	public:
		using Repo = NoSqlDb::DbCore<std::string>;

		explicit Checkin(Repo& target) : repo(target) {}
		
		void checkin(const std::string& path);
		void dependencySetter(const std::string& dependency);
		void descriptionSetter(const std::string& description);
		void categorySetter(const std::string& category);

	private:
		Repo& repo;

		//Helper functions
		void pathFinder(const std::string& path);
		void versionSolver();
		void versionChecker();
	};


}
#endif // !CHECKIN_H