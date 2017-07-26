//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsModule.h"
#include "BsRendererView.h"

namespace bs { namespace ct
{
	/** Contains common materials used throughout the renderer. */
	class GlobalMaterials : public Module<GlobalMaterials>
	{
	public:
		GlobalMaterials();
		~GlobalMaterials();

		/** Returns a material that can be used for rendering a skybox using a specific texture. */
		SkyboxMat<false>* getSkyboxTexture() const { return mSkyboxTextureMat; }

		/** Returns a material that can be used for rendering a skybox using a solid color. */
		SkyboxMat<true>* getSkyboxColor() const { return mSkyboxColorMat; }

		/** Returns a material that can be used for converting a texture encoded in a buffer, back into texture form. */
		FlatFramebufferToTextureMat* getUnflattenBuffer() const { return mFlatFramebufferToTextureMat; }

		/**
		 * Returns a version of the tile-deferred image based lighting material that matches the parameters.
		 *
		 * @param[in]   msaa	Number of samples per pixel.
		 */
		ITiledDeferredImageBasedLightingMat* getTileDeferredIBL(UINT32 msaa);

		/**
		 * Returns a version of the tile-deferred lighting material that matches the parameters.
		 *
		 * @param[in]   msaa	Number of samples per pixel.
		 */
		ITiledDeferredLightingMat* getTileDeferredLighting(UINT32 msaa);
	private:
		SkyboxMat<false>* mSkyboxTextureMat = nullptr;
		SkyboxMat<true>* mSkyboxColorMat = nullptr;

		FlatFramebufferToTextureMat* mFlatFramebufferToTextureMat = nullptr;

		ITiledDeferredImageBasedLightingMat* mTiledDeferredIBLMats[4];
		ITiledDeferredLightingMat* mTiledDeferredLightingMats[4];
	};
}}
