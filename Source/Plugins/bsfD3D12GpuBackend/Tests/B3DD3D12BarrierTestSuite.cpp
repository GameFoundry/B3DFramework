//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierTestSuite.h"
#include "B3DD3D12BarrierUtility.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12Utility.h"
#include "Utility/B3DD3D12BarrierBatch.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuSubmitThread.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "CoreObject/B3DRenderThread.h"
#include "Image/B3DTexture.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	GpuDevice* GetActiveD3D12Device()
	{
		GpuBackend& backend = GpuBackend::Instance();
		if(String(backend.GetBackendName()) != "bsfD3D12GpuBackend" || backend.GetDeviceCount() == 0)
			return nullptr;

		return backend.GetDevice(0).get();
	}

	void CreateResolveTextures(GpuDevice& device, TShared<render::Texture>& outSource, TShared<render::Texture>& outDestination)
	{
		TextureCreateInformation sourceCreateInformation;
		sourceCreateInformation.Name = "D3D12 resolve test source";
		sourceCreateInformation.Format = PF_RGBA8;
		sourceCreateInformation.Width = 16;
		sourceCreateInformation.Height = 16;
		sourceCreateInformation.SampleCount = 4;
		sourceCreateInformation.Usage = TextureUsageFlag::RenderTarget;
		outSource = device.CreateTexture(sourceCreateInformation);

		TextureCreateInformation destinationCreateInformation(sourceCreateInformation);
		destinationCreateInformation.Name = "D3D12 resolve test destination";
		destinationCreateInformation.SampleCount = 1;
		outDestination = device.CreateTexture(destinationCreateInformation);
	}

	void AddTextureBarrier(D3D12BarrierBatch& batch, ID3D12Resource* resource, const GpuTextureSubresourceRange& range,
		const GpuBarrierScope& scope, GpuImageLayout logicalBeforeLayout, GpuImageLayout logicalAfterLayout,
		const D3D12TextureLayout& nativeBeforeLayout, const D3D12TextureLayout& nativeAfterLayout,
		D3D12_TEXTURE_BARRIER_FLAGS flags = D3D12_TEXTURE_BARRIER_FLAG_NONE)
	{
		batch.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(resource, range, scope, logicalBeforeLayout,
			logicalAfterLayout, nativeBeforeLayout, nativeAfterLayout, flags));
	}
}

D3D12BarrierTestSuite::D3D12BarrierTestSuite() : TestSuite("D3D12BarrierTestSuite")
{
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestBufferBarrierScopes)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestTextureBarrierScopes)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestTextureBarrierBatch)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestPlacedRenderTargetAllocation)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestShaderReadableDepthTextures)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestResolveBarrierMappings)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestTextureLayoutMappings)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestCopyQueueLayoutMappings)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestQueueSharing)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestRestingTransitionSubmission)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestMultisampleResolve)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestResolveValidation)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestFrameFence)
}

void D3D12BarrierTestSuite::TestBufferBarrierScopes()
{
	const D3D12BarrierScope vertexBufferScope = D3D12BarrierUtility::GetBufferScope(GpuStageFlag::VertexInputAttributes,
		GpuAccessFlag::Read, D3D12_RESOURCE_FLAG_NONE);
	B3D_TEST_ASSERT(vertexBufferScope.Sync == D3D12_BARRIER_SYNC_VERTEX_SHADING)
	B3D_TEST_ASSERT(vertexBufferScope.Access == D3D12_BARRIER_ACCESS_VERTEX_BUFFER)

	const D3D12BarrierScope indexBufferScope = D3D12BarrierUtility::GetBufferScope(GpuStageFlag::VertexInputIndices,
		GpuAccessFlag::Read, D3D12_RESOURCE_FLAG_NONE);
	B3D_TEST_ASSERT(indexBufferScope.Sync == D3D12_BARRIER_SYNC_INDEX_INPUT)
	B3D_TEST_ASSERT(indexBufferScope.Access == D3D12_BARRIER_ACCESS_INDEX_BUFFER)
}

void D3D12BarrierTestSuite::TestTextureBarrierScopes()
{
	const GpuTextureAspectFlags colorAspect = GpuTextureAspectFlag::Color;
	const D3D12BarrierScope renderTargetScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::ColorAttachment,
		GpuAccessFlag::Write, GpuImageLayout::ColorAttachment, colorAspect);
	B3D_TEST_ASSERT(renderTargetScope.Access == D3D12_BARRIER_ACCESS_RENDER_TARGET)
	B3D_TEST_ASSERT(renderTargetScope.Sync == D3D12_BARRIER_SYNC_RENDER_TARGET)

	const D3D12BarrierScope shaderReadScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::FragmentShaderNonUniform,
		GpuAccessFlag::Read, GpuImageLayout::ShaderReadOnly, colorAspect);
	B3D_TEST_ASSERT(shaderReadScope.Access == D3D12_BARRIER_ACCESS_SHADER_RESOURCE)
	B3D_TEST_ASSERT(shaderReadScope.Sync == D3D12_BARRIER_SYNC_PIXEL_SHADING)

	const GpuStageFlags depthShaderStages = GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests |
		GpuStageFlag::FragmentShaderNonUniform;
	const D3D12BarrierScope sampledDepthScope = D3D12BarrierUtility::GetTextureScope(depthShaderStages,
		GpuAccessFlag::Read, GpuImageLayout::DepthStencilReadOnly, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(sampledDepthScope.Access ==
		(D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ | D3D12_BARRIER_ACCESS_SHADER_RESOURCE))
	B3D_TEST_ASSERT(sampledDepthScope.Sync == (D3D12_BARRIER_SYNC_DEPTH_STENCIL | D3D12_BARRIER_SYNC_PIXEL_SHADING))

	const D3D12BarrierScope depthAttachmentReadScope = D3D12BarrierUtility::GetTextureScope(
		GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests, GpuAccessFlag::Read,
		GpuImageLayout::DepthStencilAttachment, GpuTextureAspectFlag::Depth);
	B3D_TEST_ASSERT(depthAttachmentReadScope.Access == D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ)
	B3D_TEST_ASSERT(depthAttachmentReadScope.Sync == D3D12_BARRIER_SYNC_DEPTH_STENCIL)

	const D3D12BarrierScope unorderedAccessScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::ComputeShaderNonUniform,
		GpuAccessFlag::Write, GpuImageLayout::General, colorAspect);
	B3D_TEST_ASSERT(unorderedAccessScope.Access == D3D12_BARRIER_ACCESS_UNORDERED_ACCESS)
	B3D_TEST_ASSERT(unorderedAccessScope.Sync == D3D12_BARRIER_SYNC_COMPUTE_SHADING)

	const D3D12BarrierScope executionScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::ColorAttachment,
		GpuAccessFlag::None, GpuImageLayout::ColorAttachment, colorAspect);
	B3D_TEST_ASSERT(executionScope.Sync == D3D12_BARRIER_SYNC_RENDER_TARGET)
	B3D_TEST_ASSERT(executionScope.Access == D3D12_BARRIER_ACCESS_NO_ACCESS)

	const D3D12BarrierScope noAccessScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::None,
		GpuAccessFlag::None, GpuImageLayout::Undefined, colorAspect);
	B3D_TEST_ASSERT(noAccessScope.Sync == D3D12_BARRIER_SYNC_NONE)
	B3D_TEST_ASSERT(noAccessScope.Access == D3D12_BARRIER_ACCESS_NO_ACCESS)
	const D3D12BarrierScope allStageNoAccessScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::All,
		GpuAccessFlag::None, GpuImageLayout::Undefined, colorAspect);
	B3D_TEST_ASSERT(allStageNoAccessScope.Sync == D3D12_BARRIER_SYNC_ALL)
	B3D_TEST_ASSERT(allStageNoAccessScope.Access == D3D12_BARRIER_ACCESS_NO_ACCESS)

	const D3D12TextureLayout nativeShaderLayout(D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE);
	const D3D12BarrierScope carriedWriteScope = D3D12BarrierUtility::GetTextureLayoutScope(GpuImageLayout::ShaderReadOnly,
		nativeShaderLayout, colorAspect, GpuStageFlag::ColorAttachment);
	B3D_TEST_ASSERT(carriedWriteScope.Sync == D3D12_BARRIER_SYNC_PIXEL_SHADING)
	B3D_TEST_ASSERT(carriedWriteScope.Access == D3D12_BARRIER_ACCESS_SHADER_RESOURCE)

	const D3D12BarrierScope presentScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::ColorAttachment,
		GpuAccessFlag::Read, GpuImageLayout::Present, colorAspect);
	B3D_TEST_ASSERT(presentScope.Sync == D3D12_BARRIER_SYNC_RENDER_TARGET)
	B3D_TEST_ASSERT(presentScope.Access == D3D12_BARRIER_ACCESS_COMMON)
}

