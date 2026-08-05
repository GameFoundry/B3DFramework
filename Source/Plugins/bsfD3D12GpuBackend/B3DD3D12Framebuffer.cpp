//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12Texture.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12SwapChain.h"
#include "B3DD3D12RenderWindowSurface.h"
#include "B3DD3D12Utility.h"
#include "Managers/B3DD3D12DescriptorManager.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "GpuBackend/B3DRenderTexture.h"
#include "GpuBackend/B3DRenderWindow.h"

using namespace b3d;
using namespace b3d::render;

D3D12Framebuffer::D3D12Framebuffer(const RenderTarget* renderTarget, u32 backBufferIndex)
	: mRenderTarget(renderTarget)
	, mBackBufferIndex(backBufferIndex)
{
	if (!mRenderTarget)
		return;

	mWidth = mRenderTarget->GetProperties().Width;
	mHeight = mRenderTarget->GetProperties().Height;

	CreateViews();
}

D3D12Framebuffer::~D3D12Framebuffer()
{
	// Swap-chain framebuffers reference views owned by the swap chain, only views allocated by us are returned. This
	// is safe with respect to in-flight GPU work: RTV/DSV descriptors are consumed at command list record time, so
	// recycling the heap slot cannot affect already-recorded command lists.
	if (!mOwnsViews)
		return;

	D3D12DescriptorManager& descriptorManager = GetD3D12GpuBackend().GetPrimaryDevice()->GetDescriptorManager();
	for (u32 attachmentIndex = 0; attachmentIndex < kMaxColorAttachments; attachmentIndex++)
	{
		if (mRenderTargetViews[attachmentIndex].ptr != 0)
			descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::RTV, mRenderTargetViews[attachmentIndex]);
	}

	if (mDepthStencilView.ptr != 0)
		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::DSV, mDepthStencilView);
}

