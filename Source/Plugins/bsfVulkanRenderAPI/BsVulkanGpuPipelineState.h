//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsGpuPipelineState.h"

namespace bs
{
	namespace ct
	{
		class VulkanRenderPass;

		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Wrapper around a Vulkan graphics pipeline that manages its usage and lifetime. */
		class VulkanPipeline : public VulkanResource
		{
		public:
			VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, const std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT>& colorReadOnly, bool depthStencilReadOnly, u32 vertexBufferBindingCount);
			VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline);
			~VulkanPipeline();

			/** Returns the internal handle to the Vulkan object. */
			VkPipeline GetHandle() const { return mPipeline; }

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
		class VulkanGraphicsPipelineState : public GraphicsPipelineState
		{
		public:
			~VulkanGraphicsPipelineState();

			/** Returns the vertex input declaration from the vertex GPU program bound on the pipeline. */
			const SPtr<VertexDescription>& GetInputDeclaration() const { return mVertexDescription; }

			/**
			 * Attempts to find an existing pipeline matching the provided parameters, or creates a new one if one cannot be
			 * found.
			 *
			 * @param[in]	deviceIdx			Index of the device to retrieve the pipeline for.
			 * @param[in]	renderPass			Render pass that the pipeline will be used with, or one compatible.
			 * @param[in]	readOnlyFlags		Flags that control which portion of the framebuffer is read-only. Accepts
			 *									combinations of FrameBufferType enum.
			 * @param[in]	drawOp				Type of geometry that will be drawn using the pipeline.
			 * @param[in]	vertexInput			State describing inputs to the vertex program.
			 * @return							Vulkan graphics pipeline object.
			 *
			 * @note	Thread safe.
			 */
			VulkanPipeline* GetPipeline(u32 deviceIdx, VulkanRenderPass* renderPass, u32 readOnlyFlags, DrawOperationType drawOp, const SPtr<VulkanVertexInput>& vertexInput);

			/**
			 * Returns a pipeline layout object for the specified device index. If the device index doesn't match a bit in the
			 * device mask provided on pipeline creation, null is returned.
			 */
			VkPipelineLayout GetPipelineLayout(u32 deviceIdx) const;

			/**
			 * Registers any resources used by the pipeline with the provided command buffer. This should be called whenever
			 * a pipeline is bound to a command buffer.
			 */
			void RegisterPipelineResources(VulkanInternalCommandBuffer* cmdBuffer);

		protected:
			friend class VulkanRenderStateManager;

			VulkanGraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask);

			void Initialize() override;

			/**
			 * Create a new Vulkan graphics pipeline.
			 *
			 * @param[in]	deviceIndex			Index of the device to create the pipeline for.
			 * @param[in]	renderPass			Render pass that the pipeline will be used with, or one compatible.
			 * @param[in]	readOnlyFlags		Flags that control which portion of the framebuffer is read-only. Accepts
			 *									combinations of FrameBufferType enum.
			 * @param[in]	primitiveType		Type of geometry that will be drawn using the pipeline.
			 * @param[in]	vertexInput			State describing inputs to the vertex program.
			 * @return							Vulkan graphics pipeline object.
			 *
			 * @note	Thread safe.
			 */
			VulkanPipeline* CreatePipeline(u32 deviceIndex, VulkanRenderPass* renderPass, u32 readOnlyFlags, DrawOperationType primitiveType, const SPtr<VulkanVertexInput>& vertexInput);

			/**	Key uniquely identifying GPU pipelines. */
			struct GpuPipelineKey
			{
				GpuPipelineKey(u32 framebufferId, u32 vertexInputId, u32 readOnlyFlags, DrawOperationType drawOp);

				u32 FramebufferId;
				u32 VertexInputId;
				u32 ReadOnlyFlags;
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

			/** Contains pipeline data specific to a single Vulkan device. */
			struct PerDeviceData
			{
				VulkanGpuDevice* Device;
				VkPipelineLayout PipelineLayout;
				UnorderedMap<GpuPipelineKey, VulkanPipeline*, HashFunc, EqualFunc> Pipelines;
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

			GpuDeviceFlags mDeviceMask;
			PerDeviceData mPerDeviceData[B3D_MAX_DEVICES];

			Mutex mMutex;
		};

		/**	Vulkan implementation of a compute pipeline state. */
		class VulkanComputePipelineState : public ComputePipelineState
		{
		public:
			~VulkanComputePipelineState();

			/**
			 * Returns a pipeline object for the specified device index. If the device index doesn't match a bit in the
			 * device mask provided on pipeline creation, null is returned.
			 */
			VulkanPipeline* GetPipeline(u32 deviceIndex) const;

			/**
			 * Returns a pipeline layout object for the specified device index. If the device index doesn't match a bit in the
			 * device mask provided on pipeline creation, null is returned.
			 */
			VkPipelineLayout GetPipelineLayout(u32 deviceIndex) const;

			/**
			 * Registers any resources used by the pipeline with the provided command buffer. This should be called whenever
			 * a pipeline is bound to a command buffer.
			 */
			void RegisterPipelineResources(VulkanInternalCommandBuffer* cmdBuffer);

		protected:
			friend class VulkanRenderStateManager;

			VulkanComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask);

			void Initialize() override;

			/** Contains pipeline data specific to a single Vulkan device. */
			struct PerDeviceData
			{
				VulkanGpuDevice* Device;
				VulkanPipeline* Pipeline;
				VkPipelineLayout PipelineLayout;
			};

			GpuDeviceFlags mDeviceMask;
			PerDeviceData mPerDeviceData[B3D_MAX_DEVICES];
		};

		/** @} */
	} // namespace ct
} // namespace bs