void D3D12BarrierTestSuite::TestTextureBarrierBatch()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr)
		return;

	TextureCreateInformation createInformation;
	createInformation.Name = "D3D12 barrier batch ordering texture";
	createInformation.Width = 16;
	createInformation.Height = 16;
	createInformation.MipMapCount = 3;
	createInformation.ArraySliceCount = 4;
	createInformation.Usage = TextureUsageFlag::RenderTarget;
	const TShared<render::Texture> texture = device->CreateTexture(createInformation);
	B3D_TEST_ASSERT(texture != nullptr)
	if(texture == nullptr)
		return;

	D3D12Image* const image = std::static_pointer_cast<D3D12Texture>(texture)->GetD3D12Image();
	B3D_TEST_ASSERT(image != nullptr)
	if(image == nullptr)
		return;

	ID3D12Resource* const resource = image->GetD3D12Resource();
	const GpuTextureAspectFlags colorAspect = GpuTextureAspectFlag::Color;
	const D3D12TextureLayout copyDestinationLayout(D3D12_BARRIER_LAYOUT_COPY_DEST);
	const D3D12TextureLayout shaderLayout(D3D12_BARRIER_LAYOUT_SHADER_RESOURCE);
	const D3D12TextureLayout unorderedAccessLayout(D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS);
	const GpuBarrierScope shaderReadBarrier(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuStageFlag::VertexShaderNonUniform | GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	const GpuBarrierScope unorderedAccessBarrier(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);

	D3D12BarrierBatch orderedBatch;
	const GpuTextureSubresourceRange fullRange(0, 2, 0, 2, colorAspect);
	const GpuBarrierScope leadingBarrier(GpuStageFlag::All, GpuAccessFlag::Read | GpuAccessFlag::Write,
		GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	const GpuBarrierScope prologueBarrier(GpuStageFlag::None, GpuAccessFlag::None,
		leadingBarrier.DestinationStages, leadingBarrier.DestinationAccess);
	D3D12BarrierBatch prologueBatch;
	AddTextureBarrier(prologueBatch, resource, fullRange, prologueBarrier, GpuImageLayout::Undefined,
		GpuImageLayout::General, copyDestinationLayout, unorderedAccessLayout);
	D3D12BarrierBatch leadingBatch;
	AddTextureBarrier(leadingBatch, resource, fullRange, leadingBarrier, GpuImageLayout::General,
		GpuImageLayout::ShaderReadOnly, unorderedAccessLayout, shaderLayout);
	B3D_TEST_ASSERT(prologueBatch.mTextureBarriers.Size() == 1)
	B3D_TEST_ASSERT(leadingBatch.mTextureBarriers.Size() == 1)
	B3D_TEST_ASSERT(prologueBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_NONE)
	B3D_TEST_ASSERT(prologueBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_NO_ACCESS)
	B3D_TEST_ASSERT(prologueBatch.mTextureBarriers[0].SyncAfter == D3D12_BARRIER_SYNC_PIXEL_SHADING)
	B3D_TEST_ASSERT(prologueBatch.mTextureBarriers[0].AccessAfter == D3D12_BARRIER_ACCESS_UNORDERED_ACCESS)
	B3D_TEST_ASSERT(leadingBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_ALL)
	B3D_TEST_ASSERT(leadingBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_UNORDERED_ACCESS)

	const GpuBarrierScope renderTargetPrologueBarrier(GpuStageFlag::None, GpuAccessFlag::None,
		GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	D3D12BarrierBatch renderTargetPrologueBatch;
	AddTextureBarrier(renderTargetPrologueBatch, resource, fullRange, renderTargetPrologueBarrier, GpuImageLayout::Undefined,
		GpuImageLayout::ColorAttachment, copyDestinationLayout, D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RENDER_TARGET));
	D3D12BarrierBatch renderTargetLeadingBatch;
	AddTextureBarrier(renderTargetLeadingBatch, resource, fullRange, leadingBarrier, GpuImageLayout::ColorAttachment,
		GpuImageLayout::ShaderReadOnly, D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RENDER_TARGET), shaderLayout);
	B3D_TEST_ASSERT(renderTargetPrologueBatch.mTextureBarriers[0].SyncAfter == D3D12_BARRIER_SYNC_RENDER_TARGET)
	B3D_TEST_ASSERT(renderTargetPrologueBatch.mTextureBarriers[0].AccessAfter == D3D12_BARRIER_ACCESS_RENDER_TARGET)
	B3D_TEST_ASSERT(renderTargetLeadingBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_ALL)
	B3D_TEST_ASSERT(renderTargetLeadingBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_RENDER_TARGET)

	D3D12BarrierBatch layoutOnlyBatch;
	AddTextureBarrier(layoutOnlyBatch, resource, fullRange, GpuBarrierScope(), GpuImageLayout::Undefined,
		GpuImageLayout::Undefined, copyDestinationLayout, shaderLayout);
	B3D_TEST_ASSERT(layoutOnlyBatch.mTextureBarriers.Size() == 1)
	B3D_TEST_ASSERT(layoutOnlyBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_NONE)
	B3D_TEST_ASSERT(layoutOnlyBatch.mTextureBarriers[0].SyncAfter == D3D12_BARRIER_SYNC_NONE)
	B3D_TEST_ASSERT(layoutOnlyBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_NO_ACCESS)
	B3D_TEST_ASSERT(layoutOnlyBatch.mTextureBarriers[0].AccessAfter == D3D12_BARRIER_ACCESS_NO_ACCESS)

	D3D12BarrierBatch noAccessSubmissionBatch;
	AddTextureBarrier(noAccessSubmissionBatch, resource, fullRange, GpuBarrierScope(), GpuImageLayout::Undefined,
		GpuImageLayout::TransferSource, copyDestinationLayout, D3D12TextureLayout(D3D12_BARRIER_LAYOUT_COMMON));
	B3D_TEST_ASSERT(noAccessSubmissionBatch.mTextureBarriers.Size() == 1)
	B3D_TEST_ASSERT(noAccessSubmissionBatch.mTextureBarriers[0].AccessAfter == D3D12_BARRIER_ACCESS_NO_ACCESS)
	B3D_TEST_ASSERT(noAccessSubmissionBatch.mTextureBarriers[0].SyncAfter == D3D12_BARRIER_SYNC_NONE)

	AddTextureBarrier(orderedBatch, resource, fullRange, shaderReadBarrier,
		GpuImageLayout::TransferDestination, GpuImageLayout::ShaderReadOnly, copyDestinationLayout, shaderLayout);
	AddTextureBarrier(orderedBatch, resource, fullRange,
		unorderedAccessBarrier, GpuImageLayout::ShaderReadOnly, GpuImageLayout::General, shaderLayout,
		unorderedAccessLayout);
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers.Size() == 2)
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[0].LayoutBefore == D3D12_BARRIER_LAYOUT_COPY_DEST)
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[0].LayoutAfter == D3D12_BARRIER_LAYOUT_SHADER_RESOURCE)
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[1].LayoutBefore == D3D12_BARRIER_LAYOUT_SHADER_RESOURCE)
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[1].LayoutAfter == D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS)
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_COPY)
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[0].SyncAfter ==
		(D3D12_BARRIER_SYNC_VERTEX_SHADING | D3D12_BARRIER_SYNC_PIXEL_SHADING))
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[1].SyncBefore == D3D12_BARRIER_SYNC_PIXEL_SHADING)
	B3D_TEST_ASSERT(orderedBatch.mTextureBarriers[1].SyncAfter == D3D12_BARRIER_SYNC_COMPUTE_SHADING)
	B3D_TEST_ASSERT(orderedBatch.mBarrierGroups.Size() == 1)
	B3D_TEST_ASSERT(orderedBatch.mBarrierGroups[0].BarrierCount == 2)

	D3D12BarrierBatch typeOrderBatch;
	typeOrderBatch.AddTextureBarrier(orderedBatch.mTextureBarriers[0]);
	typeOrderBatch.AddGlobalBarrier(D3D12_GLOBAL_BARRIER{});
	typeOrderBatch.AddTextureBarrier(orderedBatch.mTextureBarriers[1]);
	B3D_TEST_ASSERT(typeOrderBatch.mBarrierGroups.Size() == 3)
	B3D_TEST_ASSERT(typeOrderBatch.mBarrierGroups[0].Type == D3D12_BARRIER_TYPE_TEXTURE)
	B3D_TEST_ASSERT(typeOrderBatch.mBarrierGroups[1].Type == D3D12_BARRIER_TYPE_GLOBAL)
	B3D_TEST_ASSERT(typeOrderBatch.mBarrierGroups[2].Type == D3D12_BARRIER_TYPE_TEXTURE)

	D3D12BarrierBatch overlapBatch;
	const GpuTextureSubresourceRange firstRange(0, 2, 0, 2, colorAspect);
	const GpuTextureSubresourceRange overlappingRange(1, 2, 1, 2, colorAspect);
	const GpuTextureSubresourceRange separateRange(3, 1, 3, 1, colorAspect);
	AddTextureBarrier(overlapBatch, resource, firstRange, shaderReadBarrier, GpuImageLayout::TransferDestination, GpuImageLayout::ShaderReadOnly, copyDestinationLayout, shaderLayout);
	AddTextureBarrier(overlapBatch, resource, overlappingRange, unorderedAccessBarrier, GpuImageLayout::ShaderReadOnly, GpuImageLayout::General, shaderLayout, unorderedAccessLayout);
	AddTextureBarrier(overlapBatch, resource, separateRange, unorderedAccessBarrier, GpuImageLayout::ShaderReadOnly, GpuImageLayout::General, shaderLayout, unorderedAccessLayout);
	B3D_TEST_ASSERT(overlapBatch.mTextureBarriers.Size() == 3)
	B3D_TEST_ASSERT(overlapBatch.mTextureBarriers[0].Subresources.IndexOrFirstMipLevel == firstRange.BaseMipLevel)
	B3D_TEST_ASSERT(overlapBatch.mTextureBarriers[1].Subresources.IndexOrFirstMipLevel == overlappingRange.BaseMipLevel)
	B3D_TEST_ASSERT(overlapBatch.mTextureBarriers[2].Subresources.IndexOrFirstMipLevel == separateRange.BaseMipLevel)

	D3D12BarrierBatch discardBatch;
	const GpuTextureSubresourceRange topMip = GpuTextureSubresourceRange::TopMip(0, 1, colorAspect);
	const GpuBarrierScope discardBarrier(GpuStageFlag::None, GpuAccessFlag::None, GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);
	const GpuBarrierScope renderTargetReadBarrier(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read);
	const D3D12_TEXTURE_BARRIER unflaggedUndefinedBarrier = D3D12BarrierUtility::GetTextureBarrier(resource, topMip, discardBarrier,
		GpuImageLayout::Undefined, GpuImageLayout::ColorAttachment, D3D12TextureLayout::Undefined(),
		D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RENDER_TARGET), D3D12_TEXTURE_BARRIER_FLAG_NONE);
	B3D_TEST_ASSERT(unflaggedUndefinedBarrier.Flags == D3D12_TEXTURE_BARRIER_FLAG_NONE)

	AddTextureBarrier(discardBatch, resource, topMip, discardBarrier, GpuImageLayout::Undefined, GpuImageLayout::ColorAttachment,
		D3D12TextureLayout::Undefined(), D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RENDER_TARGET), D3D12_TEXTURE_BARRIER_FLAG_DISCARD);
	AddTextureBarrier(discardBatch, resource, topMip, renderTargetReadBarrier, GpuImageLayout::ColorAttachment, GpuImageLayout::ShaderReadOnly, D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RENDER_TARGET), shaderLayout);
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers.Size() == 2)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[0].Flags == D3D12_TEXTURE_BARRIER_FLAG_DISCARD)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[0].LayoutBefore == D3D12_BARRIER_LAYOUT_UNDEFINED)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[0].LayoutAfter == D3D12_BARRIER_LAYOUT_RENDER_TARGET)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_NO_ACCESS)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[0].AccessAfter == D3D12_BARRIER_ACCESS_RENDER_TARGET)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[1].Flags == D3D12_TEXTURE_BARRIER_FLAG_NONE)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[1].LayoutBefore == D3D12_BARRIER_LAYOUT_RENDER_TARGET)
	B3D_TEST_ASSERT(discardBatch.mTextureBarriers[1].LayoutAfter == D3D12_BARRIER_LAYOUT_SHADER_RESOURCE)

	D3D12BarrierBatch depthStencilBatch;
	const GpuBarrierScope depthStencilBarrier(GpuStageFlag::EarlyFragmentTests, GpuAccessFlag::Read | GpuAccessFlag::Write, GpuStageFlag::FragmentShaderNonUniform | GpuStageFlag::LateFragmentTests, GpuAccessFlag::Read | GpuAccessFlag::Write);
	const D3D12TextureLayout depthReadStencilWrite(D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ, D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE);
	const D3D12TextureLayout depthWriteStencilRead(D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE, D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ);
	const GpuTextureSubresourceRange depthRange(0, 1, 0, 1, GpuTextureAspectFlag::Depth);
	const GpuTextureSubresourceRange stencilRange(0, 1, 0, 1, GpuTextureAspectFlag::Stencil);
	AddTextureBarrier(depthStencilBatch, resource, depthRange, depthStencilBarrier, GpuImageLayout::DepthReadOnlyStencilAttachment, GpuImageLayout::DepthAttachmentStencilReadOnly, depthReadStencilWrite, depthWriteStencilRead);
	AddTextureBarrier(depthStencilBatch, resource, stencilRange, depthStencilBarrier, GpuImageLayout::DepthReadOnlyStencilAttachment, GpuImageLayout::DepthAttachmentStencilReadOnly, depthReadStencilWrite, depthWriteStencilRead);
	B3D_TEST_ASSERT(depthStencilBatch.mTextureBarriers.Size() == 2)
	B3D_TEST_ASSERT(depthStencilBatch.mTextureBarriers[0].Subresources.FirstPlane == 0)
	B3D_TEST_ASSERT(depthStencilBatch.mTextureBarriers[1].Subresources.FirstPlane == 1)
	B3D_TEST_ASSERT(depthStencilBatch.mTextureBarriers[0].Subresources.NumPlanes == 1)
	B3D_TEST_ASSERT(depthStencilBatch.mTextureBarriers[1].Subresources.NumPlanes == 1)

	const GpuAccessState colorAttachmentWrite(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write, GpuImageLayout::ColorAttachment);
	const GpuAccessState fragmentShaderRead(GpuStageFlag::FragmentShaderNonUniform, GpuAccessFlag::Read, GpuImageLayout::ShaderReadOnly);

	const TShared<GpuCommandBufferPool> graphicsPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
	const TShared<GpuCommandBuffer> commandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 sequential texture barrier validation"));
	commandBuffer->IssueBarriers(GpuTextureBarrier(texture, GpuBarrierTestUtility::GetDiscardedRestingState(*texture), colorAttachmentWrite, topMip));
	commandBuffer->IssueBarriers(GpuTextureBarrier(texture, colorAttachmentWrite, fragmentShaderRead, topMip));
	commandBuffer->End();
}

