//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierTestSuite.h"
#include "B3DD3D12BarrierUtility.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12Texture.h"
#include "Utility/B3DD3D12BarrierBatch.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuWorkContext.h"
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
		GpuStageFlags precedingBarrierDestinationStages = GpuStageFlag::None, D3D12_TEXTURE_BARRIER_FLAGS flags = D3D12_TEXTURE_BARRIER_FLAG_NONE)
	{
		batch.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(resource, range, scope, logicalBeforeLayout,
			logicalAfterLayout, nativeBeforeLayout, nativeAfterLayout, flags, precedingBarrierDestinationStages));
	}
}

D3D12BarrierTestSuite::D3D12BarrierTestSuite() : TestSuite("D3D12BarrierTestSuite")
{
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestBufferBarrierScopes)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestTextureBarrierScopes)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestTextureBarrierBatch)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestPlacedRenderTargetAllocation)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestResolveBarrierMappings)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestTextureLayoutMappings)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestCopyQueueLayoutMappings)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestConcurrentQueueReadTexture)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestCrossQueueTextureHandoffs)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestCrossQueueBufferHandoffs)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestSubmissionBarrierChaining)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestSameQueueSubmissionHazards)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestMultisampleResolve)
	B3D_ADD_TEST(D3D12BarrierTestSuite::TestResolveValidation)
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

	D3D12BarrierBatch chainedBatch;
	AddTextureBarrier(chainedBatch, resource, fullRange, unorderedAccessBarrier, GpuImageLayout::ShaderReadOnly,
		GpuImageLayout::General, shaderLayout, unorderedAccessLayout, shaderReadBarrier.DestinationStages);
	B3D_TEST_ASSERT(chainedBatch.mTextureBarriers.Size() == 1)
	B3D_TEST_ASSERT(chainedBatch.mTextureBarriers[0].SyncBefore ==
		(D3D12_BARRIER_SYNC_VERTEX_SHADING | D3D12_BARRIER_SYNC_PIXEL_SHADING))
	B3D_TEST_ASSERT(chainedBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_SHADER_RESOURCE)

	const GpuBarrierScope depthReadBarrier(GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests,
		GpuAccessFlag::Read, GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests, GpuAccessFlag::Write);
	D3D12BarrierBatch chainedDepthBatch;
	AddTextureBarrier(chainedDepthBatch, resource, fullRange, depthReadBarrier, GpuImageLayout::DepthStencilReadOnly,
		GpuImageLayout::DepthStencilAttachment, D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ),
		D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE), shaderReadBarrier.DestinationStages);
	B3D_TEST_ASSERT(chainedDepthBatch.mTextureBarriers.Size() == 1)
	B3D_TEST_ASSERT(chainedDepthBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_DRAW)
	B3D_TEST_ASSERT(chainedDepthBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ)

	const GpuBarrierScope depthWriteToComputeRead(GpuStageFlag::EarlyFragmentTests | GpuStageFlag::LateFragmentTests,
		GpuAccessFlag::Write, GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Read);
	D3D12BarrierBatch chainedShaderReadBatch;
	AddTextureBarrier(chainedShaderReadBatch, resource, fullRange, depthWriteToComputeRead, GpuImageLayout::ShaderReadOnly,
		GpuImageLayout::ShaderReadOnly, shaderLayout, shaderLayout, GpuStageFlag::FragmentShaderNonUniform);
	B3D_TEST_ASSERT(chainedShaderReadBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_PIXEL_SHADING)
	B3D_TEST_ASSERT(chainedShaderReadBatch.mTextureBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_SHADER_RESOURCE)
	B3D_TEST_ASSERT(chainedShaderReadBatch.mTextureBarriers[0].SyncAfter == D3D12_BARRIER_SYNC_COMPUTE_SHADING)

	D3D12BarrierBatch copiedDepthBatch;
	AddTextureBarrier(copiedDepthBatch, resource, fullRange, depthReadBarrier, GpuImageLayout::DepthStencilReadOnly,
		GpuImageLayout::DepthStencilAttachment, D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DIRECT_QUEUE_GENERIC_READ),
		D3D12TextureLayout(D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE), GpuStageFlag::Transfer);
	B3D_TEST_ASSERT(copiedDepthBatch.mTextureBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_ALL)

	const GpuBarrierScope vertexBufferBarrier(GpuStageFlag::VertexInputAttributes, GpuAccessFlag::Read,
		GpuStageFlag::VertexInputAttributes, GpuAccessFlag::Read);
	D3D12BarrierBatch chainedVertexBufferBatch;
	chainedVertexBufferBatch.AddBufferBarrier(D3D12BarrierUtility::GetBufferBarrier(resource, vertexBufferBarrier,
		GpuStageFlag::ComputeShaderNonUniform));
	B3D_TEST_ASSERT(chainedVertexBufferBatch.mBufferBarriers[0].SyncBefore == D3D12_BARRIER_SYNC_ALL_SHADING)
	B3D_TEST_ASSERT(chainedVertexBufferBatch.mBufferBarriers[0].AccessBefore == D3D12_BARRIER_ACCESS_VERTEX_BUFFER)

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
		D3D12TextureLayout::Undefined(), D3D12TextureLayout(D3D12_BARRIER_LAYOUT_RENDER_TARGET), GpuStageFlag::None, D3D12_TEXTURE_BARRIER_FLAG_DISCARD);
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

	const GpuResourceUseFlags colorAttachmentUsage = GpuResourceUseFlag::ColorAttachment;
	const GpuResourceUseFlags fragmentShaderUsage = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
	GpuBarriers sequentialBarriers;
	sequentialBarriers.TextureBarriers.Add(GpuTextureBarrier(texture, colorAttachmentUsage, GpuAccessFlag::Write, GpuImageLayout::ColorAttachment, topMip));
	sequentialBarriers.TextureBarriers.Add(GpuTextureBarrier(texture, fragmentShaderUsage, GpuAccessFlag::Read, GpuImageLayout::ShaderReadOnly, topMip));

	const TShared<GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	const TShared<GpuCommandBuffer> commandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 sequential texture barrier validation"));
	commandBuffer->IssueBarriers(sequentialBarriers);
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

