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

		explicit queryResult(NoSqlDb::DbCore<T>& target) : db(target) { };

		queryResult<T>& from(NoSqlDb::DbCore<T>& target);
		queryResult<T>& from(const Results& target);

		queryResult<T>& insert(const std::string& record, bool shouldStrict = false);

		queryResult<T>& find(const std::string& queryString);
		queryResult<T>& find(const std::string& key, const std::string& value);

		queryResult<T>& orFind(const std::string& key, const std::string& value);

		queryResult<T>& notFind(const std::string& key, const std::string& value);

		queryResult<T>& childOf();

		void ascendBy(const std::string& key);

		void descendBy(const std::string& key);

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

		//help functions
		bool checker(Instance& element, const std::string& key, const std::string& value);
		kvPairs keyValuePairGenerator(std::string queryString, bool updateCheck = false);
		std::pair<std::string, std::string> filterSolver(std::string& valueWithFilter);
		std::vector<std::string> valueListGenerator(std::string& children);
		DateTime dateTimeFormatter(const std::string& toFormat);
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
	typename queryResult<T>& queryResult<T>::insert(const std::string& record, bool shouldStrict) {
		if (hasSource == false) throw("Insert: Cannot insert to empty db, do you forget add \"from\"?\n");
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
		if (hasSource == false) throw("Find: Cannot find in empty db, do you forget add \"from\"?\n");
		std::cout << "find in db with size " << result.size() << "\n";
		std::string queryStringAct = queryString;
		if (queryStringAct == "") {
			NoSqlDb::DbCore<T>::iterator iter = db.begin();
			while (iter != db.end()) {
				result.push_back(iter->second);
				iter++;
			}
			return *this;
		}
		kvPairs keyValuePair = keyValuePairGenerator(queryStringAct);
		size_t pairLen = keyValuePair.size();
		for (size_t i = 0; i < pairLen; ++i) {
			if (i > 0 && result.size() == 0) {
				return *this;
			}
			if (keyValuePair[i].first == "children") {
				std::vector<std::string> childrenName = valueListGenerator(keyValuePair[i].second);
				size_t nameLen = childrenName.size();
				for (size_t j = 0; j < nameLen; ++j) {
					find(keyValuePair[i].first, childrenName[j]);
					std::cout << "Internal find receive " << std::to_string(result.size()) << " result(s) from finder" << std::endl;
				}
			}
			else {
				find(keyValuePair[i].first, keyValuePair[i].second);
				std::cout << "Internal find receive " << std::to_string(result.size()) << " result(s) from finder" << std::endl;
			}
		}
		return *this;
	}

	//
	template<typename T>
	typename queryResult<T>& queryResult<T>::find(const std::string& key, const std::string& expression) {
		if (hasSource == false) throw("Find: Cannot find in empty db, do you forget add \"from\"?\n");
		std::string expressionAct = expression;
		std::cout << "Querying " << key << " " << expressionAct << " in db size " << result.size() << std::endl;
		if (expressionAct[0] == '$') {
			std::pair<std::string, std::string> filterValue = filterSolver(expressionAct);
			std::string filter = filterValue.first;
			std::string value = filterValue.second;
			if (filter == "not") {
				typename queryResult<T>::Results::iterator iter = result.begin();
				while (iter != result.end()) {
					if (checker(*iter, key, value) == true) {
						iter = result.erase(iter);
					}
					else if (iter != result.end()) iter++;
				}
			}
			else if (filter == "before" && key == "dateTime") {
				std::string trueDateTime = std::string(dateTimeFormatter(value));
				typename queryResult<T>::Results::iterator iter = result.begin();
				while (iter != result.end()) {
					if ((*iter).dateTime() > trueDateTime) {
						iter = result.erase(iter);
					}
					else if (iter != result.end()) iter++;
				}
			}
			else if (filter == "between" && key == "dateTime") {
				std::vector<std::string> bonds = Utilities::splitPlus(value);
				DateTime leftBond = std::string(dateTimeFormatter(Utilities::trim(bonds[0])));
				DateTime rightBond = std::string(dateTimeFormatter(Utilities::trim(bonds[1])));
				if (leftBond > rightBond) std::swap(leftBond, rightBond);
				typename queryResult<T>::Results::iterator iter = result.begin();
				while (iter != result.end()) {
					if ((*iter).dateTime() > rightBond || (*iter).dateTime() < leftBond ) {
						iter = result.erase(iter);
					}
					else if (iter != result.end()) iter++;
				}
			}
			else throw("Unrecognized filter or invalid use of filter: " + filter + "\n");
		}
		else {
			typename queryResult<T>::Results::iterator iter = result.begin();
			while (iter != result.end()) {
				if (checker(*iter, key, expressionAct) == false) {
					iter = result.erase(iter);
				}
				else if (iter != result.end()) iter++;
			}
		}
		std::cout << "find operation returns " << result.size() << " result(s)" << std::endl;
		return *this;
	}

	template<typename T>
	typename queryResult<T>& queryResult<T>::orFind(const std::string& key, const std::string& value) {
		if (hasSource == false) throw("orFind: Cannot find in empty db, do you forget add \"from\"?\n");
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
	typename queryResult<T>& queryResult<T>::notFind(const std::string& key, const std::string& value) {
		if (hasSource == false) throw("notFind: Cannot find in empty db, do you forget add \"from\"?\n");
		NoSqlDb::DbCore<T> previousResult = db;
		typename queryResult<T>::Results::iterator iter = result.begin();
		while (iter != result.end()) {
			if (previousResult.contains((*iter).name()) == true) iter = previousResult.erase((*iter).name());
			iter++;
		}
		result.clear();
		from(previousResult).find(key, value);
		return *this;
	}

	template<typename T>
	typename queryResult<T>& queryResult<T>::childOf() {
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
		result.clear();
		std::cout << "childOf filter find " << std::to_string((int)(childMap.size())) << " children from previous result." << std::endl;
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
		else if (key == "children" || key == "category") throw("Ascend by: Cannot use " + key + " as index of sort.\n");
		else throw("Ascend by: Unrecognized sort index: " + key + ".\n");
		return;
	}

	template<typename T>
	void queryResult<T>::descendBy(const std::string& key) {
		if (key == "name") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.name() > b.name(); });
		else if (key == "description") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.descrip() > b.descrip(); });
		else if (key == "dateTime") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.dateTime() > b.dateTime(); });
		else if (key == "payLoad") std::sort(result.begin(), result.end(), [](Instance a, Instance b) { return a.payLoad() > b.payLoad(); });
		else if (key == "children" || key == "category") throw("Ascend by: Cannot use " + key + " as index of sort.\n");
		else throw("Ascend by: Unrecognized sort index: " + key + ".\n");
		return;
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
		if (hasSource == false) throw("Update: Cannot update in empty db, do you forget add \"from\"?\n");
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
						if (filter != "add" && filter != "remove") throw("filter usage invalid in new value: " + newValue);
						else {
							std::vector<std::string>& targetVector = (keyValuePair[i].first == "children" ?
																				db[(*iter).name()].children() :
																				db[(*iter).name()].category());
							std::vector<std::string> valueName = valueListGenerator(filterChildren.second);
							std::vector<std::string>::iterator p;
							std::vector<std::string>::iterator q;
							p = targetVector.begin();
							q = valueName.begin();
							if (filter == "remove") {
								while (q != valueName.end() && p != targetVector.end()) {
									if (*q < *p) q++;
									else if (*q > *p) p++;
									else { p = targetVector.erase(p); q++; }
								}
							}
							else if (filter == "add") {
								std::vector<std::string> tmp;
								while (q != valueName.end() && p != targetVector.end()) {
									if (*q < *p) { tmp.push_back(*q); q++; }
									else if (*q > *p) { tmp.push_back(*p); p++; }
									else { tmp.push_back(*p); p++, q++; }
								}
								while (p != targetVector.end()) { tmp.push_back(*p); p++; }
								while (q != valueName.end()) { tmp.push_back(*q); q++; }
								targetVector = tmp;
							}
							else {
								throw("Unrecognized filter: " + filter + " at new value: " + newValue);
							}
						}
					}
					else {
						std::vector<std::string>& targetVector = (keyValuePair[i].first == "children" ?
																			db[(*iter).name()].children() :
																			db[(*iter).name()].category());
						std::vector<std::string> valueName = valueListGenerator(keyValuePair[i].second);
						targetVector.clear();
						for (size_t j = 0; j < valueName.size(); ++j) {
							targetVector.push_back(valueName[(int)j]);
						}
					}
				}
			}
		}
		std::cout << "\n  Update operation affects " << std::to_string(result.size()) + " result" << ((result.size() > 1) ? "s." : ".") << std::endl;
		return;
	}

	template<typename T>
	void queryResult<T>::update(Instance& instance) {
		if (db.contains(instance.name()) == false) throw("No element find with name: " + instance.name() + "\n");
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
		if (hasSource == false) throw("Remove: Cannot remove in empty db, do you forget add \"from\"?\n");
		if (queryString != "") find(queryString);
		typename queryResult<T>::Results::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			if (db.contains((*iter).name())) db.dbStore().erase((*iter).name());
		}
		return;
	}

	template<typename T>
	void queryResult<T>::drop() {
		if (hasSource == false) throw("Drop: Cannot drop empty db, do you forget add \"from\"?\n");
		db.dbStore().clear();
		return;
	}

	template<typename T>
	void queryResult<T>::resultDisplay() {
		if (hasSource == false) throw("Result: Please add \"from\".\n");
		std::cout << "\n  The query returns " << std::to_string(result.size()) << " result(s)" << std::endl;
		showHeader(std::cout);
		typename queryResult<T>::Results::iterator iter;
		std::ostream& out = std::cout;
		for (iter = result.begin(); iter != result.end(); iter++) {
			out << "\n  ";
			out << std::setw(26) << std::left << std::string((*iter).dateTime());
			out << std::setw(10) << std::left << (*iter).name();
			out << std::setw(25) << std::left << (*iter).descrip();
			out << std::setw(25) << std::left << (*iter).payLoad();
			typename NoSqlDb::DbElement<T>::Children children = (*iter).children();
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

	template<typename T>
	typename queryResult<T>::Results& queryResult<T>::eval() {
		return result;
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
	bool queryResult<T>::checker(typename queryResult<T>::Instance& element, const std::string& key, const std::string& value) {
		std::string valueAct = value;
		std::string keyAct = key;
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
					if (flag == true) i++;
				}
				if (valueName[i] == candidate[j]) { i++; j++; }
				else j += 1;
			}
			if (i == len1) return true;
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
	DateTime queryResult<T>::dateTimeFormatter(const std::string& toFormat_) {
		std::string toFormat = toFormat_;
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
			if ('0' <= toFormat[i] && toFormat[i] <= '9') num += toFormat[i];
			else break;
		}
		if (i >= strLen) throw("Please indicate the time unit.\n");
		int timeNum;
		try {
			timeNum = std::stoi(num);
		}
		catch (const std::exception& ex) {
			throw("invalid or no number argument in dateTime string at: " + num);
		}
		timeUnit = Utilities::trim(toFormat.substr(i + 1, toFormat.length()));
		size_t lavarage;
		if (timeUnit == "year" || timeUnit == "years") { lavarage = (size_t)(3.154e+7); }
		else if (timeUnit == "month" || timeUnit == "monthes") { lavarage = (size_t)(2.628e+6); }
		else if (timeUnit == "week" || timeUnit == "weeks") { lavarage = 604800; }
		else if (timeUnit == "day" || timeUnit == "days") { lavarage = 86400; }
		else if (timeUnit == "hour" || timeUnit == "hours") { lavarage = 3600; }
		else if (timeUnit == "minute" || timeUnit == "minutes") { lavarage = 60; }
		else if (timeUnit == "second" || timeUnit == "seconds") { lavarage = 1; }
		else throw("Unrecognizable time unit of: " + timeUnit + ".\n");
		std::chrono::seconds seconds(lavarage* timeNum);
		DateTime ans(DateTime().now());
		ans = ans - ans.makeDuration(0, 0, seconds.count(), 0);
		if (ans > DateTime().now()) { std::cout << "Warning: Chronosphere activated.\n"; }
		return ans;
	}
}
#endif