void D3D12BarrierTestSuite::TestPlacedRenderTargetAllocation()
{
	GpuDevice* const gpuDevice = GetActiveD3D12Device();
	if(gpuDevice == nullptr)
		return;

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(*gpuDevice);
	auto fnTestPlacedAllocation = [this, gpuDevice](PixelFormat format, TextureUsageFlags usage, u32 sampleCount)
	{
		TextureCreateInformation createInformation;
		createInformation.Format = format;
		createInformation.Width = 64;
		createInformation.Height = 64;
		createInformation.SampleCount = sampleCount;
		createInformation.Usage = usage;

		const TShared<render::Texture> texture = gpuDevice->CreateTexture(createInformation);
		B3D_TEST_ASSERT(texture != nullptr)
		if(texture != nullptr)
		{
			D3D12Image* const image = static_cast<D3D12Texture*>(texture.get())->GetD3D12Image();
			B3D_TEST_ASSERT(image != nullptr)
			if(image == nullptr)
				return;

			D3D12_HEAP_PROPERTIES heapProperties = {};
			D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE;
			B3D_TEST_ASSERT(SUCCEEDED(image->GetD3D12Resource()->GetHeapProperties(&heapProperties, &heapFlags)))
			B3D_TEST_ASSERT((heapFlags & D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES) == D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES)
		}
	};

	fnTestPlacedAllocation(PF_RGBA8, TextureUsageFlag::RenderTarget, 1);
	fnTestPlacedAllocation(PF_D32, TextureUsageFlag::DepthStencil, 1);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels = {};
	qualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	qualityLevels.SampleCount = 4;
	if(SUCCEEDED(device.GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&qualityLevels, sizeof(qualityLevels))) && qualityLevels.NumQualityLevels > 0)
	{
		fnTestPlacedAllocation(PF_RGBA8, TextureUsageFlag::RenderTarget, 4);
	}
}

