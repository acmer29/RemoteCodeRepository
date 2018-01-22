#pragma once
#ifndef DBQUERY_H
#define DBQUERY_H
#include "DbCore.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include <algorithm>
#include <utility>
namespace DbQuery {
	template<typename T>
	class queryResult {
	public:
		// Change these names to type less letters.
		using Results = std::vector<NoSqlDb::DbElement<T>>;
		using Instance = NoSqlDb::DbElement<T>;
		using kvpair = std::pair<std::string, std::string>;

		explicit queryResult(NoSqlDb::DbCore<T>& target);
		//Copy constructor, used in return value of and(), or(), not() operation.
		explicit queryResult(Results input, NoSqlDb::DbCore<T>& target);

		// Insert operation, accept string format input.
		// shouldStrict = false: If the record contains child that does not exist, 
		//                       create it and add relationship and insert.
		// shouldStrict = true: If the record contains child that does not exist, 
		//                      reject the operation and return false instantly.
		bool insert(NoSqlDb::DbCore<T>& targetDb, std::string record, bool shouldStrict = false);

		Results find(NoSqlDb::DbCore<T>& targetDb, std::string queryString);

		bool remove(std::string queryString);

		bool update(std::string queryString, std::string& newValue, bool updateMode = false, bool updateMulti = false);

		/*queryResult<T> and (const queryResult<T>&) const;

		queryResult<T> or (const queryResult<T>&) const;
		
		queryResult<T> not(const queryResult<T>&) const;*/

		Results eval();

		void resultDisplay(Results& toOutput);
	private:
		Results result;
		NoSqlDb::DbCore<T> db;
		bool checker(Instance& element, std::string& key, std::string& value);
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
		std::sort(dbRecord.children().begin(), dbRecord.children().end(), [](std::string a, std::string b) {return a < b; });
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
	
	// The queryString has a basic form - (column: "expression") (no parentheses)
	// Multiple conditions are sepreated with ",". Ex: name: "name", description: "description"
	// column MUST NOT be wrapped with quotes and expression MUST be wrapped with quotes.
	// Children names MUST be wrapped with brackets. Ex: children: "[child1, child2]"
	// column: the key of the data. Parameter of column are {"name", "dataTime", "description", "children", "payload"}
	//         Any string that cannot match the items in array will consider as invalid key, which will result of empty return value.
	// expression: the expression of the condition of query, which can be represented into three forms:
	//			   1. Exactly the string, the string can be a string or a "[]" wrapped array. 
	//				  ex: name: "name", children: "["child1", "child2"]"
	//			   2. Part of the string, like SQL, use "*" for 0 or multiple characters, and use "?" for exactly one charachter
	//				  ex: name: "n*me", dateTime: "$Jan-20-2018$"
	//			   3. Regular expression, represent as /exp/.
	//				  ex: name: "/[A-Z]*/"
	// Mainwhile, expression can contain filters, start with "$".
	// 
	template<typename T> 
	typename queryResult<T>::Results queryResult<T>::find(NoSqlDb::DbCore<T>& targetDb, std::string queryString) {
		if (queryString == "") {
			NoSqlDb::DbCore<T>::iterator iter = targetDb.begin();
			while (iter != targetDb.end()) {
				result.push_back(iter->second);
				iter++;
			}
			return result;
		}
		std::vector<std::string> conditions;
		std::vector<kvpair> keyValuePair;
		conditions = Utilities::splitPlus(queryString);
		int items = conditions.size();
		for (int i = 0; i < items; ++i) {
			std::vector<std::string> condition;
			condition = Utilities::splitPlus(conditions[i], ':');
			if (condition.size() != 2) throw("invalid query at: " + condition[i]);
			condition[0] = Utilities::trim(condition[0]);
			condition[1] = Utilities::trim(condition[1]);
			condition[1] = Utilities::unwrapPlus(condition[1], '\"');
			// std::cout << condition[0] << " " << condition[1] << std::endl;
			keyValuePair.push_back(kvpair(condition[0], condition[1]));
		}
		
		std::sort(keyValuePair.begin(), keyValuePair.end(), [](kvpair a, kvpair b) { return a.first > b.first; });
		int pairLen = keyValuePair.size();
		for (int i = 0; i < pairLen; ++i) {
			if (i > 0 && result.size() == 0) return result;
			if (keyValuePair[i].first == "name") {
				if (targetDb.contains(keyValuePair[i].second)) result.push_back(targetDb[keyValuePair[i].second]);
			}
			else if (keyValuePair[i].first == "description") {
				if (result.size() == 0) {
					NoSqlDb::DbCore<T>::iterator iter = targetDb.begin();
					while (iter != targetDb.end()) {
						if (targetDb[iter->first].descrip() == keyValuePair[i].second) result.push_back(targetDb[iter->first]);
						iter++;
					}
				}
				else {
					for (int j = 0; j < result.size(); ++j) {
						if (result[j].descrip() != keyValuePair[i].second) {
							typename queryResult<T>::Results::iterator iter = result.begin() + j;
							result.erase(iter);
						}
					}
				}
			}
			else if (keyValuePair[i].first == "dateTime") {
				if (result.size() == 0) {
					NoSqlDb::DbCore<T>::iterator iter = targetDb.begin();
					while (iter != targetDb.end()) {
						if (std::string(targetDb[iter->first].dateTime()) == keyValuePair[i].second) result.push_back(targetDb[iter->first]);
						iter++;
					}
				}
				else {
					for (int j = 0; j < result.size(); ++j) {
						if (std::string(result[j].dateTime()) != keyValuePair[i].second) {
							typename queryResult<T>::Results::iterator iter = result.begin() + j;
							result.erase(iter);
						}
					}
				}
			}
			else if (keyValuePair[i].first == "children") {
				if (result.size() == 0) {
					NoSqlDb::DbCore<T>::iterator iter = targetDb.begin();
					while (iter != targetDb.end()) {
						if (checker(targetDb[iter->first], keyValuePair[i].first, keyValuePair[i].second)) result.push_back(targetDb[iter->first]);
						iter++;
					}
				}
				else {
					for (int j = 0; j < result.size(); ++j) {
						if (checker(result[j], keyValuePair[i].first, keyValuePair[i].second) == false) {
							typename queryResult<T>::Results::iterator iter = result.begin() + j;
							result.erase(iter);
						}
					}
				}
			}
		}
		return result;
	}
	
