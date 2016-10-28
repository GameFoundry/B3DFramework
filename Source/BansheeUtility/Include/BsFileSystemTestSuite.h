//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsTestSuite.h"

namespace BansheeEngine
{
	class FileSystemTestSuite : public TestSuite
	{
	public:
		FileSystemTestSuite();
		virtual void startUp();
		virtual void shutDown();

	private:
		Path testDirectory;
		void testExists_yes_file();
		void testExists_yes_dir();
		void testExists_no();
		void testGetFileSize_zero();
		void testGetFileSize_not_zero();
		void testIsFile_yes();
		void testIsFile_no();
		void testIsDirectory_yes();
		void testIsDirectory_no();
		void testRemove_file();
		void testRemove_directory();
		void testMove();
		void testMove_overwrite_existing();
		void testMove_no_overwrite_existing();
		void testCopy();
		void testCopy_recursive();
		void testCopy_overwrite_existing();
		void testCopy_no_overwrite_existing();
		void testGetChildren();
		void testGetLastModifiedTime();
		void testGetTempDirectoryPath();
	};
}