void D3D12BarrierTestSuite::TestShaderReadableDepthTextures()
{
	B3D_TEST_ASSERT(D3D12Utility::GetTextureResourceFormat(DXGI_FORMAT_D16_UNORM) == DXGI_FORMAT_R16_TYPELESS)
	B3D_TEST_ASSERT(D3D12Utility::GetShaderResourceViewFormat(DXGI_FORMAT_D16_UNORM) == DXGI_FORMAT_R16_UNORM)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureResourceFormat(DXGI_FORMAT_D32_FLOAT) == DXGI_FORMAT_R32_TYPELESS)
	B3D_TEST_ASSERT(D3D12Utility::GetShaderResourceViewFormat(DXGI_FORMAT_D32_FLOAT) == DXGI_FORMAT_R32_FLOAT)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureResourceFormat(DXGI_FORMAT_D24_UNORM_S8_UINT) == DXGI_FORMAT_R24G8_TYPELESS)
	B3D_TEST_ASSERT(D3D12Utility::GetShaderResourceViewFormat(DXGI_FORMAT_D24_UNORM_S8_UINT) == DXGI_FORMAT_R24_UNORM_X8_TYPELESS)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureResourceFormat(DXGI_FORMAT_D32_FLOAT_S8X24_UINT) == DXGI_FORMAT_R32G8X24_TYPELESS)
	B3D_TEST_ASSERT(D3D12Utility::GetShaderResourceViewFormat(DXGI_FORMAT_D32_FLOAT_S8X24_UINT) == DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureResourceFormat(DXGI_FORMAT_R8G8B8A8_UNORM) == DXGI_FORMAT_R8G8B8A8_UNORM)
	B3D_TEST_ASSERT(D3D12Utility::GetShaderResourceViewFormat(DXGI_FORMAT_R8G8B8A8_UNORM) == DXGI_FORMAT_R8G8B8A8_UNORM)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureShaderResourceViewDimension(TEX_TYPE_2D, false, false, 1) == D3D12_SRV_DIMENSION_TEXTURE2D)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureShaderResourceViewDimension(TEX_TYPE_2D, false, true, 1) == D3D12_SRV_DIMENSION_TEXTURE2DARRAY)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureShaderResourceViewDimension(TEX_TYPE_2D, false, false, 4) == D3D12_SRV_DIMENSION_TEXTURE2DMS)
	B3D_TEST_ASSERT(D3D12Utility::GetTextureShaderResourceViewDimension(TEX_TYPE_2D, false, true, 4) == D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY)

	GpuDevice* const gpuDevice = GetActiveD3D12Device();
	if(gpuDevice == nullptr)
		return;

	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(*gpuDevice);
	auto fnTestDepthTexture = [this, gpuDevice, &device](PixelFormat pixelFormat, DXGI_FORMAT viewFormat, DXGI_FORMAT resourceFormat, DXGI_FORMAT shaderResourceViewFormat, u32 sampleCount, u32 arraySliceCount)
	{
		D3D12_FEATURE_DATA_FORMAT_SUPPORT viewFormatSupport = {};
		viewFormatSupport.Format = viewFormat;
		if(FAILED(device.GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &viewFormatSupport, sizeof(viewFormatSupport))) ||
			(viewFormatSupport.Support1 & (D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL)) !=
			(D3D12_FORMAT_SUPPORT1_TEXTURE2D | D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL))
		{
			return;
		}

		D3D12_FEATURE_DATA_FORMAT_SUPPORT shaderResourceViewFormatSupport = {};
		shaderResourceViewFormatSupport.Format = shaderResourceViewFormat;
		if(FAILED(device.GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &shaderResourceViewFormatSupport, sizeof(shaderResourceViewFormatSupport))) ||
			(shaderResourceViewFormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) == 0)
		{
			return;
		}

		if(sampleCount > 1)
		{
			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels = {};
			qualityLevels.Format = viewFormat;
			qualityLevels.SampleCount = sampleCount;
			if(FAILED(device.GetD3D12Device()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(qualityLevels))) || qualityLevels.NumQualityLevels == 0)
				return;
		}

		TextureCreateInformation createInformation;
		createInformation.Name = "D3D12 shader-readable depth texture test";
		createInformation.Format = pixelFormat;
		createInformation.Width = 64;
		createInformation.Height = 64;
		createInformation.ArraySliceCount = arraySliceCount;
		createInformation.SampleCount = sampleCount;
		createInformation.Usage = TextureUsageFlag::DepthStencil;

		const TShared<render::Texture> texture = gpuDevice->CreateTexture(createInformation);
		B3D_TEST_ASSERT(texture != nullptr)
		if(texture == nullptr)
			return;

		D3D12Texture* const d3d12Texture = static_cast<D3D12Texture*>(texture.get());
		D3D12Image* const image = d3d12Texture->GetD3D12Image();
		B3D_TEST_ASSERT(image != nullptr)
		if(image == nullptr)
			return;

		B3D_TEST_ASSERT(image->GetD3D12Resource()->GetDesc().Format == resourceFormat)
		B3D_TEST_ASSERT(image->GetViewFormat() == viewFormat)
	};

	fnTestDepthTexture(PF_D16, DXGI_FORMAT_D16_UNORM, DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_R16_UNORM, 1, 1);
	fnTestDepthTexture(PF_D32, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, 1, 2);
	fnTestDepthTexture(PF_D24S8, DXGI_FORMAT_D24_UNORM_S8_UINT, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS, 1, 1);
	fnTestDepthTexture(PF_D32_S8X24, DXGI_FORMAT_D32_FLOAT_S8X24_UINT, DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS, 1, 1);
	fnTestDepthTexture(PF_D32, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT, 4, 2);
}

