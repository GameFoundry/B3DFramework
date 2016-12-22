//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuPipelineState.h"
#include "BsVulkanDevice.h"
#include "BsVulkanGpuProgram.h"
#include "BsVulkanFramebuffer.h"
#include "BsVulkanUtility.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanVertexInputManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsRasterizerState.h"
#include "BsDepthStencilState.h"
#include "BsBlendState.h"
#include "BsRenderStats.h"

namespace bs
{
	VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, 
		const std::array<bool, BS_MAX_MULTIPLE_RENDER_TARGETS>& colorReadOnly, bool depthStencilReadOnly)
		: VulkanResource(owner, true), mPipeline(pipeline), mReadOnlyColor(colorReadOnly)
		, mReadOnlyDepthStencil(depthStencilReadOnly)
	{ }

	VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline)
		: VulkanResource(owner, true), mPipeline(pipeline), mReadOnlyColor(), mReadOnlyDepthStencil(false)
	{ }

	VulkanPipeline::~VulkanPipeline()
	{
		vkDestroyPipeline(mOwner->getDevice().getLogical(), mPipeline, gVulkanAllocator);
	}

	VulkanGraphicsPipelineStateCore::GpuPipelineKey::GpuPipelineKey(
		UINT32 framebufferId, UINT32 vertexInputId, bool readOnlyDepth, DrawOperationType drawOp)
		: framebufferId(framebufferId), vertexInputId(vertexInputId), readOnlyDepth(readOnlyDepth)
		, drawOp(drawOp)
	{
		
	}

	size_t VulkanGraphicsPipelineStateCore::HashFunc::operator()(const GpuPipelineKey& key) const
	{
		size_t hash = 0;
		hash_combine(hash, key.framebufferId);
		hash_combine(hash, key.vertexInputId);
		hash_combine(hash, key.readOnlyDepth);
		hash_combine(hash, key.drawOp);

		return hash;
	}

	bool VulkanGraphicsPipelineStateCore::EqualFunc::operator()(const GpuPipelineKey& a, const GpuPipelineKey& b) const
	{
		if (a.framebufferId != b.framebufferId)
			return false;

		if (a.vertexInputId != b.vertexInputId)
			return false;

		if (a.readOnlyDepth != b.readOnlyDepth)
			return false;

		if (a.drawOp != b.drawOp)
			return false;

		return true;
	}

	VulkanGraphicsPipelineStateCore::VulkanGraphicsPipelineStateCore(const PIPELINE_STATE_CORE_DESC& desc,
																	 GpuDeviceFlags deviceMask)
		:GraphicsPipelineStateCore(desc, deviceMask), mScissorEnabled(false), mDeviceMask(deviceMask)
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			mPerDeviceData[i].device = nullptr;
			mPerDeviceData[i].pipelineLayout = VK_NULL_HANDLE;
		}
	}

	VulkanGraphicsPipelineStateCore::~VulkanGraphicsPipelineStateCore()
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

	void VulkanGraphicsPipelineStateCore::initialize()
	{
		Lock(mMutex);

		GraphicsPipelineStateCore::initialize();

		std::pair<VkShaderStageFlagBits, GpuProgramCore*> stages[] =
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
			VulkanGpuProgramCore* program = static_cast<VulkanGpuProgramCore*>(stages[i].second);
			if (program == nullptr)
				continue;

			VkPipelineShaderStageCreateInfo& stageCI = mShaderStageInfos[stageOutputIdx];
			stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stageCI.pNext = nullptr;
			stageCI.flags = 0;
			stageCI.stage = stages[i].first;
			stageCI.module = VK_NULL_HANDLE;
			stageCI.pName = program->getProperties().getEntryPoint().c_str();
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
		mTesselationInfo.patchControlPoints = 3; // Assigned at runtime

		mViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		mViewportInfo.pNext = nullptr;
		mViewportInfo.flags = 0;
		mViewportInfo.viewportCount = 1; // Spec says this need to be at least 1...
		mViewportInfo.scissorCount = 1;
		mViewportInfo.pViewports = nullptr; // Dynamic
		mViewportInfo.pScissors = nullptr; // Dynamic

		RasterizerStateCore* rasterizerState = getRasterizerState().get();
		if (rasterizerState == nullptr)
			rasterizerState = RasterizerStateCore::getDefault().get();

		BlendStateCore* blendState = getBlendState().get();
		if (blendState == nullptr)
			blendState = BlendStateCore::getDefault().get();

		DepthStencilStateCore* depthStencilState = getDepthStencilState().get();
		if (depthStencilState == nullptr)
			depthStencilState = DepthStencilStateCore::getDefault().get();

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
		mRasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());

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

	VulkanPipeline* VulkanGraphicsPipelineStateCore::getPipeline(
		UINT32 deviceIdx, VulkanFramebuffer* framebuffer, bool readOnlyDepth, DrawOperationType drawOp, 
			const SPtr<VulkanVertexInput>& vertexInput)
	{
		Lock(mMutex);

		if (mPerDeviceData[deviceIdx].device == nullptr)
			return nullptr;

		GpuPipelineKey key(framebuffer->getId(), vertexInput->getId(), readOnlyDepth, drawOp);

		PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
		auto iterFind = perDeviceData.pipelines.find(key);
		if (iterFind != perDeviceData.pipelines.end())
			return iterFind->second;

		VulkanPipeline* newPipeline = createPipeline(deviceIdx, framebuffer, readOnlyDepth, drawOp, vertexInput);
		perDeviceData.pipelines[key] = newPipeline;

		return newPipeline;
	}

	VkPipelineLayout VulkanGraphicsPipelineStateCore::getPipelineLayout(UINT32 deviceIdx) const
	{
		return mPerDeviceData[deviceIdx].pipelineLayout;
	}

	void VulkanGraphicsPipelineStateCore::registerPipelineResources(VulkanCmdBuffer* cmdBuffer)
	{
		UINT32 deviceIdx = cmdBuffer->getDeviceIdx();

		std::array<VulkanGpuProgramCore*, 5> programs = {
			static_cast<VulkanGpuProgramCore*>(mData.vertexProgram.get()),
			static_cast<VulkanGpuProgramCore*>(mData.hullProgram.get()),
			static_cast<VulkanGpuProgramCore*>(mData.domainProgram.get()),
			static_cast<VulkanGpuProgramCore*>(mData.geometryProgram.get()),
			static_cast<VulkanGpuProgramCore*>(mData.fragmentProgram.get()),
		};

		for(auto& entry : programs)
		{
			if (entry != nullptr)
			{
				VulkanShaderModule* module = entry->getShaderModule(deviceIdx);

				if(module != nullptr)
					cmdBuffer->registerResource(module, VulkanUseFlag::Read);
			}
		}
	}

	VulkanPipeline* VulkanGraphicsPipelineStateCore::createPipeline(UINT32 deviceIdx, VulkanFramebuffer* framebuffer,
		bool readOnlyDepth, DrawOperationType drawOp, const SPtr<VulkanVertexInput>& vertexInput)
	{
		mInputAssemblyInfo.topology = VulkanUtility::getDrawOp(drawOp);
		mTesselationInfo.patchControlPoints = 3; // Not provided by our shaders for now
		mMultiSampleInfo.rasterizationSamples = framebuffer->getSampleFlags();
		mColorBlendStateInfo.attachmentCount = framebuffer->getNumAttachments();

		const DepthStencilProperties dsProps = getDepthStencilState()->getProperties();
		bool enableDepthWrites = dsProps.getDepthWriteEnable() && !readOnlyDepth;

		mDepthStencilInfo.depthWriteEnable = enableDepthWrites; // If depth stencil attachment is read only, depthWriteEnable must be VK_FALSE

		// Save stencil ops as we might need to change them if depth/stencil is read-only
		VkStencilOp oldFrontPassOp = mDepthStencilInfo.front.passOp;
		VkStencilOp oldFrontFailOp = mDepthStencilInfo.front.failOp;
		VkStencilOp oldFrontZFailOp = mDepthStencilInfo.front.depthFailOp;

		VkStencilOp oldBackPassOp = mDepthStencilInfo.back.passOp;
		VkStencilOp oldBackFailOp = mDepthStencilInfo.back.failOp;
		VkStencilOp oldBackZFailOp = mDepthStencilInfo.back.depthFailOp;

		if(!enableDepthWrites)
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

		bool depthStencilReadOnly;
		if (framebuffer->hasDepthAttachment())
		{
			mPipelineInfo.pDepthStencilState = &mDepthStencilInfo;

			depthStencilReadOnly = !enableDepthWrites;
		}
		else
		{
			mPipelineInfo.pDepthStencilState = nullptr;
			depthStencilReadOnly = true;
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
			{
				VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[i];
				colorReadOnly[i] = true;
			}
		}

		std::pair<VkShaderStageFlagBits, GpuProgramCore*> stages[] =
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
			VulkanGpuProgramCore* program = static_cast<VulkanGpuProgramCore*>(stages[i].second);
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

		return device->getResourceManager().create<VulkanPipeline>(pipeline, colorReadOnly, depthStencilReadOnly);
	}

	VulkanComputePipelineStateCore::VulkanComputePipelineStateCore(const SPtr<GpuProgramCore>& program, 
		GpuDeviceFlags deviceMask)
		:ComputePipelineStateCore(program, deviceMask), mDeviceMask(deviceMask)
	{
		bs_zero_out(mPerDeviceData);
	}

	VulkanComputePipelineStateCore::~VulkanComputePipelineStateCore()
	{
		for (UINT32 i = 0; i < BS_MAX_DEVICES; i++)
		{
			if (mPerDeviceData[i].device == nullptr)
				continue;

			mPerDeviceData[i].pipeline->destroy();
		}
	}

	void VulkanComputePipelineStateCore::initialize()
	{
		ComputePipelineStateCore::initialize();

		// This might happen fairly often if shaders with unsupported languages are loaded, in which case the pipeline
		// will never get used, and its fine not to initialize it.
		if (!mProgram->isCompiled())
			return;

		VulkanGpuProgramCore* vkProgram = static_cast<VulkanGpuProgramCore*>(mProgram.get());

		VkPipelineShaderStageCreateInfo stageCI;
		stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCI.pNext = nullptr;
		stageCI.flags = 0;
		stageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		stageCI.module = VK_NULL_HANDLE;
		stageCI.pName = vkProgram->getProperties().getEntryPoint().c_str();
		stageCI.pSpecializationInfo = nullptr;

		VkComputePipelineCreateInfo pipelineCI;
		pipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCI.pNext = nullptr;
		pipelineCI.flags = 0;
		pipelineCI.stage = stageCI;
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		pipelineCI.basePipelineIndex = -1;

		VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPICore::instance());

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

	VulkanPipeline* VulkanComputePipelineStateCore::getPipeline(UINT32 deviceIdx) const
	{
		return mPerDeviceData[deviceIdx].pipeline;
	}

	VkPipelineLayout VulkanComputePipelineStateCore::getPipelineLayout(UINT32 deviceIdx) const
	{
		return mPerDeviceData[deviceIdx].pipelineLayout;
	}

	void VulkanComputePipelineStateCore::registerPipelineResources(VulkanCmdBuffer* cmdBuffer)
	{
		UINT32 deviceIdx = cmdBuffer->getDeviceIdx();

		VulkanGpuProgramCore* program = static_cast<VulkanGpuProgramCore*>(mProgram.get());
		if(program != nullptr)
		{
			VulkanShaderModule* module = program->getShaderModule(deviceIdx);

			if (module != nullptr)
				cmdBuffer->registerResource(module, VulkanUseFlag::Read);
		}
	}
}