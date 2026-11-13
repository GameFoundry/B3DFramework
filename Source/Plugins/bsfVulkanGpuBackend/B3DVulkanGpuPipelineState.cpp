//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanGpuPipelineState.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanGpuProgram.h"
#include "B3DVulkanUtility.h"
#include "B3DVulkanGpuPipelineParameterLayout.h"
#include "B3DVulkanVertexInput.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "Profiling/B3DRenderStats.h"
#include "B3DVulkanRenderPass.h"
#include "B3DVulkanGpuBackend.h"
#include "Utility/B3DConfigVariable.h"
#include "Utility/B3DTimer.h"

using namespace b3d;
using namespace b3d::render;

#if B3D_BUILD_TYPE_DEVELOPMENT
/** 
 * When enabled, captures and logs per-shader pipeline executable statistics (VGPR/LDS/occupancy) at compute-pipeline creation. 
 * Development builds only.
 */
static TConfigVariable gDumpPipelineStats("gpu.DumpPipelineStats",
	"Capture and log per-shader pipeline executable statistics (VGPR/LDS/occupancy) at pipeline creation. "
	"Requires VK_KHR_pipeline_executable_properties.",
	false,
	ConfigVariableFlag::ReadOnly);

/**
 * Print every per-shader executable statistic the driver exposes for a freshly created pipeline
 * (NVIDIA reports register count / occupancy hints; AMD reports VGPRs/SGPRs/LDS/wavefront occupancy).
 */
static void DumpPipelineExecutableStats(VkDevice device, VkPipeline pipeline, const String& programName)
{
	VkPipelineInfoKHR pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INFO_KHR;
	pipelineInfo.pipeline = pipeline;

	uint32_t executablePropertyCount = 0;
	b3d::vkGetPipelineExecutablePropertiesKHR(device, &pipelineInfo, &executablePropertyCount, nullptr);
	if(executablePropertyCount == 0)
		return;

	Vector<VkPipelineExecutablePropertiesKHR> executableProperties(executablePropertyCount);
	for(uint32_t propertyIndex = 0; propertyIndex < executablePropertyCount; ++propertyIndex)
	{
		executableProperties[propertyIndex] = {};
		executableProperties[propertyIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_PROPERTIES_KHR;
	}
	b3d::vkGetPipelineExecutablePropertiesKHR(device, &pipelineInfo, &executablePropertyCount, executableProperties.data());

	for(uint32_t propertyIndex = 0; propertyIndex < executablePropertyCount; ++propertyIndex)
	{
		VkPipelineExecutableInfoKHR executableInfo = {};
		executableInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_INFO_KHR;
		executableInfo.pipeline = pipeline;
		executableInfo.executableIndex = propertyIndex;

		uint32_t statisticCount = 0;
		b3d::vkGetPipelineExecutableStatisticsKHR(device, &executableInfo, &statisticCount, nullptr);

		if(statisticCount == 0)
			continue;

		Vector<VkPipelineExecutableStatisticKHR> statistics(statisticCount);
		for(uint32_t statisticIndex = 0; statisticIndex < statisticCount; ++statisticIndex)
		{
			statistics[statisticIndex] = {};
			statistics[statisticIndex].sType = VK_STRUCTURE_TYPE_PIPELINE_EXECUTABLE_STATISTIC_KHR;
		}

		b3d::vkGetPipelineExecutableStatisticsKHR(device, &executableInfo, &statisticCount, statistics.data());

		for(uint32_t statisticIndex = 0; statisticIndex < statisticCount; ++statisticIndex)
		{
			const VkPipelineExecutableStatisticKHR& statistic = statistics[statisticIndex];

			String value;
			switch(statistic.format)
			{
			case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_BOOL32_KHR:
				value = ToString((u32)statistic.value.b32); break;
			case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_INT64_KHR:
				value = ToString((i64)statistic.value.i64); break;
			case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_UINT64_KHR:
				value = ToString((u64)statistic.value.u64); break;
			case VK_PIPELINE_EXECUTABLE_STATISTIC_FORMAT_FLOAT64_KHR:
				value = ToString((double)statistic.value.f64); break;
			default: break;
			}

			B3D_LOG(Info, LogRenderBackend, "[PIPESTATS] {0} | exec='{1}' subgroup={2} | {3} = {4}",
				programName, executableProperties[propertyIndex].name, executableProperties[propertyIndex].subgroupSize, statistic.name, value);
		}
	}
}
#endif

VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, const std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT>& colorReadOnly, bool depthStencilReadOnly, u32 vertexBufferBindingCount, const StringView& name)
	: VulkanResource(owner, true, name), mPipeline(pipeline), mReadOnlyColor(colorReadOnly), mReadOnlyDepth(depthStencilReadOnly), mVertexBufferBindingCount(vertexBufferBindingCount)
{}

