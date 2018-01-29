#pragma once
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
		using Content = std::vector<NoSqlDb::DbElement<T>>;
		using Document = XmlProcessing::XmlDocument; 
		using sPtr = std::shared_ptr < XmlProcessing::AbstractXmlElement >;
	public:
		void persist(Content toPersist, const std::string& fileName, const std::string& path = "");
		Content restore(const std::string& fileName);
	};

	template<typename T>
	void persistence<T>::persist(Content toPersist, const std::string& fileName, const std::string& path) {
		sPtr root = XmlProcessing::makeTaggedElement("Database");
		Document file(XmlProcessing::makeDocElement(root));
		Content::iterator iter;
		for (iter = toPersist.begin(); iter != toPersist.end(); iter++) {
			// std::cout << (*iter).name() << " " << (*iter).descrip() << " " << std::string((*iter).dateTime()) << " " << (*iter).payLoad() << std::endl;
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
			sPtr payLoad = XmlProcessing::makeTaggedElement("payLoad");
			sPtr payLoadValue = XmlProcessing::makeTaggedElement("value");
			payLoadValue->addChild(XmlProcessing::makeTextElement((*iter).payLoad()));
			payLoad->addChild(payLoadValue);
			sPtr categories = XmlProcessing::makeTaggedElement("category");
			std::vector<std::string> categoryName = (*iter).category();
			std::vector<std::string>::iterator cateIter;
			for (cateIter = categoryName.begin(); cateIter != categoryName.end(); cateIter++) {
				sPtr category = XmlProcessing::makeTaggedElement("value");
				category->addChild(XmlProcessing::makeTextElement(*cateIter));
				categories->addChild(category);
			}
			payLoad->addChild(categories);
			record->addChild(payLoad);
			root->addChild(record);
		}
		std::ofstream saveFile(path + fileName + ".xml");
		saveFile << file.toString();
		std::cout << "Saved " << toPersist.size() << " to " << path << fileName << std::endl;
		return;
	}

	template<typename T>
	typename persistence<T>::Content persistence<T>::restore(const std::string& fileName) {
		Content collection;
		try {
			Document file(fileName, XmlProcessing::XmlDocument::file);
			std::vector<sPtr> records = file.elements("Database").select();
			std::cout << "Restore operation retrieves " << records.size() << " records from xml file" << std::endl;
			std::vector<sPtr>::iterator iter;
			for (iter = records.begin(); iter != records.end(); iter++) {
				NoSqlDb::DbElement<T> record;
				std::vector<sPtr> data = (*iter)->children();
				for (std::vector<sPtr>::iterator rData = data.begin(); rData != data.end(); rData++) {
					if ((*rData)->tag() == "name") {
						if ((*rData)->children().size() == 0) throw(std::exception("Bad XML content detected, at least one record has no name metadata. Restore terminates.\n"));
						auto nameString = (*rData)->children()[0];
						std::string name = Utilities::trim(nameString->value());
						record.name(name.substr(0, name.length() - 1));
					}
					else if ((*rData)->tag() == "description") {
						if ((*rData)->children().size() == 0) continue;
						auto descriptionString = (*rData)->children()[0];
						std::string description = Utilities::trim(descriptionString->value());
						record.descrip(description.substr(0, description.length() - 1));
					}
					else if ((*rData)->tag() == "dateTime") {
						if ((*rData)->children().size() == 0) continue;
						auto dateTimeString = (*rData)->children()[0];
						std::string dateTime = Utilities::trim(dateTimeString->value());
						dateTime = dateTime.substr(0, dateTime.length() - 1);
						record.dateTime(dateTime);
					}
					else if ((*rData)->tag() == "children") {
						std::vector<sPtr> childrenName = (*rData)->children();
						if (childrenName.size() == 0) continue;
						for (std::vector<sPtr>::iterator childIter = childrenName.begin(); childIter != childrenName.end(); childIter++) {
							auto childName = (*childIter)->children()[0];
							std::string childNameFine = Utilities::trim(childName->value());
							record.children().push_back(childNameFine.substr(0, childNameFine.length() - 1));
						}
					}
					else if ((*rData)->tag() == "payLoad") {
						std::vector<sPtr> payLoads = (*rData)->children();
						if (payLoads.size() != 2) throw(std::exception("Bad XML format detected, restore terminate.\n"));
						auto payLoadValue = (*(payLoads[0])).children()[0];
						std::string payLoadFine = Utilities::trim(payLoadValue->value());
						record.payLoad(payLoadFine.substr(0, payLoadFine.length() - 1));
						if ((*(payLoads[1])).children().size() != 0) {
							auto categories = (*(payLoads[1])).children()[0];
							std::vector<sPtr> categoriesName = categories->children();
							for (std::vector<sPtr>::iterator cateIter = categoriesName.begin(); cateIter != categoriesName.end(); cateIter++) {
								auto categoryName = (*cateIter)->children()[0];
								std::string categoryFine = Utilities::trim(categoryName->value());
								record.category().push_back(categoryFine.substr(0, categoryFine.length() - 1));
							}
						}
					}
				}
				/*std::cout << "One record" << std::endl;
				std::cout << record.name() << " " << record.descrip() << " " << std::string(record.dateTime()) << " " << record.payLoad() << std::endl;
				for (int i = 0; i < record.children().size(); ++i) std::cout << record.children()[i] << " ";
				std::cout << std::endl;
				for (int i = 0; i < record.category().size(); ++i) std::cout << record.category()[i] << " ";
				std::cout << std::endl;*/
				collection.push_back(record);
			}
			std::cout << "Restrored " << collection.size() << " records" << std::endl;
			return collection;
		}
		catch(std::exception& ex) {
			std::cout << "\n\n  " << ex.what();
			return collection;
		}
	}
}
#endif // !PERSISTENCE_H
