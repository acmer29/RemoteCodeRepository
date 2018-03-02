#pragma once
#ifndef BROWSE_H
#define BROWSE_H
#include "../Process/Process/Process.h"
#include "../FileSystem-Windows/FileSystemDemo/FileSystem.h"
namespace SWRTB {
	class Browse {
	public:
		void browse(const std::string& pathFileNameVersion);
	private:
		Process worker;
	};

	inline void Browse::browse(const std::string& pathFileNameVersion) {
		std::string appPath = "c:/windows/system32/notepad.exe";
		worker.application(appPath);
		worker.commandLine("/A " + pathFileNameVersion);
		worker.create();
		std::function<void(void)> callback = []() { std::cout << "\n  --- child process exited with this message ---"; };
		worker.setCallBackProcessing(callback);
		worker.registerCallback();
		return;
	}
}
#endif // !BROWSE_H
