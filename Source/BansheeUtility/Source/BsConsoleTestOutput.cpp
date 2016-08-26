//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsConsoleTestOutput.h"

#include <iostream>

namespace BansheeEngine
{
	void ConsoleTestOutput::outputFail(const String& desc,
	                                   const String& function,
	                                   const String& file,
	                                   long line)
	{
		std::cout << file << ":" << line << ": failure: " << desc << std::endl;
	}
}
