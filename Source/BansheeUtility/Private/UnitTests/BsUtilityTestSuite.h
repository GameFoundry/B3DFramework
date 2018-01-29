//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Testing/BsTestSuite.h"

namespace bs
{
	class UtilityTestSuite : public TestSuite
	{
	public:
		UtilityTestSuite();
		void startUp() override;
		void shutDown() override;

	private:
		void testOctree();
	};
}
