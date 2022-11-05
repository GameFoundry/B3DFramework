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
#include "BsVulkanRenderPass.h"

using namespace bs;
using namespace bs::ct;

VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, const std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT>& colorReadOnly, bool depthStencilReadOnly)
	: VulkanResource(owner, true), mPipeline(pipeline), mReadOnlyColor(colorReadOnly), mReadOnlyDepth(depthStencilReadOnly)
{}

VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline)
	: VulkanResource(owner, true), mPipeline(pipeline), mReadOnlyColor()
{}

VulkanPipeline::~VulkanPipeline()
{
	vkDestroyPipeline(mOwner->GetDevice().GetLogical(), mPipeline, gVulkanAllocator);
}

VulkanGraphicsPipelineState::GpuPipelineKey::GpuPipelineKey(
	u32 framebufferId, u32 vertexInputId, u32 readOnlyFlags, DrawOperationType drawOp)
	: FramebufferId(framebufferId), VertexInputId(vertexInputId), ReadOnlyFlags(readOnlyFlags), DrawOp(drawOp)
{
}

size_t VulkanGraphicsPipelineState::HashFunc::operator()(const GpuPipelineKey& key) const
{
	size_t hash = 0;
	B3DCombineHash(hash, key.FramebufferId);
	B3DCombineHash(hash, key.VertexInputId);
	B3DCombineHash(hash, key.ReadOnlyFlags);
	B3DCombineHash(hash, key.DrawOp);

	return hash;
}

bool VulkanGraphicsPipelineState::EqualFunc::operator()(const GpuPipelineKey& a, const GpuPipelineKey& b) const
{
	if(a.FramebufferId != b.FramebufferId)
		return false;

	if(a.VertexInputId != b.VertexInputId)
		return false;

	if(a.ReadOnlyFlags != b.ReadOnlyFlags)
		return false;

	if(a.DrawOp != b.DrawOp)
		return false;

	return true;
}

VulkanGraphicsPipelineState::VulkanGraphicsPipelineState(const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask)
	: GraphicsPipelineState(desc, deviceMask), mScissorEnabled(false), mDeviceMask(deviceMask)
{
	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		mPerDeviceData[i].Device = nullptr;
		mPerDeviceData[i].PipelineLayout = VK_NULL_HANDLE;
	}
}

VulkanGraphicsPipelineState::~VulkanGraphicsPipelineState()
{
	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].Device == nullptr)
			continue;

		for(auto& entry : mPerDeviceData[i].Pipelines)
			entry.second->Destroy();
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_PipelineState);
}

