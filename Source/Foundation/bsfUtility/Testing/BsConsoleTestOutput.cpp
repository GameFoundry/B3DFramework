//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Testing/BsConsoleTestOutput.h"

#include <iostream>

using namespace bs;

void ConsoleTestOutput::OutputFail(const String& desc, const String& function, const String& file, long line)
{
	std::cout << file << ":" << line << ": failure: " << desc << std::endl;
}