VulkanPipeline::VulkanPipeline(VulkanResourceManager* owner, VkPipeline pipeline, const StringView& name)
	: VulkanResource(owner, true, name), mPipeline(pipeline), mReadOnlyColor()
{}

VulkanPipeline::~VulkanPipeline()
{
	vkDestroyPipeline(mOwner->GetDevice().GetLogical(), mPipeline, gVulkanAllocator);
}

VulkanGpuGraphicsPipelineState::GpuPipelineKey::GpuPipelineKey(
	u32 framebufferId, u32 vertexInputId, RenderSurfaceMask readOnlyMask, DrawOperationType drawOp)
	: FramebufferId(framebufferId), VertexInputId(vertexInputId), ReadOnlyMask(readOnlyMask), DrawOp(drawOp)
{
}

size_t VulkanGpuGraphicsPipelineState::HashFunc::operator()(const GpuPipelineKey& key) const
{
	size_t hash = 0;
	B3DCombineHash(hash, key.FramebufferId);
	B3DCombineHash(hash, key.VertexInputId);
	B3DCombineHash(hash, key.ReadOnlyMask);
	B3DCombineHash(hash, key.DrawOp);

	return hash;
}

bool VulkanGpuGraphicsPipelineState::EqualFunc::operator()(const GpuPipelineKey& a, const GpuPipelineKey& b) const
{
	if(a.FramebufferId != b.FramebufferId)
		return false;

	if(a.VertexInputId != b.VertexInputId)
		return false;

	if(a.ReadOnlyMask != b.ReadOnlyMask)
		return false;

	if(a.DrawOp != b.DrawOp)
		return false;

	return true;
}

VulkanGpuGraphicsPipelineState::VulkanGpuGraphicsPipelineState(VulkanGpuDevice& gpuDevice, const GpuGraphicsPipelineStateInformation& createInformation)
	: GpuGraphicsPipelineState(gpuDevice, createInformation)
{ }

VulkanGpuGraphicsPipelineState::~VulkanGpuGraphicsPipelineState()
{
	for(auto& entry : mPipelines)
		entry.second->Destroy();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_PipelineState);
}

