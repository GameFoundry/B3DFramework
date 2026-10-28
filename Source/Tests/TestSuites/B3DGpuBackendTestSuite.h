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
#if B3D_BUILD_TYPE_DEVELOPMENT
		/** Verifies per-draw UAV conflicts, aliasing, rejected-draw rollback and render-pass reset. */
		void TestDrawAccessValidation();

#endif

		/** Verifies the pipeline stages each queue type can execute. */
		void TestQueueStageFlags();

		/**
		 * Verifies the transient resource cache across scopes and frames, cache eviction by idle frames and by cache size, the release of
		 * idle heaps, concurrent scopes of two contexts sharing a heap, the persistent memory fallback and the validation of transient
		 * resource use.
		 */
		void TestTransientResourceAllocator();

		/**
		 * Allocates transient resources from a context's allocator, in scopes spanning the compute and graphics queues, over two frames.
		 * Verifies the contents each resource reads back, and that the resources shared memory.
		 */
		void TestTransientResourceExecution();

		/**
		 * Verifies textures returned before their memory is known: placement at the cached memory or at other memory, held textures that
		 * are neither returned again nor evicted, Return(), Detach() and the unplaced mark.
		 */
		void TestTransientResourcePlacement();

		/** Verifies framebuffer attachment normalization and render-pass usage construction. */
		void TestFramebufferAttachmentUsage();

		/**
		 * Verifies the tracker binds every used resource and subresource once and accumulates their access, lets shaders read read-only
		 * attachments of the render pass in the attachment's layout, and unbinds everything when cleared.
		 */
		void TestResourceUseTracking();

		/**
		 * Verifies that barriers order GPU writes before later reads: full barriers, split barriers within a command buffer,
		 * across command buffers in both recording orders, and across queues.
		 */
		void TestBarriers();

		/**
		 * Verifies that split barriers order a long-running compute write before a copy that reads it, within a command buffer and
		 * across command buffers on one queue. The write runs long enough that, within a command buffer, a backend that drops the
		 * barrier lets the copy read stale data. Runs only with backends that compile
		 * shaders on the host.
		 */
		void TestSplitBarrierExecution();

		/**
		 * Records tens of thousands of dispatches and thousands of texture copies into one command buffer, on each queue, changing push
		 * constants and parameter sets along the way, and verifies every write. Covers backends that grow their recording memory in
		 * blocks, which must carry the bound state across them. Runs only with backends that resolve the test shaders.
		 */
		void TestLongRecording();

		/**
		 * Verifies that a barrier orders a long-running compute write before a copy recorded after a render pass that follows the
		 * barrier, and before a copy in a later command buffer on the same queue. On backends that record passes as separate
		 * encoders the copy runs in a later pass than the barrier, so a backend that only orders the pass the barrier was issued in
		 * lets the copy read stale data. Runs only with backends that compile shaders on
		 * the host.
		 */
		void TestBarrierAcrossPasses();

		/**
		 * Places transient resources over each other's memory and hands the memory between them with alias acquires, issued as full
		 * and as split barriers. Then binds resources before they are placed, recreates them at other memory and writes them with a dispatch.
		 * Verifies the contents each resource reads back.
		 */
		void TestTransientAliasing();

#if B3D_BUILD_TYPE_DEVELOPMENT
		/**
		 * Verifies the errors for barriers and command buffer methods that do not match the state earlier barriers
		 * declared. Each method is checked on its own resource, so the test fails if a backend override skips the validating base
		 * implementation.
		 */
		void TestBarrierValidation();

		/**
		 * Verifies the errors of a validator shared by two command buffers: resources used outside of their lifetime on the memory, alias
		 * acquires that do not cover their source or share a call with a barrier of a resource they supersede, states carried over to
		 * the next command buffer, accesses outside of the resting state and dispatches that access what an earlier dispatch wrote
		 * without a barrier.
		 */
		void TestBarrierResourceValidation();
#endif

		/** Verifies push-constant metadata merging and carrier separation across program stages. */
		void TestPushConstantMetadata();

		/** Verifies push-constant partial updates and API validation rules. */
		void TestPushConstantWrites();

		/** Verifies push-constant buffer metadata survives bytecode serialization. */
		void TestPushConstantSerialization();

		/** Verifies only uniform buffers declared with a dynamic offset receive dynamic-offset indices, and that stages must agree on the declaration. */
		void TestDynamicOffsetUniformBufferLayout();

		/** Verifies creation of buffers and textures at pending, owned and non-owning memory locations, and rejection of invalid ones. */
		void TestResourceLocations();

#if !B3D_PLATFORM_PS5
		/** Reads back static and dynamic uniform buffers across offset, pipeline and command-buffer changes on Vulkan and D3D12. */
		void TestDynamicUniformBufferOffsets();

		/** Compiles host shader fixtures and verifies push-constant size and separation from ordinary resources. */
		void TestHostPushConstantShaderCompilation();
#endif

#if B3D_PLATFORM_MACOS
		/** Verifies Metal reflects dynamic-offset uniform buffers as argument-table bindings rather than argument-buffer members. */
		void TestMetalDynamicUniformBufferReflection();
#endif

		/** Verifies Vulkan storage-buffer reflection distinguishes read-only blocks and members from writable buffers. */
		void TestVulkanStorageBufferAccessReflection();

		/** Verifies DXC compiles every supported HLSL stage and preserves the engine's reflection contract. */
		void TestHlslShaderModel66Compilation();
	};
} // namespace b3d
