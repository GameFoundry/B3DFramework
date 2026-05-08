//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRenderTargetEx.h"
#include "Generated/B3DScriptShaderParameter.generated.h"

using namespace b3d;
u32 RenderTargetEx::GetWidth(const TShared<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().Width;
}

u32 RenderTargetEx::GetHeight(const TShared<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().Height;
}

bool RenderTargetEx::GetGammaCorrection(const TShared<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().HwGamma;
}

i32 RenderTargetEx::GetPriority(const TShared<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().Priority;
}

void RenderTargetEx::SetPriority(const TShared<RenderTarget>& thisPtr, i32 priority)
{
	thisPtr->SetPriority(priority);
}

u32 RenderTargetEx::GetSampleCount(const TShared<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().MultisampleCount;
}

TShared<RenderTexture> RenderTextureEx::Create(PixelFormat format, int width, int height, int numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
{
	TextureCreateInformation texDesc;
	texDesc.Name = "Script Render Texture Target";
	texDesc.Type = TEX_TYPE_2D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = format;
	texDesc.UseHardwareSRGB = gammaCorrection;
	texDesc.SampleCount = numSamples;

	return RenderTexture::Create(texDesc, createDepth, depthStencilFormat);
}

TShared<RenderTexture> RenderTextureEx::Create(const HTexture& colorSurface)
{
	return Create(Vector<HTexture>{ colorSurface }, HTexture());
}

TShared<RenderTexture> RenderTextureEx::Create(const HTexture& colorSurface, const HTexture& depthStencilSurface)
{
	return Create(Vector<HTexture>{ colorSurface }, depthStencilSurface);
}

TShared<RenderTexture> RenderTextureEx::Create(const Vector<HTexture>& colorSurface)
{
	return Create(Vector<HTexture>{ colorSurface }, HTexture());
}

TShared<RenderTexture> RenderTextureEx::Create(const Vector<HTexture>& colorSurfaces, const HTexture& depthStencilSurface)
{
	RenderSurfaceInformation depthStencilSurfaceDesc;
	if(depthStencilSurface != nullptr)
	{
		depthStencilSurfaceDesc.Face = 0;
		depthStencilSurfaceDesc.MipLevel = 0;
		depthStencilSurfaceDesc.FaceCount = 1;

		if(!depthStencilSurface.IsLoaded())
			B3D_LOG(Error, LogRenderBackend, "Render texture must be created using a fully loaded texture.");
		else
			depthStencilSurfaceDesc.Texture = depthStencilSurface;
	}

	u32 numSurfaces = std::min((u32)colorSurfaces.size(), (u32)B3D_MAXIMUM_RENDER_TARGET_COUNT);

	RenderTextureCreateInformation desc;
	for(u32 i = 0; i < numSurfaces; i++)
	{
		RenderSurfaceInformation surfaceDesc;
		surfaceDesc.Face = 0;
		surfaceDesc.MipLevel = 0;
		surfaceDesc.FaceCount = 1;

		if(!colorSurfaces[i].IsLoaded())
			B3D_LOG(Error, LogRenderBackend, "Render texture must be created using a fully loaded texture.");
		else
			surfaceDesc.Texture = colorSurfaces[i];

		desc.ColorSurfaces[i] = surfaceDesc;
	}

	desc.DepthStencilSurface = depthStencilSurfaceDesc;

	return RenderTexture::Create(desc);
}

Vector<HTexture> RenderTextureEx::GetColorSurfaces(const TShared<RenderTexture>& thisPtr)
{
	u32 numColorSurfaces = B3D_MAXIMUM_RENDER_TARGET_COUNT;

	Vector<HTexture> output;
	output.reserve(numColorSurfaces);

	for(u32 i = 0; i < numColorSurfaces; i++)
	{
		HTexture colorTex = thisPtr->GetColorTexture(i);

		if(colorTex != nullptr)
			output.push_back(colorTex);
	}

	return output;
}

HTexture RenderTextureEx::GetColorSurface(const TShared<RenderTexture>& thisPtr)
{
	return thisPtr->GetColorTexture(0);
}

HTexture RenderTextureEx::GetDepthStencilSurface(const TShared<RenderTexture>& thisPtr)
{
	return thisPtr->GetDepthStencilTexture();
}
