#pragma once
#ifndef DBQUERY_H
#define DBQUERY_H
#include "DbCore.h"
#include "../Utilities/StringUtilities/StringUtilities.h"
#include <algorithm>
#include <utility>
#include <stack>
#include <regex>
namespace DbQuery {
	template<typename T>
	class queryResult {
	public:
		// Change these names to type less letters.
		using Results = std::vector<NoSqlDb::DbElement<T>>;
		using Instance = NoSqlDb::DbElement<T>;
		using kvPair = std::pair<std::string, std::string>;
		using kvPairs = std::vector<kvPair>;

		explicit queryResult() {}
		//Copy constructor, used in return value of and(), or(), not() operation.
		explicit queryResult(Results input, NoSqlDb::DbCore<T>& target);

		// Insert operation, accept string format input.
		// shouldStrict = false: If the record contains child that does not exist, 
		//                       create it and add relationship and insert.
		// shouldStrict = true: If the record contains child that does not exist, 
		//                      reject the operation and return false instantly.
		bool insert(NoSqlDb::DbCore<T>& targetDb, std::string record, bool shouldStrict = false);

		Results find(NoSqlDb::DbCore<T>& targetDb, std::string queryString);

		void remove(NoSqlDb::DbCore<T>& targetDb, std::string queryString);
		void remove(NoSqlDb::DbCore<T>& targetDb, Results& toRemove);

		void update(NoSqlDb::DbCore<T>& targetDb, std::string queryString, std::string newValue, bool updateMode = false, bool updateMulti = true);
		void update(NoSqlDb::DbCore<T>& targetDb, Results toUpdate, std::string newValue, bool updateMode = false, bool updateMulti = true);
		void update(NoSqlDb::DbCore<T>& targetDb, std::string queryString, Instance newValue, bool updateMode = false, bool updateMulti = true);

		void drop(NoSqlDb::DbCore<T>& targetDb);
		/*queryResult<T> and (const queryResult<T>&) const;

		queryResult<T> or (const queryResult<T>&) const;
		
		queryResult<T> not(const queryResult<T>&) const;*/

		Results eval();

