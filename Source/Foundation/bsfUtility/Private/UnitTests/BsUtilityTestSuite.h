//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		void testBitfield();
		void testOctree();
		void testSmallVector();
		void testDynArray();
		void testComplex();
		void testMinHeap();
		void testQuadtree();
		void testVarInt();
		void testBitStream();
	};
}
