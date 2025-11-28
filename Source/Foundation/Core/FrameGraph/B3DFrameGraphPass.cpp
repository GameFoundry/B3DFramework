#include "B3DFrameGraphPass.h"
#include "B3DFrameGraph.h"
#include "B3DFrameGraphResource.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "Debug/B3DDebug.h"

using namespace b3d;
using namespace b3d::render;

FrameGraphPass::FrameGraphPass(
	u32 index,
	const StringView& name,
	GpuQueueUsage queue,
	FrameGraph* frameGraph,
	FrameGraphPassType type)
	: mIndex(index)
	, mName(name)
	, mQueue(queue)
	, mFrameGraph(frameGraph)
	, mType(type)
	, mDepthAttachment(kInvalidFrameGraphResourceId)
{
	B3D_ENSURE(frameGraph != nullptr);
}

void FrameGraphPass::Read(FrameGraphResourceId resource, GpuResourceUseFlags usage)
{
	B3D_ENSURE(resource.IsValid());
	mResourceAccesses.emplace_back(resource, usage, GpuAccessFlag::Read);
}

void FrameGraphPass::Write(FrameGraphResourceId resource, GpuResourceUseFlags usage)
{
	B3D_ENSURE(resource.IsValid());
	mResourceAccesses.emplace_back(resource, usage, GpuAccessFlag::Write);
}

void FrameGraphPass::ReadWrite(FrameGraphResourceId resource, GpuResourceUseFlags usage)
{
	B3D_ENSURE(resource.IsValid());
	mResourceAccesses.emplace_back(resource, usage, GpuAccessFlag::Read | GpuAccessFlag::Write);
}

void FrameGraphPass::WriteColor(FrameGraphResourceId resource, u32 index)
{
	B3D_ENSURE(mType == FrameGraphPassType::Render);
	B3D_ENSURE(resource.IsValid());

	mColorAttachments[index] = resource;

	// Automatically add as a write with RenderTarget usage
	Write(resource, GpuResourceUseFlag::ColorAttachment);
}

void FrameGraphPass::WriteDepth(FrameGraphResourceId resource)
{
	B3D_ENSURE(mType == FrameGraphPassType::Render);
	B3D_ENSURE(resource.IsValid());

	mDepthAttachment = resource;
	mDepthReadOnly = false;

	// Automatically add as a write with DepthStencil usage
	Write(resource, GpuResourceUseFlag::DepthStencilAttachment);
}

void FrameGraphPass::ReadDepth(FrameGraphResourceId resource)
{
	B3D_ENSURE(mType == FrameGraphPassType::Render);
	B3D_ENSURE(resource.IsValid());

	mDepthAttachment = resource;
	mDepthReadOnly = true;

	// Automatically add as a read with DepthStencil usage
	Read(resource, GpuResourceUseFlag::DepthStencilAttachment);
}

