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
		/** Verifies the flat write-generation hazard state and command-buffer summary. */
		void TestResourceHazardState();

		/** Verifies cross-command-buffer dependencies and propagation of unresolved hazards. */
		void TestResourceTransition();

		/** Verifies writer epochs allow parallel reads and only wait for active conflicting queues. */
		void TestSubmissionTransitionPlanning();

		/** Verifies image partitions and persistent submission state are independent per aspect. */
		void TestImageAspectTracking();

		/** Verifies incompatible shader layouts are coalesced only within one access epoch. */
		void TestImageAccessEpochTracking();

		/** Verifies framebuffer attachment normalization and render-pass usage construction. */
		void TestFramebufferAttachmentUsage();

		/** Verifies render-pass attachment and shader usage is combined through core subresource partitions. */
		void TestRenderPassResourceTracking();
	};
} // namespace b3d
