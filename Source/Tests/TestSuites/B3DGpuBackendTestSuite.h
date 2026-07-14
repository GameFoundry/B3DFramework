//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests for backend-independent GPU resource tracking and synchronization logic. */
	class GpuBackendTestSuite : public TestSuite
	{
	public:
		GpuBackendTestSuite();

	private:
		/** Verifies that accesses are accumulated into barrier-delimited hazard tracking epochs. */
		void TestHazardTrackingAccessEpochs();

		/** Verifies cross-command-buffer dependencies and propagation of unresolved hazards. */
		void TestCrossCommandBufferRecipe();
	};
} // namespace b3d
