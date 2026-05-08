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
	// Descriptor handles are managed by the descriptor manager
	// No explicit cleanup needed here
}

void D3D12Framebuffer::CreateViews()
{
	// Initialize to empty handles
	for (u32 i = 0; i < kMaxColorAttachments; i++)
	{
		mRenderTargetViews[i].ptr = 0;
	}
	mDepthStencilView.ptr = 0;

	if (!mRenderTarget)
		return;

	// Get the device and descriptor manager
	D3D12GpuDevice& device = static_cast<D3D12GpuDevice&>(GetD3D12GpuBackend().GetPrimaryDevice());
	D3D12DescriptorManager& descriptorManager = device.GetDescriptorManager();
	ID3D12Device* d3d12Device = device.GetD3D12Device();

	// Check if this is a RenderTexture (off-screen) or RenderWindow (swap chain)
	const RenderTexture* renderTexture = dynamic_cast<const RenderTexture*>(mRenderTarget);

	if (renderTexture)
	{
		// Handle RenderTexture - create views for color and depth-stencil textures
		for (u32 i = 0; i < B3D_MAXIMUM_RENDER_TARGET_COUNT; i++)
		{
			TShared<Texture> colorTexture = renderTexture->GetColorTexture(i);
			if (!colorTexture)
				continue;

			// Get the D3D12 texture resource
			D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(colorTexture.get());
			ID3D12Resource* resource = d3d12Texture->GetD3D12Resource();

			if (!resource)
				continue;

			// Allocate RTV descriptor
			mRenderTargetViews[i] = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::RTV);

			// Create render target view
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = D3D12Utility::GetDXGIFormat(colorTexture->GetProperties().Format);

			const TextureProperties& props = colorTexture->GetProperties();
			switch (props.TextureType)
			{
			case TEX_TYPE_2D:
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				rtvDesc.Texture2D.MipSlice = 0;
				rtvDesc.Texture2D.PlaneSlice = 0;
				break;
			case TEX_TYPE_2D_ARRAY:
			case TEX_TYPE_CUBE_MAP:
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.MipSlice = 0;
				rtvDesc.Texture2DArray.FirstArraySlice = 0;
				rtvDesc.Texture2DArray.ArraySize = props.NumFaces;
				rtvDesc.Texture2DArray.PlaneSlice = 0;
				break;
			case TEX_TYPE_3D:
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
				rtvDesc.Texture3D.MipSlice = 0;
				rtvDesc.Texture3D.FirstWSlice = 0;
				rtvDesc.Texture3D.WSize = props.Depth;
				break;
			default:
				B3D_LOG(Error, LogRenderBackend, "Unsupported texture type for render target view");
				continue;
			}

			d3d12Device->CreateRenderTargetView(resource, &rtvDesc, mRenderTargetViews[i]);
			mNumColorAttachments++;
		}

		// Handle depth-stencil texture
		TShared<Texture> depthTexture = renderTexture->GetDepthStencilTexture();
		if (depthTexture)
		{
			D3D12Texture* d3d12Texture = static_cast<D3D12Texture*>(depthTexture.get());
			ID3D12Resource* resource = d3d12Texture->GetD3D12Resource();

			if (resource)
			{
				// Allocate DSV descriptor
				mDepthStencilView = descriptorManager.AllocateCPUDescriptor(D3D12DescriptorHeapType::DSV);

				// Create depth-stencil view
				D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = D3D12Utility::GetDXGIFormat(depthTexture->GetProperties().Format);
				dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

				const TextureProperties& props = depthTexture->GetProperties();
				switch (props.TextureType)
				{
				case TEX_TYPE_2D:
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
					dsvDesc.Texture2D.MipSlice = 0;
					break;
				case TEX_TYPE_2D_ARRAY:
				case TEX_TYPE_CUBE_MAP:
					dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
					dsvDesc.Texture2DArray.MipSlice = 0;
					dsvDesc.Texture2DArray.FirstArraySlice = 0;
					dsvDesc.Texture2DArray.ArraySize = props.NumFaces;
					break;
				default:
					B3D_LOG(Error, LogRenderBackend, "Unsupported texture type for depth-stencil view");
					return;
				}

				d3d12Device->CreateDepthStencilView(resource, &dsvDesc, mDepthStencilView);
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

		// Get depth stencil view if it exists
		D3D12_CPU_DESCRIPTOR_HANDLE dsv = swapChain->GetDepthStencilView();
		if (dsv.ptr != 0)
		{
			mDepthStencilView = dsv;
			mHasDepthStencil = true;
		}

		B3D_LOG(Info, LogRenderBackend, "Created framebuffer from RenderWindow swap chain: {0}x{1}", mWidth, mHeight);
	}
}
