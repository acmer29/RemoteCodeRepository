#include "TestProj2.h"
using namespace SWRTB;

// ----< demo requirement #1 >-----
bool testProj2::testR1() {
	Utilities::Title("#1: Use VS2017 and standard C++ libraries.\n");
	std::cout << "  " << typeid(std::function<bool()>).name()
		<< ", declared in this function, "
		<< "\n  is only valid for C++11 and later versions.";
	return true;
}

// ----< demo requirement #2 >-----
bool testProj2::testR2() {
	Utilities::Title("#2: Use standard library streams and heap management.\n");
	std::cout << " A visual examination of all the submitted code "
		<< "will show only\n  use of streams and operators new and delete.";
	return true;
}

// ----< demo requirement #3 >-----
bool testProj2::testR3() {
	Utilities::Title("#3: Provide packages including TestExecutive, \
					RepositoryCore,	Check-in, Check-out, Version and Browse");
	// TODO: Add isDirectory code.
	return true;
}

// ----< demo requirement #4 >-----
bool testProj2::testR4() {
	Utilities::Title("#4: Demostrate check-in function.\n");
	// TODO: Add test code.
	return true;
}

#ifdef TEST_TESTPROJ2
int main() {
	return 0;
}
#endif