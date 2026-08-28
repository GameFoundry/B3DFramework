//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuBackend.h"
#include "Managers/B3DD3D12DescriptorManager.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	bool GetRenderTargetViewDescription(const D3D12FramebufferAttachmentCreateInformation& attachment, D3D12_RENDER_TARGET_VIEW_DESC& outDesc)
	{
		ID3D12Resource* resource = attachment.Image->GetD3D12Resource();
		if (resource == nullptr)
			return false;

		const D3D12_RESOURCE_DESC resourceDescription = resource->GetDesc();
		const TextureSurface& surface = attachment.Surface;
		if (surface.MipLevelCount != 1 || surface.FaceCount == 0 || surface.MipLevel >= resourceDescription.MipLevels)
		{
			B3D_LOG(Error, LogRenderBackend, "Invalid D3D12 framebuffer color attachment surface.");
			return false;
		}

		outDesc = {};
		outDesc.Format = attachment.Image->GetDXGIFormat();
		switch (resourceDescription.Dimension)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if (surface.Face + surface.FaceCount > resourceDescription.DepthOrArraySize)
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12 framebuffer color attachment array range is out of bounds.");
				return false;
			}

			if (resourceDescription.SampleDesc.Count > 1)
			{
				if (surface.MipLevel != 0)
				{
					B3D_LOG(Error, LogRenderBackend, "Multisampled D3D12 framebuffer attachments cannot select a mip level.");
					return false;
				}

				if (resourceDescription.DepthOrArraySize > 1)
				{
					outDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
					outDesc.Texture2DMSArray.FirstArraySlice = surface.Face;
					outDesc.Texture2DMSArray.ArraySize = surface.FaceCount;
				}
				else
					outDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
			}
			else if (resourceDescription.DepthOrArraySize > 1)
			{
				outDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				outDesc.Texture2DArray.MipSlice = surface.MipLevel;
				outDesc.Texture2DArray.FirstArraySlice = surface.Face;
				outDesc.Texture2DArray.ArraySize = surface.FaceCount;
				outDesc.Texture2DArray.PlaneSlice = 0;
			}
			else
			{
				outDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				outDesc.Texture2D.MipSlice = surface.MipLevel;
				outDesc.Texture2D.PlaneSlice = 0;
			}
			break;
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
		{
			const u32 mipDepth = Math::Max(1u, (u32)resourceDescription.DepthOrArraySize >> surface.MipLevel);
			if (surface.Face + surface.FaceCount > mipDepth)
			{
				B3D_LOG(Error, LogRenderBackend, "D3D12 framebuffer color attachment depth-slice range is out of bounds.");
				return false;
			}

			outDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
			outDesc.Texture3D.MipSlice = surface.MipLevel;
			outDesc.Texture3D.FirstWSlice = surface.Face;
			outDesc.Texture3D.WSize = surface.FaceCount;
			break;
		}
		default:
			B3D_LOG(Error, LogRenderBackend, "Unsupported D3D12 framebuffer color attachment resource dimension.");
			return false;
		}

		return true;
	}

	bool GetDepthStencilViewDescription(const D3D12FramebufferAttachmentCreateInformation& attachment, D3D12_DEPTH_STENCIL_VIEW_DESC& outDesc)
	{
		ID3D12Resource* resource = attachment.Image->GetD3D12Resource();
		if (resource == nullptr)
			return false;

		const D3D12_RESOURCE_DESC resourceDescription = resource->GetDesc();
		const TextureSurface& surface = attachment.Surface;
		if (resourceDescription.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || surface.MipLevelCount != 1 || surface.FaceCount == 0 || surface.MipLevel >= resourceDescription.MipLevels || surface.Face + surface.FaceCount > resourceDescription.DepthOrArraySize)
		{
			B3D_LOG(Error, LogRenderBackend, "Invalid D3D12 framebuffer depth/stencil attachment surface.");
			return false;
		}

		outDesc = {};
		outDesc.Format = attachment.Image->GetDXGIFormat();
		if (resourceDescription.SampleDesc.Count > 1)
		{
			if (surface.MipLevel != 0)
			{
				B3D_LOG(Error, LogRenderBackend, "Multisampled D3D12 framebuffer attachments cannot select a mip level.");
				return false;
			}

			if (resourceDescription.DepthOrArraySize > 1)
			{
				outDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
				outDesc.Texture2DMSArray.FirstArraySlice = surface.Face;
				outDesc.Texture2DMSArray.ArraySize = surface.FaceCount;
			}
			else
				outDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		}
		else if (resourceDescription.DepthOrArraySize > 1)
		{
			outDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
			outDesc.Texture2DArray.MipSlice = surface.MipLevel;
			outDesc.Texture2DArray.FirstArraySlice = surface.Face;
			outDesc.Texture2DArray.ArraySize = surface.FaceCount;
		}
		else
		{
			outDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
			outDesc.Texture2D.MipSlice = surface.MipLevel;
		}

		return true;
	}
}

