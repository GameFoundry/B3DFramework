//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Testing/B3DTestSuite.h"

namespace b3d
{
	/** Validates D3D12 enhanced-barrier mappings and queue-specific texture operations. */
	class D3D12BarrierTestSuite : public TestSuite
	{
	public:
		/** Registers the D3D12 barrier tests. */
		D3D12BarrierTestSuite();

	private:
		/** Checks logical texture scopes and the conservative layout-derived source scope. */
		void TestTextureBarrierScopes();

		/** Checks conservative ordered texture transitions, discard and depth/stencil planes. */
		void TestTextureBarrierBatchMerging();

		/** Checks exact native resolve mappings and verifies ordinary copies retain copy semantics. */
		void TestResolveBarrierMappings();

		/** Checks queue-specific, concurrent-read and aspect-specific native texture layouts. */
		void TestTextureLayoutMappings();

		/** Checks that transfer layouts remain COMMON on copy queues. */
		void TestCopyQueueLayoutMappings();

		/** Checks concurrent-read creation metadata across graphics, compute and copy queue use. */
		void TestConcurrentQueueReadTexture();

		/** Copies texture data between graphics, copy and compute queues under D3D12 validation. */
		void TestCrossQueueTextureHandoffs();

		/** Copies buffer data between graphics, compute and copy queues using fence-only ordering. */
		void TestCrossQueueBufferHandoffs();

		/** Exercises submission prologue barriers followed by barriers in the primary command list. */
		void TestSubmissionBarrierChaining();

		/** Executes RAW, WAR and WAW buffer hazards across consecutive same-queue submissions. */
		void TestSameQueueSubmissionHazards();

		/** Resolves a multisampled texture on a graphics command buffer under D3D12 validation. */
		void TestMultisampleResolve();

		/** Rejects resolve requests that cannot be represented by D3D12 ResolveSubresource. */
		void TestResolveValidation();
	};
} // namespace b3d
