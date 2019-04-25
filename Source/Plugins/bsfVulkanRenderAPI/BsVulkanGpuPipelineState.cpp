//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuPipelineState.h"
#include "BsVulkanDevice.h"
#include "BsVulkanGpuProgram.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuPipelineParamInfo.h"
#include "Managers/BsVulkanVertexInputManager.h"
#include "BsVulkanCommandBuffer.h"
#include "RenderAPI/BsRasterizerState.h"
#include "RenderAPI/BsDepthStencilState.h"
#include "RenderAPI/BsBlendState.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, 
		const std::array<bool, BS_MAX_MULTIPLE_RENDER_TARGETS>& colorReadOnly, bool depthStencilReadOnly)
		: VulkanResource(owner, true), mPipeline(pipeline), mReadOnlyColor(colorReadOnly)
		, mReadOnlyDepth(depthStencilReadOnly)
	{ }

	VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline)
		: VulkanResource(owner, true), mPipeline(pipeline), mReadOnlyColor()
	{ }

	VulkanPipeline::~VulkanPipeline()
	{
		vkDestroyPipeline(mOwner->getDevice().getLogical(), mPipeline, gVulkanAllocator);
	}

	VulkanGraphicsPipelineState::GpuPipelineKey::GpuPipelineKey(
		UINT32 framebufferId, UINT32 vertexInputId, UINT32 readOnlyFlags, DrawOperationType drawOp)
		: framebufferId(framebufferId), vertexInputId(vertexInputId), readOnlyFlags(readOnlyFlags)
		, drawOp(drawOp)
	{
		
	}

	size_t VulkanGraphicsPipelineState::HashFunc::operator()(const GpuPipelineKey& key) const
	{
		size_t hash = 0;
		bs_hash_combine(hash, key.framebufferId);
		bs_hash_combine(hash, key.vertexInputId);
		bs_hash_combine(hash, key.readOnlyFlags);
		bs_hash_combine(hash, key.drawOp);

		return hash;
	}

	bool VulkanGraphicsPipelineState::EqualFunc::operator()(const GpuPipelineKey& a, const GpuPipelineKey& b) const
	{
		if (a.framebufferId != b.framebufferId)
			return false;

		if (a.vertexInputId != b.vertexInputId)
			return false;

		if (a.readOnlyFlags != b.readOnlyFlags)
			return false;

		if (a.drawOp != b.drawOp)
			return false;

		return true;
	}

	VulkanGraphicsPipelineState::VulkanGraphicsPipelineState(const PIPELINE_STATE_DESC& desc,
																	 GpuDeviceFlags deviceMask)
		:GraphicsPipelineState(desc, deviceMask), mScissorEnabled(false), mDeviceMask(deviceMask)
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			mPerDeviceData[i].device = nullptr;
			mPerDeviceData[i].pipelineLayout = VK_NULL_HANDLE;
		}
	}

	VulkanGraphicsPipelineState::~VulkanGraphicsPipelineState()
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].device == nullptr)
				continue;

			for(auto& entry : mPerDeviceData[i].pipelines)
				entry.second->destroy();
		}

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_PipelineState);
	}

	void VulkanGraphicsPipelineState::initialize()
	{
		Lock lock(mMutex);

		GraphicsPipelineState::initialize();

		std::pair<VkShaderStageFlagBits, GpuProgram*> stages[] =
			{ 
				{ VK_SHADER_STAGE_VERTEX_BIT, mData.vertexProgram.get() },
				{ VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, mData.hullProgram.get() },
				{ VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, mData.domainProgram.get() },
				{ VK_SHADER_STAGE_GEOMETRY_BIT, mData.geometryProgram.get() },
				{ VK_SHADER_STAGE_FRAGMENT_BIT, mData.fragmentProgram.get() }
			};

		UINT32 stageOutputIdx = 0;
		UINT32 numStages = sizeof(stages) / sizeof(stages[0]);
		for(UINT32 i = 0; i < numStages; i++)
		{
			VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(stages[i].second);
			if (program == nullptr)
				continue;

			VkPipelineShaderStageCreateInfo& stageCI = mShaderStageInfos[stageOutputIdx];
			stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCI.pNext = nullptr;
			stageCI.flags = 0;
			stageCI.stage = stages[i].first;
			stageCI.module = VK_NULL_HANDLE;
			stageCI.pName = program->getEntryPoint().c_str();
			stageCI.pSpecializationInfo = nullptr;

			stageOutputIdx++;
		}

		UINT32 numUsedStages = stageOutputIdx;

		bool tesselationEnabled = mData.hullProgram != nullptr && mData.domainProgram != nullptr;

		mInputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mInputAssemblyInfo.pNext = nullptr;
		mInputAssemblyInfo.flags = 0;
		mInputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Assigned at runtime
		mInputAssemblyInfo.primitiveRestartEnable = false;

		mTesselationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		mTesselationInfo.pNext = nullptr;
		mTesselationInfo.flags = 0;
		mTesselationInfo.patchControlPoints = 3; // Assigned at runtime

		mViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportInfo.pNext = nullptr;
		mViewportInfo.flags = 0;
		mViewportInfo.viewportCount = 1; // Spec says this need to be at least 1...
		mViewportInfo.scissorCount = 1;
		mViewportInfo.pViewports = nullptr; // Dynamic
		mViewportInfo.pScissors = nullptr; // Dynamic

		RasterizerState* rasterizerState = getRasterizerState().get();
		if (rasterizerState == nullptr)
			rasterizerState = RasterizerState::getDefault().get();

		BlendState* blendState = getBlendState().get();
		if (blendState == nullptr)
			blendState = BlendState::getDefault().get();

		DepthStencilState* depthStencilState = getDepthStencilState().get();
		if (depthStencilState == nullptr)
			depthStencilState = DepthStencilState::getDefault().get();

		const RasterizerProperties& rstProps = rasterizerState->getProperties();
		const BlendProperties& blendProps = blendState->getProperties();
		const DepthStencilProperties dsProps = depthStencilState->getProperties();

		mRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mRasterizationInfo.pNext = nullptr;
		mRasterizationInfo.flags = 0;
		mRasterizationInfo.depthClampEnable = !rstProps.getDepthClipEnable();
		mRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		mRasterizationInfo.polygonMode = VulkanUtility::getPolygonMode(rstProps.getPolygonMode());
		mRasterizationInfo.cullMode = VulkanUtility::getCullMode(rstProps.getCullMode());
		mRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		mRasterizationInfo.depthBiasEnable = rstProps.getDepthBias() != 0.0f;
		mRasterizationInfo.depthBiasConstantFactor = rstProps.getDepthBias();
		mRasterizationInfo.depthBiasSlopeFactor = rstProps.getSlopeScaledDepthBias();
		mRasterizationInfo.depthBiasClamp = mRasterizationInfo.depthClampEnable ? rstProps.getDepthBiasClamp() : 0.0f;
		mRasterizationInfo.lineWidth = 1.0f;

		mMultiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		mMultiSampleInfo.pNext = nullptr;
		mMultiSampleInfo.flags = 0;
		mMultiSampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Assigned at runtime
		mMultiSampleInfo.sampleShadingEnable = VK_FALSE; // When enabled, perform shading per sample instead of per pixel (more expensive, essentially FSAA)
		mMultiSampleInfo.minSampleShading = 1.0f; // Minimum percent of samples to run full shading for when sampleShadingEnable is enabled (1.0f to run for all)
		mMultiSampleInfo.pSampleMask = nullptr; // Normally one bit for each sample: e.g. 0x0000000F to enable all samples in a 4-sample setup
		mMultiSampleInfo.alphaToCoverageEnable = blendProps.getAlphaToCoverageEnabled();
		mMultiSampleInfo.alphaToOneEnable = VK_FALSE;

		VkStencilOpState stencilFrontInfo;
		stencilFrontInfo.compareOp = VulkanUtility::getCompareOp(dsProps.getStencilFrontCompFunc());
		stencilFrontInfo.depthFailOp = VulkanUtility::getStencilOp(dsProps.getStencilFrontZFailOp());
		stencilFrontInfo.passOp = VulkanUtility::getStencilOp(dsProps.getStencilFrontPassOp());
		stencilFrontInfo.failOp = VulkanUtility::getStencilOp(dsProps.getStencilFrontFailOp());
		stencilFrontInfo.reference = 0; // Dynamic
		stencilFrontInfo.compareMask = (UINT32)dsProps.getStencilReadMask();
		stencilFrontInfo.writeMask = (UINT32)dsProps.getStencilWriteMask();

		VkStencilOpState stencilBackInfo;
		stencilBackInfo.compareOp = VulkanUtility::getCompareOp(dsProps.getStencilBackCompFunc());
		stencilBackInfo.depthFailOp = VulkanUtility::getStencilOp(dsProps.getStencilBackZFailOp());
		stencilBackInfo.passOp = VulkanUtility::getStencilOp(dsProps.getStencilBackPassOp());
		stencilBackInfo.failOp = VulkanUtility::getStencilOp(dsProps.getStencilBackFailOp());
		stencilBackInfo.reference = 0; // Dynamic
		stencilBackInfo.compareMask = (UINT32)dsProps.getStencilReadMask();
		stencilBackInfo.writeMask = (UINT32)dsProps.getStencilWriteMask();

		mDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		mDepthStencilInfo.pNext = nullptr;
		mDepthStencilInfo.flags = 0;
		mDepthStencilInfo.depthBoundsTestEnable = false;
		mDepthStencilInfo.minDepthBounds = 0.0f;
		mDepthStencilInfo.maxDepthBounds = 1.0f;
		mDepthStencilInfo.depthTestEnable = dsProps.getDepthReadEnable();
		mDepthStencilInfo.depthWriteEnable = dsProps.getDepthWriteEnable();
		mDepthStencilInfo.depthCompareOp = VulkanUtility::getCompareOp(dsProps.getDepthComparisonFunc());
		mDepthStencilInfo.front = stencilFrontInfo;
		mDepthStencilInfo.back = stencilBackInfo;
		mDepthStencilInfo.stencilTestEnable = dsProps.getStencilEnable();

		for(UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
		{
			UINT32 rtIdx = 0;
			if (blendProps.getIndependantBlendEnable())
				rtIdx = i;

			VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[i];
			blendState.blendEnable = blendProps.getBlendEnabled(rtIdx);
			blendState.colorBlendOp = VulkanUtility::getBlendOp(blendProps.getBlendOperation(rtIdx));
			blendState.srcColorBlendFactor = VulkanUtility::getBlendFactor(blendProps.getSrcBlend(rtIdx));
			blendState.dstColorBlendFactor = VulkanUtility::getBlendFactor(blendProps.getDstBlend(rtIdx));
			blendState.alphaBlendOp = VulkanUtility::getBlendOp(blendProps.getAlphaBlendOperation(rtIdx));
			blendState.srcAlphaBlendFactor = VulkanUtility::getBlendFactor(blendProps.getAlphaSrcBlend(rtIdx));
			blendState.dstAlphaBlendFactor = VulkanUtility::getBlendFactor(blendProps.getAlphaDstBlend(rtIdx));
			blendState.colorWriteMask = blendProps.getRenderTargetWriteMask(rtIdx) & 0xF;
		}

		mColorBlendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		mColorBlendStateInfo.pNext = nullptr;
		mColorBlendStateInfo.flags = 0;
		mColorBlendStateInfo.logicOpEnable = VK_FALSE;
		mColorBlendStateInfo.logicOp = VK_LOGIC_OP_NO_OP;
		mColorBlendStateInfo.attachmentCount = 0; // Assigned at runtime
		mColorBlendStateInfo.pAttachments = mAttachmentBlendStates;
		mColorBlendStateInfo.blendConstants[0] = 0.0f;
		mColorBlendStateInfo.blendConstants[1] = 0.0f;
		mColorBlendStateInfo.blendConstants[2] = 0.0f;
		mColorBlendStateInfo.blendConstants[3] = 0.0f;

		mDynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
		mDynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
		mDynamicStates[2] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;

		UINT32 numDynamicStates = sizeof(mDynamicStates) / sizeof(mDynamicStates[0]);
		assert(numDynamicStates == 3);

		mDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		mDynamicStateInfo.pNext = nullptr;
		mDynamicStateInfo.flags = 0;
		mDynamicStateInfo.dynamicStateCount = numDynamicStates;
		mDynamicStateInfo.pDynamicStates = mDynamicStates;

		mPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		mPipelineInfo.pNext = nullptr;
		mPipelineInfo.flags = 0;
		mPipelineInfo.stageCount = numUsedStages;
		mPipelineInfo.pStages = mShaderStageInfos;
		mPipelineInfo.pVertexInputState = nullptr; // Assigned at runtime
		mPipelineInfo.pInputAssemblyState = &mInputAssemblyInfo;
		mPipelineInfo.pTessellationState = tesselationEnabled ? &mTesselationInfo : nullptr;
		mPipelineInfo.pViewportState = &mViewportInfo;
		mPipelineInfo.pRasterizationState = &mRasterizationInfo;
		mPipelineInfo.pMultisampleState = &mMultiSampleInfo;
		mPipelineInfo.pDepthStencilState = nullptr; // Assigned at runtime
		mPipelineInfo.pColorBlendState = nullptr; // Assigned at runtime
		mPipelineInfo.pDynamicState = &mDynamicStateInfo;
		mPipelineInfo.renderPass = VK_NULL_HANDLE; // Assigned at runtime
		mPipelineInfo.layout = VK_NULL_HANDLE; // Assigned at runtime
		mPipelineInfo.subpass = 0;
		mPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		mPipelineInfo.basePipelineIndex = -1;

		mScissorEnabled = rstProps.getScissorEnable();

		if(mData.vertexProgram != nullptr)
			mVertexDecl = mData.vertexProgram->getInputDeclaration();

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());

		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				continue;

			mPerDeviceData[i].device = devices[i];

			VulkanDescriptorManager& descManager = mPerDeviceData[i].device->getDescriptorManager();
			VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

			UINT32 numLayouts = vkParamInfo.getNumSets();
			VulkanDescriptorLayout** layouts = (VulkanDescriptorLayout**)bs_stack_alloc(sizeof(VulkanDescriptorLayout*) * numLayouts);

			for (UINT32 j = 0; j < numLayouts; j++)
				layouts[j] = vkParamInfo.getLayout(i, j);

			mPerDeviceData[i].pipelineLayout = descManager.getPipelineLayout(layouts, numLayouts);

			bs_stack_free(layouts);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_PipelineState);
	}

	VulkanPipeline* VulkanGraphicsPipelineState::getPipeline(
		UINT32 deviceIdx, VulkanFramebuffer* framebuffer, UINT32 readOnlyFlags, DrawOperationType drawOp, 
			const SPtr<VulkanVertexInput>& vertexInput)
	{
		Lock lock(mMutex);

		if (mPerDeviceData[deviceIdx].device == nullptr)
			return nullptr;

		readOnlyFlags &= ~FBT_COLOR; // Ignore the color
		GpuPipelineKey key(framebuffer->getId(), vertexInput->getId(), readOnlyFlags, drawOp);

		PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
		auto iterFind = perDeviceData.pipelines.find(key);
		if (iterFind != perDeviceData.pipelines.end())
			return iterFind->second;

		VulkanPipeline* newPipeline = createPipeline(deviceIdx, framebuffer, readOnlyFlags, drawOp, vertexInput);
		perDeviceData.pipelines[key] = newPipeline;

		return newPipeline;
	}

	VkPipelineLayout VulkanGraphicsPipelineState::getPipelineLayout(UINT32 deviceIdx) const
	{
		return mPerDeviceData[deviceIdx].pipelineLayout;
	}

	void VulkanGraphicsPipelineState::registerPipelineResources(VulkanCmdBuffer* cmdBuffer)
	{
		UINT32 deviceIdx = cmdBuffer->getDeviceIdx();

		std::array<VulkanGpuProgram*, 5> programs = {
			static_cast<VulkanGpuProgram*>(mData.vertexProgram.get()),
			static_cast<VulkanGpuProgram*>(mData.hullProgram.get()),
			static_cast<VulkanGpuProgram*>(mData.domainProgram.get()),
			static_cast<VulkanGpuProgram*>(mData.geometryProgram.get()),
			static_cast<VulkanGpuProgram*>(mData.fragmentProgram.get()),
		};

		for(auto& entry : programs)
		{
			if (entry != nullptr)
			{
				VulkanShaderModule* module = entry->getShaderModule(deviceIdx);

				if(module != nullptr)
					cmdBuffer->registerResource(module, VulkanAccessFlag::Read);
			}
		}
	}

	VulkanPipeline* VulkanGraphicsPipelineState::createPipeline(UINT32 deviceIdx, VulkanFramebuffer* framebuffer,
		UINT32 readOnlyFlags, DrawOperationType drawOp, const SPtr<VulkanVertexInput>& vertexInput)
	{
		mInputAssemblyInfo.topology = VulkanUtility::getDrawOp(drawOp);
		mTesselationInfo.patchControlPoints = 3; // Not provided by our shaders for now
		mMultiSampleInfo.rasterizationSamples = framebuffer->getSampleFlags();
		mColorBlendStateInfo.attachmentCount = framebuffer->getNumColorAttachments();

		DepthStencilState* dsState = getDepthStencilState().get();
		if (dsState == nullptr)
			dsState = DepthStencilState::getDefault().get();

		const DepthStencilProperties dsProps = dsState->getProperties();
		bool enableDepthWrites = dsProps.getDepthWriteEnable() && (readOnlyFlags & FBT_DEPTH) == 0;

		mDepthStencilInfo.depthWriteEnable = enableDepthWrites; // If depth stencil attachment is read only, depthWriteEnable must be VK_FALSE

		// Save stencil ops as we might need to change them if depth/stencil is read-only
		VkStencilOp oldFrontPassOp = mDepthStencilInfo.front.passOp;
		VkStencilOp oldFrontFailOp = mDepthStencilInfo.front.failOp;
		VkStencilOp oldFrontZFailOp = mDepthStencilInfo.front.depthFailOp;

		VkStencilOp oldBackPassOp = mDepthStencilInfo.back.passOp;
		VkStencilOp oldBackFailOp = mDepthStencilInfo.back.failOp;
		VkStencilOp oldBackZFailOp = mDepthStencilInfo.back.depthFailOp;

		if((readOnlyFlags & FBT_STENCIL) != 0)
		{
			// Disable any stencil writes
			mDepthStencilInfo.front.passOp = VK_STENCIL_OP_KEEP;
			mDepthStencilInfo.front.failOp = VK_STENCIL_OP_KEEP;
			mDepthStencilInfo.front.depthFailOp = VK_STENCIL_OP_KEEP;

			mDepthStencilInfo.back.passOp = VK_STENCIL_OP_KEEP;
			mDepthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
			mDepthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
		}

		// Note: We can use the default render pass here (default clear/load/read flags), even though that might not be the
		// exact one currently bound. This is because load/store operations and layout transitions are allowed to differ
		// (as per spec 7.2., such render passes are considered compatible).
		mPipelineInfo.renderPass = framebuffer->getRenderPass(RT_NONE, RT_NONE, CLEAR_NONE);
		mPipelineInfo.layout = mPerDeviceData[deviceIdx].pipelineLayout;
		mPipelineInfo.pVertexInputState = vertexInput->getCreateInfo();

		bool depthReadOnly;
		if (framebuffer->hasDepthAttachment())
		{
			mPipelineInfo.pDepthStencilState = &mDepthStencilInfo;
			depthReadOnly = (readOnlyFlags & FBT_DEPTH) != 0;
		}
		else
		{
			mPipelineInfo.pDepthStencilState = nullptr;
			depthReadOnly = true;
		}

		std::array<bool, BS_MAX_MULTIPLE_RENDER_TARGETS> colorReadOnly;
		if (framebuffer->getNumColorAttachments() > 0)
		{
			mPipelineInfo.pColorBlendState = &mColorBlendStateInfo;

			for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
			{
				VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[i];
				colorReadOnly[i] = blendState.colorWriteMask == 0;
			}
		}
		else
		{
			mPipelineInfo.pColorBlendState = nullptr;

			for (UINT32 i = 0; i < BS_MAX_MULTIPLE_RENDER_TARGETS; i++)
				colorReadOnly[i] = true;
		}

		std::pair<VkShaderStageFlagBits, GpuProgram*> stages[] =
		{
			{ VK_SHADER_STAGE_VERTEX_BIT, mData.vertexProgram.get() },
			{ VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, mData.hullProgram.get() },
			{ VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, mData.domainProgram.get() },
			{ VK_SHADER_STAGE_GEOMETRY_BIT, mData.geometryProgram.get() },
			{ VK_SHADER_STAGE_FRAGMENT_BIT, mData.fragmentProgram.get() }
		};

		UINT32 stageOutputIdx = 0;
		UINT32 numStages = sizeof(stages) / sizeof(stages[0]);
		for (UINT32 i = 0; i < numStages; i++)
		{
			VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(stages[i].second);
			if (program == nullptr)
				continue;

			VkPipelineShaderStageCreateInfo& stageCI = mShaderStageInfos[stageOutputIdx];

			VulkanShaderModule* module = program->getShaderModule(deviceIdx);

			if (module != nullptr)
				stageCI.module = module->getHandle();
			else
				stageCI.module = VK_NULL_HANDLE;

			stageOutputIdx++;
		}

		VulkanDevice* device = mPerDeviceData[deviceIdx].device;
		VkDevice vkDevice = mPerDeviceData[deviceIdx].device->getLogical();

		VkPipeline pipeline;
		VkResult result = vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &mPipelineInfo, gVulkanAllocator, &pipeline);
		assert(result == VK_SUCCESS);

		// Restore previous stencil op states
		mDepthStencilInfo.front.passOp = oldFrontPassOp;
		mDepthStencilInfo.front.failOp = oldFrontFailOp;
		mDepthStencilInfo.front.depthFailOp = oldFrontZFailOp;

		mDepthStencilInfo.back.passOp = oldBackPassOp;
		mDepthStencilInfo.back.failOp = oldBackFailOp;
		mDepthStencilInfo.back.depthFailOp = oldBackZFailOp;

		return device->getResourceManager().create<VulkanPipeline>(pipeline, colorReadOnly, depthReadOnly);
	}

	VulkanComputePipelineState::VulkanComputePipelineState(const SPtr<GpuProgram>& program, 
		GpuDeviceFlags deviceMask)
		:ComputePipelineState(program, deviceMask), mDeviceMask(deviceMask)
	{
		bs_zero_out(mPerDeviceData);
	}

	VulkanComputePipelineState::~VulkanComputePipelineState()
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].device == nullptr)
				continue;

			mPerDeviceData[i].pipeline->destroy();
		}
	}

	void VulkanComputePipelineState::initialize()
	{
		ComputePipelineState::initialize();

		// This might happen fairly often if shaders with unsupported languages are loaded, in which case the pipeline
		// will never get used, and its fine not to initialize it.
		if (!mProgram->isCompiled())
			return;

		VulkanGpuProgram* vkProgram = static_cast<VulkanGpuProgram*>(mProgram.get());

		VkPipelineShaderStageCreateInfo stageCI;
		stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCI.pNext = nullptr;
		stageCI.flags = 0;
		stageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageCI.module = VK_NULL_HANDLE;
		stageCI.pName = vkProgram->getEntryPoint().c_str();
		stageCI.pSpecializationInfo = nullptr;

		VkComputePipelineCreateInfo pipelineCI;
		pipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCI.pNext = nullptr;
		pipelineCI.flags = 0;
		pipelineCI.stage = stageCI;
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCI.basePipelineIndex = -1;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::instance());

		VulkanDevice* devices[BS_MAX_DEVICES];
		VulkanUtility::getDevices(rapi, mDeviceMask, devices);

		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (devices[i] == nullptr)
				continue;

			mPerDeviceData[i].device = devices[i];

			VulkanDescriptorManager& descManager = devices[i]->getDescriptorManager();
			VulkanResourceManager& rescManager = devices[i]->getResourceManager();
			VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

			UINT32 numLayouts = vkParamInfo.getNumSets();
			VulkanDescriptorLayout** layouts = (VulkanDescriptorLayout**)bs_stack_alloc(sizeof(VulkanDescriptorLayout*) * numLayouts);

			for (UINT32 j = 0; j < numLayouts; j++)
				layouts[j] = vkParamInfo.getLayout(i, j);

			VulkanShaderModule* module = vkProgram->getShaderModule(i);

			if (module != nullptr)
				pipelineCI.stage.module = module->getHandle();
			else
				pipelineCI.stage.module = VK_NULL_HANDLE;

			pipelineCI.layout = descManager.getPipelineLayout(layouts, numLayouts);

			VkPipeline pipeline;
			VkResult result = vkCreateComputePipelines(devices[i]->getLogical(), VK_NULL_HANDLE, 1, &pipelineCI,
														gVulkanAllocator, &pipeline);
			assert(result == VK_SUCCESS);


			mPerDeviceData[i].pipeline = rescManager.create<VulkanPipeline>(pipeline);
			mPerDeviceData[i].pipelineLayout = pipelineCI.layout;
			bs_stack_free(layouts);
		}

		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_PipelineState);
	}

	VulkanPipeline* VulkanComputePipelineState::getPipeline(UINT32 deviceIdx) const
	{
		return mPerDeviceData[deviceIdx].pipeline;
	}

	VkPipelineLayout VulkanComputePipelineState::getPipelineLayout(UINT32 deviceIdx) const
	{
		return mPerDeviceData[deviceIdx].pipelineLayout;
	}

	void VulkanComputePipelineState::registerPipelineResources(VulkanCmdBuffer* cmdBuffer)
	{
		UINT32 deviceIdx = cmdBuffer->getDeviceIdx();

		VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(mProgram.get());
		if(program != nullptr)
		{
			VulkanShaderModule* module = program->getShaderModule(deviceIdx);

			if (module != nullptr)
				cmdBuffer->registerResource(module, VulkanAccessFlag::Read);
		}
	}
}}