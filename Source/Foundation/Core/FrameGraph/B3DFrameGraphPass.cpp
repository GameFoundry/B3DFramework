#include "B3DFrameGraphPass.h"
#include "B3DFrameGraph.h"
#include "B3DFrameGraphResource.h"
#include "RenderAPI/B3DRenderTexture.h"
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

void FrameGraphPass::ExecuteSetup()
{
	if (mSetupFunction)
		mSetupFunction(*this);
}

void FrameGraphPass::ExecuteCommands(GpuCommandBuffer& commandBuffer)
{
	if (mExecuteFunction)
		mExecuteFunction(commandBuffer);
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