void VulkanGraphicsPipelineState::Initialize()
{
	Lock lock(mMutex);

	GraphicsPipelineState::Initialize();

	std::pair<VkShaderStageFlagBits, GpuProgram*> stages[] = {
		{ VK_SHADER_STAGE_VERTEX_BIT, mData.VertexProgram.get() },
		{ VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, mData.HullProgram.get() },
		{ VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, mData.DomainProgram.get() },
		{ VK_SHADER_STAGE_GEOMETRY_BIT, mData.GeometryProgram.get() },
		{ VK_SHADER_STAGE_FRAGMENT_BIT, mData.FragmentProgram.get() }
	};

	u32 stageOutputIdx = 0;
	u32 numStages = sizeof(stages) / sizeof(stages[0]);
	for(u32 i = 0; i < numStages; i++)
	{
		VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(stages[i].second);
		if(program == nullptr)
			continue;

		VkPipelineShaderStageCreateInfo& stageCI = mShaderStageInfos[stageOutputIdx];
		stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCI.pNext = nullptr;
		stageCI.flags = 0;
		stageCI.stage = stages[i].first;
		stageCI.module = VK_NULL_HANDLE;
		stageCI.pName = program->GetEntryPoint().c_str();
		stageCI.pSpecializationInfo = nullptr;

		stageOutputIdx++;
	}

	u32 numUsedStages = stageOutputIdx;

	bool tesselationEnabled = mData.HullProgram != nullptr && mData.DomainProgram != nullptr;

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

	RasterizerState* rasterizerState = GetRasterizerState().get();
	if(rasterizerState == nullptr)
		rasterizerState = RasterizerState::GetDefault().get();

	BlendState* blendState = GetBlendState().get();
	if(blendState == nullptr)
		blendState = BlendState::GetDefault().get();

	DepthStencilState* depthStencilState = GetDepthStencilState().get();
	if(depthStencilState == nullptr)
		depthStencilState = DepthStencilState::GetDefault().get();

	const RasterizerProperties& rstProps = rasterizerState->GetProperties();
	const BlendProperties& blendProps = blendState->GetProperties();
	const DepthStencilProperties dsProps = depthStencilState->GetProperties();

	mRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	mRasterizationInfo.pNext = nullptr;
	mRasterizationInfo.flags = 0;
	mRasterizationInfo.depthClampEnable = !rstProps.GetDepthClipEnable();
	mRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	mRasterizationInfo.polygonMode = VulkanUtility::GetPolygonMode(rstProps.GetPolygonMode());
	mRasterizationInfo.cullMode = VulkanUtility::GetCullMode(rstProps.GetCullMode());
	mRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	mRasterizationInfo.depthBiasEnable = rstProps.GetDepthBias() != 0.0f;
	mRasterizationInfo.depthBiasConstantFactor = rstProps.GetDepthBias();
	mRasterizationInfo.depthBiasSlopeFactor = rstProps.GetSlopeScaledDepthBias();
	mRasterizationInfo.depthBiasClamp = mRasterizationInfo.depthClampEnable ? rstProps.GetDepthBiasClamp() : 0.0f;
	mRasterizationInfo.lineWidth = 1.0f;

	mMultiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mMultiSampleInfo.pNext = nullptr;
	mMultiSampleInfo.flags = 0;
	mMultiSampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Assigned at runtime
	mMultiSampleInfo.sampleShadingEnable = VK_FALSE; // When enabled, perform shading per sample instead of per pixel (more expensive, essentially FSAA)
	mMultiSampleInfo.minSampleShading = 1.0f; // Minimum percent of samples to run full shading for when sampleShadingEnable is enabled (1.0f to run for all)
	mMultiSampleInfo.pSampleMask = nullptr; // Normally one bit for each sample: e.g. 0x0000000F to enable all samples in a 4-sample setup
	mMultiSampleInfo.alphaToCoverageEnable = blendProps.GetAlphaToCoverageEnabled();
	mMultiSampleInfo.alphaToOneEnable = VK_FALSE;

	VkStencilOpState stencilFrontInfo;
	stencilFrontInfo.compareOp = VulkanUtility::GetCompareOp(dsProps.GetStencilFrontCompFunc());
	stencilFrontInfo.depthFailOp = VulkanUtility::GetStencilOp(dsProps.GetStencilFrontZFailOp());
	stencilFrontInfo.passOp = VulkanUtility::GetStencilOp(dsProps.GetStencilFrontPassOp());
	stencilFrontInfo.failOp = VulkanUtility::GetStencilOp(dsProps.GetStencilFrontFailOp());
	stencilFrontInfo.reference = 0; // Dynamic
	stencilFrontInfo.compareMask = (u32)dsProps.GetStencilReadMask();
	stencilFrontInfo.writeMask = (u32)dsProps.GetStencilWriteMask();

	VkStencilOpState stencilBackInfo;
	stencilBackInfo.compareOp = VulkanUtility::GetCompareOp(dsProps.GetStencilBackCompFunc());
	stencilBackInfo.depthFailOp = VulkanUtility::GetStencilOp(dsProps.GetStencilBackZFailOp());
	stencilBackInfo.passOp = VulkanUtility::GetStencilOp(dsProps.GetStencilBackPassOp());
	stencilBackInfo.failOp = VulkanUtility::GetStencilOp(dsProps.GetStencilBackFailOp());
	stencilBackInfo.reference = 0; // Dynamic
	stencilBackInfo.compareMask = (u32)dsProps.GetStencilReadMask();
	stencilBackInfo.writeMask = (u32)dsProps.GetStencilWriteMask();

	mDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	mDepthStencilInfo.pNext = nullptr;
	mDepthStencilInfo.flags = 0;
	mDepthStencilInfo.depthBoundsTestEnable = false;
	mDepthStencilInfo.minDepthBounds = 0.0f;
	mDepthStencilInfo.maxDepthBounds = 1.0f;
	mDepthStencilInfo.depthTestEnable = dsProps.GetDepthReadEnable();
	mDepthStencilInfo.depthWriteEnable = dsProps.GetDepthWriteEnable();
	mDepthStencilInfo.depthCompareOp = VulkanUtility::GetCompareOp(dsProps.GetDepthComparisonFunc());
	mDepthStencilInfo.front = stencilFrontInfo;
	mDepthStencilInfo.back = stencilBackInfo;
	mDepthStencilInfo.stencilTestEnable = dsProps.GetStencilEnable();

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		u32 rtIdx = 0;
		if(blendProps.GetIndependantBlendEnable())
			rtIdx = i;

		VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[i];
		blendState.blendEnable = blendProps.GetBlendEnabled(rtIdx);
		blendState.colorBlendOp = VulkanUtility::GetBlendOp(blendProps.GetBlendOperation(rtIdx));
		blendState.srcColorBlendFactor = VulkanUtility::GetBlendFactor(blendProps.GetSrcBlend(rtIdx));
		blendState.dstColorBlendFactor = VulkanUtility::GetBlendFactor(blendProps.GetDstBlend(rtIdx));
		blendState.alphaBlendOp = VulkanUtility::GetBlendOp(blendProps.GetAlphaBlendOperation(rtIdx));
		blendState.srcAlphaBlendFactor = VulkanUtility::GetBlendFactor(blendProps.GetAlphaSrcBlend(rtIdx));
		blendState.dstAlphaBlendFactor = VulkanUtility::GetBlendFactor(blendProps.GetAlphaDstBlend(rtIdx));
		blendState.colorWriteMask = blendProps.GetRenderTargetWriteMask(rtIdx) & 0xF;
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

	u32 numDynamicStates = sizeof(mDynamicStates) / sizeof(mDynamicStates[0]);
	B3D_ASSERT(numDynamicStates == 3);

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

	mScissorEnabled = rstProps.GetScissorEnable();

	if(mData.VertexProgram != nullptr)
		mVertexDecl = mData.VertexProgram->GetInputDeclaration();

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());

	VulkanDevice* devices[BS_MAX_DEVICES];
	VulkanUtility::GetDevices(rapi, mDeviceMask, devices);

	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(devices[i] == nullptr)
			continue;

		mPerDeviceData[i].Device = devices[i];

		VulkanDescriptorManager& descManager = mPerDeviceData[i].Device->GetDescriptorManager();
		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		u32 numLayouts = vkParamInfo.GetNumSets();
		VulkanDescriptorLayout** layouts = (VulkanDescriptorLayout**)B3DStackAllocate(sizeof(VulkanDescriptorLayout*) * numLayouts);

		for(u32 j = 0; j < numLayouts; j++)
			layouts[j] = vkParamInfo.GetLayout(i, j);

		mPerDeviceData[i].PipelineLayout = descManager.GetPipelineLayout(layouts, numLayouts);

		B3DStackFree(layouts);
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineState);
}