void D3D12BarrierTestSuite::TestResolveBarrierMappings()
{
	const GpuTextureAspectFlags colorAspect = GpuTextureAspectFlag::Color;
	const D3D12TextureLayoutOptions layoutOptions;
	const D3D12TextureLayout resolveSourceLayout = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::ResolveSource, GQT_GRAPHICS, layoutOptions);
	const D3D12TextureLayout resolveDestinationLayout = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::ResolveDestination, GQT_GRAPHICS, layoutOptions);
	B3D_TEST_ASSERT(resolveSourceLayout.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_RESOLVE_SOURCE)
	B3D_TEST_ASSERT(resolveDestinationLayout.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_RESOLVE_DEST)

	const D3D12BarrierScope resolveSourceScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::Resolve,
		GpuAccessFlag::Read, GpuImageLayout::ResolveSource, colorAspect);
	const D3D12BarrierScope resolveDestinationScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::Resolve,
		GpuAccessFlag::Write, GpuImageLayout::ResolveDestination, colorAspect);
	B3D_TEST_ASSERT(resolveSourceScope.Access == D3D12_BARRIER_ACCESS_RESOLVE_SOURCE)
	B3D_TEST_ASSERT(resolveDestinationScope.Access == D3D12_BARRIER_ACCESS_RESOLVE_DEST)
	B3D_TEST_ASSERT(resolveSourceScope.Sync == D3D12_BARRIER_SYNC_RESOLVE)
	B3D_TEST_ASSERT(resolveDestinationScope.Sync == D3D12_BARRIER_SYNC_RESOLVE)
	B3D_TEST_ASSERT(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(resolveSourceLayout, colorAspect, GQT_GRAPHICS))
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(resolveSourceLayout, colorAspect, GQT_COMPUTE))
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(resolveSourceLayout, colorAspect, GQT_TRANSFER))

	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferSource, GQT_GRAPHICS,
		layoutOptions).GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_SOURCE)
	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferDestination, GQT_GRAPHICS,
		layoutOptions).GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_COPY_DEST)
	const D3D12BarrierScope transferScope = D3D12BarrierUtility::GetTextureScope(GpuStageFlag::Transfer,
		GpuAccessFlag::Read, GpuImageLayout::TransferSource, colorAspect);
	B3D_TEST_ASSERT(transferScope.Sync == D3D12_BARRIER_SYNC_COPY)
}

