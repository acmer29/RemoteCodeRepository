#pragma once
/////////////////////////////////////////////////////////////////////
// Persistence.h - Implements persist functions,				   //
//					include persist and restore                    //
// ver 1.1                                                         //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018         //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - persistence
* persistence class provides two functions, persist and restore
* persist saves results from query into a XML file
* restore retrieves records in the XML file into a std::vector, which can use query + insert load them into the database
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* StringUtilities.h, StringUtilities.cpp
* Query.h Query.cpp
* XmlDocument.h, XmlDocument.cpp
*
* Maintenance History:
* --------------------
* ver 1.0 : 28 Jan 2018 - Finished persist and restore function
* ver 1.1 : 31 Jan 2018 - Refactored restore function
*/
#ifndef PERSISTENCE_H
#define PERSISTENCE_H
#include <iostream>
#include <fstream>
#include "../DbCore/DbCore.h"
#include "../XmlDocument/XmlDocument/XmlDocument.h"
#include "../XmlDocument/XmlElement/XmlElement.h"
#include "../XmlDocument/XmlParser/XmlParser.h"
namespace DbPersistence {
	template<typename T>
	class persistence {
	public:

		using Content = std::vector<NoSqlDb::DbElement<T>>;
		using Document = XmlProcessing::XmlDocument;
		using sPtr = std::shared_ptr < XmlProcessing::AbstractXmlElement >;
		
		void persist(Content toPersist, const std::string& fileName);
		
		Content restore(const std::string& fileName);

	private:

		void restoreName(NoSqlDb::DbElement<T>& record, const sPtr& rData);
		void restoreDescription(NoSqlDb::DbElement<T>& record, const sPtr& rData);
		void restoreDateTime(NoSqlDb::DbElement<T>& record, const sPtr& rData);
		void restorePayLoads(NoSqlDb::DbElement<T>& record, const sPtr& rData);
		void restoreChildren(NoSqlDb::DbElement<T>& record, const sPtr& rData);
	};

	// Persist a vector of DbElement into "fileName".xml
	template<typename T>
	void persistence<T>::persist(Content toPersist, const std::string& fileName) {
		sPtr root = XmlProcessing::makeTaggedElement("Database");
		Document file(XmlProcessing::makeDocElement(root));
		Content::iterator iter;
		for (iter = toPersist.begin(); iter != toPersist.end(); iter++) {
			sPtr record = XmlProcessing::makeTaggedElement("record");
			sPtr name = XmlProcessing::makeTaggedElement("name");
			name->addChild(XmlProcessing::makeTextElement((*iter).name()));
			record->addChild(name);
			sPtr description = XmlProcessing::makeTaggedElement("description");
			description->addChild(XmlProcessing::makeTextElement((*iter).descrip()));
			record->addChild(description);
			sPtr dateTime = XmlProcessing::makeTaggedElement("dateTime");
			dateTime->addChild(XmlProcessing::makeTextElement(std::string((*iter).dateTime())));
			record->addChild(dateTime);
			sPtr children = XmlProcessing::makeTaggedElement("children");
			std::vector<std::string> childrenName = (*iter).children();
			std::vector<std::string>::iterator childIter;
			for (childIter = childrenName.begin(); childIter != childrenName.end(); childIter++) {
				sPtr childName = XmlProcessing::makeTaggedElement("childName");
				childName->addChild(XmlProcessing::makeTextElement(*childIter));
				children->addChild(childName);
			}
			record->addChild(children);
			sPtr payLoad = XmlProcessing::makeTaggedElement("payLoads");
			sPtr payLoadValue = XmlProcessing::makeTaggedElement("payLoad");
			payLoadValue->addChild(XmlProcessing::makeTextElement((*iter).payLoad()));
			payLoad->addChild(payLoadValue);
			sPtr categories = XmlProcessing::makeTaggedElement("categories");
			std::vector<std::string> categoryName = (*iter).category();
			std::vector<std::string>::iterator cateIter;
			for (cateIter = categoryName.begin(); cateIter != categoryName.end(); cateIter++) {
				sPtr category = XmlProcessing::makeTaggedElement("category");
				category->addChild(XmlProcessing::makeTextElement(*cateIter));
				categories->addChild(category);
			}
			payLoad->addChild(categories);
			record->addChild(payLoad);
			root->addChild(record);
		}
		std::ofstream saveFile(fileName + ".xml");
		saveFile << file.toString();
		std::cout << "Saved " << toPersist.size() << " to " << fileName << std::endl;
		return;
	}