	// The checker operation of find, internal access only (private function).
	// key: "name", "description", "dateTime", "children" (all without quotes). 
	// Any string except these will throw exception.
	// value: string without quotes, except corresponding value of children, which will be a "[]" wrapped array, 
	//        each element in array will wrapped with "\""
	// Ex: key = "name", value = "name" (all without quotes).
	//	   key = "children", value = "["childname1", "childname2"]" (all without outside quotes).
	template<typename T>
	bool queryResult<T>::checker(typename queryResult<T>::Instance& element, std::string& key, std::string& value) {
		std::cout << value << " in checker function" << std::endl;
		if (key == "name") {
			if (element.name() == value) return true;
		}
		else if (key == "description") {
			if (element.descrip() == value) return true;
		}
		else if (key == "dateTime") {
			if (std::string(element.dateTime()) == value) return true;
		}
		else if (key == "children") {
			std::vector<std::string> childrenName;
			std::string valueAfter = Utilities::unwrapPlus(value, '[');
			childrenName = Utilities::split(valueAfter);
			for (int i = 0; i < childrenName.size(); ++i) {
				childrenName[i] = Utilities::trim(childrenName[i]);
				childrenName[i] = Utilities::unwrapPlus(childrenName[i]);
			}
			std::sort(childrenName.begin(), childrenName.end(), [](std::string a, std::string b) {return a < b; });
			std::vector<std::string> candidate = element.children();
			int i = 0, j = 0, len1 = childrenName.size(), len2 = candidate.size();
			while (i < len1 && j < len2) {
				if (childrenName[i] == candidate[j]) {
					i += 1;
					j += 1;
				}
				else j += 1;
			}
			if (i == len1) {
				std::cout << element.name() << std::endl;
				return true;
			}
			else return false;
		}
		else {
			throw("Query key invalid at: " + key);
		}
	}

	template<typename T>
	void queryResult<T>::resultDisplay(typename queryResult<T>::Results& toOutput) {
		std::cout << "The query returns " << toOutput.size() << " result(s)" << std::endl;
		showHeader(std::cout);
		int vectorLen = toOutput.size();
		std::ostream& out = std::cout;
		for (int i = 0; i < vectorLen; ++i) {
			out << "\n  ";
			out << std::setw(26) << std::left << std::string(toOutput[i].dateTime());
			out << std::setw(10) << std::left << toOutput[i].name();
			out << std::setw(25) << std::left << toOutput[i].descrip();
			out << std::setw(25) << std::left << toOutput[i].payLoad();
			typename NoSqlDb::DbElement<T>::Children children = toOutput[i].children();
			if (children.size() > 0)
			{
				out << "\n    child keys: ";
				for (auto key : children)
				{
					out << " " << key;
				}
			}
		}
		std::cout << std::endl;
		return ;
	}
}
#endif