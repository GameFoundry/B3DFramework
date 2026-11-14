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
		/** Checks precise logical buffer stage and access mappings, and how barrier source synchronization widens to match the access. */
		void TestBufferBarrierScopes();

		/** Checks that removing unsupported stages keeps every supported stage synchronizing with all work, and nothing less. */
		void TestUnsupportedStageRemoval();

		/** Checks logical texture scopes and the conservative layout-derived source scope. */
		void TestTextureBarrierScopes();

		/** Checks conservative ordered texture transitions, discard and depth/stencil planes. */
		void TestTextureBarrierBatch();

		/** Checks RT/DS resources use placed allocations from compatible texture heaps. */
		void TestPlacedRenderTargetAllocation();

		/**
		 * Checks a buffer in GpuBufferMemory::Mapped memory is mapped, in a GPU upload heap with Resizable BAR, and otherwise in a custom heap,
		 * write-combined in pool L0 where GPU memory isn't mappable at full speed.
		 */
		void TestCpuWrittenBufferPlacement();

		/**
		 * Checks a buffer in GpuBufferMemory::Device memory is mapped exactly when the device can map its GPU memory, in a GPU upload heap or
		 * a custom heap in pool L0, and in an unmapped default heap otherwise, with other allocators than GpuBufferMemory::Mapped buffers.
		 * Buffers the GPU writes are in default heaps, and GpuBufferMemory::MappedCached buffers in custom heaps in pool L0. Buffers the GPU
		 * writes in mapped memory fail to create.
		 */
		void TestGpuReadBufferPlacement();

		/** Checks depth resources use compatible resource/view formats and shader-resource view dimensions. */
		void TestShaderReadableDepthTextures();

		/** Checks exact native resolve mappings and verifies ordinary copies retain copy semantics. */
		void TestResolveBarrierMappings();

		/** Checks queue-specific, concurrent-read and aspect-specific native texture layouts. */
		void TestTextureLayoutMappings();

		/** Checks that transfer layouts remain COMMON on copy queues. */
		void TestCopyQueueLayoutMappings();

		/**
		 * Checks that sampleable and concurrent-read textures use queue-independent shader-read layouts, and that other textures
		 * use queue-specific ones.
		 */
		void TestQueueSharing();

		/**
		 * Writes and reads back a new render target under D3D12 validation. The submission prologue brings the texture to rest in the
		 * same ExecuteCommandLists() call as the command list whose first barrier starts from the resting state.
		 */
		void TestRestingTransitionSubmission();

		/** Resolves a multisampled texture on a graphics command buffer under D3D12 validation. */
		void TestMultisampleResolve();

		/** Rejects resolve requests that cannot be represented by D3D12 ResolveSubresource. */
		void TestResolveValidation();

		/** Checks that a frame boundary arms frame-start waits on every queue, and that each queue's next submission alone consumes them. */
		void TestFrameFence();
	};
} // namespace b3d
