//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanResource.h"
#include "RenderAPI/B3DGpuPipelineState.h"

namespace b3d::render
{
	class VulkanResourceTracker;
}

namespace b3d
{
	namespace render
	{
		class VulkanRenderPass;

		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan graphics pipeline that manages its usage and lifetime. */
		class VulkanPipeline : public VulkanResource
		{
		public:
			VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, const std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT>& colorReadOnly, bool depthStencilReadOnly, u32 vertexBufferBindingCount, const StringView& name = "");
			VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, const StringView& name = "");
			~VulkanPipeline();

			/** Returns the internal handle to the Vulkan object. */
			VkPipeline GetVulkanHandle() const { return mPipeline; }

			/** Checks is the specified color attachment read-only. Only relevant for graphics pipelines. */
			bool IsColorReadOnly(u32 colorIdx) const { return mReadOnlyColor[colorIdx]; }

			/** Checks is the depth attachment read-only. Only relevant for graphics pipelines. */
			bool IsDepthReadOnly() const { return mReadOnlyDepth; }

			/** Gets the number of vertex buffers can be bound to the pipeline. */
			u32 GetVertexBufferBindingCount() const { return mVertexBufferBindingCount; } 

		private:
			VkPipeline mPipeline;

			std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT> mReadOnlyColor;
			bool mReadOnlyDepth = false;
			u32 mVertexBufferBindingCount = 0;
		};

		/**	Vulkan implementation of a graphics pipeline state. */
		class VulkanGpuGraphicsPipelineState : public GpuGraphicsPipelineState
		{
		public:
			VulkanGpuGraphicsPipelineState(VulkanGpuDevice& gpuDevice, const GpuGraphicsPipelineStateInformation& createInformation);
			~VulkanGpuGraphicsPipelineState();

			void Initialize() override;

			/** Returns the vertex input declaration from the vertex GPU program bound on the pipeline. */
			const SPtr<VertexDescription>& GetInputDeclaration() const { return mVertexDescription; }

			/**
			 * Attempts to find an existing pipeline matching the provided parameters, or creates a new one if one cannot be
			 * found.
			 *
			 * @param	renderPass			Render pass that the pipeline will be used with, or one compatible.
			 * @param	readOnlyMask		Flags that control which framebuffer attachments are read-only. 
			 * @param	drawOp				Type of geometry that will be drawn using the pipeline.
			 * @param	vertexInput			State describing inputs to the vertex program.
			 * @return						Vulkan graphics pipeline object.
			 *
			 * @note	Thread safe.
			 */
			VulkanPipeline* FindOrCreateVulkanResource(VulkanRenderPass* renderPass, RenderSurfaceMask readOnlyMask, DrawOperationType drawOp, const SPtr<VulkanVertexInput>& vertexInput);

			/** Returns the pipeline layout object. */
			VkPipelineLayout GetPipelineLayoutHandle() const { return mPipelineLayout; }

			/**
			 * Registers any resources used by the pipeline with the provided command buffer resource tracker. This should be called whenever
			 * a pipeline is bound to a command buffer.
			 */
			void RegisterShaderModuleResources(VulkanResourceTracker& resourceTracker);

		protected:
			/**
			 * Create a new Vulkan graphics pipeline.
			 *
			 * @param	renderPass			Render pass that the pipeline will be used with, or one compatible.
			 * @param	readOnlyMask		Flags that control which framebuffer attachment is read-only.
			 * @param	primitiveType		Type of geometry that will be drawn using the pipeline.
			 * @param	vertexInput			State describing inputs to the vertex program.
			 * @return						Vulkan graphics pipeline object.
			 *
			 * @note	Thread safe.
			 */
			VulkanPipeline* CreatePipeline(VulkanRenderPass* renderPass, RenderSurfaceMask readOnlyMask, DrawOperationType primitiveType, const SPtr<VulkanVertexInput>& vertexInput);

			/**	Key uniquely identifying GPU pipelines. */
			struct GpuPipelineKey
			{
				GpuPipelineKey(u32 framebufferId, u32 vertexInputId, RenderSurfaceMask readOnlyMask, DrawOperationType drawOp);

				u32 FramebufferId;
				u32 VertexInputId;
				RenderSurfaceMask ReadOnlyMask;
				DrawOperationType DrawOp;
			};

			/**	Creates a hash from GPU pipeline key. */
			class HashFunc
			{
			public:
				::std::size_t operator()(const GpuPipelineKey& key) const;
			};

			/**	Compares two GPU pipeline keys. */
			class EqualFunc
			{
			public:
				bool operator()(const GpuPipelineKey& a, const GpuPipelineKey& b) const;
			};

			VkPipelineShaderStageCreateInfo mShaderStageInfos[5];
			VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
			VkPipelineTessellationStateCreateInfo mTesselationInfo;
			VkPipelineViewportStateCreateInfo mViewportInfo;
			VkPipelineRasterizationStateCreateInfo mRasterizationInfo;
			VkPipelineMultisampleStateCreateInfo mMultiSampleInfo;
			VkPipelineDepthStencilStateCreateInfo mDepthStencilInfo;
			VkPipelineColorBlendAttachmentState mAttachmentBlendStates[B3D_MAXIMUM_RENDER_TARGET_COUNT];
			VkPipelineColorBlendStateCreateInfo mColorBlendStateInfo;
			VkPipelineDynamicStateCreateInfo mDynamicStateInfo;
			VkDynamicState mDynamicStates[3];
			VkGraphicsPipelineCreateInfo mPipelineInfo;
			SPtr<VertexDescription> mVertexDescription;

			VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
			UnorderedMap<GpuPipelineKey, VulkanPipeline*, HashFunc, EqualFunc> mPipelines;

			Mutex mMutex;
		};

		/**	Vulkan implementation of a compute pipeline state. */
		class VulkanGpuComputePipelineState : public GpuComputePipelineState
		{
		public:
			VulkanGpuComputePipelineState(VulkanGpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation);
			~VulkanGpuComputePipelineState();

			void Initialize() override;

			/** Returns the internal pipeline object. */
			VulkanPipeline* GetVulkanResource() const { return mPipeline; }

			/** Returns the pipeline layout object. */
			VkPipelineLayout GetPipelineLayoutHandle() const { return mPipelineLayout; }

			/**
			 * Registers any resources used by the pipeline with the provided command buffer resource tracker. This should be called whenever
			 * a pipeline is bound to a command buffer.
			 */
			void RegisterShaderModuleResources(VulkanResourceTracker& resourceTracker);

		protected:
			VulkanPipeline* mPipeline = nullptr;
			VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
		};

		/** @} */
	} // namespace render
} // namespace b3d
