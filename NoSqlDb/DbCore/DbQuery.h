#pragma once
#ifndef DBQUERY_H
#define DBQUERY_H
#include "DbCore.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
namespace DbQuery {
	template<typename T>
	class queryResult {
	public:
		// Change these names to type less letters.
		using Results = std::vector<NoSqlDb::DbElement<T>>;
		using Instance = NoSqlDb::DbElement<T>;

		explicit queryResult(NoSqlDb::DbCore<T>& target);
		//Copy constructor, used in return value of and(), or(), not() operation.
		explicit queryResult(Results input, NoSqlDb::DbCore<T>& target);

		// Insert operation, accept string format input.
		// shouldStrict = false: If the record contains child that does not exist, 
		//                       create it and add relationship and insert.
		// shouldStrict = true: If the record contains child that does not exist, 
		//                      reject the operation and return false instantly.
		bool insert(NoSqlDb::DbCore<T>& targetDb, std::string record, bool shouldStrict = false);

		Results find(std::string queryString);

		bool remove(std::string queryString);

		bool update(std::string queryString, std::string newValue, bool updateMode = false, bool updateMulti = false);

		/*queryResult<T> and (const queryResult<T>&) const;

		queryResult<T> or (const queryResult<T>&) const;
		
		queryResult<T> not(const queryResult<T>&) const;*/

		Results eval();
	private:
		Results result;
		NoSqlDb::DbCore<T> db;
	};

	template<typename T>
	queryResult<T>::queryResult(NoSqlDb::DbCore<T>& target) {
		db = target;
	}

	template<typename T> 
	queryResult<T>::queryResult(Results input, NoSqlDb::DbCore<T>& target) {
		result = input;
		db = target;
	}
	template<typename T>
	typename queryResult<T>::Results queryResult<T>::eval() {
		return result;
	}

	// Assume the accept string is like ("name", "", [], ["value", ""]), (without parentheses).
	// The second metadata - description, can presents or absents, as well as the catagory parameter.
	// The two pairs of bracket must present, the first represents to the children names, 
	// The second represents to the payload array, contains payload value and catagory.
	// All values, if present, must be wrapped with quotes.
	// Any adjacent items, must be seprated with carma.
	// Any violation of above criteria will be rejected.
	template<typename T> 
	bool queryResult<T>::insert(NoSqlDb::DbCore<T>& targetDb, std::string record, bool shouldStrict) {
		// The instance to be insert in db.
		typename queryResult<T>::Instance dbRecord;
		// The vector to contain each data/metadata item.
		std::vector<std::string> data;
		// The vector to contian children names.
		std::vector<std::string> children;
		// The vector to contain payload and catagory (if have).
		std::vector<std::string> payloads;
		// Split raw input into metadata and / or data.
		data = Utilities::splitPlus(record);
		// Rule out the possible invalid input.
		if (data.size() < 3 || data.size() > 4) return false;
		// std::cout << "here!" << std::endl;
		if (data.size() == 4) children = Utilities::split(Utilities::unwrapper(data[2]));
		payloads = Utilities::split((data.size() == 3) ? Utilities::unwrapper(data[2]) : Utilities::unwrapper(data[3]));
		if (payloads.empty() == true || payloads.size() > 2) return false;
		std::string name = Utilities::trim(data[0]);
		if (Utilities::isUnwrappable(name, '\"') || data[0].size() > 2) {
			name = Utilities::unwrapper(name);
			if (db.contains(name)) return false;
			dbRecord.name(name);
		}
		else return false;
		if (data.size() == 4) {
			std::string description = Utilities::trim(data[1]);
			if (Utilities::isUnwrappable(description, '\"')) dbRecord.descrip(unwrapper(description));
			else return false;
		}
		for (int i = 0; i < children.size(); ++i) {
			std::string childName = Utilities::trim(children[i]);
			if (Utilities::isUnwrappable(childName, '\"')) {
				if (db.contains(childName) == false) {
					if (shouldStrict == true) return false;
					else {
						std::string child = "\"" + childName + "\",\"\",[]" + ",[\"\"]";
						insert(targetDb, child);
					}
				}
				dbRecord.children().push_back(Utilities::unwrapper(childName));
			}
			else return false;
		}
		std::string payload = Utilities::trim(payloads[0]);
		if (Utilities::isUnwrappable(payloads[0], '\"')) dbRecord.payLoad(Utilities::unwrapper(payload));
		else return false;
		if (payloads.size() > 1) {
			// TODO: Add Catagory
		}
		
		dbRecord.dateTime(DateTime().now());
		targetDb[name] = dbRecord;
		// std::cout << "\n" << db[name].name() << " " << db[name].descrip() << " " << db[name].payLoad() << std::endl;
		return true;
	}
	
	template<typename T> 
	typename queryResult<T>::Results queryResult<T>::find(std::string queryString) {

	}
}
#endif