		void resultDisplay(Results& toOutput);
	private:
		Results result;
		NoSqlDb::DbCore<T> db;
		bool checker(Instance& element, std::string& key, std::string& value, bool removeDuplicate = false);
		Results dbToResult(bool clearDb = false);
		kvPairs keyValuePairGenerator(std::string queryString, bool updateCheck = false);
		void internalFind(NoSqlDb::DbCore<T>& targetDb, std::string queryString);
		std::pair<std::string, std::string> filterSolver(std::string& valueWithFilter);
		std::vector<std::string> childrenNameGenerator(std::string& children);
		NoSqlDb::DbCore<T> finder(NoSqlDb::DbCore<T>& collection, std::string& key, std::string& expression);
	};

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
			if (targetDb.contains(name)) return false;
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
				if (targetDb.contains(childName) == false) {
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
	typename queryResult<T>::Results queryResult<T>::find(NoSqlDb::DbCore<T>& targetDb, std::string queryString) {
		if (result.size() != 0) result.clear();
		if (queryString == "") {
			NoSqlDb::DbCore<T>::iterator iter = targetDb.begin();
			while (iter != targetDb.end()) {
				result.push_back(iter->second);
				iter++;
			}
			return result;
		}
		internalFind(targetDb, queryString);
		return dbToResult();
	}

	// The shadow function of "find" function, and the real worker of the find.
	// Private member, accept the queryString and generate the result dbStore.
	// The purpose of seperate these operation into a new fuction is inside the 
	// class it is more flexible to use dbStore(hashmap) than results(vector).
	template<typename T> 
	void queryResult<T>::internalFind(NoSqlDb::DbCore<T>& targetDb, std::string queryString) {
		db = targetDb;
		kvPairs keyValuePair = keyValuePairGenerator(queryString);
		// std::sort(keyValuePair.begin(), keyValuePair.end(), [](kvPair a, kvPair b) { return a.first > b.first; });
		size_t pairLen = keyValuePair.size();
		for (int i = 0; i < pairLen; ++i) {
			if (i > 0 && db.size() == 0) {
				result.clear();
				return;
			}
			if (keyValuePair[i].first == "children") {
				std::vector<std::string> childrenName = childrenNameGenerator(keyValuePair[i].second);
				size_t nameLen = childrenName.size();
				for (int j = 0; j < nameLen; ++j) {
					db = finder(db, keyValuePair[i].first, childrenName[j]);
					std::cout << "Internal find receive " << db.size() << " result(s) from finder" << std::endl;
				}
			}
			else {
				db = finder(db, keyValuePair[i].first, keyValuePair[i].second);
				std::cout << "Internal find receive " <<  db.size() << " result(s) from finder" << std::endl;
			}
		}
		return ;
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
	
	// The checker operation of find, internal access only (private function).
	// key: "name", "description", "dateTime", "children", "payLoad" (all without quotes). 
	// Any string except these will throw exception.
	// value: string without quotes, except corresponding value of children, which will be a "[]" wrapped array, 
	//        each element in array will wrapped with "\"" (no outside quotes).
	//		  01/22/18: Now value can contain regluar expression.
	// Ex: key = "name", value = "name" (all without quotes).
	//	   key = "children", value = "["childname1", "childname2"]" (all without outside quotes).
	template<typename T>
	bool queryResult<T>::checker(typename queryResult<T>::Instance& element, std::string& key, std::string& value, bool removeDuplicate) {
		if (removeDuplicate == true) {
			if (db.contains(element.name())) return false;
		}
		if (key == "name") {
			if (Utilities::checkWrapper(value, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(value, '/'));
				if (std::regex_match(element.name(), regExp)) return true;
				else return false;
			}
			else if (element.name() == value) return true;
		}
		else if (key == "description") {
			if (Utilities::checkWrapper(value, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(value, '/'));
				if (std::regex_match(element.descrip(), regExp)) return true;
				else return false;
			}
			else if (element.descrip() == value) return true;
		}
		else if (key == "dateTime") {
			if (Utilities::checkWrapper(value, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(value, '/'));
				if (std::regex_match(std::string(element.dateTime()), regExp)) return true;
				else return false;
			}
			else if (std::string(element.dateTime()) == value) return true;
		}
		else if (key == "children") {
			std::vector<std::string> childrenName = childrenNameGenerator(value);
			std::vector<std::string> candidate = element.children();
			size_t i = 0, j = 0, len1 = childrenName.size(), len2 = candidate.size();
			while (i < len1 && j < len2) {
				if (Utilities::checkWrapper(childrenName[i], '/') == true) {
					bool flag = false;
					std::regex regExp(Utilities::unwrapPlus(childrenName[i], '/'));
					for (int k = 0; k < candidate.size(); ++k) {
						if (std::regex_match(candidate[k], regExp) == true) {
							flag = true;
							break;
						}
					}
					if (flag == true) i += 1;
				}
				if (childrenName[i] == candidate[j]) {
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
		else if (key == "payLoad") {
			if (Utilities::checkWrapper(value, '/') == true) {
				std::regex regExp(Utilities::unwrapPlus(value, '/'));
				if (std::regex_match(element.payLoad(), regExp)) return true;
				else return false;
			}
			else if (element.payLoad() == value) return true;
		}
		else {
			throw("Query key invalid at: " + key);
		}
		return false;
	}

	//
	template<typename T>
	NoSqlDb::DbCore<T> queryResult<T>::finder(NoSqlDb::DbCore<T>& collection, std::string& key, std::string& expression) {
		std::cout << "Querying " << key << " " << expression << " in db size " << collection.size() << std::endl;
		if (expression[0] == '$') {
			std::pair<std::string, std::string> filterValue = filterSolver(expression);
			std::string filter = filterValue.first;
			std::string value = filterValue.second;
			if (filter == "not") {
				typename NoSqlDb::DbCore<T>::iterator iter = collection.begin();
				while (iter != collection.end()) {
					if (checker(iter->second, key, value) == true) {
						iter = collection.dbStore().erase(iter);
					}
					else if (iter != collection.end()) iter++;
				}
			}
		}
		else {
			typename NoSqlDb::DbCore<T>::iterator iter = collection.begin();
			while (iter != collection.end()) {
				if (checker(iter->second, key, expression) == false) {
					iter = collection.dbStore().erase(iter);
				}
				else if (iter != collection.end()) iter++;
			}
		}
		std::cout << "finder returns " << collection.size() << " result(s)" << std::endl;
		return collection;
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
	void queryResult<T>::remove(NoSqlDb::DbCore<T>& targetDb, std::string queryString) {
		if (result.size() != 0) result.clear();
		result = find(targetDb queryString);
		typename queryResult<T>::Results::iterator iter;
		for (iter = result.begin(); iter != result.end(); iter++) {
			if (targetDb.contains(*iter->name())) targetDb.dbStore().erase(*iter->name);
		}
		return ;
	}

	template<typename T>
	void queryResult<T>::remove(NoSqlDb::DbCore<T>& targetDb, Results& toRemove) {
		if (result.size() != 0) result.clear();
		typename queryResult<T>::Results::iterator iter;
		for (iter = toRemove.begin(); iter != toRemove.end(); iter++) {
			if (targetDb.contains(*iter->name())) targetDb.dbStore().erase(*iter->name);
		}
		return ;
	}

	template<typename T>
	void queryResult<T>::drop(NoSqlDb::DbCore<T>& targetDb) {
		targetDb.dbStore().clear();
		return ;
	}

	// The newValue is a set of key-value pairs, the format of the set is exactly same with the query string,
	// except the all keys will appear no more than once.
	// The children item can be decorated with filters (or call operators), start with "$" and wrapped with "()".
	//		normal form: children: "["childname1", "childname2"]"
	//		with filter: children: "$(filter)["childname1", "childname2"]"
	// In update operation, valid filters are:
	//		1. $(add): add all children in the "[]", if the child is already existed, skip that child.
	//		2. $(remove): remove all children in the "[]", if the child is already removed, skip that child.
	//		3. $(replace): the instance's children relationship will be replaced by children in the "[]", 
	//						and this is the default-set filter.
	// All other filter string (and / or) decorating any other kind of value will be REJECTED.
	// The updateMode determines the function behavior if child in "[childrenName]" is not stored in the database,
	//		If updateMode = false (default-set), try to add a non-exist childName will be rejected (throw exception).
	//		If updateMode = true (need explict indicate), try to add a non-exist childName will result of a insert of 
	//													   a new dbElement("childName", "", "[]", "[""]") into database.
	// The updateMulti determines the function behavior if queryString returns more than one results,
	//		If updateMulti = true (default-set), all query result will get updated by newValue.
	//		If updateMulti = false (need explict indicate), query result > 1 will be rejected (throw exception).
	template<typename T>
	void queryResult<T>::update(NoSqlDb::DbCore<T>& targetDb, std::string queryString, std::string newValue, bool updateMode, bool updateMulti) {
		if (result.size() != 0) result.clear();
		internalFind(targetDb, queryString);
		kvPairs keyValuePair = keyValuePairGenerator(newValue, true);
		NoSqlDb::DbCore<T>::iterator iter;
		for (iter = db.begin(); iter != db.end(); iter++) {
			size_t pairLen = keyValuePair.size();
			for (int i = 0; i < pairLen; ++i) {
				if (keyValuePair[i].first == "description") {
					targetDb[iter->first].descrip(keyValuePair[i].second);
				}
				else if (keyValuePair[i].first == "dateTime") {
					targetDb[iter->first].dateTime(std::string(keyValuePair[i].second));
				}
				else if (keyValuePair[i].first == "payLoad") {
					targetDb[iter->first].payLoad(keyValuePair[i].second);
				}
				else if (keyValuePair[i].first == "children") {
					if (keyValuePair[i].second[0] == '$') {
						std::pair<std::string, std::string> filterChildren = filterSolver(keyValuePair[i].second);
						std::string filter = filterChildren.first;
						if (filter != "add" && filter != "remove" && filter != "replace") throw("filter usage invalid in query string: " + queryString);
						else {
							std::vector<std::string> childrenName = childrenNameGenerator(filterChildren.second);
							if (filter == "replace") {
								targetDb[iter->first].children().clear();
								for (int j = 0; j < childrenName.size(); ++i) {
									targetDb[iter->first].children().push_back(childrenName[j]);
								}
							}
							else if (filter == "remove") {
								std::vector<std::string>::iterator p = targetDb[iter->first].children().begin();
								std::vector<std::string>::iterator q = childrenName.begin();
								while (q != childrenName.end() && p != targetDb[iter->first].children().end()) {
									if (*q < *p) q++;
									else if (*q > *p) p++;
									else {
										p = targetDb[iter->first].children().erase(p);
										q++;
									}
								}
							}
							else if (filter == "add") {
								std::vector<std::string> *tmp = new std::vector<std::string>;
								std::vector<std::string>::iterator p = targetDb[iter->first].children().begin();
								std::vector<std::string>::iterator q = childrenName.begin();
								while (p != targetDb[iter->first].children().end() && q != childrenName.end()) {
									if (*q < *p) {
										tmp->push_back(*q);
										q++;
									}
									else if (*q > *p) {
										tmp->push_back(*p);
										p++;
									}
									else {
										tmp->push_back(*p);
										p++, q++;
									}
								}
								while (p != targetDb[iter->first].children().end()) {
									tmp->push_back(*p);
									p++;
								}
								while (q != childrenName.end()) {
									tmp->push_back(*q);
									q++;
								}
								targetDb[iter->first].children().clear();
								p = tmp->begin();
								while (p != tmp->end()) {
									targetDb[iter->first].children().push_back(*p);
								}
								delete tmp;
							}
						}
					}
					else {
						std::vector<std::string> childrenName = childrenNameGenerator(keyValuePair[i].second);
						targetDb[iter->first].children().clear();
						for (int j = 0; j < childrenName.size(); ++i) {
							targetDb[iter->first].children().push_back(childrenName[j]);
						}
					}
				}
			}
		}
		db.dbStore().clear();
		return;
	}

	template<typename T>
	std::vector<std::string> queryResult<T>::childrenNameGenerator(std::string& children) {
		std::vector<std::string> childrenName;
		std::string valueAfter = Utilities::unwrapPlus(children, '[');
		childrenName = Utilities::split(valueAfter);
		for (int i = 0; i < childrenName.size(); ++i) {
			childrenName[i] = Utilities::trim(childrenName[i]);
			childrenName[i] = Utilities::unwrapPlus(childrenName[i]);
		}
		std::sort(childrenName.begin(), childrenName.end(), [](std::string a, std::string b) { return a < b; });
		return childrenName;
	}

	template<typename T>
	typename queryResult<T>::Results queryResult<T>::dbToResult(bool clearDb) {
		typename NoSqlDb::DbCore<T>::iterator iter = db.begin();
		while (iter != db.end()) {
			result.push_back(iter->second);
			iter++;
		}
		if (clearDb == true) db.dbStore().clear();
		return result;
	}

	template<typename T>
	void queryResult<T>::resultDisplay(typename queryResult<T>::Results& toOutput) {
		std::cout << "\n  The query returns " << toOutput.size() << " result(s)" << std::endl;
		showHeader(std::cout);
		size_t vectorLen = toOutput.size();
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