void D3D12BarrierTestSuite::TestConcurrentQueueReadTexture()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr)
		return;

	TextureCreateInformation createInformation;
	createInformation.Name = "D3D12 concurrent-read texture";
	createInformation.Format = PF_RGBA8;
	createInformation.Width = 8;
	createInformation.Height = 8;
	createInformation.Usage |= TextureUsageFlag::AllowConcurrentQueueReads;

	const TShared<render::Texture> texture = device->CreateTexture(createInformation);
	B3D_TEST_ASSERT(texture != nullptr)
	if(texture == nullptr)
		return;

	const TShared<D3D12Texture> d3d12Texture = std::static_pointer_cast<D3D12Texture>(texture);
	D3D12Image* const image = d3d12Texture->GetD3D12Image();
	B3D_TEST_ASSERT(image != nullptr)
	if(image == nullptr)
		return;

	B3D_TEST_ASSERT((image->GetD3D12Resource()->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS) == 0)
	B3D_TEST_ASSERT(image->AllowsConcurrentQueueReads())
}

void D3D12BarrierTestSuite::TestCrossQueueTextureHandoffs()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0 || device->GetQueueCount(GQT_COMPUTE) == 0 || device->GetQueueCount(GQT_TRANSFER) == 0)
		return;

	constexpr u32 kWidth = 8;
	constexpr u32 kHeight = 8;
	constexpr u32 kBytesPerPixel = 4;
	TextureCreateInformation textureCreateInformation;
	textureCreateInformation.Name = "D3D12 cross queue handoff texture";
	textureCreateInformation.Format = PF_RGBA8;
	textureCreateInformation.Width = kWidth;
	textureCreateInformation.Height = kHeight;

	constexpr u32 kTextureCount = 7;
	TInlineArray<TShared<render::Texture>, kTextureCount> textures;
	for(u32 textureIndex = 0; textureIndex < kTextureCount; ++textureIndex)
		textures.Add(device->CreateTexture(textureCreateInformation));

	for(const TShared<render::Texture>& texture : textures)
		B3D_TEST_ASSERT(texture != nullptr)
	if(textures.Size() != kTextureCount || std::any_of(textures.begin(), textures.end(), [](const TShared<render::Texture>& texture) { return texture == nullptr; }))
		return;

	const ImageSubresourcePitch stagingPitch = textures[0]->GetStagingBufferPitchForSubresource(0, 0);
	const u32 rowDataSize = kWidth * kBytesPerPixel;
	const u32 rowPitchSize = stagingPitch.RowPitch * kBytesPerPixel;
	const u32 stagingBufferSize = rowPitchSize * stagingPitch.SliceHeight;
	const TShared<render::GpuBuffer> uploadBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingBufferSize));
	const TShared<render::GpuBuffer> readbackBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(stagingBufferSize));
	B3D_TEST_ASSERT(uploadBuffer != nullptr)
	B3D_TEST_ASSERT(readbackBuffer != nullptr)
	if(uploadBuffer == nullptr || readbackBuffer == nullptr)
		return;

	{
		render::GpuBufferMappedScope uploadMapping = uploadBuffer->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(uploadMapping.IsValid())
		if(!uploadMapping.IsValid())
			return;

		u8* const mappedMemory = static_cast<u8*>(uploadMapping.GetMappedMemory());
		memset(mappedMemory, 0, stagingBufferSize);
		for(u32 row = 0; row < kHeight; ++row)
		{
			for(u32 byteIndex = 0; byteIndex < rowDataSize; ++byteIndex)
				mappedMemory[row * rowPitchSize + byteIndex] = (u8)((row * rowDataSize + byteIndex) ^ 0xA5);
		}
	}

	const TShared<GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	const TShared<GpuCommandBufferPool> computePool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_COMPUTE));
	const TShared<GpuCommandBufferPool> transferPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_TRANSFER));

	const TShared<GpuCommandBuffer> uploadCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 cross queue handoff upload"));
	uploadCommandBuffer->CopyBufferToTexture(uploadBuffer, textures[0], 0, 0, 0);
	uploadCommandBuffer->End();

	const TShared<GpuCommandBuffer> graphicsToComputeCommandBuffer = computePool->Create(GpuCommandBufferCreateInformation::Create("D3D12 graphics to compute texture handoff"));
	B3D_TEST_ASSERT(graphicsToComputeCommandBuffer->CopyTexture(textures[0], textures[1]))
	graphicsToComputeCommandBuffer->End();

	const TShared<GpuCommandBuffer> computeToGraphicsCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 compute to graphics texture handoff"));
	B3D_TEST_ASSERT(computeToGraphicsCommandBuffer->CopyTexture(textures[1], textures[2]))
	computeToGraphicsCommandBuffer->End();

	const TShared<GpuCommandBuffer> graphicsToTransferCommandBuffer = transferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 graphics to copy texture handoff"));
	B3D_TEST_ASSERT(graphicsToTransferCommandBuffer->CopyTexture(textures[2], textures[3]))
	B3D_TEST_ASSERT(graphicsToTransferCommandBuffer->CopyTexture(textures[3], textures[4]))
	graphicsToTransferCommandBuffer->End();

	const TShared<GpuCommandBuffer> transferToComputeCommandBuffer = computePool->Create(GpuCommandBufferCreateInformation::Create("D3D12 copy to compute texture handoff"));
	B3D_TEST_ASSERT(transferToComputeCommandBuffer->CopyTexture(textures[4], textures[5]))
	transferToComputeCommandBuffer->End();

	const TShared<GpuCommandBuffer> computeToTransferCommandBuffer = transferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 compute to copy texture handoff"));
	B3D_TEST_ASSERT(computeToTransferCommandBuffer->CopyTexture(textures[5], textures[6]))
	computeToTransferCommandBuffer->End();

	const TShared<GpuCommandBuffer> readbackCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 cross queue handoff readback"));
	readbackCommandBuffer->CopyTextureToBuffer(textures[6], readbackBuffer, 0, 0, 0);
	readbackCommandBuffer->End();

	const TShared<GpuWorkContext> workContext = GpuWorkContext::Create(*device);
	workContext->SubmitCommandBuffer(uploadCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(graphicsToComputeCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(computeToGraphicsCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(graphicsToTransferCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(transferToComputeCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(computeToTransferCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(readbackCommandBuffer, GpuQueueMask::kNone);
	device->WaitUntilIdle();

	render::GpuBufferMappedScope readbackMapping = readbackBuffer->Map(GpuMapOption::Read);
	B3D_TEST_ASSERT(readbackMapping.IsValid())
	if(!readbackMapping.IsValid())
		return;

	const u8* const mappedMemory = static_cast<const u8*>(readbackMapping.GetMappedMemory());
	for(u32 row = 0; row < kHeight; ++row)
	{
		for(u32 byteIndex = 0; byteIndex < rowDataSize; ++byteIndex)
			B3D_TEST_ASSERT(mappedMemory[row * rowPitchSize + byteIndex] == (u8)((row * rowDataSize + byteIndex) ^ 0xA5))
	}
}

void D3D12BarrierTestSuite::TestCrossQueueBufferHandoffs()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0 || device->GetQueueCount(GQT_COMPUTE) == 0 || device->GetQueueCount(GQT_TRANSFER) == 0)
		return;

	constexpr u32 kBufferSize = 256;
	constexpr u32 kUploadBufferCount = 3;
	TInlineArray<TShared<render::GpuBuffer>, kUploadBufferCount> uploadBuffers;
	for(u32 uploadBufferIndex = 0; uploadBufferIndex < kUploadBufferCount; ++uploadBufferIndex)
		uploadBuffers.Add(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(kBufferSize)));

	const GpuBufferFlags storageFlags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	const TShared<render::GpuBuffer> storageBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, kBufferSize / 4, storageFlags));
	const TShared<render::GpuBuffer> readbackBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kBufferSize));

	for(const TShared<render::GpuBuffer>& uploadBuffer : uploadBuffers)
		B3D_TEST_ASSERT(uploadBuffer != nullptr)
	B3D_TEST_ASSERT(storageBuffer != nullptr)
	B3D_TEST_ASSERT(readbackBuffer != nullptr)
	if(uploadBuffers.Size() != kUploadBufferCount || std::any_of(uploadBuffers.begin(), uploadBuffers.end(), [](const TShared<render::GpuBuffer>& uploadBuffer) { return uploadBuffer == nullptr; }) || storageBuffer == nullptr || readbackBuffer == nullptr)
		return;

	for(u32 uploadBufferIndex = 0; uploadBufferIndex < kUploadBufferCount; ++uploadBufferIndex)
	{
		render::GpuBufferMappedScope uploadMapping = uploadBuffers[uploadBufferIndex]->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(uploadMapping.IsValid())
		if(!uploadMapping.IsValid())
			return;

		const u8 pattern = (u8)(0x11 + uploadBufferIndex * 0x33);
		u8* const mappedMemory = static_cast<u8*>(uploadMapping.GetMappedMemory());
		for(u32 byteIndex = 0; byteIndex < kBufferSize; ++byteIndex)
			mappedMemory[byteIndex] = (u8)(byteIndex ^ pattern);
	}

	const TShared<GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	const TShared<GpuCommandBufferPool> computePool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_COMPUTE));
	const TShared<GpuCommandBufferPool> transferPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_TRANSFER));

	const TShared<GpuCommandBuffer> graphicsWriteCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 graphics buffer handoff write"));
	graphicsWriteCommandBuffer->CopyBufferToBuffer(uploadBuffers[0], storageBuffer, 0, 0, kBufferSize);
	graphicsWriteCommandBuffer->End();

	const TShared<GpuCommandBuffer> computeWriteCommandBuffer = computePool->Create(GpuCommandBufferCreateInformation::Create("D3D12 compute buffer handoff write"));
	computeWriteCommandBuffer->CopyBufferToBuffer(uploadBuffers[1], storageBuffer, 0, 0, kBufferSize);
	computeWriteCommandBuffer->End();

	const TShared<GpuCommandBuffer> transferWriteCommandBuffer = transferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 copy buffer handoff write"));
	transferWriteCommandBuffer->CopyBufferToBuffer(uploadBuffers[2], storageBuffer, 0, 0, kBufferSize);
	transferWriteCommandBuffer->End();

	const TShared<GpuCommandBuffer> readbackCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 buffer handoff readback"));
	readbackCommandBuffer->CopyBufferToBuffer(storageBuffer, readbackBuffer, 0, 0, kBufferSize);
	readbackCommandBuffer->End();

	const TShared<GpuWorkContext> workContext = GpuWorkContext::Create(*device);
	workContext->SubmitCommandBuffer(graphicsWriteCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(computeWriteCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(transferWriteCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(readbackCommandBuffer, GpuQueueMask::kNone);
	device->WaitUntilIdle();

	render::GpuBufferMappedScope readbackMapping = readbackBuffer->Map(GpuMapOption::Read);
	B3D_TEST_ASSERT(readbackMapping.IsValid())
	if(!readbackMapping.IsValid())
		return;

	constexpr u8 kFinalPattern = 0x77;
	const u8* const mappedMemory = static_cast<const u8*>(readbackMapping.GetMappedMemory());
	for(u32 byteIndex = 0; byteIndex < kBufferSize; ++byteIndex)
		B3D_TEST_ASSERT(mappedMemory[byteIndex] == (u8)(byteIndex ^ kFinalPattern))
}

void D3D12BarrierTestSuite::TestSubmissionBarrierChaining()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0)
		return;

	constexpr u32 kBufferSize = 256;
	const TShared<render::GpuBuffer> uploadBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(kBufferSize));
	const GpuBufferFlags storageFlags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	const TShared<render::GpuBuffer> storageBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, kBufferSize / 4, storageFlags));
	const TShared<render::GpuBuffer> readbackBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kBufferSize));
	B3D_TEST_ASSERT(uploadBuffer != nullptr)
	B3D_TEST_ASSERT(storageBuffer != nullptr)
	B3D_TEST_ASSERT(readbackBuffer != nullptr)
	if(uploadBuffer == nullptr || storageBuffer == nullptr || readbackBuffer == nullptr)
		return;

	{
		render::GpuBufferMappedScope uploadMapping = uploadBuffer->Map(GpuMapOption::Write);
		B3D_TEST_ASSERT(uploadMapping.IsValid())
		if(!uploadMapping.IsValid())
			return;

		u8* const mappedMemory = static_cast<u8*>(uploadMapping.GetMappedMemory());
		for(u32 byteIndex = 0; byteIndex < kBufferSize; ++byteIndex)
			mappedMemory[byteIndex] = (u8)(byteIndex ^ 0x5A);
	}

	const TShared<GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	const TShared<GpuCommandBuffer> bufferWriteCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 chaining buffer write"));
	bufferWriteCommandBuffer->CopyBufferToBuffer(uploadBuffer, storageBuffer, 0, 0, kBufferSize);
	bufferWriteCommandBuffer->End();

	const TShared<GpuCommandBuffer> leadingBarrierCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 chaining leading barrier"));
	const GpuResourceUseFlags fragmentShaderUse = GpuResourceUseFlag::ShaderAccess | GpuResourceUseFlag::StageFragmentShader;
	leadingBarrierCommandBuffer->IssueBarriers(GpuBarriers(GpuBufferBarrier(storageBuffer, fragmentShaderUse, GpuAccessFlag::Read)));
	leadingBarrierCommandBuffer->CopyBufferToBuffer(storageBuffer, readbackBuffer, 0, 0, kBufferSize);
	leadingBarrierCommandBuffer->End();

	constexpr u32 kTextureSize = 8;
	TextureCreateInformation textureCreateInformation;
	textureCreateInformation.Name = "D3D12 chaining texture";
	textureCreateInformation.Format = PF_RGBA8;
	textureCreateInformation.Width = kTextureSize;
	textureCreateInformation.Height = kTextureSize;
	const TShared<render::Texture> firstTexture = device->CreateTexture(textureCreateInformation);
	const TShared<render::Texture> secondTexture = device->CreateTexture(textureCreateInformation);
	const TShared<render::Texture> destinationTexture = device->CreateTexture(textureCreateInformation);
	B3D_TEST_ASSERT(firstTexture != nullptr)
	B3D_TEST_ASSERT(secondTexture != nullptr)
	B3D_TEST_ASSERT(destinationTexture != nullptr)
	if(firstTexture == nullptr || secondTexture == nullptr || destinationTexture == nullptr)
		return;

	const ImageSubresourcePitch texturePitch = firstTexture->GetStagingBufferPitchForSubresource(0, 0);
	const u32 textureUploadSize = texturePitch.RowPitch * texturePitch.SliceHeight * 4;
	const TShared<render::GpuBuffer> textureUploadBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(textureUploadSize));
	B3D_TEST_ASSERT(textureUploadBuffer != nullptr)
	if(textureUploadBuffer == nullptr)
		return;

	const TShared<GpuCommandBuffer> textureWriteCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 chaining texture writes"));
	textureWriteCommandBuffer->CopyBufferToTexture(textureUploadBuffer, firstTexture, 0, 0, 0);
	textureWriteCommandBuffer->CopyBufferToTexture(textureUploadBuffer, secondTexture, 0, 0, 0);
	textureWriteCommandBuffer->End();

	const TShared<GpuCommandBuffer> textureChainCommandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 chaining texture reads and writes"));
	textureChainCommandBuffer->IssueBarriers(GpuBarriers(GpuTextureBarrier(firstTexture, GpuResourceUseFlag::Transfer,
		GpuAccessFlag::Read, GpuImageLayout::TransferSource)));
	B3D_TEST_ASSERT(textureChainCommandBuffer->CopyTexture(firstTexture, destinationTexture))
	B3D_TEST_ASSERT(textureChainCommandBuffer->CopyTexture(secondTexture, firstTexture))
	textureChainCommandBuffer->End();

	const TShared<GpuWorkContext> workContext = GpuWorkContext::Create(*device);
	workContext->SubmitCommandBuffer(bufferWriteCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(leadingBarrierCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(textureWriteCommandBuffer, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(textureChainCommandBuffer, GpuQueueMask::kNone);
	device->WaitUntilIdle();

	render::GpuBufferMappedScope readbackMapping = readbackBuffer->Map(GpuMapOption::Read);
	B3D_TEST_ASSERT(readbackMapping.IsValid())
	if(!readbackMapping.IsValid())
		return;

	const u8* const mappedMemory = static_cast<const u8*>(readbackMapping.GetMappedMemory());
	for(u32 byteIndex = 0; byteIndex < kBufferSize; ++byteIndex)
		B3D_TEST_ASSERT(mappedMemory[byteIndex] == (u8)(byteIndex ^ 0x5A))
}

void D3D12BarrierTestSuite::TestSameQueueSubmissionHazards()
{
	GpuDevice* const device = GetActiveD3D12Device();
	if(device == nullptr || device->GetQueueCount(GQT_GRAPHICS) == 0)
		return;

	constexpr u32 kBufferSize = 256;
	constexpr u8 kFirstPattern = 0x19;
	constexpr u8 kIntermediatePattern = 0x63;
	constexpr u8 kFinalPattern = 0xA7;
	const TShared<render::GpuBuffer> firstUploadBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(kBufferSize));
	const TShared<render::GpuBuffer> intermediateUploadBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(kBufferSize));
	const TShared<render::GpuBuffer> finalUploadBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(kBufferSize));
	const GpuBufferFlags storageFlags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU;
	const TShared<render::GpuBuffer> storageBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(4, kBufferSize / 4, storageFlags));
	const TShared<render::GpuBuffer> firstReadbackBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kBufferSize));
	const TShared<render::GpuBuffer> finalReadbackBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kBufferSize));
	B3D_TEST_ASSERT(firstUploadBuffer != nullptr)
	B3D_TEST_ASSERT(intermediateUploadBuffer != nullptr)
	B3D_TEST_ASSERT(finalUploadBuffer != nullptr)
	B3D_TEST_ASSERT(storageBuffer != nullptr)
	B3D_TEST_ASSERT(firstReadbackBuffer != nullptr)
	B3D_TEST_ASSERT(finalReadbackBuffer != nullptr)
	if(firstUploadBuffer == nullptr || intermediateUploadBuffer == nullptr || finalUploadBuffer == nullptr || storageBuffer == nullptr || firstReadbackBuffer == nullptr || finalReadbackBuffer == nullptr)
		return;

	auto fnFillUploadBuffer = [kBufferSize](const TShared<render::GpuBuffer>& buffer, u8 pattern)
	{
		render::GpuBufferMappedScope mapping = buffer->Map(GpuMapOption::Write);
		if(!mapping.IsValid())
			return false;

		u8* const mappedMemory = static_cast<u8*>(mapping.GetMappedMemory());
		for(u32 byteIndex = 0; byteIndex < kBufferSize; ++byteIndex)
			mappedMemory[byteIndex] = (u8)(byteIndex ^ pattern);

		return true;
	};
	const bool firstUploadFilled = fnFillUploadBuffer(firstUploadBuffer, kFirstPattern);
	const bool intermediateUploadFilled = fnFillUploadBuffer(intermediateUploadBuffer, kIntermediatePattern);
	const bool finalUploadFilled = fnFillUploadBuffer(finalUploadBuffer, kFinalPattern);
	B3D_TEST_ASSERT(firstUploadFilled)
	B3D_TEST_ASSERT(intermediateUploadFilled)
	B3D_TEST_ASSERT(finalUploadFilled)
	if(!firstUploadFilled || !intermediateUploadFilled || !finalUploadFilled)
		return;

	const TShared<GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	auto fnCreateCopyCommandBuffer = [&graphicsPool, kBufferSize](const StringView& name, const TShared<render::GpuBuffer>& source, const TShared<render::GpuBuffer>& destination)
	{
		const TShared<GpuCommandBuffer> commandBuffer = graphicsPool->Create(GpuCommandBufferCreateInformation::Create(name));
		commandBuffer->CopyBufferToBuffer(source, destination, 0, 0, kBufferSize);
		commandBuffer->End();
		return commandBuffer;
	};

	const TShared<GpuCommandBuffer> firstWrite = fnCreateCopyCommandBuffer("D3D12 same-queue first write", firstUploadBuffer, storageBuffer);
	const TShared<GpuCommandBuffer> firstRead = fnCreateCopyCommandBuffer("D3D12 same-queue first read", storageBuffer, firstReadbackBuffer);
	const TShared<GpuCommandBuffer> secondWrite = fnCreateCopyCommandBuffer("D3D12 same-queue second write", intermediateUploadBuffer, storageBuffer);
	const TShared<GpuCommandBuffer> thirdWrite = fnCreateCopyCommandBuffer("D3D12 same-queue third write", finalUploadBuffer, storageBuffer);
	const TShared<GpuCommandBuffer> finalRead = fnCreateCopyCommandBuffer("D3D12 same-queue final read", storageBuffer, finalReadbackBuffer);

	const TShared<GpuWorkContext> workContext = GpuWorkContext::Create(*device);
	workContext->SubmitCommandBuffer(firstWrite, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(firstRead, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(secondWrite, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(thirdWrite, GpuQueueMask::kNone);
	workContext->SubmitCommandBuffer(finalRead, GpuQueueMask::kNone);
	device->WaitUntilIdle();

	auto fnVerifyReadbackBuffer = [kBufferSize](const TShared<render::GpuBuffer>& buffer, u8 pattern)
	{
		render::GpuBufferMappedScope mapping = buffer->Map(GpuMapOption::Read);
		if(!mapping.IsValid())
			return false;

		const u8* const mappedMemory = static_cast<const u8*>(mapping.GetMappedMemory());
		for(u32 byteIndex = 0; byteIndex < kBufferSize; ++byteIndex)
		{
			if(mappedMemory[byteIndex] != (u8)(byteIndex ^ pattern))
				return false;
		}

		return true;
	};
	B3D_TEST_ASSERT(fnVerifyReadbackBuffer(firstReadbackBuffer, kFirstPattern))
	B3D_TEST_ASSERT(fnVerifyReadbackBuffer(finalReadbackBuffer, kFinalPattern))
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

	const TShared<GpuCommandBufferPool> commandBufferPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
	const TShared<GpuCommandBuffer> commandBuffer = commandBufferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 MSAA resolve test"));
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

	const TShared<GpuCommandBufferPool> graphicsPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS));
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
		const TShared<GpuCommandBufferPool> transferPool = device->CreateGpuCommandBufferPool(GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_TRANSFER));
		const TShared<GpuCommandBuffer> transferResolveCommandBuffer = transferPool->Create(GpuCommandBufferCreateInformation::Create("D3D12 transfer queue resolve validation test"));
		LoggingScope loggingScope(*this);
		loggingScope.ExpectError("D3D12 texture resolves require a graphics command buffer.");
		B3D_TEST_ASSERT(!transferResolveCommandBuffer->CopyTexture(source, destination))
	}
}
