//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsReflectionCubemap.h"
#include "BsTexture.h"
#include "BsGpuParamsSet.h"
#include "BsRendererCamera.h"
#include "BsRendererUtility.h"

namespace bs { namespace ct
{
	ReflectionCubemapFilterParamDef gReflectionCubemapFilterDef;

	ReflectionCubemapFilterMat::ReflectionCubemapFilterMat()
	{
		mParamBuffer = gReflectionCubemapFilterDef.createBuffer();

		mParamsSet->setParamBlockBuffer("Input", mParamBuffer);
		mParamsSet->getGpuParams()->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mInputTexture);
	}

	void ReflectionCubemapFilterMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void ReflectionCubemapFilterMat::execute(const SPtr<Texture>& source, UINT32 face, const TextureSurface& surface, 
											 const SPtr<RenderTarget>& target)
	{
		mInputTexture.set(source, surface);
		gReflectionCubemapFilterDef.gCubeFace.set(mParamBuffer, face);

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(target);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().drawScreenQuad();
	}

	void ReflectionCubemap::filterCubemapForSpecular(const SPtr<Texture>& cubemap)
	{
		static ReflectionCubemapFilterMat filterMaterial;

		UINT32 numMips = cubemap->getProperties().getNumMipmaps();

		for (UINT32 mip = 1; mip < numMips; mip++)
		{
			for (UINT32 face = 0; face < 6; face++)
			{
				RENDER_TEXTURE_DESC cubeFaceRTDesc;
				cubeFaceRTDesc.colorSurfaces[0].texture = cubemap;
				cubeFaceRTDesc.colorSurfaces[0].face = face;
				cubeFaceRTDesc.colorSurfaces[0].numFaces = 1;
				cubeFaceRTDesc.colorSurfaces[0].mipLevel = mip;

				SPtr<RenderTarget> target = RenderTexture::create(cubeFaceRTDesc);

				UINT32 sourceMip = mip - 1;
				TextureSurface sourceSurface(sourceMip, 1, 0, 6);
				filterMaterial.execute(cubemap, face, sourceSurface, target);
			}
		}
	}
}}
