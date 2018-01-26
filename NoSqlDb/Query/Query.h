#pragma once
#ifndef DBQUERY_H
#define DBQUERY_H
#include "../DbCore/DbCore.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include <algorithm>
#include <utility>
#include <regex>
#include <chrono>
namespace DbQuery {
	template<typename T>
	class queryResult {
	public:
		// Change these names to type less letters.
		using Results = std::vector<NoSqlDb::DbElement<T>>;
		using Instance = NoSqlDb::DbElement<T>;
		using kvPair = std::pair<std::string, std::string>;
		using kvPairs = std::vector<kvPair>;

		explicit queryResult(NoSqlDb::DbCore<T>& target) : db(target) { internalDb = target; };

		queryResult<T>& from(NoSqlDb::DbCore<T>& target);
		queryResult<T>& from(Results& result);

		queryResult<T>& insert(std::string record, bool shouldStrict = false);

		queryResult<T>& find(const std::string& queryString);
		queryResult<T>& find(const std::string& key, const std::string& value);

		queryResult<T>& orFind(const std::string& key, const std::string& value);

		queryResult<T>& notFind(const std::string& key, const std::string& value);

		void update(const std::string& newValue);

		void remove(const std::string& queryString = "");

		void drop();

		void resultDisplay();
	private:
		//data members
		NoSqlDb::DbCore<T>& db;
		NoSqlDb::DbCore<T> internalDb;

		//help functions
		bool checker(Instance& element, const std::string& key, const std::string& value, bool removeDuplicate = false);
		Results dbToResult(NoSqlDb::DbCore<T>& targetDb, bool clearDb = false);
		kvPairs keyValuePairGenerator(std::string queryString, bool updateCheck = false);
		std::pair<std::string, std::string> filterSolver(std::string& valueWithFilter);
		std::vector<std::string> valueListGenerator(std::string& children);
		DateTime dateTimeFormatter(std::string& toFormat);
	};

	template<typename T>
	typename queryResult<T>& queryResult<T>::from(NoSqlDb::DbCore<T>& target) {
		internalDb.dbStore().clear();
		internalDb = target;
		return *this;
	}

