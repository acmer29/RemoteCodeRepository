#pragma once
////////////////////////////////////////////////////////////////////////////
// LoopHandler.h - Provide means for detect loops						  //
// ver 1.0																  //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018                //
////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - LoopHandler.
* This class provides means to detect the loop, list loop components, and 
* check if a record is in the loop.
*
* The public interface function operations are as below:
*  - remakeGraph: Provides when records for checking changes, remake the
*	 directed graph and run loop detect algorithm
*  - isLoopExists: Return there is a loop, it only check size of loop item vector
*	 so it is O(1) complexity.
*  - isInloop: Return if the record for check is a component of the loop, 
*    accepts the name field of the record.
*  - listLoopItems: Return the loop item vector, which is a vector of string.
*
* Notes:
* ------
* - Designed to provide all functionality in header file.
* - Implementation file only needed for test and demo.
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
*
* Build Process:
* --------------
* devenv SoftwareRepoTB.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 28 Feb 2018 - First release
*/
#ifndef LOOPHANDLER_H
#define LOOPHANDLER_H
#include "../NoSqlDb/DbCore/DbCore.h"
#include <vector>
#include <queue>
#include <iostream>
namespace SWRTB {
	class LoopHandler {
	public:
		using DbRecords = std::vector<NoSqlDb::DbElement<std::string>>;

		explicit LoopHandler(const DbRecords& records);

		void remakeGraph(const DbRecords& records);
		
		bool isLoopExists();

		bool isInLoop(const std::string& NSNFileNameVersion);
		
		std::vector<std::string> listLoopItems();

	private:

		// data members
		std::unordered_map<std::string, int> hashHelper;
		std::vector<std::vector<bool>> abstractGraph;
		std::vector<std::string> loopRecords;

		// helper functions
		void makeGraph(const DbRecords& records);
		void ListAbstractLoopItems();
		void abstractToActual(const std::vector<int>& abstractItems);
		
	};

	// -----< Constructor: make the graph and run loop detect algorithm >-----
	inline LoopHandler::LoopHandler(const DbRecords& records) {
		makeGraph(records);
		ListAbstractLoopItems();
	}

	// -----< remakeGraph: remake the graph, use when test cases changes >-----
	inline void LoopHandler::remakeGraph(const DbRecords& records) {
		abstractGraph.clear();
		hashHelper.clear();
		makeGraph(records);
		ListAbstractLoopItems();
		return;
	}

	// -----< isLoopExists: Checkin if loop exists, O(1) complexity >-----
	inline bool LoopHandler::isLoopExists() {
		return loopRecords.size();
	}

	// -----< isInLoop: Accepts a DbElement.name() parameter, checkin if this record is a component of the loop >-----
	inline bool LoopHandler::isInLoop(const std::string& NSNFileNameVersion) {
		for (auto item : loopRecords) {
			if (item == NSNFileNameVersion) return true;
		}
		return false;
	}

	// -----< listLoopItems: List all componments in the loop >-----
	inline std::vector<std::string> LoopHandler::listLoopItems() {
		return loopRecords;
	}

	// -----< makeGraph: Map the DbElements into numbers and use these numbers to make directed graph >-----
	inline void LoopHandler::makeGraph(const DbRecords& records) {
		int recordSize = records.size();
		abstractGraph.resize(recordSize);
		for (int i = 0; i < recordSize; ++i)
			abstractGraph[i].resize(recordSize);
		int index = 0;
		auto iter = records.begin();
		for (auto iter = records.begin(); iter != records.end(); iter++) {
			if (hashHelper.find(iter->name()) == hashHelper.end()) {
				hashHelper[iter->name()] = index++;
			}
		}
		for (auto iter = records.begin(); iter != records.end(); iter++) {
			for (auto childIter = iter->children().begin(); childIter != iter->children().end(); childIter++) {
				if (hashHelper.find(*childIter) != hashHelper.end())
					abstractGraph[hashHelper[*childIter]][hashHelper[iter->name()]] = true;
			}
		}		
		return;
	}

	// -----< ListAbstractLoopItems: Typical topsort algorithm >----------------------
	// -----< Only who failed CIS675 or passed by cheating cannot understand it >----- 
	inline void LoopHandler::ListAbstractLoopItems() {
		int num = abstractGraph.size(), current = -1;
		std::vector<int> result;
		std::queue<int> queHelper;
		if (num == 0) return;
		std::vector<int> depend(num);
		for (int i = 0; i < num; ++i) {
			for (int j = 0; j < num; ++j) {
				if (abstractGraph[j][i] == true) depend[i] += 1;
			}
			if (depend[i] == 0) queHelper.push(i);
		}
		while (queHelper.empty() == false) {
			int over = queHelper.front();
			depend[over] = -1;
			queHelper.pop();
			for (int i = 0; i < num; ++i) {
				depend[i] -= abstractGraph[over][i];
				abstractGraph[over][i] = false;
				if (depend[i] == 0) queHelper.push(i);
			}
		}
		for (int i = 0; i < num; ++i) {
			if (depend[i] != -1) result.push_back(i);
		}
		abstractToActual(result);
		return;
	}

	// -----< abstractToActual: Fill loopRecords with looped items list with the result of loopItems >-----
	inline void LoopHandler::abstractToActual(const std::vector<int>& abstractItems) {
		for (auto index : abstractItems) {
			for (auto hashIter = hashHelper.begin(); hashIter != hashHelper.end(); hashIter++) {
				if (index == hashIter->second) {
					loopRecords.push_back(hashIter->first);
					break;
				}
			}
		}
		return;
	}
}

#endif // !LOOPHANDLER_H