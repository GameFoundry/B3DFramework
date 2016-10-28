//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFileSystemTestSuite.h"
#include "BsConsoleTestOutput.h"

using namespace BansheeEngine;

int main()
{

	SPtr<TestSuite> tests = FileSystemTestSuite::create<FileSystemTestSuite>();
	ConsoleTestOutput testOutput;
	tests->run(testOutput);

	return 0;
}
