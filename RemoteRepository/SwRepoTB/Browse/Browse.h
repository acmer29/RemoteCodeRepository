#pragma once
////////////////////////////////////////////////////////////////////////
// Browse.h - Provide means to display file attributs, and contents   //
// ver 1.1                                                            //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018            //
////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - Browse
* Browse class provides means to display a Db record's all metadata, 
* include name, dateTime, description, category, dependecies, path, status, 
* and uses notepad as application to display its content.
* 
* The public interface function operations are as below:
*  - browse : Display file attributes, and uses notepad to display file content
*
* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* DateTime.h, DateTime.cpp
* Process.h
* FileSystem.h, FileSystem.cpp
* SWRTBUtilities.h
*
* Notes:
* ------
* - Designed to provide all functionality in header file.
* - Implementation file only needed for test and demo.
*
* Build Process:
* --------------
* devenv SoftwareRepoTB.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 28 Feb 2018 - First release
* ver 1.1 : 04 Mar 2018 - Add display file attribute feature
*/
#ifndef BROWSE_H
#define BROWSE_H
#include "../NoSqlDb/DbCore/DbCore.h"
#include "../Process/Process/Process.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../SWRTBUtilities/SWRTBUtilities.h"
namespace SWRTB {
	class Browse {
	public:
		void browse(const NoSqlDb::DbElement<std::string>& toBrowse, std::ostream& out = std::cout);
	private:
		Process worker;
	};

	// -----< browse: Display file specs, and uses notepad to display file content >----------------
	inline void Browse::browse(const NoSqlDb::DbElement<std::string>& toBrowse, std::ostream& out) {
		
		out << "  Browse: The name of the file: " << toBrowse.name() << std::endl;
		out << "  Browse: Last modification time of the file: " << std::string(toBrowse.dateTime()) << std::endl;
		out << "  Browse: The description of the file: " << toBrowse.descrip() << std::endl;
		out << "  Browse: The category of the file: " << std::endl;
		for (auto item : toBrowse.category()) {
			out << "                                   -" << item << std::endl;
		}
		out << "  Browse: The path of the file: " << pathNSPFileNameVersionOf(toBrowse.payLoad()) << std::endl;
		out << "  Browse: The current status of the file: " << modeOf(toBrowse.payLoad()) << std::endl;
		out << "  Browse: The dependencies of the file: " << std::endl;
		for (auto item : toBrowse.children()) {
			out << "                                   -" << item << std::endl;
		}

		std::string appPath = "c:/windows/system32/notepad.exe";
		worker.application(appPath);
		worker.commandLine("/A " + pathNSPFileNameVersionOf(toBrowse.payLoad()));
		worker.create();
		return;
	}
}
#endif // !BROWSE_H