void VulkanGpuGraphicsPipelineState::Initialize()
{
	Lock lock(mMutex);

	GpuGraphicsPipelineState::Initialize();

	std::pair<VkShaderStageFlagBits, GpuProgram*> stages[] = {
		{ VK_SHADER_STAGE_VERTEX_BIT, mData.VertexProgram.get() },
		{ VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, mData.HullProgram.get() },
		{ VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, mData.DomainProgram.get() },
		{ VK_SHADER_STAGE_GEOMETRY_BIT, mData.GeometryProgram.get() },
		{ VK_SHADER_STAGE_FRAGMENT_BIT, mData.FragmentProgram.get() }
	};

	u32 stageOutputIndex = 0;
	const u32 stageCount = sizeof(stages) / sizeof(stages[0]);
	for(u32 i = 0; i < stageCount; i++)
	{
		VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(stages[i].second);
		if(program == nullptr)
			continue;

		VkPipelineShaderStageCreateInfo& stageCI = mShaderStageInfos[stageOutputIndex];
		stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCI.pNext = nullptr;
		stageCI.flags = 0;
		stageCI.stage = stages[i].first;
		stageCI.module = VK_NULL_HANDLE;
		stageCI.pName = program->GetEntryPoint().c_str();
		stageCI.pSpecializationInfo = nullptr;

		stageOutputIndex++;
	}

	u32 numUsedStages = stageOutputIndex;

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

	const RasterizerStateInformation& rasterizerStateInformation = GetRasterizerState();
	const BlendStateInformation& blendStateInformation = GetBlendState();
	const DepthStencilStateInformation depthStencilStateInformation = GetDepthStencilState();

	mRasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	mRasterizationInfo.pNext = nullptr;
	mRasterizationInfo.flags = 0;
	mRasterizationInfo.depthClampEnable = !rasterizerStateInformation.DepthClipEnable;
	mRasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	mRasterizationInfo.polygonMode = VulkanUtility::GetPolygonMode(rasterizerStateInformation.PolygonMode);
	mRasterizationInfo.cullMode = VulkanUtility::GetCullMode(rasterizerStateInformation.CullMode);
	mRasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	mRasterizationInfo.depthBiasEnable = rasterizerStateInformation.DepthBias != 0.0f;
	mRasterizationInfo.depthBiasConstantFactor = rasterizerStateInformation.DepthBias;
	mRasterizationInfo.depthBiasSlopeFactor = rasterizerStateInformation.SlopeScaledDepthBias;
	mRasterizationInfo.depthBiasClamp = mRasterizationInfo.depthClampEnable ? rasterizerStateInformation.DepthBiasClamp : 0.0f;
	mRasterizationInfo.lineWidth = 1.0f;

	mMultiSampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	mMultiSampleInfo.pNext = nullptr;
	mMultiSampleInfo.flags = 0;
	mMultiSampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Assigned at runtime
	mMultiSampleInfo.sampleShadingEnable = VK_FALSE; // When enabled, perform shading per sample instead of per pixel (more expensive, essentially FSAA)
	mMultiSampleInfo.minSampleShading = 1.0f; // Minimum percent of samples to run full shading for when sampleShadingEnable is enabled (1.0f to run for all)
	mMultiSampleInfo.pSampleMask = nullptr; // Normally one bit for each sample: e.g. 0x0000000F to enable all samples in a 4-sample setup
	mMultiSampleInfo.alphaToCoverageEnable = blendStateInformation.EnableAlphaToCoverage;
	mMultiSampleInfo.alphaToOneEnable = VK_FALSE;

	VkStencilOpState stencilFrontInfo;
	stencilFrontInfo.compareOp = VulkanUtility::GetCompareOp(depthStencilStateInformation.FrontStencilComparisonFunc);
	stencilFrontInfo.depthFailOp = VulkanUtility::GetStencilOp(depthStencilStateInformation.FrontStencilZFailOp);
	stencilFrontInfo.passOp = VulkanUtility::GetStencilOp(depthStencilStateInformation.FrontStencilPassOp);
	stencilFrontInfo.failOp = VulkanUtility::GetStencilOp(depthStencilStateInformation.FrontStencilFailOp);
	stencilFrontInfo.reference = 0; // Dynamic
	stencilFrontInfo.compareMask = (u32)depthStencilStateInformation.StencilReadMask;
	stencilFrontInfo.writeMask = (u32)depthStencilStateInformation.StencilWriteMask;

	VkStencilOpState stencilBackInfo;
	stencilBackInfo.compareOp = VulkanUtility::GetCompareOp(depthStencilStateInformation.BackStencilComparisonFunc);
	stencilBackInfo.depthFailOp = VulkanUtility::GetStencilOp(depthStencilStateInformation.BackStencilZFailOp);
	stencilBackInfo.passOp = VulkanUtility::GetStencilOp(depthStencilStateInformation.BackStencilPassOp);
	stencilBackInfo.failOp = VulkanUtility::GetStencilOp(depthStencilStateInformation.BackStencilFailOp);
	stencilBackInfo.reference = 0; // Dynamic
	stencilBackInfo.compareMask = (u32)depthStencilStateInformation.StencilReadMask;
	stencilBackInfo.writeMask = (u32)depthStencilStateInformation.StencilWriteMask;

	mDepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	mDepthStencilInfo.pNext = nullptr;
	mDepthStencilInfo.flags = 0;
	mDepthStencilInfo.depthBoundsTestEnable = false;
	mDepthStencilInfo.minDepthBounds = 0.0f;
	mDepthStencilInfo.maxDepthBounds = 1.0f;
	mDepthStencilInfo.depthTestEnable = depthStencilStateInformation.DepthReadEnable;
	mDepthStencilInfo.depthWriteEnable = depthStencilStateInformation.DepthWriteEnable;
	mDepthStencilInfo.depthCompareOp = VulkanUtility::GetCompareOp(depthStencilStateInformation.DepthComparisonFunc);
	mDepthStencilInfo.front = stencilFrontInfo;
	mDepthStencilInfo.back = stencilBackInfo;
	mDepthStencilInfo.stencilTestEnable = depthStencilStateInformation.StencilEnable;

	for(u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
	{
		u32 rtIdx = 0;
		if(blendStateInformation.EnableIndependantBlend)
			rtIdx = i;

		VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[i];
		blendState.blendEnable = blendStateInformation.RenderTargets[rtIdx].BlendEnable;
		blendState.colorBlendOp = VulkanUtility::GetBlendOp(blendStateInformation.RenderTargets[rtIdx].ColorBlendOperation);
		blendState.srcColorBlendFactor = VulkanUtility::GetBlendFactor(blendStateInformation.RenderTargets[rtIdx].ColorSourceFactor);
		blendState.dstColorBlendFactor = VulkanUtility::GetBlendFactor(blendStateInformation.RenderTargets[rtIdx].ColorDestinationFactor);
		blendState.alphaBlendOp = VulkanUtility::GetBlendOp(blendStateInformation.RenderTargets[rtIdx].AlphaBlendOperation);
		blendState.srcAlphaBlendFactor = VulkanUtility::GetBlendFactor(blendStateInformation.RenderTargets[rtIdx].AlphaSourceFactor);
		blendState.dstAlphaBlendFactor = VulkanUtility::GetBlendFactor(blendStateInformation.RenderTargets[rtIdx].AlphaDestinationFactor);
		blendState.colorWriteMask = blendStateInformation.RenderTargets[rtIdx].RenderTargetWriteMask & 0xF;
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

	const u32 dynamicStateCount = sizeof(mDynamicStates) / sizeof(mDynamicStates[0]);
	B3D_ASSERT(dynamicStateCount == 3);

	mDynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	mDynamicStateInfo.pNext = nullptr;
	mDynamicStateInfo.flags = 0;
	mDynamicStateInfo.dynamicStateCount = dynamicStateCount;
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

	if(mData.VertexProgram != nullptr)
		mVertexDescription = mData.VertexProgram->GetVertexInputDescription();

	mPipelineLayout = static_cast<VulkanGpuPipelineParameterLayout*>(mParameterLayout.get())->GetPipelineLayout();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineState);
}

VulkanPipeline* VulkanGpuGraphicsPipelineState::FindOrCreateVulkanResource(VulkanRenderPass* renderPass, RenderSurfaceMask readOnlyMask, DrawOperationType drawOp, const VulkanVertexInput& vertexInput)
{
	Lock lock(mMutex);

	GpuPipelineKey key(renderPass->GetId(), vertexInput.GetId(), readOnlyMask, drawOp);

	auto iterFind = mPipelines.find(key);
	if(iterFind != mPipelines.end())
		return iterFind->second;

	VulkanPipeline* newPipeline = CreatePipeline(renderPass, readOnlyMask, drawOp, vertexInput);
	mPipelines[key] = newPipeline;

	return newPipeline;
}

void VulkanGpuGraphicsPipelineState::RegisterShaderModuleResources(GpuResourceTracker& resourceTracker)
{
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
			VulkanShaderModule* module = entry->GetVulkanResource();

			if(module != nullptr)
				resourceTracker.TrackResourceUsage(module, GpuAccessFlag::Read);
		}
	}
}

