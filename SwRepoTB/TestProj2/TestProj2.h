#pragma once
#ifndef TESTPROJ2_H
#define TESTPROJ2_H
#include "../SoftwareRepoTB/SWRepoCore.h"
#include "../Checkin/Checkin.h"
#include "../Checkout/Checkout.h"
#include "../Browse/Browse.h"
#include "../NoSqlDb/Test/Test.h"
namespace SWRTB {
	class testProj2 {
	public:
		bool testR1();
		bool testR2();
		bool testR3();
		bool testR4();
		bool testR5();
		bool testR6();
		bool testR7();
		void casesRun(std::ostream& out = std::cout);
	private:
		std::vector<std::_Mem_fn<bool(testProj2::*) ()>> cases;
		void check(bool result, std::ostream& out);
	};
}
#endif // !TESTPROJ2_H
