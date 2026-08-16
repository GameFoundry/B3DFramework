//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12HeadlessRenderWindowSurface.h"
#include "B3DD3D12Framebuffer.h"
#include "B3DD3D12GpuBackend.h"
#include "B3DD3D12GpuDevice.h"
#include "B3DD3D12ResourceManager.h"
#include "B3DD3D12Texture.h"

using namespace b3d;
using namespace b3d::render;

D3D12HeadlessRenderWindowSurface::D3D12HeadlessRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation) 
	:mDevice(static_cast<D3D12GpuDevice&>(*GetD3D12GpuBackend().GetPrimaryDevice())), mBackBufferCount(GetD3D12BackBufferCount()), mWidth(createInformation.Width), mHeight(createInformation.Height), mVSync(createInformation.VSync), mVsyncInterval(createInformation.VsyncInterval), mCreateDepthBuffer(createInformation.CreateDepthBuffer)
{
	mColorFormat = createInformation.UseHardwareSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
	if (mCreateDepthBuffer)
		mDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	CreateSurfaceResources();

	B3D_LOG(Verbose, LogRenderBackend, "Created headless D3D12 render window surface: width={0}, height={1}, srgb={2}", mWidth, mHeight, createInformation.UseHardwareSRGB);
}

D3D12HeadlessRenderWindowSurface::~D3D12HeadlessRenderWindowSurface()
{
	Destroy();
}

void D3D12HeadlessRenderWindowSurface::CreateSurfaceResources()
{
	if (mWidth == 0 || mHeight == 0)
	{
		B3D_LOG(Error, LogRenderBackend, "Headless D3D12 render window surface created with zero size ({0}x{1}).", mWidth, mHeight);
		return;
	}

	D3D12_RESOURCE_DESC colorDescription = {};
	colorDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	colorDescription.Width = mWidth;
	colorDescription.Height = mHeight;
	colorDescription.DepthOrArraySize = 1;
	colorDescription.MipLevels = 1;
	colorDescription.Format = mColorFormat;
	colorDescription.SampleDesc.Count = 1;
	colorDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	colorDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_CLEAR_VALUE colorClearValue = {};
	colorClearValue.Format = mColorFormat;

	for (u32 imageIndex = 0; imageIndex < mBackBufferCount; imageIndex++)
	{
		GpuResourceLocation allocation;
		const HRESULT result = mDevice.CreateResource(colorDescription, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_UNDEFINED, &colorClearValue, mColorBuffers[imageIndex], allocation);
		if (FAILED(result))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create headless D3D12 color image {0}: HRESULT={1}", imageIndex, (u32)result);
			DestroySurfaceResources();
			return;
		}

		const String name = "Headless BackBuffer " + ToString(imageIndex);
		mColorBuffers[imageIndex]->SetName(ToWideString(name).c_str());

		D3D12ImageCreateInformation imageCreateInformation;
		imageCreateInformation.Resource = mColorBuffers[imageIndex];
		imageCreateInformation.Allocation = allocation;
		imageCreateInformation.Format = mColorFormat;
		imageCreateInformation.InitialLayout = D3D12TextureLayout::Undefined();
		imageCreateInformation.FaceCount = 1;
		imageCreateInformation.MipLevelCount = 1;
		imageCreateInformation.Aspect = GpuTextureAspectFlag::Color;
		imageCreateInformation.Name = name;

		mColorImages[imageIndex] = mDevice.GetResourceManager().Create<D3D12Image>(imageCreateInformation);
	}

	if (mCreateDepthBuffer)
	{
		D3D12_RESOURCE_DESC depthStencilDescription = {};
		depthStencilDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDescription.Width = mWidth;
		depthStencilDescription.Height = mHeight;
		depthStencilDescription.DepthOrArraySize = 1;
		depthStencilDescription.MipLevels = 1;
		depthStencilDescription.Format = mDepthFormat;
		depthStencilDescription.SampleDesc.Count = 1;
		depthStencilDescription.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthStencilDescription.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE depthStencilClearValue = {};
		depthStencilClearValue.Format = mDepthFormat;
		depthStencilClearValue.DepthStencil.Depth = 1.0f;

		GpuResourceLocation allocation;
		const HRESULT result = mDevice.CreateResource(depthStencilDescription, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_UNDEFINED, &depthStencilClearValue, mDepthStencilBuffer, allocation);
		if (FAILED(result))
		{
			B3D_LOG(Error, LogRenderBackend, "Failed to create headless D3D12 depth-stencil image: HRESULT={0}", (u32)result);
			DestroySurfaceResources();
			return;
		}

		mDepthStencilBuffer->SetName(L"Headless DepthStencil Buffer");

		D3D12ImageCreateInformation imageCreateInformation;
		imageCreateInformation.Resource = mDepthStencilBuffer;
		imageCreateInformation.Allocation = allocation;
		imageCreateInformation.Format = mDepthFormat;
		imageCreateInformation.InitialLayout = D3D12TextureLayout::Undefined();
		imageCreateInformation.FaceCount = 1;
		imageCreateInformation.MipLevelCount = 1;
		imageCreateInformation.Aspect = GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil;
		imageCreateInformation.Name = "Headless DepthStencil Buffer";

		mDepthStencilImage = mDevice.GetResourceManager().Create<D3D12Image>(imageCreateInformation);
	}

	mCurrentImageIndex = 0;
	mImageAdvancePending = false;
	mIsValid = true;

	for (u32 imageIndex = 0; imageIndex < mBackBufferCount; imageIndex++)
	{
		D3D12FramebufferCreateInformation framebufferCreateInformation;
		framebufferCreateInformation.Width = mWidth;
		framebufferCreateInformation.Height = mHeight;
		framebufferCreateInformation.ColorAttachments[0].Image = mColorImages[imageIndex];
		framebufferCreateInformation.DepthStencilAttachment.Image = mDepthStencilImage;

		mFramebuffers[imageIndex] = B3DNew<D3D12Framebuffer>(framebufferCreateInformation);
	}
}

