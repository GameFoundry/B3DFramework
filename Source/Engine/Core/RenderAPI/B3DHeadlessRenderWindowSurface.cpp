//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DHeadlessRenderWindowSurface.h"

#include "B3DApplication.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DGpuDevice.h"

using namespace b3d::render;

HeadlessRenderWindowSurface::HeadlessRenderWindowSurface(const RenderWindowSurfaceCreateInformation& createInformation)
	: mWidth(createInformation.Width)
	, mHeight(createInformation.Height)
	, mVSync(createInformation.VSync)
	, mCreateDepthBuffer(createInformation.CreateDepthBuffer)
	, mUseHardwareSRGB(createInformation.UseHardwareSRGB)
{
	CreateImages();
}

HeadlessRenderWindowSurface::~HeadlessRenderWindowSurface()
{
	if(!mIsDestroyed)
		Destroy();
}

void HeadlessRenderWindowSurface::CreateImages()
{
	const SPtr<b3d::GpuDevice>& gpuDevice = b3d::GetApplication().GetPrimaryGpuDevice();

	// Create a color texture for each swap chain image
	for(u32 imageIndex = 0; imageIndex < kImageCount; imageIndex++)
	{
		RenderTextureCreateInformation createInformation;

		// Color surface
		b3d::TextureCreateInformation colorTexDesc;
		colorTexDesc.Type = TEX_TYPE_2D;
		colorTexDesc.Width = mWidth;
		colorTexDesc.Height = mHeight;
		colorTexDesc.Format = PF_RGBA8;
		colorTexDesc.UseHardwareSRGB = mUseHardwareSRGB;
		colorTexDesc.Usage = TextureUsageFlag::RenderTarget;
		colorTexDesc.MipMapCount = 0;
		colorTexDesc.SampleCount = 1;
		colorTexDesc.Name = "HeadlessSwapChainImage" + b3d::ToString(imageIndex);

		SPtr<Texture> colorTexture = gpuDevice->CreateTexture(colorTexDesc);

		RenderSurfaceInformation colorSurface;
		colorSurface.Face = 0;
		colorSurface.MipLevel = 0;
		colorSurface.FaceCount = 1;
		colorSurface.Texture = colorTexture;
		createInformation.ColorSurfaces[0] = colorSurface;

		// Create depth buffer if requested
		if(mCreateDepthBuffer)
		{
			b3d::TextureCreateInformation depthTexDesc;
			depthTexDesc.Type = TEX_TYPE_2D;
			depthTexDesc.Width = mWidth;
			depthTexDesc.Height = mHeight;
			depthTexDesc.Format = PF_D32;
			depthTexDesc.Usage = TextureUsageFlag::DepthStencil;
			depthTexDesc.MipMapCount = 0;
			depthTexDesc.SampleCount = 1;
			depthTexDesc.Name = "HeadlessSwapChainDepth" + b3d::ToString(imageIndex);

			SPtr<Texture> depthTexture = gpuDevice->CreateTexture(depthTexDesc);

			RenderSurfaceInformation depthSurface;
			depthSurface.Face = 0;
			depthSurface.MipLevel = 0;
			depthSurface.FaceCount = 1;
			depthSurface.Texture = depthTexture;
			createInformation.DepthStencilSurface = depthSurface;
		}

		mImages[imageIndex] = RenderTexture::Create(createInformation);
	}
}

void HeadlessRenderWindowSurface::DestroyImages()
{
	for(u32 imageIndex = 0; imageIndex < kImageCount; imageIndex++)
		mImages[imageIndex] = nullptr;
}

void HeadlessRenderWindowSurface::RebuildSwapChain(u32 width, u32 height, bool vsync)
{
	if(mIsDestroyed)
		return;

	// Only rebuild if dimensions changed
	if(mWidth == width && mHeight == height && mVSync == vsync && mIsValid)
		return;

	mWidth = width;
	mHeight = height;
	mVSync = vsync;
	mIsValid = true;
	mCurrentImageIndex = 0;

	DestroyImages();
	CreateImages();
}

void HeadlessRenderWindowSurface::MarkSwapChainAsInvalid()
{
	mIsValid = false;
}

void HeadlessRenderWindowSurface::Destroy()
{
	if(mIsDestroyed)
		return;

	DestroyImages();
	mIsDestroyed = true;
}

void HeadlessRenderWindowSurface::Present()
{
	// In headless mode, present just cycles to the next image
	// No actual GPU presentation needed
}

void HeadlessRenderWindowSurface::AcquireNextImage()
{
	// Cycle to next image in the swap chain
	mCurrentImageIndex = (mCurrentImageIndex + 1) % kImageCount;
}
