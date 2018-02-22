#pragma once
/////////////////////////////////////////////////////////////////////
// Query.h - Implements database query operations, filters,        //
//           and help functions                                    //
// ver 1.3                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - queryResult
* queryResult implements the from, insert, find, orFind, remove, update, drop functions to manipulate the database.
* also provides childOf, hasChild, hasCategory, betweenTime, beforeTime as filter of find result.
* also provides ascendBy, descendBy as decorater of find result.
* also provides resultDisplay, eval as result output and result convertion.

* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* StringUtilities.h, StringUtilities.cpp
* Query.h Query.cpp
*
* Build Process:
* --------------
* devenv NoSqlDb.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 19 Jan 2018 - Add insert and primitive features of find
* ver 1.1 : 24 Jan 2018 - Refactored find, added update, delete, drop, and filters
* ver 1.2 : 26 Jan 2018 - Compelete all filters, split query into a new package
* ver 1.3 : 30 Jan 2018 - Refactored functions.
*/

#ifndef QUERY_H
#define QUERY_H
#include "../DbCore/DbCore.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include <algorithm>
#include <utility>
#include <regex>
#include <chrono>
#include <queue>
namespace DbQuery {
	template<typename T>
	class queryResult {
	public:
		// Change these names to type less letters.
		using Results = std::vector<NoSqlDb::DbElement<T>>;
		using Instance = NoSqlDb::DbElement<T>;
		using kvPair = std::pair<std::string, std::string>;
		using kvPairs = std::vector<kvPair>;

		explicit queryResult(NoSqlDb::DbCore<T>& target);

		queryResult<T>& from(NoSqlDb::DbCore<T>& target);
		queryResult<T>& from(const Results& target);

		queryResult<T>& insert(const std::string& record, bool shouldStrict = false);
		queryResult<T>& insert(const Instance& instance, bool shouldStrict = false);

		queryResult<T>& find(const std::string& queryString = "");
		queryResult<T>& find(const std::string& key, const std::string& value);

		queryResult<T>& orFind(const std::string& key, const std::string& value);

		queryResult<T>& childOf(bool returnAll = false);

		queryResult<T>& beforeTime(const std::string& time);

		queryResult<T>& betweenTime(const std::string& leftBond, const std::string& rightBond);

		queryResult<T>& ascendBy(const std::string& key);

		queryResult<T>& descendBy(const std::string& key);

		bool hasChild(const std::string& child);

		bool hasCategory(const std::string& category);

		queryResult<T>& update(const std::string& newValue);
		queryResult<T>& update(const std::string& key, const std::string& newValue);
		queryResult<T>& update(const Instance& instance);

		void remove(const std::string& queryString = "");

		void drop();

		void resultDisplay();

		Results eval();

	private:
		//data members
		Results result;
		NoSqlDb::DbCore<T>& db;

		//check functions
		bool checker(Instance& element, const std::string& key, const std::string& value);
		bool nameChecker(Instance& element, const std::string& value);
		bool descriptionChecker(Instance& element, const std::string& value);
		bool dateTimeChecker(Instance& element, const std::string& value);
		bool payLoadChecker(Instance& element, const std::string& value);
		bool collectionChecker(Instance& element, const std::string& key, std::string& value);

		//help functions
		kvPairs keyValuePairGenerator(const std::string& queryString, bool updateCheck = false);
		std::pair<std::string, std::string> filterSolver(const std::string& valueWithFilter);
		std::vector<std::string> valueListGenerator(const std::string& children);
		DateTime dateTimeFormatter(const std::string& toFormat);
		std::pair<int, std::string> dateTimeFormatterHelper(const std::string& toFormat);
		void addCollection(Instance& element, const std::string& name, const std::string& value);
		void deleteCollection(Instance& element, const std::string& name, const std::string& value);
		void replaceCollection(Instance& element, const std::string& name, const std::string& value);
	};

	template<typename T>
	queryResult<T>::queryResult(NoSqlDb::DbCore<T>& target) : db(target) {}

	template<typename T>
	queryResult<T>& queryResult<T>::from(NoSqlDb::DbCore<T>& target) {
		result.clear();
		NoSqlDb::DbCore<T>::iterator iter;
		for (iter = target.begin(); iter != target.end(); iter++) {
			result.push_back(iter->second);
		}
		return *this;
	}

