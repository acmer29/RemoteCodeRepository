#pragma once
#ifndef SWREPOCORE_H
#include <iostream>
#include <vector>
#include <string>
#include "../NoSqlDb/DbCore/DbCore.h"
#include "../NoSqlDb/Utilities/StringUtilities/StringUtilities.h"
namespace SWRTBCore{
	class Core {
	public:
		using Repo = NoSqlDb::DbCore<std::string>;

		Repo& repo() { return repo_; }
		Repo repo() const { return repo_;  }
		void repo(const Repo& repo) { repo_ = repo; }

	private:
		NoSqlDb::DbCore<std::string> repo_;
	};
}
#endif // SWREPOCORE_H

