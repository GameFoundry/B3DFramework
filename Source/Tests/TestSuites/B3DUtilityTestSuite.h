//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class UtilityTestSuite : public TestSuite
	{
	public:
		UtilityTestSuite();

	private:
		void TestBitfield();
		void TestOctree();
		void TestInlineArray();
		void TestArray();
		void TestComplex();
		void TestMinHeap();
		void TestQuadtree();
		void TestVarInt();
		void TestBitStream();
		void TestRTTIIterator();
		void TestMPSCQueue();
		void TestSPSCQueue();
		void TestHashedString();
	};
} // namespace b3d
