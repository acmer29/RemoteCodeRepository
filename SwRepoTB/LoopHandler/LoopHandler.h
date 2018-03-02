#pragma once
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

		std::unordered_map<std::string, int> hashHelper;
		std::vector<std::vector<bool>> abstractGraph;
		std::vector<std::string> loopRecords;

		void makeGraph(const DbRecords& records);
		void ListAbstractLoopItems();
		void abstractToActual(const std::vector<int>& abstractItems);
		
	};

	inline LoopHandler::LoopHandler(const DbRecords& records) {
		makeGraph(records);
		ListAbstractLoopItems();
	}

	inline void LoopHandler::remakeGraph(const DbRecords& records) {
		abstractGraph.clear();
		hashHelper.clear();
		makeGraph(records);
		ListAbstractLoopItems();
		return;
	}

	inline bool LoopHandler::isLoopExists() {
		return loopRecords.size();
	}

	inline bool LoopHandler::isInLoop(const std::string& NSNFileNameVersion) {
		for (auto item : loopRecords) {
			if (item == NSNFileNameVersion) return true;
		}
		return false;
	}

	inline std::vector<std::string> LoopHandler::listLoopItems() {
		return loopRecords;
	}

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