VulkanPipeline* VulkanGpuGraphicsPipelineState::CreatePipeline(VulkanRenderPass* renderPass, RenderSurfaceMask readOnlyMask, DrawOperationType primitiveType, const VulkanVertexInput& vertexInput)
{
	mInputAssemblyInfo.topology = VulkanUtility::GetDrawOp(primitiveType);
	mTesselationInfo.patchControlPoints = 3; // Not provided by our shaders for now
	mMultiSampleInfo.rasterizationSamples = renderPass->GetSampleFlags();
	mColorBlendStateInfo.attachmentCount = renderPass->GetColorAttachmentCount() > 0 ? (renderPass->GetMaximumColorAttachmentIndex() + 1) : 0;

	const DepthStencilStateInformation depthStencilStateInformation = GetDepthStencilState();
	bool enableDepthWrites = depthStencilStateInformation.DepthWriteEnable && !readOnlyMask.IsSet(RT_DEPTH);

	mDepthStencilInfo.depthWriteEnable = enableDepthWrites; // If depth stencil attachment is read only, depthWriteEnable must be VK_FALSE

	// Save stencil ops as we might need to change them if depth/stencil is read-only
	VkStencilOp oldFrontPassOp = mDepthStencilInfo.front.passOp;
	VkStencilOp oldFrontFailOp = mDepthStencilInfo.front.failOp;
	VkStencilOp oldFrontZFailOp = mDepthStencilInfo.front.depthFailOp;

	VkStencilOp oldBackPassOp = mDepthStencilInfo.back.passOp;
	VkStencilOp oldBackFailOp = mDepthStencilInfo.back.failOp;
	VkStencilOp oldBackZFailOp = mDepthStencilInfo.back.depthFailOp;

	if(readOnlyMask.IsSet(RT_STENCIL))
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
	mPipelineInfo.renderPass = renderPass->GetVkRenderPass(RT_NONE, RT_NONE, RT_NONE, RT_ALL);
	mPipelineInfo.layout = mPipelineLayout;
	mPipelineInfo.pVertexInputState = vertexInput.GetCreateInfo();

	bool depthReadOnly;
	if(renderPass->HasDepthAttachment())
	{
		mPipelineInfo.pDepthStencilState = &mDepthStencilInfo;
		depthReadOnly = readOnlyMask.IsSet(RT_DEPTH);
	}
	else
	{
		mPipelineInfo.pDepthStencilState = nullptr;
		depthReadOnly = true;
	}

	std::array<bool, B3D_MAXIMUM_RENDER_TARGET_COUNT> colorReadOnly;
	if(renderPass->GetColorAttachmentCount() > 0)
	{
		mPipelineInfo.pColorBlendState = &mColorBlendStateInfo;

		for(u32 colorAttachmentIndex = 0; colorAttachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorAttachmentIndex++)
		{
			VkPipelineColorBlendAttachmentState& blendState = mAttachmentBlendStates[colorAttachmentIndex];
			if(readOnlyMask.IsSet((RenderSurfaceMaskBits)(1 << colorAttachmentIndex)))
			{
				// Disable writes to this color attachment
				blendState.colorWriteMask = 0;
			}

			colorReadOnly[colorAttachmentIndex] = blendState.colorWriteMask == 0;
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

		VulkanShaderModule* module = program->GetVulkanResource();

		if(module != nullptr)
			stageCI.module = module->GetVulkanHandle();
		else
			stageCI.module = VK_NULL_HANDLE;

		stageOutputIdx++;
	}

	VulkanGpuDevice& gpuDevice = static_cast<VulkanGpuDevice&>(mGpuDevice);
	VkDevice vkDevice = gpuDevice.GetLogical();

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

	return gpuDevice.GetResourceManager().Create<VulkanPipeline>(pipeline, colorReadOnly, depthReadOnly, vertexInput.GetVertexBufferBindingCount());
}

VulkanGpuComputePipelineState::VulkanGpuComputePipelineState(VulkanGpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation)
	: GpuComputePipelineState(gpuDevice, createInformation)
{
}

VulkanGpuComputePipelineState::~VulkanGpuComputePipelineState()
{
	if(mPipeline != nullptr)
		mPipeline->Destroy();
}

void VulkanGpuComputePipelineState::Initialize()
{
	GpuComputePipelineState::Initialize();

	// This might happen fairly often if shaders with unsupported languages are loaded, in which case the pipeline
	// will never get used, and its fine not to initialize it.
	if(!mData.Program->IsCompiled())
		return;

	VulkanGpuProgram* vkProgram = static_cast<VulkanGpuProgram*>(mData.Program.get());

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

	VulkanGpuDevice& gpuDevice = static_cast<VulkanGpuDevice&>(mGpuDevice);
	VulkanResourceManager& rescManager = gpuDevice.GetResourceManager();

	VulkanShaderModule* module = vkProgram->GetVulkanResource();

	if(module != nullptr)
		pipelineCI.stage.module = module->GetVulkanHandle();
	else
		pipelineCI.stage.module = VK_NULL_HANDLE;

	pipelineCI.layout = static_cast<VulkanGpuPipelineParameterLayout*>(mParameterLayout.get())->GetPipelineLayout();

#if B3D_BUILD_TYPE_DEVELOPMENT
	// When the gpu.DumpPipelineStats config variable is enabled, ask the driver to keep per-shader executable
	// statistics (VGPR count, occupancy, LDS, etc.) so we can reason about why a compute kernel is occupancy-bound.
	// Requires VK_KHR_pipeline_executable_properties, enabled at device creation.
	const bool captureStats = gDumpPipelineStats.Get() && (b3d::vkGetPipelineExecutableStatisticsKHR != nullptr);
	if(captureStats)
		pipelineCI.flags |= VK_PIPELINE_CREATE_CAPTURE_STATISTICS_BIT_KHR;
#endif

	VkPipeline pipeline;
#if B3D_BUILD_TYPE_DEVELOPMENT
	const bool timeCompile = gGpuDumpPipelineTimings.Get();
	const Timer compileTimer; // starts timing on construction; only read below when timeCompile is set
#endif
	VkResult result = vkCreateComputePipelines(gpuDevice.GetLogical(), VK_NULL_HANDLE, 1, &pipelineCI, gVulkanAllocator, &pipeline);
	B3D_ASSERT(result == VK_SUCCESS);

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(timeCompile && result == VK_SUCCESS)
	{
		const double compileMs = compileTimer.GetMicroseconds() / 1000.0;
		B3D_LOG(Info, LogRenderBackend, "[PIPELINE] compute {0} ms {1}", compileMs, vkProgram->GetName());
	}

	if(captureStats && result == VK_SUCCESS)
		DumpPipelineExecutableStats(gpuDevice.GetLogical(), pipeline, vkProgram->GetName());
#endif

	mPipeline = rescManager.Create<VulkanPipeline>(pipeline);
	mPipelineLayout = pipelineCI.layout;

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineState);
}

