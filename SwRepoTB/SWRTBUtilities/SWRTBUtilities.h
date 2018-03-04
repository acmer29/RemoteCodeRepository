#pragma once
////////////////////////////////////////////////////////////////////////////
// SWRTBUtilities.h - Contains all miscellaneous helper functions used    //
//					  in the whole Project2								  //
// ver 1.0																  //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018                //
////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides no class, used as miscellaneous package
* This package provides all helper functions used in all other packages.
* 
* The public interface function operations are as below:
*  - isFile : Check if the path represents a file
*  - isDirectory : Check if the path represents a directory
*  - nameCleaner : Remove the "nameSpace::" from the "nameSpace::fileName"
*  - nameConcater : Concate the nameSpace with fileName by seperator
*  - NSPFileNameToNSNFileName : Convert nameSpace_fileName to nameSpace::fileName
*  - NSPFileNameToNSNFileName : Convert nameSpace::fileName to nameSpace_fileName
*  - copyFile : copy "path/to/source/file.ext" to "path/to/target/file.ext"
*  - pathNSPFileNameVersionOf : Returns the path part of the payLoad of a DB record
*  - modeOf : Returns the mode / status of the payLoad of a DB record
*  - changeFileMode : Changes the mode of a file from current mode to newMode
*  - checkFileMode : Returns the mode of a file by accepting its Db record's payLoad

* Required Files:
* ---------------
* DbCore.h, DbCore.cpp
* FileSystem.h, FileSystem.cpp
*
* Build Process:
* --------------
* devenv NoSqlDb.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 28 Feb 2018 - First release
*/
#ifndef SWRTBUTILITIES_H
#define SWRTBUTILITIES_H
#include <string>
#include <vector>
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
#include "../NoSqlDb/DbCore/DbCore.h"
namespace SWRTB {

	// -----< isFile: Check if the path represents a file >-----
	inline bool isFile(const std::string& path) {
		DWORD dwAttrib = GetFileAttributesA(path.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0);
	}

	// -----< isDirectory: Check if the path represents a directory >-----
	inline bool isDirectory(const std::string& path) {
		DWORD dwAttrib = GetFileAttributesA(path.c_str());
		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY) != 0);
	}

	// -----< nameCleaner: Remove the "nameSpace::" from the nameSpace::fileName >-----
	inline std::string nameCleaner(const std::string& NSNFileName) {
		size_t start = 0, end = NSNFileName.length() - 1;
		while (start != end) {
			if (NSNFileName[start] == ':' && NSNFileName[start + 1] != ':') {
				start += 1;
				break;
			}
			else start += 1;
		}
		return NSNFileName.substr(start, NSNFileName.length());
	}

	// -----< nameConcater: Concate the nameSpace with fileName by seperator >------------------------------------------------
	inline std::string nameConcater(const std::string& fileName, const std::string& nameSpace, const std::string& seperator) {
		return (nameSpace + seperator + fileName);
	}

	// -----< NSPFileNameToNSNFileName: Convert nameSpace_fileName to nameSpace::fileName >-----
	inline std::string NSPFileNameToNSNFileName(const std::string& NSPFileName) {
		if (NSPFileName == "") return "";
		std::string result = NSPFileName;
		std::string::size_type toReplace = result.find_first_of("_");
		if (toReplace == std::string::npos) throw std::exception("NSPFileNameToNSNFileName: Invalid NSPFileName given.\n");
		return result.replace(toReplace, 1, "::");
	}

	// -----< NSPFileNameToNSNFileName: Convert nameSpace::fileName to nameSpace_fileName >-----
	inline std::string NSNFileNameToNSPFileName(const std::string& NSNFileName) {
		if (NSNFileName == "") return "";
		std::string result = NSNFileName;
		std::string::size_type toReplace = result.find_first_of("::");
		if (toReplace == std::string::npos) throw std::exception("NSNFileNameToNSPFileName: Invalid NSNFileName given.\n");
		return result.replace(toReplace, 2, "_");
	}

	// -----< copyFile: copy "path/to/source/file.ext" to "path/to/target/file.ext" >-----
	inline void copyFile(const std::string& fromPathFileName, const std::string& toPathFileName) {
		if (fromPathFileName == toPathFileName) return;
		FileSystem::File me(fromPathFileName);
		me.open(FileSystem::File::in, FileSystem::File::binary);
		if (!me.isGood())
			throw std::exception("CopyFile: Bad state of source file.\n");
		FileSystem::File you(toPathFileName);
		you.open(FileSystem::File::out, FileSystem::File::binary);
		if (you.isGood()) {
			while (me.isGood()) {
				FileSystem::Block filePiece = me.getBlock(1024);
				you.putBlock(filePiece);
			}
		}
		else throw std::exception("CopyFile: Bad state of target file.\n");
	}

	// -----< pathNSPFileNameVersionOf: Returns the path part of the payLoad of a DB record >-----
	inline std::string pathNSPFileNameVersionOf(const std::string& payLoad) {
		size_t index = payLoad.find('$');
		if (index == std::string::npos) throw::std::exception("pathNSPFileNameVersionOf: Cannot find the $ seperator.\n");
		return payLoad.substr(0, index);
	}

	// -----< modeOf: Returns the mode / status of the payLoad of a DB record >-----
	inline std::string modeOf(const std::string& payLoad) {
		size_t index = payLoad.find('$');
		if (index == std::string::npos) throw::std::exception("modeOf: Cannot find the $ seperator.\n");
		return payLoad.substr(index + 1, payLoad.length());
	}

	// -----< changeFileMode: Changes the mode of a file from current mode to newMode >--------
	inline std::string changeFileMode(const std::string& payLoad, const std::string& newMode) {
		size_t index = payLoad.find('$');
		if (index != std::string::npos)
			return pathNSPFileNameVersionOf(payLoad) + "$" + newMode;
		else return payLoad + "$" + newMode;
	}

	// -----< checkFileMode: Returns the mode of a file by accepting its Db record's payLoad >-----
	inline bool checkFileMode(const std::string& payLoad, const std::string& toCheck) {
		size_t i = payLoad.length() - 1, j = toCheck.length() - 1;
		while (j != 0) {
			if (payLoad[i] != toCheck[j]) return false;
			i -= 1, j -= 1;
		}
		return true;
	}

	inline void displayCore(const std::vector<NoSqlDb::DbElement<std::string>>& result, std::ostream& out = std::cout) {
		NoSqlDb::showHeader();
		for (auto iter = result.begin(); iter != result.end(); iter++) {
			out << "\n  ";
			out << std::setw(26) << std::left << std::string((*iter).dateTime());
			out << std::setw(10) << std::left << (*iter).name();
			out << std::setw(25) << std::left << (*iter).descrip();
			out << std::setw(25) << std::left << pathNSPFileNameVersionOf((*iter).payLoad());
			if ((*iter).children().size() > 0)
			{
				out << "\n    dependencies: ";
				for (auto key : (*iter).children())
					out << " " << key; 
			}
			if ((*iter).category().size() > 0)
			{
				out << "\n    category: ";
				for (auto key : (*iter).category())
					out << " " << key;
			}
			out << "\n    status: " << modeOf((*iter).payLoad());
		}
		out << std::endl;
		return;
	}
}
#endif