	template<typename T>
	queryResult<T>& queryResult<T>::from(const Results& target) {
		result.clear();
		result = target;
		return *this;
	}

	// -----< insert: Insert an Instance described by a string list >------------------
	// Assume the accept string is like ("name", "descip", "child1, child2", "value", "categories1, categories2").
	// If the input is corrent, operation will be rejected but no exception will be throwed.
	template<typename T>
	queryResult<T>& queryResult<T>::insert(const std::string& record, bool shouldStrict) {
		typename queryResult<T>::Instance dbRecord;
		std::vector<std::string> data;
		data = Utilities::splitPlus(record);
		if (data.size() != 5) 
			throw std::exception("\n Insert: Incorrent number arguments given in insert operation.\n");
		dbRecord.name(Utilities::unwrapPlus(Utilities::trim(data[0])));
		if (db.contains(dbRecord.name()) == true) {
			std::cout << "Element with same key exists, insert skip.\n" << std::endl;
			return *this;
		}
		dbRecord.descrip(Utilities::unwrapPlus(Utilities::trim(data[1])));
		dbRecord.children(valueListGenerator(Utilities::unwrapPlus(Utilities::trim(data[2]))));
		dbRecord.payLoad(Utilities::unwrapPlus(Utilities::trim(data[3])));
		dbRecord.category(valueListGenerator(Utilities::unwrapPlus(Utilities::trim(data[4]))));
		dbRecord.dateTime(DateTime().now());
		if (shouldStrict == true) {
			for (auto iter = dbRecord.children().begin(); iter != dbRecord.children().end(); iter++) {
				if (db.contains(*iter) == false) {
					std::cout << "No child " + *iter + " found in target database, insert fail.\n" << std::endl;
					return *this;
				}
			}
		}
		db[dbRecord.name()] = dbRecord;
		return *this;
	}

	// -----< insert: Insert an instance to the database >--------------------------
	template<typename T>
	queryResult<T>& queryResult<T>::insert(const Instance& instance, bool shouldStrict) {
		if (db.contains(instance.name()) == true) {
			std::cout << "Element with same key exists, insert skip.\n" << std::endl;
			return *this;
		}
		if (shouldStrict == true) {
			if (instance.children().size() == 0) return *this;
			for (auto iter = instance.children().begin(); iter != instance.children().end(); iter++) {
				if (db.contains(*iter) == false) {
					std::cout << "No child " + *iter + " found in target database, insert fail.\n" << std::endl;
					return *this;
				}
			}
		}
		db[instance.name()] = instance;
		return *this;
	}

	// -----< find: Using a list of conditions as the query string to query. >--------------------
	// -----< The query string has a basic form as (key1: "value1", key2: "value2") >-------------
	template<typename T>
	queryResult<T>& queryResult<T>::find(const std::string& queryString) {
		if (queryString == "") return *this;
		kvPairs keyValuePair = keyValuePairGenerator(queryString);
		size_t pairLen = keyValuePair.size();
		for (size_t i = 0; i < pairLen; ++i) {
			if (i > 0 && result.size() == 0) return *this;
			if (keyValuePair[i].first == "children") {
				std::vector<std::string> childrenName = valueListGenerator(keyValuePair[i].second);
				size_t nameLen = childrenName.size();
				for (size_t j = 0; j < nameLen; ++j) {
					find(keyValuePair[i].first, childrenName[j]);
				}
			}
			else if (keyValuePair[i].first == "category") {
				std::vector<std::string> categoryName = valueListGenerator(keyValuePair[i].second);
				size_t nameLen = categoryName.size();
				for (size_t j = 0; j < nameLen; ++j) {
					find(keyValuePair[i].first, categoryName[j]);
				}
			}
			else find(keyValuePair[i].first, keyValuePair[i].second);
		}
		return *this;
	}

	// -----< find: Using one key and one value as the query string >-----------------------
	template<typename T>
	queryResult<T>& queryResult<T>::find(const std::string& key, const std::string& expression) {
		if (expression[0] == '$') {
			std::pair<std::string, std::string> filterValue = filterSolver(expression);
			std::string filter = filterValue.first;
			std::string value = filterValue.second;
			if (filter == "before" && key == "dateTime") 
				beforeTime(value);
			else if (filter == "between" && key == "dateTime") {
				std::vector<std::string> bonds = Utilities::splitPlus(value);
				betweenTime(bonds[0], bonds[1]);
			}
			else throw std::exception("\n Find: Unrecognized filter or invalid use of filter.\n");
		}
		else {
			typename queryResult<T>::Results::iterator iter = result.begin();
			while (iter != result.end()) {
				if (checker(*iter, key, expression) == false) {
					iter = result.erase(iter);
				}
				else if (iter != result.end()) iter++;
			}
		}
		return *this;
	}