void D3D12BarrierTestSuite::TestTextureLayoutMappings()
{
	const GpuTextureAspectFlags colorAspect = GpuTextureAspectFlag::Color;
	const D3D12TextureLayoutOptions defaultOptions;
	D3D12TextureLayoutOptions concurrentReadOptions;
	concurrentReadOptions.AllowConcurrentQueueReads = true;

	const D3D12TextureLayout graphicsShaderRead = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::ShaderReadOnly, GQT_GRAPHICS, defaultOptions);
	const D3D12TextureLayout graphicsUnorderedAccess = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::General, GQT_GRAPHICS, defaultOptions);
	const D3D12TextureLayout computeShaderRead = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::ShaderReadOnly, GQT_COMPUTE, defaultOptions);
	const D3D12TextureLayout computeUnorderedAccess = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::General, GQT_COMPUTE, defaultOptions);
	const D3D12TextureLayout concurrentGraphicsRead = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::ShaderReadOnly, GQT_GRAPHICS, concurrentReadOptions);
	const D3D12TextureLayout concurrentComputeRead = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::ShaderReadOnly, GQT_COMPUTE, concurrentReadOptions);
	const D3D12TextureLayout concurrentGraphicsUnorderedAccess = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::General, GQT_GRAPHICS, concurrentReadOptions);

	B3D_TEST_ASSERT(graphicsShaderRead.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_SHADER_RESOURCE)
	B3D_TEST_ASSERT(graphicsUnorderedAccess.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_UNORDERED_ACCESS)
	B3D_TEST_ASSERT(computeShaderRead.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_SHADER_RESOURCE)
	B3D_TEST_ASSERT(computeUnorderedAccess.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_UNORDERED_ACCESS)
	B3D_TEST_ASSERT(concurrentGraphicsRead.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_SHADER_RESOURCE)
	B3D_TEST_ASSERT(concurrentComputeRead == concurrentGraphicsRead)
	B3D_TEST_ASSERT(concurrentGraphicsUnorderedAccess == graphicsUnorderedAccess)
	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferSource, GQT_COMPUTE,
		defaultOptions).GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_SOURCE)
	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferDestination, GQT_COMPUTE,
		defaultOptions).GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_COMPUTE_QUEUE_COPY_DEST)

	// Layouts that queues of both types access are the layouts every queue type can use
	D3D12TextureLayoutOptions severalQueueTypesOptions;
	severalQueueTypesOptions.IsAccessedOnSeveralQueueTypes = true;
	const GpuImageLayout sharedLayouts[] = { GpuImageLayout::General, GpuImageLayout::ShaderReadOnly, GpuImageLayout::TransferSource, GpuImageLayout::TransferDestination };
	const D3D12_BARRIER_LAYOUT sharedNativeLayouts[] = { D3D12_BARRIER_LAYOUT_UNORDERED_ACCESS, D3D12_BARRIER_LAYOUT_SHADER_RESOURCE, D3D12_BARRIER_LAYOUT_COPY_SOURCE,
		D3D12_BARRIER_LAYOUT_COPY_DEST };
	for(u32 layoutIndex = 0; layoutIndex < 4; layoutIndex++)
	{
		const D3D12TextureLayout graphicsLayout = D3D12BarrierUtility::TranslateTextureLayout(sharedLayouts[layoutIndex], GQT_GRAPHICS, severalQueueTypesOptions);
		const D3D12TextureLayout computeLayout = D3D12BarrierUtility::TranslateTextureLayout(sharedLayouts[layoutIndex], GQT_COMPUTE, severalQueueTypesOptions);
		B3D_TEST_ASSERT(graphicsLayout.GetLayout(colorAspect) == sharedNativeLayouts[layoutIndex])
		B3D_TEST_ASSERT(computeLayout == graphicsLayout)
		B3D_TEST_ASSERT(D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(graphicsLayout, colorAspect, GQT_GRAPHICS))
		B3D_TEST_ASSERT(D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(graphicsLayout, colorAspect, GQT_COMPUTE))
	}

	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::DepthStencilReadOnly, GQT_GRAPHICS,
		severalQueueTypesOptions).GetLayout(GpuTextureAspectFlag::Depth) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ_COMPUTE_QUEUE_ACCESSIBLE)

	B3D_TEST_ASSERT(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(graphicsShaderRead, colorAspect, GQT_GRAPHICS))
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(graphicsShaderRead, colorAspect, GQT_COMPUTE))
	B3D_TEST_ASSERT(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(computeShaderRead, colorAspect, GQT_COMPUTE))
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(computeShaderRead, colorAspect, GQT_GRAPHICS))
	B3D_TEST_ASSERT(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(concurrentGraphicsRead, colorAspect, GQT_GRAPHICS))
	B3D_TEST_ASSERT(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(concurrentComputeRead, colorAspect, GQT_COMPUTE))

	const D3D12TextureLayout depthRead = D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::DepthStencilReadOnly,
		GQT_GRAPHICS, defaultOptions);
	const D3D12TextureLayout concurrentDepthRead = D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::DepthStencilReadOnly,
		GQT_GRAPHICS, concurrentReadOptions);
	B3D_TEST_ASSERT(depthRead.GetLayout(GpuTextureAspectFlag::Depth) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ)
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(depthRead, GpuTextureAspectFlag::Depth, GQT_COMPUTE))
	B3D_TEST_ASSERT(D3D12BarrierUtility::CanTransitionTextureLayoutOnQueue(depthRead, GpuTextureAspectFlag::Depth, GQT_GRAPHICS))
	B3D_TEST_ASSERT(concurrentDepthRead.GetLayout(GpuTextureAspectFlag::Depth) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ_COMPUTE_QUEUE_ACCESSIBLE)
	B3D_TEST_ASSERT(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(concurrentDepthRead, GpuTextureAspectFlag::Depth, GQT_COMPUTE))

	const GpuTextureAspectFlags depthStencilAspects = GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil;
	const D3D12TextureLayout depthReadStencilWrite = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::DepthReadOnlyStencilAttachment, GQT_GRAPHICS, defaultOptions);
	const D3D12TextureLayout depthWriteStencilRead = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::DepthAttachmentStencilReadOnly, GQT_GRAPHICS, defaultOptions);
	B3D_TEST_ASSERT(depthReadStencilWrite.GetLayout(GpuTextureAspectFlag::Depth) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ)
	B3D_TEST_ASSERT(depthReadStencilWrite.GetLayout(GpuTextureAspectFlag::Stencil) == D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE)
	B3D_TEST_ASSERT(depthWriteStencilRead.GetLayout(GpuTextureAspectFlag::Depth) == D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE)
	B3D_TEST_ASSERT(depthWriteStencilRead.GetLayout(GpuTextureAspectFlag::Stencil) == D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ)
}