VulkanGpuRayTracingPipelineState::VulkanGpuRayTracingPipelineState(VulkanGpuDevice& gpuDevice, const GpuRayTracingPipelineStateCreateInformation& createInformation)
	: GpuRayTracingPipelineState(gpuDevice, createInformation)
{ }

VulkanGpuRayTracingPipelineState::~VulkanGpuRayTracingPipelineState()
{
	if(mPipeline != nullptr)
		mPipeline->Destroy();
}

void VulkanGpuRayTracingPipelineState::Initialize()
{
	GpuRayTracingPipelineState::Initialize();

	// Like compute pipelines, a pipeline with a program that didn't compile is never created, and dispatches with it fail
	Vector<VkPipelineShaderStageCreateInfo> stages;
	bool areProgramsCompiled = true;
	const auto fnAddStage = [&stages, &areProgramsCompiled](const TShared<GpuProgram>& program) -> u32
	{
		if(program == nullptr)
			return VK_SHADER_UNUSED_KHR;

		const VulkanGpuProgram& vulkanProgram = static_cast<const VulkanGpuProgram&>(*program);
		const VulkanShaderModule* const module = vulkanProgram.GetVulkanResource();
		if(!vulkanProgram.IsCompiled() || module == nullptr)
		{
			areProgramsCompiled = false;
			return VK_SHADER_UNUSED_KHR;
		}

		// Every program is a module of its own, so entry point names can't collide
		VkPipelineShaderStageCreateInfo stageCI;
		stageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stageCI.pNext = nullptr;
		stageCI.flags = 0;
		stageCI.stage = VulkanUtility::GetShaderStage(vulkanProgram.GetType());
		stageCI.module = module->GetVulkanHandle();
		stageCI.pName = vulkanProgram.GetEntryPoint().c_str();
		stageCI.pSpecializationInfo = nullptr;

		stages.push_back(stageCI);
		return (u32)stages.size() - 1;
	};

	// Groups in the order GetShaderGroupHandle() numbers them
	Vector<VkRayTracingShaderGroupCreateInfoKHR> groups;
	const auto fnAddGroup = [&groups](VkRayTracingShaderGroupTypeKHR type, u32 generalStage, u32 closestHitStage, u32 anyHitStage, u32 intersectionStage)
	{
		VkRayTracingShaderGroupCreateInfoKHR groupCI;
		groupCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
		groupCI.pNext = nullptr;
		groupCI.type = type;
		groupCI.generalShader = generalStage;
		groupCI.closestHitShader = closestHitStage;
		groupCI.anyHitShader = anyHitStage;
		groupCI.intersectionShader = intersectionStage;
		groupCI.pShaderGroupCaptureReplayHandle = nullptr;

		groups.push_back(groupCI);
	};

	const auto fnAddGeneralGroups = [&fnAddStage, &fnAddGroup](const Vector<TShared<GpuProgram>>& programs)
	{
		for(const TShared<GpuProgram>& program : programs)
			fnAddGroup(VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR, fnAddStage(program), VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR, VK_SHADER_UNUSED_KHR);
	};

	fnAddGeneralGroups(mData.RayGenerationPrograms);
	fnAddGeneralGroups(mData.MissPrograms);

	for(const GpuRayTracingHitGroup& hitGroup : mData.HitGroups)
	{
		const VkRayTracingShaderGroupTypeKHR type = hitGroup.IntersectionProgram != nullptr ? VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR
			: VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;

		const u32 closestHitStage = fnAddStage(hitGroup.ClosestHitProgram);
		const u32 anyHitStage = fnAddStage(hitGroup.AnyHitProgram);
		const u32 intersectionStage = fnAddStage(hitGroup.IntersectionProgram);
		fnAddGroup(type, VK_SHADER_UNUSED_KHR, closestHitStage, anyHitStage, intersectionStage);
	}

	fnAddGeneralGroups(mData.CallablePrograms);

	if(!areProgramsCompiled)
		return;

	VulkanGpuDevice& gpuDevice = static_cast<VulkanGpuDevice&>(mGpuDevice);
	const VkPipelineLayout pipelineLayout = static_cast<VulkanGpuPipelineParameterLayout*>(mParameterLayout.get())->GetPipelineLayout();

	VkRayTracingPipelineCreateInfoKHR pipelineCI;
	pipelineCI.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	pipelineCI.pNext = nullptr;
	pipelineCI.flags = 0;
	pipelineCI.stageCount = (u32)stages.size();
	pipelineCI.pStages = stages.data();
	pipelineCI.groupCount = (u32)groups.size();
	pipelineCI.pGroups = groups.data();
	pipelineCI.maxPipelineRayRecursionDepth = mData.MaximumRecursionDepth;
	pipelineCI.pLibraryInfo = nullptr;
	pipelineCI.pLibraryInterface = nullptr;
	pipelineCI.pDynamicState = nullptr;
	pipelineCI.layout = pipelineLayout;
	pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCI.basePipelineIndex = -1;

	const String& pipelineName = mData.RayGenerationPrograms[0]->GetName();

#if B3D_BUILD_TYPE_DEVELOPMENT
	const bool timeCompile = gGpuDumpPipelineTimings.Get();
	const Timer compileTimer; // starts timing on construction; only read below when timeCompile is set
#endif

	// No deferred operation, pipeline cache or libraries: one pipeline with every stage, created now
	VkPipeline pipeline = VK_NULL_HANDLE;
	const VkResult result = vkCreateRayTracingPipelinesKHR(gpuDevice.GetLogical(), VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCI, gVulkanAllocator, &pipeline);
	if(!B3D_ENSURE_LOG(result == VK_SUCCESS, "Failed to create the ray tracing pipeline of '{0}' (VkResult {1}).", pipelineName, (i32)result))
		return;

#if B3D_BUILD_TYPE_DEVELOPMENT
	if(timeCompile)
	{
		const double compileMs = compileTimer.GetMicroseconds() / 1000.0;
		B3D_LOG(Info, LogRenderBackend, "[PIPELINE] ray tracing {0} ms {1}", compileMs, pipelineName);
	}
#endif

	// Read once, for every shader binding table created from the pipeline
	const u32 handleSize = gpuDevice.GetCapabilities().RayTracing.ShaderGroupHandleSize;
	mShaderGroupHandles.resize((size_t)handleSize * groups.size());

	const VkResult handleResult = vkGetRayTracingShaderGroupHandlesKHR(gpuDevice.GetLogical(), pipeline, 0, (u32)groups.size(), mShaderGroupHandles.size(), mShaderGroupHandles.data());
	B3D_ASSERT(handleResult == VK_SUCCESS);
	(void)handleResult;

	mPipeline = gpuDevice.GetResourceManager().Create<VulkanPipeline>(pipeline);
	mPipelineLayout = pipelineLayout;

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_PipelineState);
}