void D3D12Framebuffer::CreateViews()
{
	D3D12GpuDevice& device = *GetD3D12GpuBackend().GetPrimaryDevice();
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	// Check if this is a RenderTexture (off-screen) or RenderWindow (swap chain). Note that dynamic_cast cannot
	// be used as the engine builds without RTTI.
	const RenderTexture* renderTexture = !mRenderTarget->GetProperties().IsWindow
		? static_cast<const RenderTexture*>(mRenderTarget)
		: nullptr;

	if (renderTexture)
	{
		mOwnsViews = true;

		for (u32 attachmentIndex = 0; attachmentIndex < B3D_MAXIMUM_RENDER_TARGET_COUNT; attachmentIndex++)
		{
			// Create render target view over the requested face/mip sub-range (rendering into a single cube face or
			// texture-array slice is how cubemaps get filled, e.g. sky irradiance - a view always starting at face 0
			// makes every such pass overwrite the first face).
			const RenderSurfaceInformation& surfaceInformation = renderTexture->GetColorSurfaceInformation(attachmentIndex);
			const TShared<Texture>& colorTexture = surfaceInformation.Texture;
			if (!colorTexture)
				continue;

			D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(colorTexture.get());
			ID3D12Resource* resource = d3d12Texture->GetD3D12Resource();

			if (!resource)
				continue;

			mRenderTargetViews[attachmentIndex] = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::RTV);

			const TextureProperties& properties = colorTexture->GetProperties();

			const u32 baseFace = surfaceInformation.Face;
			const u32 faceCount = surfaceInformation.FaceCount == 0 ? (properties.GetFaceCount() - baseFace) : surfaceInformation.FaceCount;
			const u32 mipLevel = surfaceInformation.MipLevel;

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			// Use the resource's actual format rather than re-deriving from the pixel format - the two diverge
			// for sRGB textures (the RTV format must match the resource's).
			rtvDesc.Format = d3d12Texture->GetDXGIFormat();

			switch (properties.Type)
			{
			case TEX_TYPE_2D:
				if (properties.GetFaceCount() > 1)
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDesc.Texture2DArray.MipSlice = mipLevel;
					rtvDesc.Texture2DArray.FirstArraySlice = baseFace;
					rtvDesc.Texture2DArray.ArraySize = faceCount;
					rtvDesc.Texture2DArray.PlaneSlice = 0;
				}
				else
				{
					rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					rtvDesc.Texture2D.MipSlice = mipLevel;
					rtvDesc.Texture2D.PlaneSlice = 0;
				}
				break;
			case TEX_TYPE_CUBE_MAP:
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.MipSlice = mipLevel;
				rtvDesc.Texture2DArray.FirstArraySlice = baseFace;
				rtvDesc.Texture2DArray.ArraySize = faceCount;
				rtvDesc.Texture2DArray.PlaneSlice = 0;
				break;
			case TEX_TYPE_3D:
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
				rtvDesc.Texture3D.MipSlice = mipLevel;
				rtvDesc.Texture3D.FirstWSlice = surfaceInformation.Face;
				rtvDesc.Texture3D.WSize = surfaceInformation.FaceCount == 0 ? properties.Depth - surfaceInformation.Face : surfaceInformation.FaceCount;
				break;
			default:
				B3D_LOG(Error, LogRenderBackend, "Unsupported texture type for render target view");
				continue;
			}

			d3d12Device->CreateRenderTargetView(resource, &rtvDesc, mRenderTargetViews[attachmentIndex]);

			mColorAttachments[attachmentIndex].Image = d3d12Texture->GetD3D12Image();
			mColorAttachments[attachmentIndex].Surface = TextureSurface(mipLevel, 1, baseFace, faceCount);
			mColorFormats[attachmentIndex] = rtvDesc.Format;
			mSampleCount = Math::Max(1u, properties.SampleCount);

			mColorAttachmentCount++;
		}

		const TShared<Texture>& depthTexture = renderTexture->GetDepthStencilSurfaceInformation().Texture;
		if (depthTexture)
		{
			D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(depthTexture.get());
			ID3D12Resource* resource = d3d12Texture->GetD3D12Resource();

			if (resource)
			{
				mDepthStencilView = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::DSV);

				// Create depth-stencil view over the requested face/mip sub-range (see the color path above).
				const RenderSurfaceInformation& surfaceInformation = renderTexture->GetDepthStencilSurfaceInformation();
				const TextureProperties& properties = depthTexture->GetProperties();

				const u32 baseFace = surfaceInformation.Face;
				const u32 faceCount = surfaceInformation.FaceCount == 0 ? (properties.GetFaceCount() - baseFace) : surfaceInformation.FaceCount;
				const u32 mipLevel = surfaceInformation.MipLevel;

				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = D3D12Utility::GetDXGIFormat(properties.Format);
				dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

				switch (properties.Type)
				{
				case TEX_TYPE_2D:
					if (properties.GetFaceCount() > 1)
					{
						dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
						dsvDesc.Texture2DArray.MipSlice = mipLevel;
						dsvDesc.Texture2DArray.FirstArraySlice = baseFace;
						dsvDesc.Texture2DArray.ArraySize = faceCount;
					}
					else
					{
						dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
						dsvDesc.Texture2D.MipSlice = mipLevel;
					}
					break;
				case TEX_TYPE_CUBE_MAP:
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.MipSlice = mipLevel;
					dsvDesc.Texture2DArray.FirstArraySlice = baseFace;
					dsvDesc.Texture2DArray.ArraySize = faceCount;
					break;
				default:
					B3D_LOG(Error, LogRenderBackend, "Unsupported texture type for depth-stencil view");
					return;
				}

				d3d12Device->CreateDepthStencilView(resource, &dsvDesc, mDepthStencilView);

				mDepthStencilAttachment.Image = d3d12Texture->GetD3D12Image();
				mDepthStencilAttachment.Surface = TextureSurface(mipLevel, 1, baseFace, faceCount);
				mDepthStencilFormat = dsvDesc.Format;
				mSampleCount = Math::Max(1u, properties.SampleCount);

				mHasDepthStencil = true;
			}
		}
	}
	else
	{
		const RenderWindow* renderWindow = static_cast<const RenderWindow*>(mRenderTarget);

		const TShared<IRenderWindowSurface>& surface = renderWindow->GetRenderWindowSurface();
		if (!surface)
		{
			B3D_LOG(Warning, LogRenderBackend, "RenderWindow has no surface, cannot create framebuffer");
			return;
		}

		D3D12SwapChain* swapChain = static_cast<D3D12RenderWindowSurface*>(surface.get())->GetSwapChain();
		if (!swapChain)
		{
			B3D_LOG(Warning, LogRenderBackend, "RenderWindow surface has no swap chain, cannot create framebuffer");
			return;
		}

		// The swap chain owns these views, so they must not be freed on destruction (mOwnsViews stays false).
		mRenderTargetViews[0] = swapChain->GetBackBufferRTV(mBackBufferIndex);
		mColorAttachmentCount = 1;
		mColorFormats[0] = swapChain->GetCreateInformation().ColorFormat;

		mColorAttachments[0].Image = swapChain->GetBackBufferImage(mBackBufferIndex);
		mColorAttachments[0].Surface = TextureSurface(0, 1, 0, 1);

		const D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView = swapChain->GetDepthStencilView();
		if (depthStencilView.ptr != 0)
		{
			mDepthStencilView = depthStencilView;
			mHasDepthStencil = true;
			mDepthStencilFormat = swapChain->GetCreateInformation().DepthStencilFormat;

			mDepthStencilAttachment.Image = swapChain->GetDepthStencilImage();
			mDepthStencilAttachment.Surface = TextureSurface(0, 1, 0, 1);
		}

		B3D_LOG(Verbose, LogRenderBackend, "Created framebuffer from RenderWindow swap chain: {0}x{1}", mWidth, mHeight);
	}
}