	// -----< beforeTime: Using for querying specified dateTime form, this filter handles time before now. >-----
	template<typename T>
	queryResult<T>& queryResult<T>::beforeTime(const std::string& time) {
		std::string trueDateTime = std::string(dateTimeFormatter(time));
		typename queryResult<T>::Results::iterator iter = result.begin();
		while (iter != result.end()) {
			if ((*iter).dateTime() > trueDateTime) {
				iter = result.erase(iter);
			}
			else if (iter != result.end()) iter++;
		}
		return *this;
	}

	// -----< betweenTime: Using for querying specified dateTime form, this filter handles time between two bonds. >-----
	template<typename T>
	queryResult<T>& queryResult<T>::betweenTime(const std::string& leftBond, const std::string& rightBond) {
		DateTime leftAfter = std::string(dateTimeFormatter(Utilities::trim(leftBond)));
		DateTime rightAfter = std::string(dateTimeFormatter(Utilities::trim(rightBond)));
		if (leftAfter > rightAfter) std::swap(leftAfter, rightAfter);
		typename queryResult<T>::Results::iterator iter = result.begin();
		while (iter != result.end()) {
			if ((*iter).dateTime() > rightAfter || (*iter).dateTime() < leftAfter) {
				iter = result.erase(iter);
			}
			else if (iter != result.end()) iter++;
		}
		return *this;
	}