void D3D12BarrierTestSuite::TestCopyQueueLayoutMappings()
{
	const GpuTextureAspectFlags colorAspect = GpuTextureAspectFlag::Color;
	const D3D12TextureLayout commonLayout = D3D12TextureLayout::Common();
	const D3D12TextureLayoutOptions defaultOptions;
	D3D12TextureLayoutOptions presentableOptions;
	presentableOptions.IsPresentable = true;
	const D3D12TextureLayout copySourceLayout = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::TransferSource, GQT_GRAPHICS, defaultOptions);
	const D3D12TextureLayout copyDestinationLayout = D3D12BarrierUtility::TranslateTextureLayout(
		GpuImageLayout::TransferDestination, GQT_GRAPHICS, defaultOptions);

	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferSource, GQT_TRANSFER,
		defaultOptions) == commonLayout)
	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferDestination, GQT_TRANSFER,
		defaultOptions) == commonLayout)
	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferSource, GQT_GRAPHICS,
		presentableOptions) == commonLayout)
	B3D_TEST_ASSERT(D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::TransferDestination, GQT_GRAPHICS,
		presentableOptions) == commonLayout)
	B3D_TEST_ASSERT(D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(commonLayout, colorAspect, GQT_TRANSFER))
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(copySourceLayout, colorAspect, GQT_TRANSFER))
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(copyDestinationLayout, colorAspect, GQT_TRANSFER))
	B3D_TEST_ASSERT(!D3D12BarrierUtility::IsTextureLayoutSupportedOnQueue(D3D12TextureLayout::Undefined(), colorAspect, GQT_TRANSFER))
}

void D3D12BarrierTestSuite::TestQueueSharing()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr)
		return;

	struct TextureCase
	{
		const char* Name;
		PixelFormat Format;
		TextureUsageFlags Usage;
		bool IsShared;
	};

	const TextureCase textureCases[] =
	{
		{ "D3D12 sampleable texture", PF_RGBA8, TextureUsageFlag::StoreOnGPU, true },
		{ "D3D12 concurrent-read render target", PF_RGBA8, TextureUsageFlag::RenderTarget | TextureUsageFlag::AllowConcurrentQueueReads, true },
		{ "D3D12 render target", PF_RGBA8, TextureUsageFlag::RenderTarget, false },
		{ "D3D12 depth-stencil target", PF_D32, TextureUsageFlag::DepthStencil, false },
		{ "D3D12 UAV texture", PF_RGBA8, TextureUsageFlag::AllowUnorderedAccessOnTheGPU, false },
		{ "D3D12 CPU-writable texture", PF_RGBA8, TextureUsageFlag::StoreOnCPUWithGPUAccess, false },
	};

	const GpuTextureAspectFlags colorAspect = GpuTextureAspectFlag::Color;
	for(const TextureCase& textureCase : textureCases)
	{
		TextureCreateInformation createInformation;
		createInformation.Name = textureCase.Name;
		createInformation.Format = textureCase.Format;
		createInformation.Width = 8;
		createInformation.Height = 8;
		createInformation.Usage = textureCase.Usage;

		const TShared<render::Texture> texture = device->CreateTexture(createInformation);
		B3D_TEST_ASSERT(texture != nullptr)
		if(texture == nullptr)
			continue;

		D3D12Image* const image = std::static_pointer_cast<D3D12Texture>(texture)->GetD3D12Image();
		B3D_TEST_ASSERT(image != nullptr)
		if(image == nullptr)
			continue;

		// Shared textures use queue-independent layouts, not simultaneous access
		B3D_TEST_ASSERT((image->GetD3D12Resource()->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS) == 0)
		B3D_TEST_ASSERT(image->AllowsConcurrentQueueReads() == textureCase.IsShared)
		if(!textureCase.IsShared)
			continue;

		D3D12TextureLayoutOptions options;
		options.AllowConcurrentQueueReads = image->AllowsConcurrentQueueReads();
		options.IsPresentable = image->IsPresentable();

		const D3D12TextureLayout graphicsRead = D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::ShaderReadOnly, GQT_GRAPHICS, options);
		const D3D12TextureLayout computeRead = D3D12BarrierUtility::TranslateTextureLayout(GpuImageLayout::ShaderReadOnly, GQT_COMPUTE, options);
		B3D_TEST_ASSERT(graphicsRead.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_SHADER_RESOURCE)
		B3D_TEST_ASSERT(computeRead.GetLayout(colorAspect) == D3D12_BARRIER_LAYOUT_SHADER_RESOURCE)
	}
}

void D3D12BarrierTestSuite::TestRestingTransitionSubmission()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0)
		return;

	static constexpr u32 kSize = 16;
	static constexpr u32 kPattern = 0x5A000000;

	TextureCreateInformation createInformation;
	createInformation.Name = "D3D12 resting transition test";
	createInformation.Format = PF_RGBA8;
	createInformation.Width = kSize;
	createInformation.Height = kSize;
	createInformation.Usage = TextureUsageFlag::RenderTarget;
	const TShared<render::Texture> texture = device->CreateTexture(createInformation);
	B3D_TEST_ASSERT(texture != nullptr)
	if(texture == nullptr)
		return;

	const ImageSubresourcePitch pitch = texture->GetStagingBufferPitchForSubresource(0, 0);
	const u32 stagingSize = pitch.RowPitch * pitch.SliceHeight * sizeof(u32);
	const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingSize));
	const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingSize));
	{
		const render::GpuBufferMappedScope mapping = upload->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
		for(u32 elementIndex = 0; elementIndex < pitch.RowPitch * pitch.SliceHeight; elementIndex++)
			elements[elementIndex] = kPattern | elementIndex;
	}

	// The new render target requests a transition to rest, which the submission prologue records. The first barrier starts from the
	// resting state and discards the contents.
	const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
	const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read, GpuImageLayout::TransferSource);
	const TShared<GpuCommandBufferPool> commandBufferPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
	const TShared<GpuCommandBuffer> commandBuffer = commandBufferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 resting transition test"));
	commandBuffer->IssueBarriers(GpuTextureBarrier(texture, GpuBarrierTestUtility::GetDiscardedRestingState(*texture), transferWrite));
	commandBuffer->CopyBufferToTexture(upload, texture, 0, 0, 0);

	const GpuBufferBarrier readbackBarrier(readback, GpuAccessState(), transferWrite);
	const GpuTextureBarrier copyBarrier(texture, transferWrite, transferRead);
	commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&readbackBarrier, 1), TArrayView<const GpuTextureBarrier>(&copyBarrier, 1)));
	commandBuffer->CopyTextureToBuffer(texture, readback, 0, 0, 0);

	// The texture returns to rest, and the copy becomes visible to the host
	const GpuBufferBarrier hostReadBarrier(readback, transferWrite, GpuAccessState(GpuStageFlag::Host, GpuAccessFlag::Read));
	const GpuTextureBarrier restBarrier(texture, transferRead, GpuBackendUtility::GetRestingState(*texture, GQT_GRAPHICS));
	commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(&hostReadBarrier, 1), TArrayView<const GpuTextureBarrier>(&restBarrier, 1)));

	const TShared<GpuWorkContext> workContext = GpuWorkContext::Create(*device);
	workContext->SubmitCommandBuffer(commandBuffer, GpuQueueMask::kNone);
	device->WaitUntilIdle();

	const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
	B3D_TEST_ASSERT(mapping.IsValid())
	if(!mapping.IsValid())
		return;

	const u32* elements = static_cast<const u32*>(mapping.GetMappedMemory());
	bool matches = true;
	for(u32 row = 0; row < kSize; row++)
	{
		for(u32 column = 0; column < kSize; column++)
		{
			const u32 elementIndex = row * pitch.RowPitch + column;
			matches &= elements[elementIndex] == (kPattern | elementIndex);
		}
	}

	B3D_TEST_ASSERT_MSG(matches, "The readback does not hold the written values.")
}

