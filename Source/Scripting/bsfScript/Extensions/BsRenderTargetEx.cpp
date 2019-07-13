//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRenderTargetEx.h"
#include "Generated/BsScriptShaderParameter.generated.h"

namespace bs
{
	UINT32 RenderTargetEx::getWidth(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->getProperties().width;
	}

	UINT32 RenderTargetEx::getHeight(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->getProperties().height;
	}

	bool RenderTargetEx::getGammaCorrection(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->getProperties().hwGamma;
	}

	INT32 RenderTargetEx::getPriority(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->getProperties().priority;
	}

	void RenderTargetEx::setPriority(const SPtr<RenderTarget>& thisPtr, INT32 priority)
	{
		thisPtr->setPriority(priority);
	}

	UINT32 RenderTargetEx::getSampleCount(const SPtr<RenderTarget>& thisPtr)
	{
		return thisPtr->getProperties().multisampleCount;
	}

	SPtr<RenderTexture> RenderTextureEx::create(PixelFormat format, int width, int height, int numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
	{
		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = width;
		texDesc.height = height;
		texDesc.format = format;
		texDesc.hwGamma = gammaCorrection;
		texDesc.numSamples = numSamples;

		return RenderTexture::create(texDesc, createDepth, depthStencilFormat);
	}

	SPtr<RenderTexture> RenderTextureEx::create(const HTexture& colorSurface)
	{
		return create(Vector<HTexture>{ colorSurface }, HTexture());
	}

	SPtr<RenderTexture> RenderTextureEx::create(const HTexture& colorSurface, const HTexture& depthStencilSurface)
	{
		return create(Vector<HTexture>{ colorSurface }, depthStencilSurface);
	}

	SPtr<RenderTexture> RenderTextureEx::create(const Vector<HTexture>& colorSurface)
	{
		return create(Vector<HTexture>{ colorSurface }, HTexture());
	}
		
	SPtr<RenderTexture> RenderTextureEx::create(const Vector<HTexture>& colorSurfaces, const HTexture& depthStencilSurface)
	{
		RENDER_SURFACE_DESC depthStencilSurfaceDesc;
		if (depthStencilSurface != nullptr)
		{
			depthStencilSurfaceDesc.face = 0;
			depthStencilSurfaceDesc.mipLevel = 0;
			depthStencilSurfaceDesc.numFaces = 1;

			if (!depthStencilSurface.isLoaded())
				BS_LOG(Error, RenderBackend, "Render texture must be created using a fully loaded texture.");
			else
				depthStencilSurfaceDesc.texture = depthStencilSurface;
		}

		UINT32 numSurfaces = std::min((UINT32)colorSurfaces.size(), (UINT32)BS_MAX_MULTIPLE_RENDER_TARGETS);

		RENDER_TEXTURE_DESC desc;
		for (UINT32 i = 0; i < numSurfaces; i++)
		{
			RENDER_SURFACE_DESC surfaceDesc;
			surfaceDesc.face = 0;
			surfaceDesc.mipLevel = 0;
			surfaceDesc.numFaces = 1;

			if (!colorSurfaces[i].isLoaded())
				BS_LOG(Error, RenderBackend, "Render texture must be created using a fully loaded texture.");
			else
				surfaceDesc.texture = colorSurfaces[i];

			desc.colorSurfaces[i] = surfaceDesc;
		}

		desc.depthStencilSurface = depthStencilSurfaceDesc;

		return RenderTexture::create(desc);
	}

	Vector<HTexture> RenderTextureEx::getColorSurfaces(const SPtr<RenderTexture>& thisPtr)
	{
		UINT32 numColorSurfaces = BS_MAX_MULTIPLE_RENDER_TARGETS;

		Vector<HTexture> output;
		output.reserve(numColorSurfaces);

		for (UINT32 i = 0; i < numColorSurfaces; i++)
		{
			HTexture colorTex = thisPtr->getColorTexture(i);

			if (colorTex != nullptr)
				output.push_back(colorTex);
		}

		return output;
	}

	HTexture RenderTextureEx::getColorSurface(const SPtr<RenderTexture>& thisPtr)
	{
		return thisPtr->getColorTexture(0);
	}

	HTexture RenderTextureEx::getDepthStencilSurface(const SPtr<RenderTexture>& thisPtr)
	{
		return thisPtr->getDepthStencilTexture();
	}
}