void FrameGraphPass::UseParameters(const SPtr<GpuParameterSet>& params)
{
	if (!params)
		return;

	auto layout = params->GetPipelineParameterLayout();
	if (!layout)
		return;

	// Each GpuParameterSet handles a single descriptor set
	const u32 set = params->GetSet();

	// Import sampled textures (always read-only)
	const u32 sampledTextureCount = layout->GetBindingCount(set, GpuParameterType::SampledTexture);
	for (u32 bindingIndex = 0; bindingIndex < sampledTextureCount; bindingIndex++)
	{
		const u32 arraySize = layout->GetArraySize(GpuParameterType::SampledTexture, set, bindingIndex);
		const u32 slot = layout->GetSlot(GpuParameterType::SampledTexture, set, bindingIndex);

		for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			auto texture = params->GetSampledTexture(slot, arrayIndex);
			if (texture)
			{
				auto texId = mFrameGraph->ImportTexture("Tex_" + ToString(set) + "_" + ToString(slot), texture);
				Read(texId, GpuResourceUseFlag::ShaderAccess);
			}
		}
	}

	// Import storage textures (always read-write)
	const u32 storageTextureCount = layout->GetBindingCount(set, GpuParameterType::StorageTexture);
	for (u32 bindingIndex = 0; bindingIndex < storageTextureCount; bindingIndex++)
	{
		const u32 arraySize = layout->GetArraySize(GpuParameterType::StorageTexture, set, bindingIndex);
		const u32 slot = layout->GetSlot(GpuParameterType::StorageTexture, set, bindingIndex);

		for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			auto texture = params->GetStorageTexture(slot, arrayIndex);
			if (texture)
			{
				auto texId = mFrameGraph->ImportTexture("StorageTex_" + ToString(set) + "_" + ToString(slot), texture);
				ReadWrite(texId, GpuResourceUseFlag::ShaderAccess);
			}
		}
	}

	// Import uniform buffers (always read-only)
	const u32 uniformBufferCount = layout->GetBindingCount(set, GpuParameterType::UniformBuffer);
	for (u32 bindingIndex = 0; bindingIndex < uniformBufferCount; bindingIndex++)
	{
		const u32 arraySize = layout->GetArraySize(GpuParameterType::UniformBuffer, set, bindingIndex);
		const u32 slot = layout->GetSlot(GpuParameterType::UniformBuffer, set, bindingIndex);

		for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			auto buffer = params->GetUniformBuffer(slot, arrayIndex);
			if (buffer)
			{
				auto bufId = mFrameGraph->ImportBuffer("UB_" + ToString(set) + "_" + ToString(slot), buffer);
				Read(bufId, GpuResourceUseFlag::UniformBuffer);
			}
		}
	}

	// Import storage buffers (check type for access flags)
	const u32 storageBufferCount = layout->GetBindingCount(set, GpuParameterType::StorageBuffer);
	for (u32 bindingIndex = 0; bindingIndex < storageBufferCount; bindingIndex++)
	{
		const u32 arraySize = layout->GetArraySize(GpuParameterType::StorageBuffer, set, bindingIndex);
		const u32 slot = layout->GetSlot(GpuParameterType::StorageBuffer, set, bindingIndex);

		// Get uniform information to determine the object type
		const UniformInformation* uniformInfo = layout->TryGetUniformInformation(GpuParameterBinding(set, slot));

		for (u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
		{
			auto buffer = params->GetStorageBuffer(slot, arrayIndex);
			if (buffer)
			{
				auto bufId = mFrameGraph->ImportBuffer("StorageBuf_" + ToString(set) + "_" + ToString(slot), buffer);

				// Determine access based on type (matches PrepareForBind logic)
				if (uniformInfo && (uniformInfo->ObjectType == GPOT_RWBYTE_BUFFER || uniformInfo->ObjectType == GPOT_RWSTRUCTURED_BUFFER))
					ReadWrite(bufId, GpuResourceUseFlag::ShaderAccess);
				else
					Read(bufId, GpuResourceUseFlag::ShaderAccess);
			}
		}
	}
}

void FrameGraphPass::ExecuteSetup()
{
	if (mSetupFunction)
		mSetupFunction(*this);
}

void FrameGraphPass::ExecuteCommands(GpuCommandBuffer& commandBuffer, FrameGraphPassResources& resources)
{
	if (mExecuteFunction)
		mExecuteFunction(commandBuffer, resources);
}

void FrameGraphPass::Reset()
{
	// Clear resource accesses (will be repopulated during next setup)
	mResourceAccesses.clear();

	// Clear attachment data
	mColorAttachments.clear();
	mDepthAttachment = kInvalidFrameGraphResourceId;
	mDepthReadOnly = false;

	// Reset dependency metadata
	mIncomingDependencies.clear();
	mOutgoingDependencies.clear();
	mReferenceCount = 0;
	mCulled = false;
}

void FrameGraphPass::AddIncomingDependency(const FrameGraphPassDependency& dependency)
{
	mIncomingDependencies.push_back(dependency);
}

void FrameGraphPass::AddOutgoingDependency(const FrameGraphPassDependency& dependency)
{
	mOutgoingDependencies.push_back(dependency);
}
