//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderTargetEx.h"
#include "Generated/BsScriptShaderParameter.generated.h"

namespace bs
{
	UINT32 RenderTargetEx::GetWidth(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->GetProperties().width;
	}

	UINT32 RenderTargetEx::GetHeight(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->GetProperties().height;
	}

	bool RenderTargetEx::GetGammaCorrection(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->GetProperties().hwGamma;
	}

	INT32 RenderTargetEx::GetPriority(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->GetProperties().priority;
	}

	void RenderTargetEx::SetPriority(const SPtr<RenderTarget>& thisPtr, INT32 priority)
	{
		thisPtr->SetPriority(priority);
	}

	UINT32 RenderTargetEx::GetSampleCount(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->GetProperties().multisampleCount;
	}

	SPtr<RenderTexture> RenderTextureEx::Create(PixelFormat format, int width, int height, int numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
	{
		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = width;
		texDesc.height = height;
		texDesc.format = format;
		texDesc.hwGamma = gammaCorrection;
		texDesc.numSamples = numSamples;

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
		if (depthStencilSurface != nullptr)
		{
			depthStencilSurfaceDesc.face = 0;
			depthStencilSurfaceDesc.mipLevel = 0;
			depthStencilSurfaceDesc.numFaces = 1;

			if (!depthStencilSurface.IsLoaded())
				BS_LOG(Error, RenderBackend, "Render texture must be created using a fully loaded texture.");
			else
				depthStencilSurfaceDesc.texture = depthStencilSurface;
		}

		UINT32 numSurfaces = std::min((UINT32)colorSurfaces.Size(), (UINT32)BS_MAX_MULTIPLE_RENDER_TARGETS);

		RENDER_TEXTURE_DESC desc;
		for (UINT32 i = 0; i < numSurfaces; i++)
		{
			RENDER_SURFACE_DESC surfaceDesc;
			surfaceDesc.face = 0;
			surfaceDesc.mipLevel = 0;
			surfaceDesc.numFaces = 1;

			if (!colorSurfaces[i].IsLoaded())
				BS_LOG(Error, RenderBackend, "Render texture must be created using a fully loaded texture.");
			else
				surfaceDesc.texture = colorSurfaces[i];

			desc.colorSurfaces[i] = surfaceDesc;
		}

		desc.depthStencilSurface = depthStencilSurfaceDesc;

		return RenderTexture::Create(desc);
	}

	Vector<HTexture> RenderTextureEx::GetColorSurfaces(const SPtr<RenderTexture>& thisPtr)
	{
		UINT32 numColorSurfaces = BS_MAX_MULTIPLE_RENDER_TARGETS;

		Vector<HTexture> output;
		output.Reserve(numColorSurfaces);

		for (UINT32 i = 0; i < numColorSurfaces; i++)
		{
			HTexture colorTex = thisPtr->GetColorTexture(i);

			if (colorTex != nullptr)
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
}
