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
	, mNumColorAttachments(0)
	, mHasDepthStencil(false)
	, mWidth(0)
	, mHeight(0)
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
	for (u32 i = 0; i < kMaxColorAttachments; i++)
	{
		if (mRenderTargetViews[i].ptr != 0)
			descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::RTV, mRenderTargetViews[i]);
	}

	if (mDepthStencilView.ptr != 0)
		descriptorManager.FreeCPUDescriptor(D3D12DescriptorHeapType::DSV, mDepthStencilView);
}

void D3D12Framebuffer::CreateViews()
{
	// Initialize to empty handles
	for (u32 i = 0; i < kMaxColorAttachments; i++)
	{
		mRenderTargetViews[i].ptr = 0;
		mColorAttachments[i] = Attachment{};
		mColorFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
	mDepthStencilView.ptr = 0;
	mDepthStencilAttachment = Attachment{};
	mDepthStencilFormat = DXGI_FORMAT_UNKNOWN;
	mSampleCount = 1;

	if (!mRenderTarget)
		return;

	// Get the device and descriptor manager
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

		// Handle RenderTexture - create views for color and depth-stencil textures
		for (u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		{
			// Create render target view over the requested face/mip sub-range (rendering into a single cube face or
			// texture-array slice is how cubemaps get filled, e.g. sky irradiance - a view always starting at face 0
			// makes every such pass overwrite the first face).
			const RenderSurfaceInformation& surfaceInformation = renderTexture->GetColorSurfaceInformation(i);
			const TShared<Texture>& colorTexture = surfaceInformation.Texture;
			if (!colorTexture)
				continue;

			// Get the D3D12 texture resource
			D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(colorTexture.get());
			ID3D12Resource* resource = d3d12Texture->GetD3D12Resource();

			if (!resource)
				continue;

			// Allocate RTV descriptor
			mRenderTargetViews[i] = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::RTV);

			const TextureProperties& props = colorTexture->GetProperties();

			const u32 baseFace = surfaceInformation.Face;
			const u32 faceCount = surfaceInformation.FaceCount == 0 ? (props.GetFaceCount() - baseFace) : surfaceInformation.FaceCount;
			const u32 mipLevel = surfaceInformation.MipLevel;

			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			// Use the resource's actual format rather than re-deriving from the pixel format - the two diverge
			// for sRGB textures (the RTV format must match the resource's).
			rtvDesc.Format = d3d12Texture->GetDXGIFormat();

			switch (props.Type)
			{
			case TEX_TYPE_2D:
				if (props.GetFaceCount() > 1)
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
				rtvDesc.Texture3D.WSize = surfaceInformation.FaceCount == 0 ? props.Depth - surfaceInformation.Face : surfaceInformation.FaceCount;
				break;
			default:
				B3D_LOG(Error, LogRenderBackend, "Unsupported texture type for render target view");
				continue;
			}

			d3d12Device->CreateRenderTargetView(resource, &rtvDesc, mRenderTargetViews[i]);

			mColorAttachments[i].Image = d3d12Texture->GetD3D12Image();
			mColorAttachments[i].Surface = TextureSurface(mipLevel, 1, baseFace, faceCount);
			mColorFormats[i] = rtvDesc.Format;
			mSampleCount = Math::Max(1u, props.SampleCount);

			mNumColorAttachments++;
		}

		// Handle depth-stencil texture
		const TShared<Texture>& depthTexture = renderTexture->GetDepthStencilSurfaceInformation().Texture;
		if (depthTexture)
		{
			D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(depthTexture.get());
			ID3D12Resource* resource = d3d12Texture->GetD3D12Resource();

			if (resource)
			{
				// Allocate DSV descriptor
				mDepthStencilView = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::DSV);

				// Create depth-stencil view over the requested face/mip sub-range (see the color path above).
				const RenderSurfaceInformation& surfaceInformation = renderTexture->GetDepthStencilSurfaceInformation();
				const TextureProperties& props = depthTexture->GetProperties();

				const u32 baseFace = surfaceInformation.Face;
				const u32 faceCount = surfaceInformation.FaceCount == 0 ? (props.GetFaceCount() - baseFace) : surfaceInformation.FaceCount;
				const u32 mipLevel = surfaceInformation.MipLevel;

				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = D3D12Utility::GetDXGIFormat(props.Format);
				dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

				switch (props.Type)
				{
				case TEX_TYPE_2D:
					if (props.GetFaceCount() > 1)
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
				mSampleCount = Math::Max(1u, props.SampleCount);

				mHasDepthStencil = true;
			}
		}
	}
	else
	{
		// Handle RenderWindow - get views from swap chain
		const RenderWindow* renderWindow = static_cast<const RenderWindow*>(mRenderTarget);

		// Get the render window surface which contains the swap chain
		const TShared<IRenderWindowSurface>& surfacePtr = renderWindow->GetRenderWindowSurface();
		if (!surfacePtr)
		{
			B3D_LOG(Warning, LogRenderBackend, "RenderWindow has no surface, cannot create framebuffer");
			return;
		}

		D3D12RenderWindowSurface* d3d12Surface = static_cast<D3D12RenderWindowSurface*>(surfacePtr.get());
		D3D12SwapChain* swapChain = d3d12Surface->GetSwapChain();

		if (!swapChain)
		{
			B3D_LOG(Warning, LogRenderBackend, "RenderWindow surface has no swap chain, cannot create framebuffer");
			return;
		}

		// Get the RTV for the specified back buffer
		mRenderTargetViews[0] = swapChain->GetBackBufferRTV(mBackBufferIndex);
		mNumColorAttachments = 1;
		mColorFormats[0] = swapChain->GetCreateInformation().ColorFormat;

		mColorAttachments[0].Image = swapChain->GetBackBufferImage(mBackBufferIndex);
		mColorAttachments[0].Surface = TextureSurface(0, 1, 0, 1);

		// Get depth stencil view if it exists
		D3D12_CPU_DESCRIPTOR_HANDLE dsv = swapChain->GetDepthStencilView();
		if (dsv.ptr != 0)
		{
			mDepthStencilView = dsv;
			mHasDepthStencil = true;
			mDepthStencilFormat = swapChain->GetCreateInformation().DepthStencilFormat;

			mDepthStencilAttachment.Image = swapChain->GetDepthStencilImage();
			mDepthStencilAttachment.Surface = TextureSurface(0, 1, 0, 1);
		}

		B3D_LOG(Info, LogRenderBackend, "Created framebuffer from RenderWindow swap chain: {0}x{1}", mWidth, mHeight);
	}
}