void D3D12HeadlessRenderWindowSurface::DestroySurfaceResources()
{
	mDevice.WaitUntilIdle();

	for (u32 imageIndex = 0; imageIndex < mBackBufferCount; imageIndex++)
	{
		if (mFramebuffers[imageIndex] != nullptr)
		{
			B3DDelete(mFramebuffers[imageIndex]);
			mFramebuffers[imageIndex] = nullptr;
		}

		if (mColorImages[imageIndex] != nullptr)
		{
			mColorImages[imageIndex]->Destroy();
			mColorImages[imageIndex] = nullptr;
		}

		mColorBuffers[imageIndex].Reset();
	}

	if (mDepthStencilImage != nullptr)
	{
		mDepthStencilImage->Destroy();
		mDepthStencilImage = nullptr;
	}

	mDepthStencilBuffer.Reset();
	mIsValid = false;
}

D3D12Framebuffer* D3D12HeadlessRenderWindowSurface::GetActiveFramebuffer()
{
	if (mIsDestroyed || !mIsValid)
		return nullptr;

	if (mImageAdvancePending)
	{
		mCurrentImageIndex = (mCurrentImageIndex + 1) % mBackBufferCount;
		mImageAdvancePending = false;
	}

	return mFramebuffers[mCurrentImageIndex];
}

void D3D12HeadlessRenderWindowSurface::SwapBuffers(GpuQueue&, GpuQueueMask)
{
	if (mIsDestroyed || !mIsValid)
		return;

	mImageAdvancePending = true;
}

void D3D12HeadlessRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync, u32 vsyncInterval)
{
	if (mIsDestroyed)
		return;

	if (mIsValid && mWidth == width && mHeight == height && mVSync == vsync && mVsyncInterval == vsyncInterval)
		return;

	mWidth = width;
	mHeight = height;
	mVSync = vsync;
	mVsyncInterval = vsyncInterval == 0 ? 1 : vsyncInterval;
	mImageAdvancePending = false;

	DestroySurfaceResources();
	CreateSurfaceResources();
}

void D3D12HeadlessRenderWindowSurface::MarkSwapChainAsInvalid()
{
	if (!mIsDestroyed)
		mIsValid = false;
}

void D3D12HeadlessRenderWindowSurface::Destroy()
{
	if (mIsDestroyed)
		return;

	DestroySurfaceResources();
	mIsDestroyed = true;
}
