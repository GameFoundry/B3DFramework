//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Tests Vulkan pipeline-layout identity for native push constants. */
	class VulkanPushConstantTestSuite : public TestSuite
	{
	public:
		VulkanPushConstantTestSuite();

	private:
		/** Checks that push-constant size, offset, and stage visibility participate in pipeline-layout identity. */
		void TestPipelineLayoutKey();
	};
} // namespace b3d
