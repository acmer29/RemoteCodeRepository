#pragma once
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

		explicit queryResult(NoSqlDb::DbCore<T>& target) : db(target) { hasSource = true; }

		queryResult<T>& from(NoSqlDb::DbCore<T>& target);
		queryResult<T>& from(const Results& target);

		queryResult<T>& insert(const std::string& record, bool shouldStrict = false);
		queryResult<T>& insert(Instance& instance, bool shouldStrict = false);

		queryResult<T>& find(const std::string& queryString = "");
		queryResult<T>& find(const std::string& key, const std::string& value);

		queryResult<T>& orFind(const std::string& key, const std::string& value);

		queryResult<T>& childOf(bool returnAll = false);

		queryResult<T>& beforeTime(const std::string& time);

		queryResult<T>& betweenTime(const std::string& leftBond, const std::string& rightBond);

		queryResult<T>& ascendBy(const std::string& key);

		queryResult<T>& descendBy(const std::string& key);

		bool hasChild(const std::string& child);

		void update(const std::string& newValue);
		void update(Instance& instance);

		void remove(const std::string& queryString = "");

		void drop();

		void resultDisplay();

		Results& eval();

	private:
		//data members
		Results result;
		NoSqlDb::DbCore<T>& db;
		bool hasSource = false;

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
	typename queryResult<T>& queryResult<T>::from(NoSqlDb::DbCore<T>& target) {
		result.clear();
		NoSqlDb::DbCore<T>::iterator iter;
		for (iter = target.begin(); iter != target.end(); iter++) {
			result.push_back(iter->second);
		}
		hasSource = true;
		return *this;
	}

	template<typename T>
	typename queryResult<T>& queryResult<T>::from(const Results& target) {
		result.clear();
		result = target;
		hasSource = true;
		return *this;
	}

	// Assume the accept string is like ("name", "descip", "child1, child2", "value", "categories1, categories2").
	template<typename T>
	typename queryResult<T>& queryResult<T>::insert(const std::string& record, bool shouldStrict) {
		if (hasSource == false) throw std::exception("\n Insert: Cannot insert to empty db, do you forget add \"from\"?\n");
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
			for (std::vector<std::string>::iterator iter = dbRecord.children().begin(); iter != dbRecord.children().end(); iter++) {
				if (db.contains(*iter) == false) {
					std::cout << "No child " + *iter + " found in target database, insert fail.\n" << std::endl;
					return *this;
				}
			}
		}
		db[dbRecord.name()] = dbRecord;
		return *this;
	}

	template<typename T>
	typename queryResult<T>& queryResult<T>::insert(Instance& instance, bool shouldStrict) {
		if (db.contains(instance.name()) == true) {
			std::cout << "Element with same key exists, insert skip.\n" << std::endl;
			return *this;
		}
		if (shouldStrict == true) {
			for (std::vector<std::string>::iterator iter = instance.children().begin(); iter != instance.children().end(); iter++) {
				if (db.contains(*iter) == false) {
					std::cout << "No child " + *iter + " found in target database, insert fail.\n" << std::endl;
					return *this;
				}
			}
		}
		db[instance.name()] = instance;
		return *this;
	}

	// The queryString has a basic form - (column: "expression") (no parentheses)
	template<typename T>
	typename queryResult<T>& queryResult<T>::find(const std::string& queryString_) {
		if (hasSource == false) throw std::exception("\n Find: Cannot find in empty db, do you forget add \"from\"?\n");
		// std::cout << "find in db with size " << result.size() << "\n";
		std::string queryString = queryString_;
		if (queryString == "") {
			NoSqlDb::DbCore<T>::iterator iter = db.begin();
			while (iter != db.end()) {
				result.push_back(iter->second);
				iter++;
			}
			return *this;
		}
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

	//
	template<typename T>
	queryResult<T>& queryResult<T>::find(const std::string& key, const std::string& expression) {
		if (hasSource == false) throw std::exception("\n Find: Cannot find in empty db, do you forget add \"from\"?\n");
		// std::cout << "Querying " << key << " " << expression << " in db size " << result.size() << std::endl;
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
		// std::cout << "find operation returns " << result.size() << " result(s)" << std::endl;
		return *this;
	}

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

	template<typename T>
	typename queryResult<T>& queryResult<T>::orFind(const std::string& key, const std::string& value) {
		if (hasSource == false) throw std::exception("\n orFind: Cannot find in empty db, do you forget add \"from\"?\n");
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

	template<typename T>
	typename queryResult<T>& queryResult<T>::childOf(bool returnAll) {
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
				}
			}
		}
		result.clear();
		// std::cout << "childOf filter find " << std::to_string((int)(childMap.size())) << " children from previous result." << std::endl;
		NoSqlDb::DbCore<T>::iterator iter = childMap.begin();
		while (iter != childMap.end()) {
			result.push_back(iter->second);
			iter++;
		}
		return *this;
	}

	template<typename T>
	void queryResult<T>::ascendBy(const std::string& key) {
		if (key == "name") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.name() < b.name(); });
		else if (key == "description") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.descrip() < b.descrip(); });
		else if(key == "dateTime") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.dateTime() < b.dateTime(); });
		else if(key == "payLoad") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.payLoad() < b.payLoad(); });
		else if (key == "children" || key == "category") throw std::exception("Ascend by: Index cannot use for sort.\n");
		else throw std::exception("\n Ascend by: Unrecognized sort index.\n");
		return *this;
	}

	template<typename T>
	void queryResult<T>::descendBy(const std::string& key) {
		if (key == "name") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.name() > b.name(); });
		else if (key == "description") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.descrip() > b.descrip(); });
		else if (key == "dateTime") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.dateTime() > b.dateTime(); });
		else if (key == "payLoad") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.payLoad() > b.payLoad(); });
		else if (key == "children" || key == "category") throw("Descend by: Index cannot use for sort.\n");
		else throw std::exception("\n Descend by: Unrecognized sort index.\n");
		return *this;
	}

	template<typename T>
	bool queryResult<T>::hasChild(const std::string& child) {
		for (Results::iterator iter = result.begin(); iter != result.end(); iter++) {
			for (std::vector<std::string>::iterator childIter = iter->children().begin(); childIter != iter->children().end(); childIter++) {
				if (child == *childIter) return true;
			}
		}
		return false;
	}

	// The newValue is a set of key-value pairs, the format of the set is exactly same with the query string,
	// except the all keys will appear no more than once.
	// The children item can be decorated with filters (or call operators), start with "$" and wrapped with "()".
	//		normal form: children: "["childname1", "childname2"]"
	//		with filter: children: "$(filter)(["childname1", "childname2"])"
	// In update operation, valid filters are:
	//		1. $(add): add all children in the "[]", if the child is already existed, skip that child.
	//		2. $(remove): remove all children in the "[]", if the child is already removed, skip that child.
	// All other filter string (and / or) decorating any other kind of value will be REJECTED.
	template<typename T>
	void queryResult<T>::update(const std::string& newValue) {
		if (hasSource == false) throw std::exception("\n Update: Cannot update in empty db, do you forget add \"from\"?\n");
		kvPairs keyValuePair = keyValuePairGenerator(newValue, true);
		typename queryResult<T>::Results::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			size_t pairLen = keyValuePair.size();
			for (size_t i = 0; i < pairLen; ++i) {
				if (keyValuePair[i].first == "description") {
					db[(*iter).name()].descrip(keyValuePair[i].second);
				}
				else if (keyValuePair[i].first == "dateTime") {
					db[(*iter).name()].dateTime(std::string(keyValuePair[i].second));
				}
				else if (keyValuePair[i].first == "payLoad") {
					db[(*iter).name()].payLoad(keyValuePair[i].second);
				}
				else if (keyValuePair[i].first == "children" || keyValuePair[i].first == "category") {
					if (keyValuePair[i].second[0] == '$') {
						std::pair<std::string, std::string> filterChildren = filterSolver(keyValuePair[i].second);
						std::string filter = filterChildren.first;
						if (filter == "remove") deleteCollection(db[(*iter).name()], keyValuePair[i].first, filterChildren.second);
						else if (filter == "add") addCollection(db[(*iter).name()], keyValuePair[i].first, filterChildren.second);
						else throw std::exception("\n Update: filter usage invalid in new value.");
					}
					else replaceCollection(*iter, keyValuePair[i].first, keyValuePair[i].second);
				}
			}
		}
		// std::cout << "\n  Update operation affects " << std::to_string(result.size()) + " result" << ((result.size() > 1) ? "s." : ".") << std::endl;
		return;
	}

	template<typename T>
	void queryResult<T>::update(Instance& instance) {
		if (db.contains(instance.name()) == false) throw std::exception("\n Update: No element find with name given.\n");
		Instance& toUpdate = db[instance.name()];
		toUpdate.descrip(instance.descrip());
		toUpdate.dateTime(instance.dateTime());
		toUpdate.payLoad(instance.payLoad());
		toUpdate.category(instance.category());
		toUpdate.children(instance.children());
		return;
	}

	template<typename T>
	void queryResult<T>::remove(const std::string& queryString) {
		if (hasSource == false) throw std::exception("\n Remove: Cannot remove in empty db, do you forget add \"from\"?\n");
		if (queryString != "") find(queryString);
		typename queryResult<T>::Results::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			if (db.contains((*iter).name())) db.dbStore().erase((*iter).name());
		}
		return;
	}

	template<typename T>
	void queryResult<T>::drop() {
		if (hasSource == false) throw std::exception("\n Drop: Cannot drop empty db, do you forget add \"from\"?\n");
		db.dbStore().clear();
		return;
	}

	template<typename T>
	void queryResult<T>::resultDisplay() {
		if (hasSource == false) throw std::exception("\n Result: Please add \"from\".\n");
		// std::cout << "\n  The query returns " << std::to_string(result.size()) << " result(s)" << std::endl;
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

	template<typename T>
	typename queryResult<T>::Results& queryResult<T>::eval() {
		return result;
	}

	// The entry of all checkers operation used by find, internal access only.
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

	template<typename T>
	bool queryResult<T>::collectionChecker(Instance& element, const std::string& key_, std::string& value_) {
		std::string value = value_;
		std::string key = key_;
		std::vector<std::string>& candidate = (key == "children" ? element.children() : element.category());
		size_t i = 0, j = 0, len2 = candidate.size();
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
				if (value == candidate[j]) {
					return true;
				}
			}
		}
		return false;
	}

	template<typename T>
	typename queryResult<T>::kvPairs queryResult<T>::keyValuePairGenerator(const std::string& queryString, bool updateCheck) {
		std::vector<std::string> conditions;
		kvPairs keyValuePair;
		conditions = Utilities::splitPlus(Utilities::trim(queryString));
		/*std::cout << "debug output" << std::endl;
		for (size_t i = 0; i < conditions.size(); ++i) {
			std::cout << conditions[i] << std::endl;
		}*/
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

	//
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

	template<typename T>
	std::vector<std::string> queryResult<T>::valueListGenerator(const std::string& values) {
		std::vector<std::string> valueName;
		valueName = Utilities::split(values);
		for (size_t i = 0; i < valueName.size(); ++i) {
			valueName[i] = Utilities::trim(valueName[i]);
		}
		std::sort(valueName.begin(), valueName.end(), [](std::string a, std::string b) { return a < b; });
		/*for (int i = 0; i < valueName.size(); ++i) std::cout << valueName[i] << std::endl;
		std::cout << std::endl;*/
		return valueName;
	}

	// 1 day
	// year, month, week, day, hour, minute, second
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
		timeUnit = Utilities::trim(toFormat.substr(i + 1, toFormat.length()));
		return make_pair(timeNum, timeUnit);
	}

	template<typename T>
	void queryResult<T>::addCollection(Instance& element, const std::string& name, const std::string& value) {
		std::cout << value << std::endl;
		std::vector<std::string>& targetVector = (name == "children" ? element.children() : element.category());
		std::vector<std::string> valueName = valueListGenerator(value);
		std::vector<std::string>::iterator p;
		std::vector<std::string>::iterator q;
		p = targetVector.begin();
		q = valueName.begin();
		std::vector<std::string> tmp;
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

	template<typename T>
	void queryResult<T>::deleteCollection(Instance& element, const std::string& name, const std::string& value) {
		std::vector<std::string>& targetVector = (name == "children" ? element.children() : element.category());
		std::vector<std::string> valueName = valueListGenerator(value);
		std::vector<std::string>::iterator p;
		std::vector<std::string>::iterator q;
		p = targetVector.begin();
		q = valueName.begin();
		while (q != valueName.end() && p != targetVector.end()) {
			if (*q < *p) q++;
			else if (*q > *p) p++;
			else { p = targetVector.erase(p); q++; }
		}
		return;
	}

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