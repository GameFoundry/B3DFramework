//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/BsTestSuite.h"

namespace bs
{
	class FileSystemTestSuite : public TestSuite
	{
	public:
		FileSystemTestSuite();
		void StartUp() ;
		void ShutDown() ;

	private:
		void TestExistsYesFile();
		void TestExistsYesDir();
		void TestExistsNo();
		void TestGetFileSizeZero();
		void TestGetFileSizeNotZero();
		void TestIsFileYes();
		void TestIsFileNo();
		void TestIsDirectoryYes();
		void TestIsDirectoryNo();
		void TestRemoveFile();
		void TestRemoveDirectory();
		void TestMove();
		void TestMoveOverwriteExisting();
		void TestMoveNoOverwriteExisting();
		void TestCopy();
		void TestCopyRecursive();
		void TestCopyOverwriteExisting();
		void TestCopyNoOverwriteExisting();
		void TestGetChildren();
		void TestGetLastModifiedTime();
		void TestGetTempDirectoryPath();

		Path mTestDirectory;
	};
} // namespace bs