	template<typename T>
	typename queryResult<T>& queryResult<T>::from(Results& result) {
		internalDb.dbStore().clear();
		typename queryResult<T>::Results::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			internalDb[*iter->name] = *iter;
		}
		return *this;
	}

	// Record: 
	//     Assume the accept string is like ("name", "", [], "value", [""]), (without outside parentheses).
	//     The second metadata - description, can presents or absents, as well as the catagory parameter,
	//     which is represented by the last array.
	//     The two pairs of bracket must present, the first represents to the children names, 
	//     and the second represents to the category array, which contains catagory.
	//     All values, if present, must be wrapped with quotes.
	//     Any adjacent items, must be seprated with ",".
	//     Any violation of above criteria will be REJECTED.
	// shouldStrict:
	//		true (need explicate indicate): if inserting element whose child is not currently in the db, 
	//										the operation will be REJECTED.
	//		false (by default): if inserting element whose child is not currently in the db, a element 
	//							like ("this/child/name", "", [], "", [""]) will be inserted. 
	template<typename T>
	typename queryResult<T>& queryResult<T>::insert(std::string record, bool shouldStrict) {
		// The instance to be insert in db.
		typename queryResult<T>::Instance dbRecord;
		// The vector / string to contain each data/metadata item.
		std::vector<std::string> data;
		std::vector<std::string> children;
		std::vector<std::string> categories;
		std::string name;
		std::string payLoad;
		std::string description;
		// Split raw input into metadata and / or data.
		data = Utilities::splitPlus(record);
		// Rule out the possible invalid input.
		if (data.size() < 2) throw("Too few argument in insert operation: " + std::to_string(data.size()) + "given, 2 needed at least.\n");
		else if (data.size() == 3) {
			name = Utilities::unwrapPlus(Utilities::trim(data[0]));
			children = valueListGenerator(data[1]);
			payLoad = Utilities::unwrapPlus(Utilities::trim(data[0]));
		}
		// 0    1  2  3
		// name "" [] ""
		// name "" [] [""]
		// name [] "" [""]
		else if (data.size() == 4) {
			name = Utilities::unwrapPlus(Utilities::trim(data[0]));
			description = (Utilities::checkWrapper(data[1], '[') == true) ? "" : Utilities::unwrapPlus(Utilities::trim(data[1]));
			children = (Utilities::checkWrapper(data[1], '[') == true) ? valueListGenerator(data[1]) : valueListGenerator(data[2]);
			payLoad = (Utilities::checkWrapper(data[1], '[') == true) ? Utilities::unwrapPlus(Utilities::trim(data[2]))
				: ((Utilities::checkWrapper(data[3], '[') == true) ? "" : Utilities::unwrapPlus(Utilities::trim(data[3])));
			if (Utilities::checkWrapper(data[3], '[') == true) categories = valueListGenerator(data[3]);
		}
		else if (data.size() == 5) {
			name = Utilities::unwrapPlus(Utilities::trim(data[0]));
			description = Utilities::unwrapPlus(Utilities::trim(data[1]));
			children = valueListGenerator(data[2]);
			payLoad = Utilities::unwrapPlus(Utilities::trim(data[3]));
			categories = valueListGenerator(data[3]);
		}
		else {
			throw("Too many arguments in insert operation: " + std::to_string(data.size()) + "given, 2 needed at most.\n");
		}
		dbRecord.name(name);
		dbRecord.descrip(description);
		dbRecord.children(children);
		dbRecord.payLoad(payLoad);
		dbRecord.category(categories);
		dbRecord.dateTime(DateTime().now());
		db[name] = dbRecord;
		std::cout << "\nInserted: -\n" << "name: " << dbRecord.name() << ", description: " << dbRecord.descrip() << ", payLoad: " << dbRecord.payLoad() << std::endl;
		std::cout << "With children: ";
		typename NoSqlDb::DbElement<T>::Children::iterator iter = dbRecord.children().begin();
		while (iter != dbRecord.children().end()) {
			std::cout << *iter << " ";
			iter++;
		}
		std::cout << std::endl;
		std::cout << "With category: ";
		iter = dbRecord.category().begin();
		while (iter != dbRecord.category().end()) {
			std::cout << *iter << " ";
			iter++;
		}
		std::cout << std::endl;
		std::cout << "Operation successful!" << std::endl;
		return *this;
	}

	// The queryString has a basic form - (column: "expression") (no parentheses)
	// Multiple conditions are sepreated with ",". Ex: name: "name", description: "description"
	// column MUST NOT be wrapped with quotes and expression MUST be wrapped with quotes.
	// Children names MUST be wrapped with brackets, and their names MUST wrapped with quotes. 
	//		ex: children: "["child1", "child2"]"
	// column: the key of the data. Parameter of column are {"name", "dataTime", "description", "children", "payload"}
	//         Any string that cannot match the items in array will consider as invalid key, 
	//		   which will result of empty return value.
	// expression: the expression of the condition of query, which can be represented into three forms:
	//		1. Exactly the string, the string can be a string or a "[]" wrapped array. 
	//		   ex: name: "name", children: "["child1", "child2"]"
	//		2. Regular expression, represent as /exp/.
	//		   ex: name: "/[A-Z]*/"
	//		3. Filter-decorated expression, represent as "$(filter) (exp)" 
	//		   (it's OK without whitespace seperator or has multiple whitespace sperator)
	//		   ex: name: "$(not)(name)"
	// The filters start with "$" and wrapped with "()".
	//		In find operation, valid filters are:
	//			1. $(not): return elements which do not match this query.
	//			2. $(children): return children of the result of the query.
	template<typename T>
	typename queryResult<T>& queryResult<T>::find(const std::string& queryString) {
		std::string queryStringAct = queryString;
		if (result.size() != 0) result.clear();
		if (queryStringAct == "") {
			NoSqlDb::DbCore<T>::iterator iter = internalDb.begin();
			while (iter != internalDb.end()) {
				result.push_back(iter->second);
				iter++;
			}
			return *this;
		}
		kvPairs keyValuePair = keyValuePairGenerator(queryStringAct);
		size_t pairLen = keyValuePair.size();
		for (int i = 0; i < pairLen; ++i) {
			if (i > 0 && internalDb.size() == 0) {
				result.clear();
				return;
			}
			if (keyValuePair[i].first == "children") {
				std::vector<std::string> childrenName = valueListGenerator(keyValuePair[i].second);
				size_t nameLen = childrenName.size();
				for (int j = 0; j < nameLen; ++j) {
					internalDb = find(keyValuePair[i].first, childrenName[j]);
					std::cout << "Internal find receive " << internalDb.size() << " result(s) from finder" << std::endl;
				}
			}
			else {
				internalDb = find(keyValuePair[i].first, keyValuePair[i].second);
				std::cout << "Internal find receive " << internalDb.size() << " result(s) from finder" << std::endl;
			}
		}
		return *this;
	}

	//
	template<typename T>
	typename queryResult<T>& queryResult<T>::find(const std::string& key, const std::string& expression) {
		std::string expressionAct = expression;
		std::cout << "Querying " << key << " " << expressionAct << " in db size " << internalDb.size() << std::endl;
		if (expressionAct[0] == '$') {
			std::pair<std::string, std::string> filterValue = filterSolver(expressionAct);
			std::string filter = filterValue.first;
			std::string value = filterValue.second;
			if (filter == "not") {
				typename NoSqlDb::DbCore<T>::iterator iter = internalDb.begin();
				while (iter != internalDb.end()) {
					if (checker(iter->second, key, value) == true) {
						iter = internalDb.dbStore().erase(iter);
					}
					else if (iter != internalDb.end()) iter++;
				}
			}
			if (filter == "before" && key == "dateTime") {
				std::string trueDateTime = std::string(dateTimeFormatter(value));
				typename NoSqlDb::DbCore<T>::iterator iter = internalDb.begin();
				while (iter != internalDb.end()) {
					if (checker(iter->second, key, trueDateTime) == true) {
						iter = internalDb.dbStore().erase(iter);
					}
					else if (iter != internalDb.end()) iter++;
				}
			}
			else throw("Unrecognized filter: " + filter + "\n");
		}
		else {
			typename NoSqlDb::DbCore<T>::iterator iter = internalDb.begin();
			while (iter != internalDb.end()) {
				if (checker(iter->second, key, expressionAct) == false) {
					iter = internalDb.dbStore().erase(iter);
				}
				else if (iter != internalDb.end()) iter++;
			}
		}
		std::cout << "finder returns " << internalDb.size() << " result(s)" << std::endl;
		return *this;
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
		kvPairs keyValuePair = keyValuePairGenerator(newValue, true);
		NoSqlDb::DbCore<T>::iterator iter;
		for (iter = internalDb.begin(); iter != internalDb.end(); iter++) {
			size_t pairLen = keyValuePair.size();
			for (int i = 0; i < pairLen; ++i) {
				if (keyValuePair[i].first == "description") {
					db[iter->first].descrip(keyValuePair[i].second);
				}
				else if (keyValuePair[i].first == "dateTime") {
					db[iter->first].dateTime(std::string(keyValuePair[i].second));
				}
				else if (keyValuePair[i].first == "payLoad") {
					db[iter->first].payLoad(keyValuePair[i].second);
				}
				else if (keyValuePair[i].first == "children" || keyValuePair[i].first == "category") {
					if (keyValuePair[i].second[0] == '$') {
						std::pair<std::string, std::string> filterChildren = filterSolver(keyValuePair[i].second);
						std::string filter = filterChildren.first;
						if (filter != "add" && filter != "remove") throw("filter usage invalid in new value: " + newValue);
						else {
							std::vector<std::string>& targetVector = (keyValuePair[i].first == "children" ?
																				db[iter->first].children() :
																				db[iter->first].category());
							std::vector<std::string> valueName = valueListGenerator(filterChildren.second);
							std::vector<std::string>::iterator p;
							std::vector<std::string>::iterator q;
							p = targetVector.begin();
							q = valueName.begin();
							if (filter == "remove") {
								while (q != valueName.end() && p != targetVector.end()) {
									if (*q < *p) q++;
									else if (*q > *p) p++;
									else {
										p = targetVector.erase(p);
										q++;
									}
								}
							}
							else if (filter == "add") {
								std::vector<std::string> tmp;
								while (q != valueName.end() && p != targetVector.end()) {
									if (*q < *p) {
										tmp.push_back(*q);
										q++;
									}
									else if (*q > *p) {
										tmp.push_back(*p);
										p++;
									}
									else {
										tmp.push_back(*p);
										p++, q++;
									}
								}
								while (p != targetVector.end()) {
									tmp.push_back(*p);
									p++;
								}
								while (q != valueName.end()) {
									tmp.push_back(*q);
									q++;
								}
								targetVector.clear();
								p = tmp.begin();
								while (p != tmp.end()) {
									targetVector.push_back(*p);
									p++;
								}
							}
							else {
								throw("Unrecognized filter: " + filter + " at new value: " + newValue);
							}
						}
					}
					else {
						std::vector<std::string>& targetVector = (keyValuePair[i].first == "children" ?
																			db[iter->first].children() :
																			db[iter->first].category());
						std::vector<std::string> valueName = valueListGenerator(keyValuePair[i].second);
						targetVector.clear();
						for (int j = 0; j < valueName.size(); ++j) {
							targetVector.push_back(valueName[j]);
						}
					}
				}
			}
		}
		std::cout << "\n  Update operation affects " << std::to_string(internalDb.size()) + " result" << ((internalDb.size() > 1) ? "s." : ".") << std::endl;
		internalDb.dbStore().clear();
		return;
	}

	template<typename T>
	void queryResult<T>::remove(const std::string& queryString) {
		if (queryString != "") internalDb = find(queryString);
		typename NoSqlDb::DbCore::iterator iter;
		for (iter = internalDb.begin(); iter != internalDb.end(); iter++) {
			if (db.contains(iter->first)) db.dbStore().erase(iter->first);
		}
		return;
	}

	template<typename T>
	void queryResult<T>::drop() {
		db.dbStore().clear();
		return;
	}

	template<typename T>
	void queryResult<T>::resultDisplay() {
		std::cout << "\n  The query returns " << internalDb.size() << " result(s)" << std::endl;
		showHeader(std::cout);
		NoSqlDb::DbCore<T>::iterator iter;
		std::ostream& out = std::cout;
		for (iter = internalDb.begin(); iter != internalDb.end(); iter++) {
			out << "\n  ";
			out << std::setw(26) << std::left << std::string(iter->second.dateTime());
			out << std::setw(10) << std::left << iter->second.name();
			out << std::setw(25) << std::left << iter->second.descrip();
			out << std::setw(25) << std::left << iter->second.payLoad();
			typename NoSqlDb::DbElement<T>::Children children = iter->second.children();
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
		return;
	}

	// The checker operation of find, internal access only (private function).
	// key: "name", "description", "dateTime", "children", "payLoad" (all without quotes). 
	// Any string except these will throw exception.
	// value: string without quotes, except corresponding value of children, which will be a "[]" wrapped array, 
	//        each element in array will wrapped with "\"" (no outside quotes).
	//		  01/22/18: Now value can contain regluar expression.
	// Ex: key = "name", value = "name" (all without quotes).
	//	   key = "children", value = "["childname1", "childname2"]" (all without outside quotes).
	template<typename T>
	bool queryResult<T>::checker(typename queryResult<T>::Instance& element, const std::string& key, const std::string& value, bool removeDuplicate) {
		std::string valueAct = value;
		std::string keyAct = key;
		if (removeDuplicate == true) {
			if (internalDb.contains(element.name())) return false;
		}
		if (keyAct == "name") {
			if (Utilities::checkWrapper(valueAct, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(valueAct, '/'));
				if (std::regex_match(element.name(), regExp)) return true;
				else return false;
			}
			else if (element.name() == valueAct) return true;
		}
		else if (keyAct == "description") {
			if (Utilities::checkWrapper(valueAct, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(valueAct, '/'));
				if (std::regex_match(element.descrip(), regExp)) return true;
				else return false;
			}
			else if (element.descrip() == valueAct) return true;
		}
		else if (keyAct == "dateTime") {
			if (Utilities::checkWrapper(valueAct, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(valueAct, '/'));
				if (std::regex_match(std::string(element.dateTime()), regExp)) return true;
				else return false;
			}
			else if (std::string(element.dateTime()) == valueAct) return true;
		}
		else if (keyAct == "children" || keyAct == "category") {
			std::vector<std::string> valueName = valueListGenerator(valueAct);
			std::vector<std::string>& candidate = (keyAct == "children" ? element.children() : element.category());
			size_t i = 0, j = 0, len1 = valueName.size(), len2 = candidate.size();
			while (i < len1 && j < len2) {
				if (Utilities::checkWrapper(valueName[i], '/') == true) {
					bool flag = false;
					std::regex regExp(Utilities::unwrapPlus(valueName[i], '/'));
					for (int k = 0; k < candidate.size(); ++k) {
						if (std::regex_match(candidate[k], regExp) == true) {
							flag = true;
							break;
						}
					}
					if (flag == true) i += 1;
				}
				if (valueName[i] == candidate[j]) {
					i += 1;
					j += 1;
				}
				else j += 1;
			}
			if (i == len1) {
				// std::cout << element.name() << std::endl;
				return true;
			}
			else return false;
		}
		else if (keyAct == "payLoad") {
			if (Utilities::checkWrapper(value, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(value, '/'));
				if (std::regex_match(element.payLoad(), regExp)) return true;
				else return false;
			}
			else if (element.payLoad() == value) return true;
		}
		else {
			throw("Query value invalid at: " + valueAct);
		}
		return false;
	}

	template<typename T>
	typename queryResult<T>::Results queryResult<T>::dbToResult(NoSqlDb::DbCore<T>& targetDb, bool clearDb) {
		if (result.size() != 0) result.clear();
		typename NoSqlDb::DbCore<T>::iterator iter = targetDb.begin();
		while (iter != targetDb.end()) {
			result.push_back(iter->second);
			iter++;
		}
		if (clearDb == true) targetDb.dbStore().clear();
		std::cout << "Converting " << std::to_string(result.size()) << " into output results" << std::endl;
		return result;
	}

	template<typename T>
	typename queryResult<T>::kvPairs queryResult<T>::keyValuePairGenerator(std::string queryString, bool updateCheck) {
		std::vector<std::string> conditions;
		kvPairs keyValuePair;
		conditions = Utilities::splitPlus(queryString);
		std::cout << "debug output" << std::endl;
		for (int i = 0; i < conditions.size(); ++i) {
			std::cout << conditions[i] << std::endl;
		}
		size_t items = conditions.size();
		for (int i = 0; i < items; ++i) {
			std::vector<std::string> condition;
			condition = Utilities::splitPlus(conditions[i], ':');
			if (condition.size() != 2) throw("invalid query at: " + condition[i]);
			condition[0] = Utilities::trim(condition[0]);
			if (condition[0] == "name" && updateCheck == true) throw("name column updating is forbidden");
			condition[1] = Utilities::trim(condition[1]);
			condition[1] = Utilities::unwrapPlus(condition[1], '\"');
			// std::cout << condition[0] << " " << condition[1] << std::endl;
			keyValuePair.push_back(kvPair(condition[0], condition[1]));
		}
		return keyValuePair;
	}

	//
	template<typename T>
	std::pair<std::string, std::string> queryResult<T>::filterSolver(std::string& valueWithFilter) {
		std::regex filterExp("\\$\\([a-z]*\\)\\s*\\(.*\\)");
		if (std::regex_match(valueWithFilter, filterExp) == false) throw("Invalid usage of filter at: " + valueWithFilter);
		std::smatch matches;
		std::regex_search(valueWithFilter, matches, std::regex("\\$\\([a-z]*\\)"));
		std::string filter = std::string(*(matches.begin()));
		std::string expression = valueWithFilter.substr(filter.length(), valueWithFilter.length());
		filter = filter.substr(2, filter.length() - 3);
		expression = Utilities::unwrapPlus(Utilities::trim(expression), '(');
		return std::make_pair(filter, expression);
	}

	template<typename T>
	std::vector<std::string> queryResult<T>::valueListGenerator(std::string& values) {
		std::vector<std::string> valueName;
		if (Utilities::checkWrapper(values, '[') == false) valueName.push_back(values);
		else {
			std::string valueAfter = Utilities::unwrapPlus(values, '[');
			valueName = Utilities::split(valueAfter);
			for (int i = 0; i < valueName.size(); ++i) {
				valueName[i] = Utilities::trim(valueName[i]);
				valueName[i] = Utilities::unwrapPlus(valueName[i]);
			}
			std::sort(valueName.begin(), valueName.end(), [](std::string a, std::string b) { return a < b; });
		}
		/*for (int i = 0; i < valueName.size(); ++i) std::cout << valueName[i] << std::endl;
		std::cout << std::endl;*/
		return valueName;
	}

	// 1 day
	// year, month, week, day, hour, minute, second
	template<typename T>
	DateTime queryResult<T>::dateTimeFormatter(std::string& toFormat) {
		if (std::regex_match(toFormat, std::regex("[A-Z][a-z][a-z]\\s[A-Z][a-z][a-z]\\s\\d\\d\\s\\d\\d:\\d\\d:\\d\\d\\s\\d\\d\\d\\d"))) {
			return DateTime(toFormat);
		}
		if (toFormat == "now") return DateTime().now();
		toFormat = Utilities::trim(toFormat);
		std::string num = "";
		std::string timeUnit = "";
		size_t strLen = toFormat.length();
		int i = 0;
		for (int i = 0; i < strLen; ++i) {
			if ('0' >= toFormat[i] && toFormat[i] >= '9') num += toFormat[i];
			else break;
		}
		if (i >= strLen) throw("Please indicate the time unit.\n");
		int timeNum = std::stoi(num);
		timeUnit = Utilities::trim(toFormat.substr(i, toFormat.length()));
		if (timeUnit == "year") {
			// using year = std::chrono::duration<>;

		}
		throw("Terminates, enough for debug.\n");
	}
}
#endif