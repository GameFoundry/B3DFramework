//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsGpuPipelineState.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a Vulkan graphics pipeline that manages its usage and lifetime. */
	class VulkanPipeline : public VulkanResource
	{
	public:
		VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, 
			const std::array<bool, BS_MAX_MULTIPLE_RENDER_TARGETS>& colorReadOnly, bool depthStencilReadOnly);
		VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline);
		~VulkanPipeline();

		/** Returns the internal handle to the Vulkan object. */
		VkPipeline getHandle() const { return mPipeline; }

		/** Checks is the specified color attachment read-only. Only relevant for graphics pipelines. */
		bool isColorReadOnly(UINT32 colorIdx) const { return mReadOnlyColor[colorIdx]; }

		/** Checks is the depth attachment read-only. Only relevant for graphics pipelines. */
		bool isDepthReadOnly() const { return mReadOnlyDepth; }
	private:
		VkPipeline mPipeline;

		std::array<bool, BS_MAX_MULTIPLE_RENDER_TARGETS> mReadOnlyColor;
		bool mReadOnlyDepth = false;
	};

	/**	Vulkan implementation of a graphics pipeline state. */
	class VulkanGraphicsPipelineState : public GraphicsPipelineState
	{
	public:
		~VulkanGraphicsPipelineState();

		/** Checks does the pipeline enable scissor tests. */
		bool isScissorEnabled() const { return mScissorEnabled; }

		/** Returns the vertex input declaration from the vertex GPU program bound on the pipeline. */
		SPtr<VertexDeclaration> getInputDeclaration() const { return mVertexDecl; }

		/** 
		 * Attempts to find an existing pipeline matching the provided parameters, or creates a new one if one cannot be 
		 * found.
		 * 
		 * @param[in]	deviceIdx			Index of the device to retrieve the pipeline for.
		 * @param[in]	framebuffer			Framebuffer object that defines the surfaces this pipeline will render to.
		 * @param[in]	readOnlyFlags		Flags that control which portion of the framebuffer is read-only. Accepts
		 *									combinations of FrameBufferType enum.
		 * @param[in]	drawOp				Type of geometry that will be drawn using the pipeline.
		 * @param[in]	vertexInput			State describing inputs to the vertex program.
		 * @return							Vulkan graphics pipeline object.
		 * 
		 * @note	Thread safe.
		 */
		VulkanPipeline* getPipeline(UINT32 deviceIdx, VulkanFramebuffer* framebuffer, UINT32 readOnlyFlags, 
			DrawOperationType drawOp, const SPtr<VulkanVertexInput>& vertexInput);

		/** 
		 * Returns a pipeline layout object for the specified device index. If the device index doesn't match a bit in the
		 * device mask provided on pipeline creation, null is returned.
		 */
		VkPipelineLayout getPipelineLayout(UINT32 deviceIdx) const;

		/** 
		 * Registers any resources used by the pipeline with the provided command buffer. This should be called whenever
		 * a pipeline is bound to a command buffer.
		 */
		void registerPipelineResources(VulkanCmdBuffer* cmdBuffer);

	protected:
		friend class VulkanRenderStateManager;

		VulkanGraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask);

		/**	@copydoc GraphicsPipelineState::initialize */
		void initialize() override;

		/** 
		 * Create a new Vulkan graphics pipeline. 
		 * 
		 * @param[in]	deviceIdx			Index of the device to create the pipeline for.
		 * @param[in]	framebuffer			Framebuffer object that defines the surfaces this pipeline will render to.
		 * @param[in]	readOnlyFlags		Flags that control which portion of the framebuffer is read-only. Accepts
		 *									combinations of FrameBufferType enum.
		 * @param[in]	drawOp				Type of geometry that will be drawn using the pipeline.
		 * @param[in]	vertexInput			State describing inputs to the vertex program.
		 * @return							Vulkan graphics pipeline object.
		 * 
		 * @note	Thread safe.
		 */
		VulkanPipeline* createPipeline(UINT32 deviceIdx, VulkanFramebuffer* framebuffer, UINT32 readOnlyFlags, 
			DrawOperationType drawOp, const SPtr<VulkanVertexInput>& vertexInput);

		/**	Key uniquely identifying GPU pipelines. */
		struct GpuPipelineKey
		{
			GpuPipelineKey(UINT32 framebufferId, UINT32 vertexInputId, UINT32 readOnlyFlags, DrawOperationType drawOp);

			UINT32 framebufferId;
			UINT32 vertexInputId;
			UINT32 readOnlyFlags;
			DrawOperationType drawOp;
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
			VulkanDevice* device;
			VkPipelineLayout pipelineLayout;
			UnorderedMap<GpuPipelineKey, VulkanPipeline*, HashFunc, EqualFunc> pipelines;
		};

		VkPipelineShaderStageCreateInfo mShaderStageInfos[5];
		VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
		VkPipelineTessellationStateCreateInfo mTesselationInfo;
		VkPipelineViewportStateCreateInfo mViewportInfo;
		VkPipelineRasterizationStateCreateInfo mRasterizationInfo;
		VkPipelineMultisampleStateCreateInfo mMultiSampleInfo;
		VkPipelineDepthStencilStateCreateInfo mDepthStencilInfo;
		VkPipelineColorBlendAttachmentState mAttachmentBlendStates[BS_MAX_MULTIPLE_RENDER_TARGETS];
		VkPipelineColorBlendStateCreateInfo mColorBlendStateInfo;
		VkPipelineDynamicStateCreateInfo mDynamicStateInfo;
		VkDynamicState mDynamicStates[3];
		VkGraphicsPipelineCreateInfo mPipelineInfo;
		bool mScissorEnabled;
		SPtr<VertexDeclaration> mVertexDecl;

		GpuDeviceFlags mDeviceMask;
		PerDeviceData mPerDeviceData[BS_MAX_DEVICES];

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
		VulkanPipeline* getPipeline(UINT32 deviceIdx) const;

		/** 
		 * Returns a pipeline layout object for the specified device index. If the device index doesn't match a bit in the
		 * device mask provided on pipeline creation, null is returned.
		 */
		VkPipelineLayout getPipelineLayout(UINT32 deviceIdx) const;

		/** 
		 * Registers any resources used by the pipeline with the provided command buffer. This should be called whenever
		 * a pipeline is bound to a command buffer.
		 */
		void registerPipelineResources(VulkanCmdBuffer* cmdBuffer);

	protected:
		friend class VulkanRenderStateManager;

		VulkanComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask);

		/**	@copydoc ComputePipelineState::initialize */
		void initialize() override;

		/** Contains pipeline data specific to a single Vulkan device. */
		struct PerDeviceData
		{
			VulkanDevice* device;
			VulkanPipeline* pipeline;
			VkPipelineLayout pipelineLayout;
		};

		GpuDeviceFlags mDeviceMask;
		PerDeviceData mPerDeviceData[BS_MAX_DEVICES];
	};

	/** @} */
}}