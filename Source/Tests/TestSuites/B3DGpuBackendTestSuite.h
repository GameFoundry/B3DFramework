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

		/** Verifies full-range accesses register on the full-range subresource and count towards every subresource. */
		void TestWholeImageRegistration();

		/** Verifies a full-range command buffer on a uniform image resolves one transition for the whole image. */
		void TestWholeImageSubmission();

		/** Verifies a partial command buffer splits a uniform image, and untouched subresources inherit the uniform state. */
		void TestImageSplit();

		/** Verifies split images merge after submission only when their subresources can share one state. */
		void TestImageMerge();

		/** Verifies a merged image still waits for readers of every subresource, including partial reads from before the merge. */
		void TestMergedStateWaits();

		/** Verifies conservative merging of submission states. */
		void TestSubmissionStateMerge();

		/** Verifies submission states from earlier frames are cleared, and that this lets split images merge. */
		void TestFrameIndexClear();

		/** Verifies a write orders only after the reader stages of its own queue type. */
		void TestRestingReaderStages();

		/** Verifies resting reads keep only lifetime tracking and register their bounding range, and which accesses are tracked instead. */
		void TestRestingReadRecording();

		/** Verifies the first tracked access of a resource turns its resting reads into ordinary tracked reads. */
		void TestRestingReadMaterialization();

		/** Verifies reads at rest are recorded without a transition, and that later writes order after them. */
		void TestRestingSubmission();

		/** Verifies layout transitions, at submission and inside the command buffer, are synchronized like writes. */
		void TestLayoutTransitionWrites();

		/** Verifies an alias acquire records its barrier at the first access, and that submission starts a new lifetime. */
		void TestAliasAcquire();

		/**
		 * Places resources on shared memory and hands the memory between them with alias acquires, within a command buffer, across command
		 * buffers and across queues. Verifies the contents each resource reads back.
		 */
		void TestAliasAcquireExecution();

		/** Verifies framebuffer attachment normalization and render-pass usage construction. */
		void TestFramebufferAttachmentUsage();

		/** Verifies render-pass attachment and shader usage is combined through core subresource partitions. */
		void TestRenderPassResourceTracking();

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