VulkanPipeline* VulkanGraphicsPipelineState::GetPipeline(
	u32 deviceIdx, VulkanRenderPass* renderPass, u32 readOnlyFlags, DrawOperationType drawOp,
	const SPtr<VulkanVertexInput>& vertexInput)
{
	Lock lock(mMutex);

	if(mPerDeviceData[deviceIdx].Device == nullptr)
		return nullptr;

	readOnlyFlags &= ~FBT_COLOR; // Ignore the color
	GpuPipelineKey key(renderPass->GetId(), vertexInput->GetId(), readOnlyFlags, drawOp);

	PerDeviceData& perDeviceData = mPerDeviceData[deviceIdx];
	auto iterFind = perDeviceData.Pipelines.find(key);
	if(iterFind != perDeviceData.Pipelines.end())
		return iterFind->second;

	VulkanPipeline* newPipeline = CreatePipeline(deviceIdx, renderPass, readOnlyFlags, drawOp, vertexInput);
	perDeviceData.Pipelines[key] = newPipeline;

	return newPipeline;
}

VkPipelineLayout VulkanGraphicsPipelineState::GetPipelineLayout(u32 deviceIdx) const
{
	return mPerDeviceData[deviceIdx].PipelineLayout;
}

void VulkanGraphicsPipelineState::RegisterPipelineResources(VulkanCmdBuffer* cmdBuffer)
{
	u32 deviceIdx = cmdBuffer->GetDeviceIdx();

	std::array<VulkanGpuProgram*, 5> programs = {
		static_cast<VulkanGpuProgram*>(mData.VertexProgram.get()),
		static_cast<VulkanGpuProgram*>(mData.HullProgram.get()),
		static_cast<VulkanGpuProgram*>(mData.DomainProgram.get()),
		static_cast<VulkanGpuProgram*>(mData.GeometryProgram.get()),
		static_cast<VulkanGpuProgram*>(mData.FragmentProgram.get()),
	};

	for(auto& entry : programs)
	{
		if(entry != nullptr)
		{
			VulkanShaderModule* module = entry->GetShaderModule(deviceIdx);

			if(module != nullptr)
				cmdBuffer->RegisterResource(module, VulkanAccessFlag::Read);
		}
	}
}