	// -----< orFind: Using for querying "or" relationship, this function usage is same as the second "find" >-----
	template<typename T>
	queryResult<T>& queryResult<T>::orFind(const std::string& key, const std::string& value) {
		NoSqlDb::DbCore<T> previousResult;
		typename queryResult<T>::Results::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			previousResult[(*iter).name()] = *iter;
		}
		from(db).find(key, value);
		for (iter = result.begin(); iter != result.end(); iter++) {
			if (previousResult.contains((*iter).name()) == false) {
				previousResult[(*iter).name()] = *iter;
			}
		}
		result.clear();
		NoSqlDb::DbCore<T>::iterator resIter;
		for (resIter = previousResult.begin(); resIter != previousResult.end(); resIter++) {
			result.push_back(resIter->second);
		}
		return *this;
	}

	// -----< childOf: Using for querying the child of the element >--------------
	// -----< if returnAll == true, it will return all descendent of the element >-----
	template<typename T>
	queryResult<T>& queryResult<T>::childOf(bool returnAll) {
		std::queue<Instance> childQue;
		NoSqlDb::DbCore<T> childMap;
		for (Results::iterator iter = result.begin(); iter != result.end(); iter++) {
			for (std::vector<std::string>::iterator childIter = iter->children().begin(); childIter != iter->children().end(); childIter++) {
				if (db.contains(*childIter) && !childMap.contains(*childIter)) {
					childQue.push(db[*childIter]);
					childMap[*childIter] = db[*childIter];
				}
			}
		}
		if (returnAll) {
			while (!childQue.empty()) {
				auto tmp = childQue.front();
				childQue.pop();
				for (std::vector<std::string>::iterator iter = tmp.children().begin(); iter != tmp.children().end(); iter++) {
					if (db.contains(*iter) && !childMap.contains(*iter)) {
						childQue.push(db[*iter]);
						childMap[*iter] = db[*iter];
					}
					else if (childMap.contains(*iter) == false) {
						std::cout << "No child " << (*iter) << " found in the datebase, maybe it has been deleted in early operation , remove this relationship now" << std::endl;
						iter = tmp.children().erase(iter);
					}
				}
			}
		}
		result.clear();
		NoSqlDb::DbCore<T>::iterator iter = childMap.begin();
		while (iter != childMap.end()) {
			result.push_back(iter->second);
			iter++;
		}
		return *this;
	}

	// -----< ascendBy: Sort the result in ascend order by spceified key >-----
	template<typename T>
	queryResult<T>& queryResult<T>::ascendBy(const std::string& key) {
		if (key == "name") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.name() < b.name(); });
		else if (key == "description") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.descrip() < b.descrip(); });
		else if(key == "dateTime") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.dateTime() < b.dateTime(); });
		else if(key == "payLoad") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.payLoad() < b.payLoad(); });
		else if (key == "children" || key == "category") throw std::exception("Ascend by: Index cannot use for sort.\n");
		else throw std::exception("\n Ascend by: Unrecognized sort index.\n");
		return *this;
	}

	// -----<descendBy: Sort the result in descend order by specified key >-----
	template<typename T>
	queryResult<T>& queryResult<T>::descendBy(const std::string& key) {
		if (key == "name") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.name() > b.name(); });
		else if (key == "description") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.descrip() > b.descrip(); });
		else if (key == "dateTime") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.dateTime() > b.dateTime(); });
		else if (key == "payLoad") std::sort(result.begin(), result.end(), [](const Instance& a, const Instance& b) { return a.payLoad() > b.payLoad(); });
		else if (key == "children" || key == "category") throw("Descend by: Index cannot use for sort.\n");
		else throw std::exception("\n Descend by: Unrecognized sort index.\n");
		return *this;
	}

	// -----< hasChild: Check if the element(s) has the specified child >-----------
	template<typename T>
	bool queryResult<T>::hasChild(const std::string& child) {
		for (Results::iterator iter = result.begin(); iter != result.end(); iter++) {
			for (std::vector<std::string>::iterator childIter = iter->children().begin(); childIter != iter->children().end(); childIter++) {
				if (child == *childIter) return true;
			}
		}
		return false;
	}

	// -----< hasCategory: Check if the element(s) has the specified category >-----
	template<typename T>
	bool queryResult<T>::hasCategory(const std::string& category) {
		for (Results::iterator iter = result.begin(); iter != result.end(); iter++) {
			for (std::vector<std::string>::iterator cateIter = iter->category().begin(); cateIter != iter->category().end(); cateIter++) {
				if (category == *cateIter) return true;
			}
		}
		return false;
	}

	// -----< update: Update one or a set of instances with a set of new values >-----
	// -----< The newValue is a set of key-value pairs. >----------------
	// -----< The format of the set is exactly same with the query string using in the first find >-----
	template<typename T>
	queryResult<T>& queryResult<T>::update(const std::string& newValue) {
		kvPairs keyValuePair = keyValuePairGenerator(newValue, true);
		size_t pairLen = keyValuePair.size();
		for (size_t i = 0; i < pairLen; ++i) {
			update(keyValuePair[i].first, std::string(keyValuePair[i].second));
		}
		return *this;
	}

	// -----< update: Update one or a set of instances with one field and one new value >--------
	template<typename T>
	queryResult<T>& queryResult<T>::update(const std::string& key, const std::string& newValue) {
		for (auto iter = result.begin(); iter != result.end(); iter++) {
			if (key == "name")
				throw std::exception("\n Editing name is forbidden.");
			else if (key == "description") {
				db[(*iter).name()].descrip(newValue);
			}
			else if (key == "dateTime") {
				db[(*iter).name()].dateTime(newValue);
			}
			else if (key == "payLoad") {
				db[(*iter).name()].payLoad(newValue);
			}
			else if (key == "children" || key == "category") {
				if (newValue.size() == 0) return *this;
				// Solve the filter-contained expression
				if (newValue[0] == '$') {
					std::pair<std::string, std::string> filterChildren = filterSolver(newValue);
					std::string filter = filterChildren.first;
					if (filter == "remove") deleteCollection(db[(*iter).name()], key, filterChildren.second);
					else if (filter == "add") addCollection(db[(*iter).name()], key, filterChildren.second);
					else throw std::exception("\n Update: filter usage invalid in new value.");
				}
				else replaceCollection(db[(*iter).name()], key, newValue);
			}
			else throw std::exception("\n Update: Unrecognized column name.");
		}
		return *this;
	}

	// -----< update: Update an element replace all its data to new one. >-----
	template<typename T>
	queryResult<T>& queryResult<T>::update(const Instance& instance) {
		if (db.contains(instance.name()) == false) throw std::exception("\n Update: No element find with name given.\n");
		Instance& toUpdate = db[instance.name()];
		toUpdate.descrip(instance.descrip());
		toUpdate.dateTime(instance.dateTime());
		toUpdate.payLoad(instance.payLoad());
		toUpdate.category(instance.category());
		toUpdate.children(instance.children());
		return *this;
	}

	// -----< remove: Remove the element either from the previous find results, or find by queryString >-----
	// -----< If queryString != "", the function will only remove the result found by queryString >----------
	template<typename T>
	void queryResult<T>::remove(const std::string& queryString) {
		if (queryString != "") find(queryString);
		typename queryResult<T>::Results::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			if (db.contains((*iter).name())) db.dbStore().erase((*iter).name());
		}
		return;
	}

	// -----< drop: Remove all records in the database >-----
	template<typename T>
	void queryResult<T>::drop() {
		db.dbStore().clear();
		return;
	}

	// -----< resultDisplay: Print all elements in the database, or the result of the operation. >-----
	template<typename T>
	void queryResult<T>::resultDisplay() {
		NoSqlDb::showHeader();
		typename queryResult<T>::Results::iterator iter;
		std::ostream& out = std::cout;
		for (iter = result.begin(); iter != result.end(); iter++) {
			out << "\n  ";
			out << std::setw(26) << std::left << std::string((*iter).dateTime());
			out << std::setw(10) << std::left << (*iter).name();
			out << std::setw(25) << std::left << (*iter).descrip();
			out << std::setw(25) << std::left << (*iter).payLoad();
			typename NoSqlDb::DbElement<T>::Children children = (*iter).children();
			typename NoSqlDb::DbElement<T>::Children category = (*iter).category();
			if (children.size() > 0)
			{
				out << "\n    child keys: ";
				for (auto key : children)
				{
					out << " " << key;
				}
			}
			if (category.size() > 0)
			{
				out << "\n    category: ";
				for (auto key : category)
				{
					out << " " << key;
				}
			}
		}
		std::cout << std::endl;
		return;
	}

	// -----< eval: Convert the query result into std::vector<NoSqlDb::DbElement<T>> >-----
	template<typename T>
	typename queryResult<T>::Results queryResult<T>::eval() {
		return result;
	}

	// -----< checker: The entry of all checkers operation used by find, internal access only. >------
	template<typename T>
	bool queryResult<T>::checker(Instance& element, const std::string& key_, const std::string& value_) {
		std::string value = value_;
		std::string key = key_;
		if (key == "name") return nameChecker(element, value);
		else if (key == "description") return descriptionChecker(element, value);
		else if (key == "dateTime") return dateTimeChecker(element, value);
		else if (key == "children" || key == "category") return collectionChecker(element, key, value);
		else if (key == "payLoad") return payLoadChecker(element, value);
		else throw std::exception("\n Checker: Query value invalid at.\n");
	}

	// -----< nameChecker: Check if the name of the element equal to the value_ >-----
	template<typename T>
	bool queryResult<T>::nameChecker(Instance& element, const std::string& value_) {
		std::string value = value_;
		if (Utilities::checkWrapper(value, '/') == true) {
			std::regex regExp(Utilities::unwrapPlus(value, '/'));
			if (std::regex_match(element.name(), regExp)) return true;
			else return false;
		}
		else if (element.name() == value) return true;
		else return false;
	}

	// -----< desciptionChecker: Check if the description of the element equal to the value_ >-----
	template<typename T>
	bool queryResult<T>::descriptionChecker(Instance& element, const std::string& value_) {
		std::string value = value_;
		if (Utilities::checkWrapper(value_, '/') == true) {
			std::regex regExp(Utilities::unwrapPlus(value_, '/'));
			if (std::regex_match(element.descrip(), regExp)) return true;
			else return false;
		}
		else if (element.descrip() == value) return true;
		else return false;
	}

	// -----< dateTimeChecker: Check if the dateTime of the element equal to the value >-----
	template<typename T>
	bool queryResult<T>::dateTimeChecker(Instance& element, const std::string& value_) {
		std::string value = value_;
		if (Utilities::checkWrapper(value, '/') == true) {
			std::regex regExp(Utilities::unwrapPlus(value, '/'));
			if (std::regex_match(std::string(element.dateTime()), regExp)) return true;
			else return false;
		}
		else if (std::string(element.dateTime()) == value) return true;
		else return false;
	}

	// -----< payLoadChecker: Check if the string part of payLoad of the element equal to the value >-----
	template<typename T>
	bool queryResult<T>::payLoadChecker(Instance& element, const std::string& value_) {
		std::string value = value_;
		if (Utilities::checkWrapper(value, '/') == true) {
			std::regex regExp(Utilities::unwrapPlus(value, '/'));
			if (std::regex_match(element.payLoad(), regExp)) return true;
			else return false;
		}
		else if (element.payLoad() == value) return true;
		else return false;
	}

	// -----< collectionChecker: Check if the children or category of the element has the value >-----
	template<typename T>
	bool queryResult<T>::collectionChecker(Instance& element, const std::string& key_, std::string& value_) {
		std::string value = value_;
		std::string key = key_;
		std::vector<std::string>& candidate = (key == "children" ? element.children() : element.category());
		if (Utilities::checkWrapper(value, '/') == true) {
			std::regex regExp(Utilities::unwrapPlus(value, '/'));
			for (size_t i = 0; i < candidate.size(); ++i) {
				if (std::regex_match(candidate[i], regExp) == true) {
					return true;
				}
			}
		}
		else {
			for (size_t i = 0; i < candidate.size(); ++i) {
				if (value == candidate[i]) {
					return true;
				}
			}
		}
		return false;
	}

	// -----< keyValuePairGenerator: Help function for generate a list of key-value pair from a string which contains them >-----
	template<typename T>
	typename queryResult<T>::kvPairs queryResult<T>::keyValuePairGenerator(const std::string& queryString, bool updateCheck) {
		std::vector<std::string> conditions;
		kvPairs keyValuePair;
		conditions = Utilities::splitPlus(Utilities::trim(queryString));
		size_t items = conditions.size();
		for (size_t i = 0; i < items; ++i) {
			std::vector<std::string> condition;
			condition = Utilities::splitPlus(Utilities::trim(conditions[i]), ':');
			if (condition.size() != 2) throw std::exception("\n keyValuePairGenerator: invalid query at.\n");
			condition[0] = Utilities::trim(condition[0]);
			if (condition[0] == "name" && updateCheck == true) throw std::exception("\n Update: Name column updating is forbidden");
			condition[1] = Utilities::trim(condition[1]);
			condition[1] = Utilities::unwrapPlus(condition[1], '\"');
			keyValuePair.push_back(kvPair(condition[0], condition[1]));
		}
		return keyValuePair;
	}

	// -----< filterSolver: Solve the filter expression into pair "filter + value" >--------------------
	template<typename T>
	std::pair<std::string, std::string> queryResult<T>::filterSolver(const std::string& valueWithFilter) {
		std::regex filterExp("\\$\\([a-z]*\\)\\s*\\(.*\\)");
		if (std::regex_match(valueWithFilter, filterExp) == false) throw std::exception("\n Filter Solver: Invalid usage of filter at.\n");
		std::smatch matches;
		std::regex_search(valueWithFilter, matches, std::regex("\\$\\([a-z]*\\)"));
		std::string filter = std::string(*(matches.begin()));
		std::string expression = valueWithFilter.substr(filter.length(), valueWithFilter.length());
		filter = filter.substr(2, filter.length() - 3);
		expression = Utilities::unwrapPlus(Utilities::trim(expression), '(');
		return std::make_pair(filter, expression);
	}

	// -----< valueListGenerator: Split a list of conditions into a vector of key value pair >-----
	template<typename T>
	std::vector<std::string> queryResult<T>::valueListGenerator(const std::string& values) {
		std::vector<std::string> valueName;
		valueName = Utilities::split(values);
		for (size_t i = 0; i < valueName.size(); ++i) {
			valueName[i] = Utilities::trim(valueName[i]);
		}
		std::sort(valueName.begin(), valueName.end(), [](std::string a, std::string b) { return a < b; });
		return valueName;
	}

	// -----< dateTimeFormatter: Format the speicified dateTime type into original dateTime type. >-----
	// -----< Accept string like "1 day" (number + time unit) >------------
	template<typename T>
	DateTime queryResult<T>::dateTimeFormatter(const std::string& toFormat_) {
		std::string toFormat = toFormat_;
		toFormat = Utilities::trim(toFormat);
		if (std::regex_match(toFormat, std::regex("[A-Z][a-z][a-z]\\s[A-Z][a-z][a-z]\\s\\d\\d\\s\\d\\d:\\d\\d:\\d\\d\\s\\d\\d\\d\\d"))) {
			return DateTime(toFormat);
		}
		if (toFormat == "now") return DateTime().now();
		std::pair<int, std::string> toFormatAfter = dateTimeFormatterHelper(toFormat);
		int timeNum = toFormatAfter.first;
		std::string timeUnit = toFormatAfter.second;
		// counting the seconds of specified time format
		size_t lavarage;
		if (timeUnit == "year" || timeUnit == "years") { lavarage = (size_t)(3.154e+7); }
		else if (timeUnit == "month" || timeUnit == "monthes") { lavarage = (size_t)(2.628e+6); }
		else if (timeUnit == "week" || timeUnit == "weeks") { lavarage = 604800; }
		else if (timeUnit == "day" || timeUnit == "days") { lavarage = 86400; }
		else if (timeUnit == "hour" || timeUnit == "hours") { lavarage = 3600; }
		else if (timeUnit == "minute" || timeUnit == "minutes") { lavarage = 60; }
		else if (timeUnit == "second" || timeUnit == "seconds") { lavarage = 1; }
		else throw std::exception("\n Date Time Formatter: Unrecognized time unit.\n");
		std::chrono::seconds seconds(lavarage* timeNum);
		DateTime ans(DateTime().now());
		ans = ans - ans.makeDuration(0, 0, size_t(seconds.count()), 0);
		return ans;
	}

	// -----< dateTimeFormatterHelper: The helper function of dateTimeFormatter >-------------------
	template<typename T>
	std::pair<int, std::string> queryResult<T>::dateTimeFormatterHelper(const std::string& toFormat_) {
		std::string toFormat = toFormat_;
		std::string num = "";
		std::string timeUnit = "";
		size_t i = 0, strLen = toFormat.length();
		for (; i < strLen; ++i) {
			if ('0' <= toFormat[i] && toFormat[i] <= '9') num += toFormat[i];
			else break;
		}
		if (i >= strLen) throw std::exception("\n Please indicate the time unit and please do not include \'-\' or \'+\' .\n");
		if (i == 0) throw std::exception("\n Please indicate the amount of time.\n");
		int timeNum = std::stoi(num);
		timeUnit = Utilities::trim(toFormat.substr(i, toFormat.length()));
		return make_pair(timeNum, timeUnit);
	}

	// -----< addCollection: Handles add child / category operation, avoid adding duplicate value >---------
	template<typename T>
	void queryResult<T>::addCollection(Instance& element, const std::string& name, const std::string& value) {
		std::vector<std::string>& targetVector = (name == "children" ? element.children() : element.category());
		std::vector<std::string> valueName = valueListGenerator(value);
		std::vector<std::string>::iterator p;
		std::vector<std::string>::iterator q;
		p = targetVector.begin();
		q = valueName.begin();
		std::vector<std::string> tmp;
		// To avoid adding duplicate value, use two pointers like merge sort
		while (q != valueName.end() && p != targetVector.end()) {
			if (*q < *p) { tmp.push_back(*q); q++; }
			else if (*q > *p) { tmp.push_back(*p); p++; }
			else { tmp.push_back(*p); p++, q++; }
		}
		while (p != targetVector.end()) { tmp.push_back(*p); p++; }
		while (q != valueName.end()) { tmp.push_back(*q); q++; }
		targetVector = tmp;
		return;
	}

	// -----< deleteCollection: Handles remove child / category operation, avoid remove duplicate value >-------
	template<typename T>
	void queryResult<T>::deleteCollection(Instance& element, const std::string& name, const std::string& value) {
		std::vector<std::string>& targetVector = (name == "children" ? element.children() : element.category());
		std::vector<std::string> valueName = valueListGenerator(value);
		std::vector<std::string>::iterator p;
		std::vector<std::string>::iterator q;
		p = targetVector.begin();
		q = valueName.begin();
		// use two pointers to avoid delete duplicate elements
		while (q != valueName.end() && p != targetVector.end()) {
			if (*q < *p) q++;
			else if (*q > *p) p++;
			else { p = targetVector.erase(p); q++; }
		}
		return;
	}

	// -----< replaceCollection: Handles replace children / category operation >-------------------------------
	template<typename T>
	void queryResult<T>::replaceCollection(Instance& element, const std::string& name, const std::string& value) {
		std::vector<std::string>& targetVector = (name == "children" ? element.children() : element.category());
		std::vector<std::string> valueName = valueListGenerator(value);
		targetVector.clear();
		for (size_t j = 0; j < valueName.size(); ++j) {
			targetVector.push_back(valueName[(int)j]);
		}
		return;
	}
}
#endif