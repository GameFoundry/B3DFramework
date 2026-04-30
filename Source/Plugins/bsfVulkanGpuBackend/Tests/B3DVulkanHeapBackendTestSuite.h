//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Plugin-private self-tests for VulkanHeapBackend. */
	class VulkanHeapBackendTestSuite : public TestSuite
	{
	public:
		VulkanHeapBackendTestSuite();

	private:
		/** A 1 MiB host-visible heap can be allocated, persistently mapped, written to, and freed. */
		void TestHostVisibleHeapCreateAndDestroy();

		/** A 1 MiB device-local heap can be allocated and freed (no mapping, mirrors texture/VBO usage). */
		void TestDeviceLocalHeapCreateAndDestroy();
	};
} // namespace b3d