VulkanPipeline* VulkanGraphicsPipelineState::CreatePipeline(u32 deviceIdx, VulkanRenderPass* renderPass, u32 readOnlyFlags, DrawOperationType drawOp, const SPtr<VulkanVertexInput>& vertexInput)
{
	mInputAssemblyInfo.topology = VulkanUtility::GetDrawOp(drawOp);
	mTesselationInfo.patchControlPoints = 3; // Not provided by our shaders for now
	mMultiSampleInfo.rasterizationSamples = renderPass->GetSampleFlags();
	mColorBlendStateInfo.attachmentCount = renderPass->GetNumColorAttachments();

	DepthStencilState* dsState = GetDepthStencilState().get();
	if(dsState == nullptr)
		dsState = DepthStencilState::GetDefault().get();

	const DepthStencilProperties dsProps = dsState->GetProperties();
	bool enableDepthWrites = dsProps.GetDepthWriteEnable() && (readOnlyFlags & FBT_DEPTH) == 0;

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
	mPipelineInfo.renderPass = renderPass->GetVkRenderPass(RT_NONE, RT_NONE, CLEAR_NONE);
	mPipelineInfo.layout = mPerDeviceData[deviceIdx].PipelineLayout;
	mPipelineInfo.pVertexInputState = vertexInput->GetCreateInfo();

	bool depthReadOnly;
	if(renderPass->HasDepthAttachment())
	{
		mPipelineInfo.pDepthStencilState = &mDepthStencilInfo;
		depthReadOnly = (readOnlyFlags & FBT_DEPTH) != 0;
	}
	else
	{
		mPipelineInfo.pDepthStencilState = nullptr;
		depthReadOnly = true;
	}

	std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT> colorReadOnly;
	if(renderPass->GetNumColorAttachments() > 0)
	{
		mPipelineInfo.pColorBlendState = &mColorBlendStateInfo;

		for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		{
			VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[i];
			colorReadOnly[i] = blendState.colorWriteMask == 0;
		}
	}
	else
	{
		mPipelineInfo.pColorBlendState = nullptr;

		for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
			colorReadOnly[i] = true;
	}

	std::pair<VkShaderStageFlagBits, GpuProgram*> stages[] = {
		{ VK_SHADER_STAGE_VERTEX_BIT, mData.VertexProgram.get() },
		{ VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, mData.HullProgram.get() },
		{ VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, mData.DomainProgram.get() },
		{ VK_SHADER_STAGE_GEOMETRY_BIT, mData.GeometryProgram.get() },
		{ VK_SHADER_STAGE_FRAGMENT_BIT, mData.FragmentProgram.get() }
	};

	u32 stageOutputIdx = 0;
	u32 numStages = sizeof(stages) / sizeof(stages[0]);
	for(u32 i = 0; i < numStages; i++)
	{
		VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(stages[i].second);
		if(program == nullptr)
			continue;

		VkPipelineShaderStageCreateInfo& stageCI = mShaderStageInfos[stageOutputIdx];

		VulkanShaderModule* module = program->GetShaderModule(deviceIdx);

		if(module != nullptr)
			stageCI.module = module->GetHandle();
		else
			stageCI.module = VK_NULL_HANDLE;

		stageOutputIdx++;
	}

	VulkanDevice* device = mPerDeviceData[deviceIdx].Device;
	VkDevice vkDevice = mPerDeviceData[deviceIdx].Device->GetLogical();

	VkPipeline pipeline;
	VkResult result = vkCreateGraphicsPipelines(vkDevice, VK_NULL_HANDLE, 1, &mPipelineInfo, gVulkanAllocator, &pipeline);
	B3D_ASSERT(result == VK_SUCCESS);

	// Restore previous stencil op states
	mDepthStencilInfo.front.passOp = oldFrontPassOp;
	mDepthStencilInfo.front.failOp = oldFrontFailOp;
	mDepthStencilInfo.front.depthFailOp = oldFrontZFailOp;

	mDepthStencilInfo.back.passOp = oldBackPassOp;
	mDepthStencilInfo.back.failOp = oldBackFailOp;
	mDepthStencilInfo.back.depthFailOp = oldBackZFailOp;

	return device->GetResourceManager().Create<VulkanPipeline>(pipeline, colorReadOnly, depthReadOnly);
}

