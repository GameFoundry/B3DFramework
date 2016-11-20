//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptRenderTexture2D.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsRenderTexture.h"
#include "BsScriptTexture2D.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"

namespace bs
{
	ScriptRenderTexture2D::ScriptRenderTexture2D(const SPtr<RenderTarget>& target, MonoObject* instance)
		:ScriptObject(instance), mRenderTarget(target)
	{

	}

	void ScriptRenderTexture2D::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateDetailed", &ScriptRenderTexture2D::internal_createDetailed);
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptRenderTexture2D::internal_create);
		metaData.scriptClass->addInternalCall("Internal_GetColorSurfaces", &ScriptRenderTexture2D::internal_getColorSurfaces);
		metaData.scriptClass->addInternalCall("Internal_GetDepthStencilSurface", &ScriptRenderTexture2D::internal_getDepthStencilSurface);
	}

	SPtr<RenderTexture> ScriptRenderTexture2D::getRenderTexture() const
	{
		return std::static_pointer_cast<RenderTexture>(mRenderTarget);
	}

	void ScriptRenderTexture2D::internal_createDetailed(MonoObject* instance, PixelFormat format, UINT32 width, UINT32 height,
		UINT32 numSamples, bool gammaCorrection, bool createDepth, PixelFormat depthStencilFormat)
	{
		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = width;
		texDesc.height = height;
		texDesc.format = format;
		texDesc.hwGamma = gammaCorrection;
		texDesc.numSamples = numSamples;

		SPtr<RenderTexture> tex = RenderTexture::create(texDesc, createDepth, depthStencilFormat);
		new (bs_alloc<ScriptRenderTexture2D>()) ScriptRenderTexture2D(tex, instance);
	}

	void ScriptRenderTexture2D::internal_create(MonoObject* instance, MonoArray* colorSurfaces, ScriptTexture2D* depthStencilSurface)
	{
		ScriptArray colorSurfacesList(colorSurfaces);

		RENDER_SURFACE_DESC depthStencilSurfaceDesc;
		if (depthStencilSurface != nullptr)
		{
			depthStencilSurfaceDesc.face = 0;
			depthStencilSurfaceDesc.mipLevel = 0;
			depthStencilSurfaceDesc.numFaces = 1;

			HTexture textureHandle = depthStencilSurface->getHandle();
			if (!textureHandle.isLoaded())
			{
				LOGERR("Render texture must be created using a fully loaded texture.");
			}
			else
				depthStencilSurfaceDesc.texture = textureHandle;
		}

		UINT32 numSurfaces = std::min(colorSurfacesList.size(), (UINT32)BS_MAX_MULTIPLE_RENDER_TARGETS);

		RENDER_TEXTURE_DESC desc;
		for (UINT32 i = 0; i < numSurfaces; i++)
		{
			RENDER_SURFACE_DESC surfaceDesc;
			surfaceDesc.face = 0;
			surfaceDesc.mipLevel = 0;
			surfaceDesc.numFaces = 1;

			ScriptTexture2D* scriptSurface = colorSurfacesList.get<ScriptTexture2D*>(i);
			if (scriptSurface != nullptr)
			{
				HTexture textureHandle = scriptSurface->getHandle();
				if (!textureHandle.isLoaded())
				{
					LOGERR("Render texture must be created using a fully loaded texture.");
				}
				else
					surfaceDesc.texture = textureHandle;
			}

			desc.colorSurfaces[i] = surfaceDesc;
		}

		desc.depthStencilSurface = depthStencilSurfaceDesc;

		SPtr<RenderTarget> tex = RenderTexture::create(desc);
		new (bs_alloc<ScriptRenderTexture2D>()) ScriptRenderTexture2D(tex, instance);
	}

	void ScriptRenderTexture2D::internal_getColorSurfaces(ScriptRenderTexture2D* thisPtr, MonoArray** value)
	{
		SPtr<RenderTexture> tex = thisPtr->getRenderTexture();

		UINT32 numColorSurfaces = BS_MAX_MULTIPLE_RENDER_TARGETS;
		ScriptArray outArray = ScriptArray::create<ScriptTexture2D>(numColorSurfaces);

		for (UINT32 i = 0; i < numColorSurfaces; i++)
		{
			HTexture colorTex = tex->getColorTexture(i);

			if (colorTex != nullptr)
			{
				ScriptTexture2D* scriptSurface;
				ScriptResourceManager::instance().getScriptResource(colorTex, &scriptSurface, true);

				outArray.set<MonoObject*>(i, scriptSurface->getManagedInstance());
			}
			else
				outArray.set<MonoObject*>(i, nullptr);
		}

		*value = outArray.getInternal();
	}

	void ScriptRenderTexture2D::internal_getDepthStencilSurface(ScriptRenderTexture2D* thisPtr, MonoObject** value)
	{
		SPtr<RenderTexture> tex = thisPtr->getRenderTexture();
		HTexture depthTex = tex->getDepthStencilTexture();

		ScriptTexture2D* scriptSurface;
		ScriptResourceManager::instance().getScriptResource(depthTex, &scriptSurface, true);

		*value = scriptSurface->getManagedInstance();
	}
}