	// Restore all records from "fileName" into a vector of DbElement
	template<typename T>
	typename persistence<T>::Content persistence<T>::restore(const std::string& fileName) {
		Content collection;
		Document file(fileName, XmlProcessing::XmlDocument::file);
		std::vector<sPtr> records = file.elements("Database").select();
		std::vector<sPtr>::iterator iter;
		for (iter = records.begin(); iter != records.end(); iter++) {
			NoSqlDb::DbElement<T> record;
			std::vector<sPtr> data = (*iter)->children();
			for (std::vector<sPtr>::iterator rData = data.begin(); rData != data.end(); rData++) {
				if ((*rData)->tag() == "name") restoreName(record, (*rData));
				else if ((*rData)->tag() == "description") restoreDescription(record, (*rData));
				else if ((*rData)->tag() == "dateTime") restoreDateTime(record, (*rData));
				else if ((*rData)->tag() == "children") restoreChildren(record, (*rData));
				else if ((*rData)->tag() == "payLoads") restorePayLoads(record, (*rData));
			}
			collection.push_back(record);
		}
		return collection;
	}

	template<typename T>
	void persistence<T>::restoreName(NoSqlDb::DbElement<T>& record, const sPtr& rData) {
		if (rData->children().size() == 0) 
			throw std::exception("Bad XML content detected, at least one record has no name metadata. Restore terminates.\n");
		auto nameString = rData->children()[0];
		std::string name = Utilities::trim(nameString->value());
		record.name(name.substr(0, name.length() - 1));
		return;
	}

	template<typename T>
	void persistence<T>::restoreDescription(NoSqlDb::DbElement<T>& record, const sPtr& rData) {
		if (rData->children().size() == 0) return;
		auto descriptionString = rData->children()[0];
		std::string description = Utilities::trim(descriptionString->value());
		record.descrip(description.substr(0, description.length() - 1));
		return;
	}

	template<typename T>
	void persistence<T>::restoreDateTime(NoSqlDb::DbElement<T>& record, const sPtr& rData) {
		if (rData->children().size() == 0) return;
		auto dateTimeString = rData->children()[0];
		std::string dateTime = Utilities::trim(dateTimeString->value());
		record.dateTime(dateTime.substr(0, dateTime.length() - 1));
		return;
	}

	template<typename T>
	void persistence<T>::restoreChildren(NoSqlDb::DbElement<T>& record, const sPtr& rData) {
		std::vector<sPtr> childrenName = rData->children();
		if (childrenName.size() == 0) return;
		for (std::vector<sPtr>::iterator childIter = childrenName.begin(); childIter != childrenName.end(); childIter++) {
			if ((*childIter)->children().size() == 0) break;
			auto childName = (*childIter)->children()[0];
			std::string childNameFine = Utilities::trim(childName->value());
			record.children().push_back(childNameFine.substr(0, childNameFine.length() - 1));
		}
		return;
	}

	template<typename T>
	void persistence<T>::restorePayLoads(NoSqlDb::DbElement<T>& record, const sPtr& rData) {
		std::vector<sPtr> payLoads = rData->children();
		if (payLoads.size() != 2) throw std::exception("Bad XML format detected, restore terminate.\n");
		auto payLoadValue = (*(payLoads[0])).children()[0];
		std::string payLoadFine = Utilities::trim(payLoadValue->value());
		record.payLoad(payLoadFine.substr(0, payLoadFine.length() - 1));
		if ((*(payLoads[1])).children().size() != 0) {
			auto categories = (*(payLoads[1])).children()[0];
			std::vector<sPtr> categoriesName = categories->children();
			for (std::vector<sPtr>::iterator cateIter = categoriesName.begin(); cateIter != categoriesName.end(); cateIter++) {
				if ((*cateIter)->children().size() == 0) break;
				auto categoryName = (*cateIter)->children()[0];
				std::string categoryFine = Utilities::trim(categoryName->value());
				record.category().push_back(categoryFine.substr(0, categoryFine.length() - 1));
			}
		}
	}
}
#endif // !PERSISTENCE_H
