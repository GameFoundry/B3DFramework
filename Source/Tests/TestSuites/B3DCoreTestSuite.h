//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class CoreTestSuite : public TestSuite
	{
	public:
		CoreTestSuite();

	private:
		void TestAnimCurveIntegration();
		void TestLookupTable();
		void TestBinarySerialization();
		void TestDataBlockSerialization();
		void TestSerializedObject();
		void TestBinaryDelta();
	};
} // namespace b3d
