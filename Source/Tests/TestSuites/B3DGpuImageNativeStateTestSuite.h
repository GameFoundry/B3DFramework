//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** CPU tests of native image state ownership, deferred accesses and submission hazards. */
	class GpuImageNativeStateTestSuite : public TestSuite
	{
	public:
		GpuImageNativeStateTestSuite();

	private:
		/** Verifies independent partitions inherit deferred accesses and barriers in command order. */
		void TestRangeSplits();

		/** Verifies only selected native writes acquire readers and publish a writer epoch. */
		void TestSubmissionSelection();
	};
}