VulkanComputePipelineState::VulkanComputePipelineState(const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask)
	: ComputePipelineState(program, deviceMask), mDeviceMask(deviceMask)
{
	B3DZeroOut(mPerDeviceData);
}

VulkanComputePipelineState::~VulkanComputePipelineState()
{
	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(mPerDeviceData[i].Device == nullptr)
			continue;

		mPerDeviceData[i].Pipeline->Destroy();
	}
}

void VulkanComputePipelineState::Initialize()
{
	ComputePipelineState::Initialize();

	// This might happen fairly often if shaders with unsupported languages are loaded, in which case the pipeline
	// will never get used, and its fine not to initialize it.
	if(!mProgram->IsCompiled())
		return;

	VulkanGpuProgram* vkProgram = static_cast<VulkanGpuProgram*>(mProgram.get());

	VkPipelineShaderStageCreateInfo stageCI;
	stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageCI.pNext = nullptr;
	stageCI.flags = 0;
	stageCI.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageCI.module = VK_NULL_HANDLE;
	stageCI.pName = vkProgram->GetEntryPoint().c_str();
	stageCI.pSpecializationInfo = nullptr;

	VkComputePipelineCreateInfo pipelineCI;
	pipelineCI.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	pipelineCI.pNext = nullptr;
	pipelineCI.flags = 0;
	pipelineCI.stage = stageCI;
	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.basePipelineIndex = -1;

	VulkanRenderAPI& rapi = static_cast<VulkanRenderAPI&>(RenderAPI::Instance());

	VulkanDevice* devices[BS_MAX_DEVICES];
	VulkanUtility::GetDevices(rapi, mDeviceMask, devices);

	for(u32 i = 0; i < BS_MAX_DEVICES; i++)
	{
		if(devices[i] == nullptr)
			continue;

		mPerDeviceData[i].Device = devices[i];

		VulkanDescriptorManager& descManager = devices[i]->GetDescriptorManager();
		VulkanResourceManager& rescManager = devices[i]->GetResourceManager();
		VulkanGpuPipelineParamInfo& vkParamInfo = static_cast<VulkanGpuPipelineParamInfo&>(*mParamInfo);

		u32 numLayouts = vkParamInfo.GetNumSets();
		VulkanDescriptorLayout** layouts = (VulkanDescriptorLayout**)B3DStackAllocate(sizeof(VulkanDescriptorLayout*) * numLayouts);

		for(u32 j = 0; j < numLayouts; j++)
			layouts[j] = vkParamInfo.GetLayout(i, j);

		VulkanShaderModule* module = vkProgram->GetShaderModule(i);

		if(module != nullptr)
			pipelineCI.stage.module = module->GetHandle();
		else
			pipelineCI.stage.module = VK_NULL_HANDLE;

		pipelineCI.layout = descManager.GetPipelineLayout(layouts, numLayouts);

		VkPipeline pipeline;
		VkResult result = vkCreateComputePipelines(devices[i]->GetLogical(), VK_NULL_HANDLE, 1, &pipelineCI, gVulkanAllocator, &pipeline);
		B3D_ASSERT(result == VK_SUCCESS);

		mPerDeviceData[i].Pipeline = rescManager.Create<VulkanPipeline>(pipeline);
		mPerDeviceData[i].PipelineLayout = pipelineCI.layout;
		B3DStackFree(layouts);
	}

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineState);
}

VulkanPipeline* VulkanComputePipelineState::GetPipeline(u32 deviceIdx) const
{
	return mPerDeviceData[deviceIdx].Pipeline;
}

VkPipelineLayout VulkanComputePipelineState::GetPipelineLayout(u32 deviceIdx) const
{
	return mPerDeviceData[deviceIdx].PipelineLayout;
}

void VulkanComputePipelineState::RegisterPipelineResources(VulkanCmdBuffer* cmdBuffer)
{
	u32 deviceIdx = cmdBuffer->GetDeviceIdx();

	VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(mProgram.get());
	if(program != nullptr)
	{
		VulkanShaderModule* module = program->GetShaderModule(deviceIdx);

		if(module != nullptr)
			cmdBuffer->RegisterResource(module, VulkanAccessFlag::Read);
	}
}
