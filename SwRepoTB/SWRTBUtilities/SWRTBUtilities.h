#pragma once
#ifndef SWRTBUTILITIES_H
#define SWRTBUTILITIES_H
#include <string>
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
namespace SWRTB {
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
	inline std::string nameConcater(const std::string& fileName, const std::string& nameSpace, const std::string& seperator) {
		return (nameSpace + seperator + fileName);
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
}
#endif