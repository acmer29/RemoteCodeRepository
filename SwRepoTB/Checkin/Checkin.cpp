#include "Checkin.h"

using namespace SWRTBCheckin;

void Checkin::checkin(const std::string& path, const std::string& fileName) {
	std::vector<std::string> fileNames;
	if (fileDir.exists(path)) {
		fileNames = fileDir.getFiles(path, fileName);
	}
	else throw std::exception("Check-in: Invalid path given.\n");

	return;
}

#ifdef TEST_CHECKIN
int main() {
	return 0;
}
#endif // TEST_CHECKIN
