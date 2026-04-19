//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	class ECSTestSuite : public TestSuite
	{
	public:
		ECSTestSuite();
		void StartUp() override;
		void ShutDown() override;

	private:
		void TestSparseSet();
		void TestComponentSparseSet();
		void TestRegistry();
		void TestView();
		void TestRuntimeView();
		void TestOwningGroup();
		void TestOwningGroupWithIncluded();
		void TestOwningGroupWithExcluded();
		void TestNonOwningGroup();
	};
} // namespace b3d