TArrayView<const u8> VulkanGpuRayTracingPipelineState::GetShaderGroupHandle(u32 groupIndex) const
{
	if(mPipeline == nullptr)
		return TArrayView<const u8>();

	const u32 handleSize = mGpuDevice.GetCapabilities().RayTracing.ShaderGroupHandleSize;
	B3D_ASSERT((groupIndex + 1) * (size_t)handleSize <= mShaderGroupHandles.size());

	return TArrayView<const u8>(mShaderGroupHandles.data() + (size_t)groupIndex * handleSize, handleSize);
}

void VulkanGpuRayTracingPipelineState::RegisterShaderModuleResources(GpuResourceTracker& resourceTracker)
{
	const auto fnTrackModule = [&resourceTracker](const TShared<GpuProgram>& program)
	{
		if(program == nullptr)
			return;

		VulkanShaderModule* const module = static_cast<VulkanGpuProgram&>(*program).GetVulkanResource();
		if(module != nullptr)
			resourceTracker.TrackResourceUsage(module, GpuAccessFlag::Read);
	};

	for(const TShared<GpuProgram>& program : mData.RayGenerationPrograms)
		fnTrackModule(program);

	for(const TShared<GpuProgram>& program : mData.MissPrograms)
		fnTrackModule(program);

	for(const GpuRayTracingHitGroup& hitGroup : mData.HitGroups)
	{
		fnTrackModule(hitGroup.ClosestHitProgram);
		fnTrackModule(hitGroup.AnyHitProgram);
		fnTrackModule(hitGroup.IntersectionProgram);
	}

	for(const TShared<GpuProgram>& program : mData.CallablePrograms)
		fnTrackModule(program);
}

void VulkanGpuComputePipelineState::RegisterShaderModuleResources(GpuResourceTracker& resourceTracker)
{
	VulkanGpuProgram* program = static_cast<VulkanGpuProgram*>(mData.Program.get());
	if(program != nullptr)
	{
		VulkanShaderModule* module = program->GetVulkanResource();

		if(module != nullptr)
			resourceTracker.TrackResourceUsage(module, GpuAccessFlag::Read);
	}
}
