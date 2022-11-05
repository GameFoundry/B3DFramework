//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderTargetEx.h"
#include "Generated/BsScriptShaderParameter.generated.h"

using namespace bs;
u32 RenderTargetEx::GetWidth(const SPtr<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().Width;
}

u32 RenderTargetEx::GetHeight(const SPtr<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().Height;
}

bool RenderTargetEx::GetGammaCorrection(const SPtr<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().HwGamma;
}

i32 RenderTargetEx::GetPriority(const SPtr<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().Priority;
}

void RenderTargetEx::SetPriority(const SPtr<RenderTarget>& thisPtr, i32 priority)
{
	thisPtr->SetPriority(priority);
}

u32 RenderTargetEx::GetSampleCount(const SPtr<RenderTarget>& thisPtr)
{
	return thisPtr->GetProperties().MultisampleCount;
}

SPtr<RenderTexture> RenderTextureEx::Create(PixelFormat format, int width, int height, int numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
{
	TEXTURE_DESC texDesc;
	texDesc.Type = TEX_TYPE_2D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.Format = format;
	texDesc.HwGamma = gammaCorrection;
	texDesc.NumSamples = numSamples;

	return RenderTexture::Create(texDesc, createDepth, depthStencilFormat);
}

SPtr<RenderTexture> RenderTextureEx::Create(const HTexture& colorSurface)
{
	return Create(Vector<HTexture>{ colorSurface }, HTexture());
}

SPtr<RenderTexture> RenderTextureEx::Create(const HTexture& colorSurface, const HTexture& depthStencilSurface)
{
	return Create(Vector<HTexture>{ colorSurface }, depthStencilSurface);
}

SPtr<RenderTexture> RenderTextureEx::Create(const Vector<HTexture>& colorSurface)
{
	return Create(Vector<HTexture>{ colorSurface }, HTexture());
}

SPtr<RenderTexture> RenderTextureEx::Create(const Vector<HTexture>& colorSurfaces, const HTexture& depthStencilSurface)
{
	RENDER_SURFACE_DESC depthStencilSurfaceDesc;
	if(depthStencilSurface != nullptr)
	{
		depthStencilSurfaceDesc.Face = 0;
		depthStencilSurfaceDesc.MipLevel = 0;
		depthStencilSurfaceDesc.NumFaces = 1;

		if(!depthStencilSurface.IsLoaded())
			B3D_LOG(Error, RenderBackend, "Render texture must be created using a fully loaded texture.");
		else
			depthStencilSurfaceDesc.Texture = depthStencilSurface;
	}

	u32 numSurfaces = std::min((u32)colorSurfaces.size(), (u32)B3D_MAXIMUM_RENDER_TARGET_COUNT);

	RENDER_TEXTURE_DESC desc;
	for(u32 i = 0; i < numSurfaces; i++)
	{
		RENDER_SURFACE_DESC surfaceDesc;
		surfaceDesc.Face = 0;
		surfaceDesc.MipLevel = 0;
		surfaceDesc.NumFaces = 1;

		if(!colorSurfaces[i].IsLoaded())
			B3D_LOG(Error, RenderBackend, "Render texture must be created using a fully loaded texture.");
		else
			surfaceDesc.Texture = colorSurfaces[i];

		desc.ColorSurfaces[i] = surfaceDesc;
	}

	desc.DepthStencilSurface = depthStencilSurfaceDesc;

	return RenderTexture::Create(desc);
}

Vector<HTexture> RenderTextureEx::GetColorSurfaces(const SPtr<RenderTexture>& thisPtr)
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

HTexture RenderTextureEx::GetColorSurface(const SPtr<RenderTexture>& thisPtr)
{
	return thisPtr->GetColorTexture(0);
}

HTexture RenderTextureEx::GetDepthStencilSurface(const SPtr<RenderTexture>& thisPtr)
{
	return thisPtr->GetDepthStencilTexture();
}
