#pragma once
////////////////////////////////////////////////////////////////////////////
// TestProj2.h - Provide means for demostrating Project2				  //
// ver 1.0																  //
// Tianyu Qi, CSE687 - Object Oriented Design, Spring 2018                //
////////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provides one class - TestProj2.
* This class provides means for demostrating requirement of Project2.
* This class provides one testCase runner and several test functions.

* Required Files:
* ---------------
* SWRepoCore.h
* SWRTBUtilities.h
* Checkin.h, Checkin.cpp
* Checkout.h, Checkout.cpp
* XMLDocument.h, XMLDocument.cpp
* Test.h, Test.cpp, 
* DateTime.h, DateTime.cpp
* FileSystem.h, FileSystem.cpp
* DbCore.h, DbCore.cpp
*
* Build Process:
* --------------
* devenv NoSqlDb.sln /rebuild debug
*
* Maintenance History:
* --------------------
* ver 1.0 : 28 Feb 2018 - First release
*/
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
		bool testR4a();
		bool testR4b();
		bool testR4c();
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