D3D12Framebuffer::D3D12Framebuffer(const D3D12FramebufferCreateInformation& createInformation)
	: GpuFramebuffer(createInformation.Width, createInformation.Height)
{
	bool hasSampleCount = false;
	auto fnRecordSampleCount = [this, &hasSampleCount](D3D12Image& image)
	{
		const u32 sampleCount = Math::Max(1u, image.GetD3D12Resource()->GetDesc().SampleDesc.Count);
		if (!hasSampleCount)
		{
			mSampleCount = sampleCount;
			hasSampleCount = true;
		}
		else if (mSampleCount != sampleCount)
			B3D_LOG(Error, LogRenderBackend, "D3D12 framebuffer attachments use mismatched sample counts.");
	};

	for (u32 colorIndex = 0; colorIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; colorIndex++)
	{
		const D3D12FramebufferAttachmentCreateInformation& attachment = createInformation.ColorAttachments[colorIndex];
		if (attachment.Image == nullptr || attachment.Image->GetD3D12Resource() == nullptr)
			continue;

		const u32 attachmentIndex = GetColorAttachmentCount();
		if (!CreateRenderTargetView(attachmentIndex, attachment))
			continue;

		AddColorAttachment(*attachment.Image, attachment.Image->GetRange(attachment.Surface), colorIndex, attachment.FinalLayout);

		mColorFormats[attachmentIndex] = attachment.Image->GetDXGIFormat();

		fnRecordSampleCount(*attachment.Image);
	}

	const D3D12FramebufferAttachmentCreateInformation& depthStencil = createInformation.DepthStencilAttachment;
	if (depthStencil.Image == nullptr || depthStencil.Image->GetD3D12Resource() == nullptr || !CreateDepthStencilViews(depthStencil))
		return;

	AddDepthStencilAttachment(*depthStencil.Image, depthStencil.Image->GetRange(depthStencil.Surface), depthStencil.FinalLayout);

	mDepthStencilFormat = depthStencil.Image->GetDXGIFormat();

	fnRecordSampleCount(*depthStencil.Image);
}

D3D12Framebuffer::~D3D12Framebuffer()
{
	// RTV/DSV descriptors are consumed at command-list record time, so recycling the heap slots cannot affect command lists that are already in flight.
	D3D12DescriptorManager& descriptorManager = GetD3D12GpuBackend().GetPrimaryDevice()->GetDescriptorManager();
	for (const D3D12_CPU_DESCRIPTOR_HANDLE renderTargetView : mRenderTargetViews)
	{
		if (renderTargetView.ptr != 0)
			descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::RTV, renderTargetView);
	}

	for(const D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView : mDepthStencilViews)
	{
		if(depthStencilView.ptr != 0)
			descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::DSV, depthStencilView);
	}
}

const D3D12_CPU_DESCRIPTOR_HANDLE* D3D12Framebuffer::GetDepthStencilView(RenderSurfaceMask readOnlyMask) const
{
	if(FindAttachment(RT_DEPTH) == nullptr && FindAttachment(RT_STENCIL) == nullptr)
		return nullptr;

	u32 viewIndex = readOnlyMask.IsSet(RT_DEPTH) ? 1u : 0u;
	if(mDepthStencilHasStencil && readOnlyMask.IsSet(RT_STENCIL))
		viewIndex |= 2u;

	return &mDepthStencilViews[viewIndex];
}

const GpuFramebufferLayoutPolicy& D3D12Framebuffer::GetLayoutPolicy()
{
	static const GpuFramebufferLayoutPolicy policy(
		GpuRenderPassAttachmentLayout(GpuImageLayout::ColorAttachment),
		GpuRenderPassAttachmentLayout(GpuImageLayout::ShaderReadOnly, GpuImageLayout::ShaderReadOnly),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilAttachment),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilAttachment, GpuImageLayout::DepthStencilReadOnly),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilAttachment, GpuImageLayout::DepthStencilReadOnly),
		GpuRenderPassAttachmentLayout(GpuImageLayout::DepthStencilAttachment, GpuImageLayout::DepthStencilReadOnly));

	return policy;
}

bool D3D12Framebuffer::CreateRenderTargetView(u32 attachmentIndex, const D3D12FramebufferAttachmentCreateInformation& attachment)
{
	D3D12GpuDevice& device = *GetD3D12GpuBackend().GetPrimaryDevice();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();
	D3D12_RENDER_TARGET_VIEW_DESC description;
	if (!GetRenderTargetViewDescription(attachment, description))
		return false;

	mRenderTargetViews[attachmentIndex] = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::RTV);
	if (mRenderTargetViews[attachmentIndex].ptr == 0)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to allocate a D3D12 framebuffer render-target view.");
		return false;
	}

	device.GetD3D12Device()->CreateRenderTargetView(attachment.Image->GetD3D12Resource(), &description, mRenderTargetViews[attachmentIndex]);
	return true;
}

bool D3D12Framebuffer::CreateDepthStencilViews(const D3D12FramebufferAttachmentCreateInformation& attachment)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC description;
	if (!GetDepthStencilViewDescription(attachment, description))
		return false;

	D3D12GpuDevice& device = *GetD3D12GpuBackend().GetPrimaryDevice();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();
	const bool hasStencil = description.Format == DXGI_FORMAT_D24_UNORM_S8_UINT || description.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
	const u32 viewCount = hasStencil ? kDepthStencilViewCount : 2u;
	for(u32 viewIndex = 0; viewIndex < viewCount; ++viewIndex)
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC viewDescription = description;
		viewDescription.Flags = D3D12_DSV_FLAG_NONE;
		if((viewIndex & 1u) != 0)
			viewDescription.Flags = (D3D12_DSV_FLAGS)(viewDescription.Flags | D3D12_DSV_FLAG_READ_ONLY_DEPTH);

		if((viewIndex & 2u) != 0)
			viewDescription.Flags = (D3D12_DSV_FLAGS)(viewDescription.Flags | D3D12_DSV_FLAG_READ_ONLY_STENCIL);

		mDepthStencilViews[viewIndex] = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::DSV);
		if (mDepthStencilViews[viewIndex].ptr == 0)
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to allocate a D3D12 framebuffer depth-stencil view.");
			return false;
		}

		device.GetD3D12Device()->CreateDepthStencilView(attachment.Image->GetD3D12Resource(), &viewDescription, mDepthStencilViews[viewIndex]);
	}

	mDepthStencilHasStencil = hasStencil;
	return true;
}