void D3D12BarrierTestSuite::TestMultisampleResolve()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0)
		return;

	TShared<render::Texture> source;
	TShared<render::Texture> destination;
	CreateResolveTextures(*device, source, destination);
	B3D_TEST_ASSERT(source != nullptr)
	B3D_TEST_ASSERT(destination != nullptr)
	if(source == nullptr || destination == nullptr)
		return;

	const TShared<GpuCommandBufferPool> commandBufferPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
	const TShared<GpuCommandBuffer> commandBuffer = commandBufferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 MSAA resolve test"));

	// The new textures start at rest. The resolve overwrites the destination, and reads the undefined contents of the source.
	const GpuTextureBarrier resolveBarriers[] =
	{
		GpuTextureBarrier(source, GpuBarrierTestUtility::GetDiscardedRestingState(*source), GpuAccessState(GpuStageFlag::Resolve, GpuAccessFlag::Read, GpuImageLayout::ResolveSource)),
		GpuTextureBarrier(destination, GpuBarrierTestUtility::GetDiscardedRestingState(*destination), GpuAccessState(GpuStageFlag::Resolve, GpuAccessFlag::Write, GpuImageLayout::ResolveDestination))
	};
	commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(), TArrayView<const GpuTextureBarrier>(resolveBarriers, 2)));
	B3D_TEST_ASSERT(commandBuffer->CopyTexture(source, destination))

	const TShared<GpuWorkContext> workContext = GpuWorkContext::Create(*device);
	workContext->SubmitCommandBuffer(commandBuffer, GpuQueueMask::kNone);
	device->WaitUntilIdle();
}

void D3D12BarrierTestSuite::TestResolveValidation()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0)
		return;

	TShared<render::Texture> source;
	TShared<render::Texture> destination;
	CreateResolveTextures(*device, source, destination);
	B3D_TEST_ASSERT(source != nullptr)
	B3D_TEST_ASSERT(destination != nullptr)
	if(source == nullptr || destination == nullptr)
		return;

	const TShared<GpuCommandBufferPool> graphicsPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_GRAPHICS);
	const TShared<GpuCommandBuffer> partialResolveCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 partial resolve validation test"));
	TextureCopyInformation partialCopyInformation;
	partialCopyInformation.SourceVolume = PixelVolume(0, 0, 0, 8, 16, 1);
	{
		LoggingScope loggingScope(*this);
		loggingScope.ExpectError("D3D12 texture resolves must cover the entire source subresource and begin at the destination origin.");
		B3D_TEST_ASSERT(!partialResolveCommandBuffer->CopyTexture(source, destination, partialCopyInformation))
	}

	if(device->GetQueueCount(GQT_TRANSFER) > 0)
	{
		const TShared<GpuCommandBufferPool> transferPool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, GQT_TRANSFER);
		const TShared<GpuCommandBuffer> transferResolveCommandBuffer = transferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 transfer queue resolve validation test"));
		LoggingScope loggingScope(*this);
		loggingScope.ExpectError("D3D12 texture resolves require a graphics command buffer.");
		B3D_TEST_ASSERT(!transferResolveCommandBuffer->CopyTexture(source, destination))
	}
}

void D3D12BarrierTestSuite::TestFrameFence()
{
	GetRenderThread().PostCommand([this]()
	{
		GpuDevice* const device = GetActiveD3D12Device();
		if(device == nullptr)
			return;

		B3D_TEST_ASSERT(device->GetQueueCount(GQT_COMPUTE) > 0)

		const GpuQueue& graphicsQueue = *device->GetQueue(GQT_GRAPHICS, 0);
		const GpuQueue& computeQueue = *device->GetQueue(GQT_COMPUTE, 0);
		const GpuSubmitThread& submitThread = device->GetSubmitThread();

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const auto fnSubmit = [device, &context](GpuQueueType queueType)
		{
			const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device, queueType);
			context->SubmitCommandBuffer(pool->Create(GpuCommandBufferCreateInformation::Create("D3D12 frame boundary test")), GpuQueueMask::kNone);
			device->WaitUntilIdle();
		};

		// Give both queues work in the ending frame, so the frame fence has non-zero values to wait on
		fnSubmit(GQT_GRAPHICS);
		fnSubmit(GQT_COMPUTE);

		device->EndFrame();
		device->WaitUntilIdle();
		B3D_TEST_ASSERT(submitThread.IsFrameFencePending(graphicsQueue))
		B3D_TEST_ASSERT(submitThread.IsFrameFencePending(computeQueue))

		// The first submission on a queue consumes the frame fence, and later submissions don't re-arm it
		fnSubmit(GQT_GRAPHICS);
		B3D_TEST_ASSERT(!submitThread.IsFrameFencePending(graphicsQueue))
		B3D_TEST_ASSERT(submitThread.IsFrameFencePending(computeQueue))

		fnSubmit(GQT_GRAPHICS);
		B3D_TEST_ASSERT(!submitThread.IsFrameFencePending(graphicsQueue))

		fnSubmit(GQT_COMPUTE);
		B3D_TEST_ASSERT(!submitThread.IsFrameFencePending(computeQueue))
	}, "D3D12BarrierTestSuite::TestFrameFence", true);
}
